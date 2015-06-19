///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2012 3Dim Laboratory s.r.o.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "AppConfigure.h"

#include <QtGui>
#include <QDockWidget>
#include <QLayout>
#include <QPrintDialog>
#include <QStylePainter>
#include <QPrinter>
#include <QFileDialog>
#include <QDesktopServices>
#if QT_VERSION >= 0x050000
    #include <QStandardPaths>
#endif
#include <QMap>

#include <data/CDicomLoader.h>
#include <data/CAppSettings.h>
#include <data/CUndoManager.h>
#include <data/CRegionData.h>
#include <data/CImageLoaderInfo.h>
#include <data/CVolumeTransformation.h>

#include <VPL/Module/Progress.h>
#include <VPL/Module/Serialization.h>
#include <VPL/Image/VolumeFiltering.h>
#include <VPL/Image/VolumeFunctions.h>
#include <VPL/Image/Filters/Anisotropic.h>

#include <render/PSVRosg.h>

#include <osgQt/GraphicsWindowQt>
#include <osgGA/TrackballManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/ViewerBase>
#include <osg/osgcompat.h>

#include <alg/CMarchingCubes.h>
#include <alg/CDecimator.h>
#include <alg/CSmoothing.h>
#include <alg/CReduceSmallSubmeshes.h>

#include <cpreferencesdialog.h>
#include <cseriesselectiondialog.h>

#include <CPluginInfoDialog.h>
#include <qtplugin/PluginInterface.h>

#include <cvolumelimiterdialog.h>
#include <CDataInfoDialog.h>
#include <dialogs/cprogress.h>
#include <CCustomUI.h>
#include <C3DimApplication.h>
#include <CFilterDialog.h>

#include <zlib.h>
#include <zip/unzip.h>

#ifdef _WIN32 // Windows specific
    #include <Windows.h>
    #include <Wbemcli.h>
    #include <comdef.h>
    #pragma comment(lib, "wbemuuid.lib")
#endif

#ifdef __APPLE__
#include <unistd.h>
#include <sys/sysctl.h>
//#include <OpenGL/OpenGL.h>
#include <sys/param.h>
#include <sys/mount.h>
#endif

// for dicom save
#include "dcmtk/config/osconfig.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdict.h"
#include "dcmtk/dcmdata/dcdeftag.h"
#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmjpeg/djrplol.h"
#include "dcmtk/dcmdata/dcmetinf.h"

//#undef USE_PSVR

bool dockWidgetNameCompare(QDockWidget* left, QDockWidget* right)
{
    return left->windowTitle().compare(right->windowTitle())<0;
}

///////////////////////////////////////////////////////////////////////////////
// simple helper class for dicom data loading from zip archive

class CZipLoader
{
protected:
    QString m_archive;
    QString m_tempDir;
public:
    //! Constructor
    CZipLoader() {}
    //! Destructor removes temporary directory with extracted zip contents
    ~CZipLoader() 
    {
        if (!m_tempDir.isEmpty())
            removeDir(m_tempDir); 
    }
    // checks filename for zip extension
    bool setZipArchive(const QString& fileName)
    {
        QFileInfo pathInfo( fileName );
        QString ext = pathInfo.suffix();
        if (0==ext.compare("zip",Qt::CaseInsensitive)) // return zip files
        {
            m_archive = fileName;
            return true;    
        }
        return false;
    }
    //! decompress zip archive to a temporary directory which will be deleted with this object
    QString decompress()
    {
        // get temp dir name
        {
            QTemporaryFile file;
            file.open();
            m_tempDir = file.fileName();
            if (m_tempDir.isEmpty())
            {
                m_tempDir = QDir::tempPath();
                m_tempDir+="/bspziptmp";
            }
            m_tempDir+="/";
        }
       
        // make sure that the directory exists
        {
            QDir dir;
            bool ok = dir.mkpath(m_tempDir);
            Q_ASSERT(ok);        
        }

        bool bDecompressedSomething = false;

        std::string str = m_archive.toStdString();
        unzFile hZIP = unzOpen(str.c_str());
        if (hZIP)
        {
#define CUSTOM_MAX_PATH 4096
            char pFileName[CUSTOM_MAX_PATH]={};
            bool bContinue = (UNZ_OK == unzGoToFirstFile(hZIP));
            while (bContinue) 
            {
                unz_file_info fi = {};
                memset(pFileName,0,CUSTOM_MAX_PATH*sizeof(char));
                if (unzGetCurrentFileInfo(hZIP, &fi, pFileName, CUSTOM_MAX_PATH, NULL, 0, NULL, 0) == UNZ_OK) 
                {
#undef CUSTOM_MAX_PATH
                    std::string s = pFileName;
                    QString file = m_tempDir;
                    file += QString::fromStdString(s);
                    if (file.endsWith('/') || file.endsWith('\\'))
                    {
                        QDir dir;
                        bool ok = dir.mkpath(file);
                        Q_ASSERT(ok);
                    }
                    else
                    {
                        if (fi.uncompressed_size>0)
                        {
                            char* pDecompressed=new char[fi.uncompressed_size];
                            if (NULL!=pDecompressed)
                            {
                                int nRead = 0;
                                if (unzOpenCurrentFile(hZIP) == UNZ_OK) 
                                {
                                    nRead = unzReadCurrentFile(hZIP, pDecompressed, fi.uncompressed_size);
                                    if (UNZ_CRCERROR == unzCloseCurrentFile(hZIP))
                                        nRead = 0;
                                }
                                if (nRead>0)
                                {
                                    QFile f(file);
                                    if( f.open(QIODevice::WriteOnly) )
                                    {
                                        f.write(pDecompressed,fi.uncompressed_size);
                                        f.close();
                                        bDecompressedSomething = true;
                                    }
                                }
                                delete pDecompressed;
                            }
                        }
                    }                    
                }
                bContinue = (UNZ_OK == unzGoToNextFile(hZIP));
            }
            unzClose(hZIP);
        }
        if (bDecompressedSomething)
            return m_tempDir;
        else
        {
            removeDir(m_tempDir);
            m_tempDir.clear();
            return m_archive; // return original zip name
        }
    }
    //! removeDir erases specified directory and all subdirectories
    bool removeDir(const QString& dirName) const
    {
        bool result = true;
        QDir dir(dirName); 
        if (dir.exists(dirName)) 
        {
            Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) 
            {
                if (info.isDir()) 
                    result = removeDir(info.absoluteFilePath());
                else 
                    result = QFile::remove(info.absoluteFilePath());
                if (!result)
                    return result;
            }
            result = dir.rmdir(dirName);
        }
        return result;
    }
};


///////////////////////////////////////////////////////////////////////////////
// MainWindow

MainWindow* MainWindow::m_pMainWindow = NULL;

MainWindow::MainWindow(QWidget *parent, CPluginManager* pPlugins) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
	m_pPlugins(pPlugins)
{    
	Q_ASSERT(NULL!=m_pPlugins);
    m_pMainWindow = this;
    m_nLayoutType = 0;
    setProperty("SegmentationChanged",false);

    m_3DView = NULL;
    m_OrthoXYSlice = NULL;
    m_OrthoXZSlice = NULL;
    m_OrthoYZSlice = NULL;

    for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        m_modelVisualizers[i] = NULL;

    m_densityWindowPanel=NULL;
    m_orthoSlicesPanel=NULL;
    m_segmentationPanel=NULL;
    m_volumeRenderingPanel=NULL;
    m_realCentralWidget = NULL;
	m_modelsPanel = NULL;
    m_helpView=NULL;
    // we have to create a permanent central widget because of some sizing issues
    // when switching workspaces without central widget
    m_centralWidget = new QWidget();
    QLayout* pCentralLayout = new QHBoxLayout(); // just some layout
    pCentralLayout->setContentsMargins(0,0,0,0);
    pCentralLayout->setSpacing(0);
    m_centralWidget->setLayout(pCentralLayout);

    // setup UI
    ui->setupUi(this);

    setDockOptions(QMainWindow::VerticalTabs);
    setAcceptDrops(true);
    
    // set up status bar
    m_grayLevelLabel = new QLabel();
    m_grayLevelLabel->setMinimumWidth(200);
    ui->statusBar->addPermanentWidget(m_grayLevelLabel);

    // set blue background color
    data::CObjectPtr<data::CAppSettings> settings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
    settings->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));

//    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setCentralWidget(m_centralWidget);

    // connect actions to slots
    connectActions();

    // add actions to toolbars
    createToolBars();

    // create OSG windows
    createOSGStuff();

    setupDockWindows();

    // load application settings
    int nThreadingMode=0;
    {   // read info on basic layout
        QSettings settings;
        nThreadingMode=settings.value("Threading").toInt();
        m_localeDir=settings.value("LocaleDir").toString();
        settings.beginGroup("MainWindow");
        m_nLayoutType=settings.value("layoutType").toInt();
        settings.endGroup();
        {   // load background color
            QColor color=settings.value("BGColor",DEFAULT_BACKGROUND_COLOR).toUInt();
            osg::Vec4 osgColor(color.redF(), color.greenF(), color.blueF(), 1.0f);

            data::CObjectPtr<data::CAppSettings> appSettings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
            appSettings->setClearColor(osgColor);
            APP_STORAGE.invalidate(appSettings.getEntryPtr());

            m_3DView->setBackgroundColor(osgColor);
            m_OrthoXYSlice->setBackgroundColor(osgColor);
            m_OrthoXZSlice->setBackgroundColor(osgColor);
            m_OrthoYZSlice->setBackgroundColor(osgColor);
        }

        // load recent projects
        settings.beginGroup("Recent");
        {
            for(int i=9;i>=0;i--) // go backwards because addToRecentProjects adds to start
            {
                QString path = settings.value(QString("%1").arg(i)).toString();
                if (!path.isEmpty())
                {
                    QFileInfo info(path);
                    if (info.exists())
                        addToRecentFiles(path);
                }
            }
        }
        settings.endGroup();

        // create menu for recent projects
        QWidget* pOpenWidget = ui->mainToolBar->widgetForAction(ui->actionLoad_Patient_Dicom_data);        
        if (NULL!=pOpenWidget)
        {
            QToolButton* pButton = qobject_cast<QToolButton*>(pOpenWidget);
            if (NULL!=pButton)
            {
                QMenu* pMenu = new QMenu();
                connect(pMenu,SIGNAL(aboutToShow()),this,SLOT(aboutToShowRecentFiles()));
                pButton->setMenu(pMenu);
                pButton->setPopupMode(QToolButton::MenuButtonPopup);
            }
        }
    }
    // load all other app settings
    loadAppSettings();

    // setup working space
    setUpWorkspace();

    // create panels
    createPanels();

    // load plugins
    Q_ASSERT(m_densityWindowPanel && m_densityWindowPanel->parentWidget());
    QDockWidget* dockPluginsTo=qobject_cast<QDockWidget*>(m_densityWindowPanel->parentWidget());
	if (NULL!=m_pPlugins)
		m_pPlugins->loadPlugins(this,&m_localeDir,dockPluginsTo);

    // load last known layout
    readLayoutSettings(false);

    // set up mouse mode change monitoring
    m_ConnectionModeChanged = APP_MODE.getModeChangedSignal().connect( this, &MainWindow::sigModeChanged );
    sigModeChanged(APP_MODE.get()); // first call has to be explicit

    APP_MODE.getSceneHitSignal().connect(this, &MainWindow::sigSceneHit);

    APP_MODE.getContinuousDensityMeasureSignal().connect(this, &MainWindow::densityMeasureHandler);

	VPL_SIGNAL(SigSetModelCutVisibility).connect(this, &MainWindow::setModelCutVisibilitySignal);
	VPL_SIGNAL(SigGetModelCutVisibility).connect(this, &MainWindow::getModelCutVisibilitySignal);

    // set threading mode
    if (1==nThreadingMode)
    {
        m_3DView->enableMultiThreaded();
        m_OrthoXYSlice->enableMultiThreaded();
        m_OrthoXZSlice->enableMultiThreaded();
        m_OrthoYZSlice->enableMultiThreaded();
    }

    {
#ifdef _WIN32
        QDir myDir = QCoreApplication::applicationDirPath();
        QStringList list = myDir.entryList(QStringList("*.exe"));
        int cntOther = 0;
        foreach(QString file,list)
        {
            if (0!=file.compare("3DimViewer.exe",Qt::CaseInsensitive) && 0!=file.compare("3DimViewerd.exe",Qt::CaseInsensitive) && !file.contains("unins",Qt::CaseInsensitive))
                cntOther++;
        }
        if (cntOther>0)
        {
            ui->menuModel->addAction(tr("Process model"),this,SLOT(processSurfaceModelExtern()));
        }
#endif
        QMenu* pMenu = ui->menuModel->addMenu(tr("Visualization"));
        QAction* pSmooth = pMenu->addAction(tr("Smooth"),this,SLOT(modelVisualizationSmooth()));
		pSmooth->setObjectName("smoothShading");
        QAction* pFlat = pMenu->addAction(tr("Flat"),this,SLOT(modelVisualizationFlat()));
		pFlat->setObjectName("flatShading");
        QAction* pWire = pMenu->addAction(tr("Wire"),this,SLOT(modelVisualizationWire()));
		pWire->setObjectName("noShading");
    }
    // disable actions which are meant to be disabled :)
    ui->action3D_View->setEnabled(false); // always visible
    ui->actionUndo->setEnabled(false); // viewer doesn't have actions that could be undone/redone
    ui->actionRedo->setEnabled(false);
    ui->actionSave_DICOM_Series->setEnabled(false);
	ui->actionSave_Original_DICOM_Series->setEnabled(false);
    ui->actionSave_Volumetric_Data->setEnabled(false);
    if (!findPluginByID("Gauge"))
    {
        ui->actionMeasure_Density_Value->setEnabled(false);
        ui->actionMeasure_Distance->setEnabled(false);
        ui->actionClear_Measurements->setEnabled(false);
    }
    if (!findPluginByID("DataExpress"))
    {
        ui->actionSend_Data->setEnabled(false);
    }
    //ui->actionVolume_Rendering->setVisible(false);

    data::CObjectPtr< data::CUndoManager > ptrManager( APP_STORAGE.getEntry(data::Storage::UndoManager::Id) );
    ptrManager->getUndoChangedSignal().connect(this, &MainWindow::undoRedoEnabler);

    m_conRegionData = APP_STORAGE.getEntrySignal(data::Storage::RegionData::Id).connect(this, &MainWindow::sigRegionDataChanged);

    for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        APP_STORAGE.getEntrySignal(data::Storage::ImportedModel::Id + i).connect(this, &MainWindow::sigBonesModelChanged);

    {   // connect our custom action to toggleViewAction of every dock widget so it is raised after it is shown
        QList<QDockWidget*> dws = findChildren<QDockWidget*>();
        qSort(dws.begin(),dws.end(),dockWidgetNameCompare);
        foreach (QDockWidget * dw, dws)
        {
            connect(dw->toggleViewAction(),SIGNAL(triggered(bool)),this,SLOT(onDockWidgetToggleView(bool)));
        }
    }

	connect(ui->menuViewFilter,SIGNAL(aboutToShow()),this,SLOT(aboutToShowViewFilterMenu()));
	connect(ui->actionViewEqualize, SIGNAL(toggled(bool)), this, SLOT(setTextureFilterEqualize(bool)));
	connect(ui->actionViewSharpen, SIGNAL(toggled(bool)), this, SLOT(setTextureFilterSharpen(bool)));

	loadShortcuts();

    // timer for OSG widget repaint - needed for eventqueue to work properly
    m_timer.setInterval(150);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(show_frame()));
    m_timer.start();	

	// first event will open files from command line etc
    QTimer::singleShot(0, this, SLOT(firstEvent()));
}

MainWindow::~MainWindow()
{
    {
        QSettings settings;
        // save recent projects
        settings.beginGroup("Recent");
        {
            int i=0;
            QList<QAction *> actions = ui->menuRecent->actions();
            foreach(QAction* pAct, actions)
            {
                if (i==10) break; // save max 10 recent files
                QString path=pAct->property("Path").toString();
                if (!path.isEmpty())
                {
                    settings.setValue(QString("%1").arg(i),path);
                    i++;
                }
            }
        }
        settings.endGroup();
    }
    APP_STORAGE.getEntrySignal(data::Storage::RegionData::Id).disconnect(m_conRegionData);
    APP_MODE.getModeChangedSignal().disconnect( m_ConnectionModeChanged );
	if (NULL!=m_pPlugins)
		m_pPlugins->disconnectPlugins();
    delete ui;
}

void MainWindow::connectActions()
{
    connect(ui->menuToolbars, SIGNAL(aboutToShow()), this, SLOT(toolbarsEnabler()));

    // Loading, Saving etc
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionLoad_Patient_Dicom_data, SIGNAL(triggered()), this, SLOT(openDICOM()));
    connect(ui->actionImport_DICOM_Data_from_ZIP_archive, SIGNAL(triggered()), this, SLOT(openDICOMZIP()));
    connect(ui->actionLoad_Patient_VLM_Data, SIGNAL(triggered()), this, SLOT(openVLM()));
    connect(ui->actionLoad_STL_Model, SIGNAL(triggered()), this, SLOT(openSTL()));
    connect(ui->actionSave_Original_DICOM_Series, SIGNAL(triggered()), this, SLOT(saveOriginalDICOM()));
    connect(ui->actionSave_DICOM_Series, SIGNAL(triggered()), this, SLOT(saveDICOM()));
    connect(ui->actionSave_Volumetric_Data, SIGNAL(triggered()), this, SLOT(saveVLMAs()));
    connect(ui->actionSave_STL_Model, SIGNAL(triggered()), this, SLOT(saveSTL()));
    connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(print()));
    connect(ui->actionShow_Data_Properties, SIGNAL(triggered()), this, SLOT(showDataProperties()));
    connect(ui->actionSend_Data, SIGNAL(triggered()), this, SLOT(sendDataExpressData()));

    // Measurements menu
    connect(ui->actionMeasure_Density_Value, SIGNAL(triggered(bool)), SLOT(measureDensity(bool)));
    connect(ui->actionMeasure_Distance, SIGNAL(triggered(bool)), SLOT(measureDistance(bool)));
    connect(ui->actionClear_Measurements, SIGNAL(triggered()), SLOT(clearMeasurements()));

    // Tools Menu
    connect(ui->actionPreferences, SIGNAL(triggered()), this, SLOT(showPreferencesDialog()));

    // Help Menu
    connect(ui->actionShow_Help, SIGNAL(triggered()), this, SLOT(showHelp()));
    connect(ui->actionAbout_Plugins, SIGNAL(triggered()), this, SLOT(showAboutPlugins()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));

    // Toolbars
    connect(ui->actionMain_Toolbar, SIGNAL(triggered()), this, SLOT(showMainToolBar()));
    connect(ui->actionViews_Toolbar, SIGNAL(triggered()), this, SLOT(showViewsToolBar()));
    connect(ui->actionMouse_Toolbar, SIGNAL(triggered()), this, SLOT(showMouseToolBar()));
    connect(ui->actionVisibility_Toolbar, SIGNAL(triggered()), this, SLOT(showVisibilityToolBar()));
    connect(ui->actionPanels_Toolbar, SIGNAL(triggered()), this, SLOT(showPanelsToolBar()));

    // Views
    connect(ui->action3D_View, SIGNAL(triggered(bool)), this, SLOT(show3DView(bool)));
    connect(ui->actionAxial_View, SIGNAL(triggered(bool)), this, SLOT(showAxialView(bool)));
    connect(ui->actionCoronal_View, SIGNAL(triggered(bool)), this, SLOT(showCoronalView(bool)));
    connect(ui->actionSagittal_View, SIGNAL(triggered(bool)), this, SLOT(showSagittalView(bool)));

    // Panels
    connect(ui->actionDensity_Window, SIGNAL(triggered(bool)), this, SLOT(showDensityWindowPanel(bool)));
    connect(ui->actionOrtho_Slices_Panel, SIGNAL(triggered(bool)), this, SLOT(showOrthoSlicesPanel(bool)));
    connect(ui->actionSegmentation_Panel, SIGNAL(triggered(bool)), this, SLOT(showSegmentationPanel(bool)));
    connect(ui->actionVolume_Rendering_Panel, SIGNAL(triggered(bool)), this, SLOT(showVRPanel(bool)));
	connect(ui->actionModels_List_Panel, SIGNAL(triggered(bool)), this, SLOT(showModelsListPanel(bool)));
	connect(ui->actionClose_Active_Panel, SIGNAL(triggered()), this, SLOT(closeActivePanel()));
	connect(ui->actionPrevious_Panel, SIGNAL(triggered()), this, SLOT(prevPanel()));
	connect(ui->actionNext_Panel, SIGNAL(triggered()), this, SLOT(nextPanel()));

    // Information Widgets
    connect(ui->actionShow_Information_Widgets, SIGNAL(triggered(bool)), this, SLOT(showInformationWidgets(bool)));

    // Mouse modes
    connect(ui->actionDensity_Window_Adjusting, SIGNAL(triggered(bool)), this, SLOT(mouseModeDensityWindow(bool)));
    connect(ui->actionTrackball_Mode, SIGNAL(triggered(bool)), this, SLOT(mouseModeTrackball(bool)));
    connect(ui->actionObject_Manipulation, SIGNAL(triggered(bool)), this, SLOT(mouseModeObjectManipulation(bool)));
    connect(ui->actionScale_Scene, SIGNAL(triggered(bool)), this, SLOT(mouseModeZoom(bool)));

    // Slices in 3D model
    connect(ui->actionAxial_Slice, SIGNAL(triggered(bool)), this, SLOT(showAxialSlice(bool)));
    connect(ui->actionCoronal_Slice, SIGNAL(triggered(bool)), this, SLOT(showCoronalSlice(bool)));
    connect(ui->actionSagittal_Slice, SIGNAL(triggered(bool)), this, SLOT(showSagittalSlice(bool)));
    connect(ui->actionVolume_Rendering, SIGNAL(triggered(bool)), this, SLOT(showMergedVR(bool)));

    // Surface model
    connect(ui->actionShow_Surface_Model, SIGNAL(triggered(bool)), this, SLOT(showSurfaceModel(bool)));
    connect(ui->actionCreate_Surface_Model, SIGNAL(triggered()), this, SLOT(createSurfaceModel()));

    // Workspace switching
    connect(ui->actionWorkspace3DView, SIGNAL(triggered()), this, SLOT(setUpWorkSpace3D()));
    connect(ui->actionWorkspaceTabs, SIGNAL(triggered()), this, SLOT(setUpWorkSpaceTabs()));
    connect(ui->actionWorkspaceGrid, SIGNAL(triggered()), this, SLOT(setUpWorkSpaceGrid()));

    connect(ui->actionLoad_User_Perspective, SIGNAL(triggered()), this, SLOT(loadUserPerspective()));
    connect(ui->actionSave_User_Perspective, SIGNAL(triggered()), this, SLOT(saveUserPerspective()));
    connect(ui->actionLoad_Default_Perspective, SIGNAL(triggered()), this, SLOT(loadDefaultPerspective()));

    // Filters
    connect(ui->actionGaussian_Filter, SIGNAL(triggered()), this, SLOT(filterGaussian()));
    connect(ui->actionMedian_Filter, SIGNAL(triggered()), this, SLOT(filterMedian()));
    connect(ui->action3D_Anisotropic_Filter, SIGNAL(triggered()), this, SLOT(filterAnisotropic()));
	connect(ui->actionSharpening_Filter, SIGNAL(triggered()), this, SLOT(filterSharpen()));

    // Undo and redo
    connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(performUndo()));
    connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(performRedo()));

    // other signals
    VPL_SIGNAL(SigVREnabledChange).connect(this, &MainWindow::onVREnabledChange);
}

void MainWindow::onVREnabledChange(bool value)
{
    ui->actionVolume_Rendering->blockSignals(true);
    ui->actionVolume_Rendering->setChecked(value);
    ui->actionVolume_Rendering->blockSignals(false);
}

void MainWindow::createToolBars()
{
    ui->mainToolBar->addAction(ui->actionLoad_Patient_Dicom_data);
    ui->mainToolBar->addAction(ui->actionLoad_Patient_VLM_Data);
    ui->mainToolBar->addAction(ui->actionSave_Volumetric_Data);
    ui->mainToolBar->addAction(ui->actionPrint);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(ui->actionUndo);
    ui->mainToolBar->addAction(ui->actionRedo);

    ui->viewsToolBar->addAction(ui->action3D_View);
    ui->viewsToolBar->addAction(ui->actionAxial_View);
    ui->viewsToolBar->addAction(ui->actionCoronal_View);
    ui->viewsToolBar->addAction(ui->actionSagittal_View);

    ui->mouseToolBar->addAction(ui->actionDensity_Window_Adjusting);
    ui->mouseToolBar->addAction(ui->actionTrackball_Mode);
    ui->mouseToolBar->addAction(ui->actionObject_Manipulation);
    ui->mouseToolBar->addAction(ui->actionScale_Scene);

    ui->visibilityToolBar->addAction(ui->actionShow_Surface_Model);
    ui->visibilityToolBar->addAction(ui->actionVolume_Rendering);
    ui->visibilityToolBar->addAction(ui->actionAxial_Slice);
    ui->visibilityToolBar->addAction(ui->actionCoronal_Slice);
    ui->visibilityToolBar->addAction(ui->actionSagittal_Slice);

	QAction *pPanels = ui->panelsToolBar->addAction(QIcon(":/icons/page.png"),tr("Panels"),this, SLOT(showPanelsMenu()));
	pPanels->setToolTip(tr("Change panel visibility"));
    ui->panelsToolBar->addAction(ui->actionDensity_Window);
    ui->panelsToolBar->addAction(ui->actionOrtho_Slices_Panel);
    ui->panelsToolBar->addAction(ui->actionVolume_Rendering_Panel);
    ui->panelsToolBar->addAction(ui->actionSegmentation_Panel);
	ui->panelsToolBar->addAction(ui->actionModels_List_Panel);
    
    QAction* pFullScreen = ui->appToolBar->addAction(QIcon(":/icons/resources/fullscreen.png"),tr("Fullscreen"));
    pFullScreen->setCheckable(true);
    connect(pFullScreen,SIGNAL(triggered(bool)),this,SLOT(fullscreen(bool)));
}

void MainWindow::workspacesEnabler()
{
    ui->actionWorkspace3DView->blockSignals(true);
    ui->actionWorkspaceGrid->blockSignals(true);
    ui->actionWorkspaceTabs->blockSignals(true);
    ui->actionWorkspace3DView->setChecked(Workspace3D==m_nLayoutType);
    ui->actionWorkspaceGrid->setChecked(WorkspaceGrid==m_nLayoutType);
    ui->actionWorkspaceTabs->setChecked(WorkspaceTabs==m_nLayoutType);
    ui->actionWorkspace3DView->blockSignals(false);
    ui->actionWorkspaceGrid->blockSignals(false);
    ui->actionWorkspaceTabs->blockSignals(false);
}

// setups windows, saves and loads visibility and positions of dock windows from QSettings
void MainWindow::setUpWorkSpace3D()
{
    if (m_nLayoutType==Workspace3D) return;
    writeLayoutSettings(m_nLayoutType,true);
    m_nLayoutType=Workspace3D;
    setUpdatesEnabled(false); // prevent screen flicker
    setUpWorkspace();
    readLayoutSettings(true);
    setUpdatesEnabled(true);
}

void MainWindow::setUpWorkSpaceTabs()
{
    if (m_nLayoutType==WorkspaceTabs) return;
    writeLayoutSettings(m_nLayoutType,true);
    m_nLayoutType=WorkspaceTabs;
    setUpdatesEnabled(false); // prevent screen flicker
    setUpWorkspace();
    readLayoutSettings(true);
    setUpdatesEnabled(true);
}

void MainWindow::setUpWorkSpaceGrid()
{
    if (m_nLayoutType==WorkspaceGrid) return;
    writeLayoutSettings(m_nLayoutType,true);
    m_nLayoutType=WorkspaceGrid;
    setUpdatesEnabled(false); // prevent screen flicker
    setUpWorkspace();
    readLayoutSettings(true);
    setUpdatesEnabled(true);
}

static void setSliceDockWidgetFeatures(QDockWidget* widget)
{
    Q_ASSERT(NULL!=widget);
    widget->setAllowedAreas(Qt::NoDockWidgetArea);
    widget->setFeatures(0);
    widget->setMinimumSize(200,150);
}

void MainWindow::removeViewsParentWidget(QWidget* view)
{
    if (NULL==view) return;
    QDockWidget *pDockW = NULL;
    QWidget     *pParent = view->parentWidget();
    // find window parent which is a dock window
    while(NULL!=pParent)
    {
        pDockW=qobject_cast<QDockWidget*>(pParent);
        if (pDockW)
            break;
        pParent=pParent->parentWidget();
    }
    if (NULL!=pDockW)
    {
        // undock dock widget and hide it        
        removeDockWidget(pDockW);
        pDockW->hide();
        // remove any custom title bar
        QWidget* pCustomTitle=pDockW->titleBarWidget();
        if (NULL!=pCustomTitle)
        {
            pDockW->setTitleBarWidget(NULL);
            delete pCustomTitle;
        }
        //if (m_realCentralWidget==pDockW)
        //    m_realCentralWidget=NULL;
    }
    Q_ASSERT(pDockW);
}

QDockWidget* MainWindow::getParentDockWidget(QWidget* view)
{
    if (NULL==view) return false;
    QWidget* pParent=view->parentWidget();
    if (NULL==pParent) return false;
    QDockWidget* pDockW=qobject_cast<QDockWidget*>(pParent);
    return pDockW;
}

void MainWindow::setUpWorkspace()
{
    Q_ASSERT(NULL!=m_3DView);
    Q_ASSERT(NULL!=m_OrthoXYSlice);
    Q_ASSERT(NULL!=m_OrthoXZSlice);
    Q_ASSERT(NULL!=m_OrthoYZSlice);

    QWidget* oldCentralWidget = m_realCentralWidget;

    // remove/hide all existing windows
    removeViewsParentWidget(m_3DView);
    removeViewsParentWidget(m_OrthoXYSlice);
    removeViewsParentWidget(m_OrthoXZSlice);
    removeViewsParentWidget(m_OrthoYZSlice);
    // recreate dock widgets for all views
    switch(m_nLayoutType)
    {
    case WorkspaceTabs: {
            QTabWidget* newCenter = new QTabWidget();
            setSliceDockWidgetFeatures(&m_wnd3DView);
            newCenter->setTabPosition(QTabWidget::South);
            newCenter->setIconSize(QSize(16,16));
            newCenter->addTab(&m_wnd3DView,m_wnd3DView.windowTitle());

            setSliceDockWidgetFeatures(&m_wndXYView);
            newCenter->addTab(&m_wndXYView,m_wndXYView.windowTitle());

            setSliceDockWidgetFeatures(&m_wndXZView);
            newCenter->addTab(&m_wndXZView,m_wndXZView.windowTitle());

            setSliceDockWidgetFeatures(&m_wndYZView);
            newCenter->addTab(&m_wndYZView,m_wndYZView.windowTitle());

            m_wnd3DView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_NONE|DWT_SLIDER_VR));
            m_wndXYView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_NONE|DWT_SLIDER_XY));
            m_wndXZView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_NONE|DWT_SLIDER_XZ));
            m_wndYZView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_NONE|DWT_SLIDER_YZ));

            //setCentralWidget(newCenter);
            if (m_realCentralWidget)
                m_centralWidget->layout()->removeWidget(m_realCentralWidget);
            m_centralWidget->layout()->addWidget(newCenter);
            m_realCentralWidget=newCenter;
        }
        break;
    case WorkspaceGrid:
        {
            QSplitter* vertical = new QSplitter(Qt::Vertical);
            CRapedSplitter* horizontalTop = new CRapedSplitter(Qt::Horizontal);
            CRapedSplitter* horizontalBottom = new CRapedSplitter(Qt::Horizontal);
            //horizontalTop->setOpaqueResize(false);

            setSliceDockWidgetFeatures(&m_wndXYView);
            horizontalTop->addWidget(&m_wndXYView);
            m_wndXYView.show();

            setSliceDockWidgetFeatures(&m_wndXZView);
            horizontalBottom->addWidget(&m_wndXZView);
            m_wndXZView.show();

            setSliceDockWidgetFeatures(&m_wndYZView);
            horizontalBottom->addWidget(&m_wndYZView);
            m_wndYZView.show();

            setSliceDockWidgetFeatures(&m_wnd3DView);
            horizontalTop->addWidget(&m_wnd3DView);
            m_wnd3DView.show();

            m_wndXYView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_MAXIMIZE|DWT_SLIDER_XY));
            m_wndXZView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_MAXIMIZE|DWT_SLIDER_XZ));
            m_wndYZView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_MAXIMIZE|DWT_SLIDER_YZ));
            m_wnd3DView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_MAXIMIZE|DWT_SLIDER_VR));

            horizontalTop->setStretchFactor(0,1);
            horizontalTop->setStretchFactor(1,1);
            vertical->addWidget(horizontalTop);
            vertical->addWidget(horizontalBottom);
            QList<int> sizes;
            sizes.append(1);
            sizes.append(1);
            horizontalTop->setSizes(sizes);
            horizontalBottom->setSizes(sizes);
            vertical->setStretchFactor(0,5);
            vertical->setStretchFactor(1,5);

            if (m_realCentralWidget)
                m_centralWidget->layout()->removeWidget(m_realCentralWidget);
            m_centralWidget->layout()->addWidget(vertical);
            m_realCentralWidget=vertical;

            connect(horizontalTop,SIGNAL(splitterMoved(int,int)),this,SLOT(topSplitterMoved(int,int)));
            connect(horizontalBottom,SIGNAL(splitterMoved(int,int)),this,SLOT(bottomSplitterMoved(int,int)));
        }
        break;
    case Workspace3D:
    default:
        {
            setSliceDockWidgetFeatures(&m_wnd3DView);            
            if (m_realCentralWidget)
                m_centralWidget->layout()->removeWidget(m_realCentralWidget);
            m_centralWidget->layout()->addWidget(&m_wnd3DView);
            m_realCentralWidget=&m_wnd3DView;
            m_wnd3DView.show();

            m_wndXYView.setAllowedAreas(Qt::AllDockWidgetAreas);
            m_wndXYView.setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable );
            addDockWidget(Qt::LeftDockWidgetArea, &m_wndXYView);
            m_wndXYView.show();

            m_wndXZView.setAllowedAreas(Qt::AllDockWidgetAreas);
            m_wndXZView.setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable );
            addDockWidget(Qt::LeftDockWidgetArea, &m_wndXZView);
            m_wndXZView.show();

            m_wndYZView.setAllowedAreas(Qt::AllDockWidgetAreas);
            m_wndYZView.setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable );
            addDockWidget(Qt::LeftDockWidgetArea, &m_wndYZView);
            m_wndYZView.show();

            m_wndYZView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_CLOSE|DWT_SLIDER_YZ));
            m_wndXYView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_CLOSE|DWT_SLIDER_XY));
            m_wndXZView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_CLOSE|DWT_SLIDER_XZ));
            m_wnd3DView.setTitleBarWidget(new CCustomDockWidgetTitle(DWT_BUTTONS_NONE|DWT_SLIDER_VR));
        }
        break;
    }

    // delete any previous parent in center area
    if (oldCentralWidget!=&m_wnd3DView)
        delete oldCentralWidget;

    // update UI
    afterWorkspaceChange(); // disables some actions in ui for some configurations
    workspacesEnabler(); // update checked states
}

void MainWindow::createPanels()
{
    m_densityWindowPanel = new CDensityWindowWidget();
    QDockWidget *dockDWP = new QDockWidget(tr("Density Window"), this);
    dockDWP->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockDWP->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable ); // QDockWidget::DockWidgetFloatable
    dockDWP->setObjectName("Density Window Panel");
    dockDWP->setWidget(m_densityWindowPanel);
	dockDWP->setProperty("Icon",":/icons/density_window.png");
    addDockWidget(Qt::RightDockWidgetArea, dockDWP);

    m_orthoSlicesPanel = new COrthoSlicesWidget();
    QDockWidget *dockOrtho = new QDockWidget(tr("Ortho Slices"), this);
    dockOrtho->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockOrtho->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable );
    dockOrtho->setObjectName("Ortho Slices Panel");
    dockOrtho->setWidget(m_orthoSlicesPanel);
	dockOrtho->setProperty("Icon",":/icons/ortho_slices_window.png");
    tabifyDockWidget(dockDWP, dockOrtho);

    m_segmentationPanel = new CSegmentationWidget();
    QDockWidget *dockSeg = new QDockWidget(tr("Tissue Segmentation"), this);
    dockSeg->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockSeg->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
    dockSeg->setObjectName("Tissue Segmentation Panel");
    dockSeg->setWidget(m_segmentationPanel);
	dockSeg->setProperty("Icon",":/icons/segmentation_window.png");
    tabifyDockWidget(dockDWP, dockSeg);

    m_volumeRenderingPanel = new CVolumeRenderingWidget();
    m_volumeRenderingPanel->setRenderer(&m_3DView->getRenderer());
    QDockWidget *dockVR = new QDockWidget(tr("Volume Rendering"), this);
    dockVR->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockVR->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
    dockVR->setObjectName("Volume Rendering Panel");
    dockVR->setWidget(m_volumeRenderingPanel);
	dockVR->setProperty("Icon",":/icons/volume_rendering_window2.png");
    dockVR->hide();
//    dockVR->setProperty("Icon",":/icons/");
    tabifyDockWidget(dockDWP, dockVR);

    m_modelsPanel = new CModelsWidget();
    QDockWidget *dockModels = new QDockWidget(tr("Models list"), this);
    dockModels->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockModels->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
    dockModels->setObjectName("Models Panel");
    dockModels->setWidget(m_modelsPanel);
	dockModels->setProperty("Icon",":/icons/resources/models.png");
    tabifyDockWidget(dockDWP, dockModels);

    m_helpView = new QWebView();
    loadHelp();
    QDockWidget *dockHelp = new QDockWidget(tr("Help"), this);
    dockHelp->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockHelp->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
    dockHelp->setObjectName("Help View");
    dockHelp->setWidget(m_helpView);
    dockHelp->hide();
	dockHelp->setProperty("Icon",":/icons/3dim.ico");
    tabifyDockWidget(dockDWP, dockHelp);


    connect(dockDWP, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
    connect(dockOrtho, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
    connect(dockSeg, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
    connect(dockVR, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
	connect(dockModels, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
	connect(dockHelp, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));

    connect(dockDWP,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea))); 
    connect(dockOrtho,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
    connect(dockSeg,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
    connect(dockVR,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
	connect(dockModels,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
	connect(dockHelp,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
}

QSizeF MainWindow::getRelativeSize(QWidget* widget)
{
    QSizeF sizeF(0,0);
    if (NULL!=widget)
    {
        QSize sizeW = widget->size();
        QSize sizeM = size();
        sizeF.setWidth(sizeW.width()/(float)sizeM.width());
        sizeF.setHeight(sizeW.height()/(float)sizeM.height());
    }
    return sizeF;
}

// we save layout type, geometry, window state and a relative size of center window
void MainWindow::writeLayoutSettings(int nLayoutType, bool bInnerLayoutOnly)
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("layoutType",nLayoutType);
    settings.endGroup();
    if (Workspace3D==nLayoutType)
    {
        settings.beginGroup("Workspace3D");
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        settings.setValue("centralSize", getRelativeSize(centralWidget()));
        settings.endGroup();
    }
    if (WorkspaceTabs==nLayoutType)
    {
        settings.beginGroup("WorkspaceTabs");
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        settings.setValue("centralSize", getRelativeSize(centralWidget()));
        settings.endGroup();
    }
    if (WorkspaceGrid==nLayoutType)
    {
        settings.beginGroup("WorkspaceGrid");
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        settings.setValue("centralSize", getRelativeSize(centralWidget()));
        settings.endGroup();
    }
}

// if bInnerLayoutOnly is false, restores not only contents, but also size of
// the application window (desired on app start only)
void MainWindow::readLayoutSettings(bool bInnerLayoutOnly)
{
    QSettings settings;
    switch(m_nLayoutType)
    {            
    case WorkspaceTabs:
        settings.beginGroup("WorkspaceTabs");
        break;
    case WorkspaceGrid:
        settings.beginGroup("WorkspaceGrid");
        break;
    case Workspace3D:
        settings.beginGroup("Workspace3D");
        break;
    default:
        Q_ASSERT(false);
        return;
    }
    if (!bInnerLayoutOnly)
        restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    if (bInnerLayoutOnly)
    {
        QSizeF sizew=settings.value("centralSize").toSizeF();
        if (!sizew.isEmpty())
        {
            QSize sizeM = size();
            centralWidget()->resize(sizeM.width()*sizew.width(),sizeM.height()*sizew.height());
        }
    }
    settings.endGroup();
}

void MainWindow::afterWorkspaceChange()
{
    switch(m_nLayoutType)
    {
    case WorkspaceTabs:
    case WorkspaceGrid:
        ui->actionAxial_View->setEnabled(false);
        ui->actionCoronal_View->setEnabled(false);
        ui->actionSagittal_View->setEnabled(false);
        break;
    case Workspace3D:
        ui->actionAxial_View->setEnabled(true);
        ui->actionCoronal_View->setEnabled(true);
        ui->actionSagittal_View->setEnabled(true);
        break;
    default:
        Q_ASSERT(false);
    }
}

// Save user perspective (workspace layout)
void MainWindow::saveUserPerspective()
{
    QSettings settings;
    settings.beginGroup("WorkspaceCustom");
    settings.setValue("layoutType",m_nLayoutType);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("centralSize", getRelativeSize(centralWidget()));
    settings.endGroup();
}

// Load user perspective (workspace layout)
void MainWindow::loadUserPerspective()
{
    QSettings settings;
    settings.beginGroup("WorkspaceCustom");
    int nLayoutType=settings.value("layoutType").toInt();
    writeLayoutSettings(m_nLayoutType,true);
    //
    setUpdatesEnabled(false); // prevent screen flicker

    if (m_nLayoutType!=nLayoutType)
    {
        m_nLayoutType=nLayoutType;
        setUpWorkspace();
    }

//    if (!bInnerLayoutOnly)
//        restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
//    if (bInnerLayoutOnly)
    {
        QSizeF sizew=settings.value("centralSize").toSizeF();
        if (!sizew.isEmpty())
        {
            QSize sizeM = size();
            centralWidget()->resize(sizeM.width()*sizew.width(),sizeM.height()*sizew.height());
        }
    }
    setUpdatesEnabled(true);
    settings.endGroup();
}

// Load default perspective (workspace layout)
void MainWindow::loadDefaultPerspective()
{
#if(0) // resets to default layout
    writeLayoutSettings(m_nLayoutType,true);
    m_nLayoutType=Workspace3D;
    setUpdatesEnabled(false); // prevent screen flicker
    setUpWorkspace();
    //readLayoutSettings(true);
    setUpdatesEnabled(true);
#else
    QDockWidget* pDock1=NULL, *pDock=NULL;
    // change positions and visibility of scene dock windows (left dock area)
    if (Workspace3D==m_nLayoutType)
    {
        pDock=getParentDockWidget(m_OrthoXYSlice);
        removeDockWidget(pDock);
        addDockWidget(Qt::LeftDockWidgetArea,pDock);
        pDock->show();
        pDock=getParentDockWidget(m_OrthoXZSlice);
        removeDockWidget(pDock);
        addDockWidget(Qt::LeftDockWidgetArea,pDock);
        pDock->show();
        pDock=getParentDockWidget(m_OrthoYZSlice);
        removeDockWidget(pDock);
        addDockWidget(Qt::LeftDockWidgetArea,pDock);
        pDock->show();
    }
    // changes positions and visibility of dock windows only (right dock area)
    pDock1=getParentDockWidget(m_densityWindowPanel);
    addDockWidget(Qt::RightDockWidgetArea,pDock1);
    pDock1->show();
    pDock=getParentDockWidget(m_orthoSlicesPanel);
    tabifyDockWidget(pDock1,pDock);
    pDock->show();
    pDock=getParentDockWidget(m_segmentationPanel);
    tabifyDockWidget(pDock1,pDock);
    pDock->show();
    pDock=getParentDockWidget(m_modelsPanel);
    tabifyDockWidget(pDock1,pDock);
    pDock->show();
    pDock=getParentDockWidget(m_volumeRenderingPanel);
    tabifyDockWidget(pDock1,pDock);
    pDock->hide();
    pDock=getParentDockWidget(m_helpView);
    tabifyDockWidget(pDock1,pDock);
    pDock->hide();
	if (NULL!=m_pPlugins)
		m_pPlugins->tabifyAndHidePanels(pDock1);
#endif
}

void MainWindow::showEvent(QShowEvent *event)
{
    // update UI according to settings
    toolbarsEnabler();
    actionsEnabler();
    ui->actionAxial_Slice->setChecked(VPL_SIGNAL(SigGetPlaneXYVisibility).invoke2());
    ui->actionCoronal_Slice->setChecked(VPL_SIGNAL(SigGetPlaneXZVisibility).invoke2());
    ui->actionSagittal_Slice->setChecked(VPL_SIGNAL(SigGetPlaneYZVisibility).invoke2());

    // Test if any model is visible
    bool bAnyVisible(false);
    for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        if(VPL_SIGNAL(SigGetModelVisibility).invoke2(data::Storage::ImportedModel::Id + i))
        {
            bAnyVisible = true;
            break;
        }

    ui->actionShow_Surface_Model->setChecked(VPL_SIGNAL(SigGetModelVisibility).invoke2(bAnyVisible));
    QMainWindow::showEvent(event);
}

// save settings on main window close event
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (ui->actionSave_DICOM_Series->isEnabled() || ui->actionSave_Volumetric_Data->isEnabled())
    {
        if (QMessageBox::Yes!=QMessageBox::question(this,QCoreApplication::applicationName(),
                                property("SegmentationChanged").toBool()?tr("You have an unsaved segmentation data! All unsaved changes will be lost. Are your sure?"):tr("All unsaved changes will be lost. Are your sure?"),
                                QMessageBox::Yes|QMessageBox::No))
        {
            event->ignore();
            return;
        }
    }
    saveAppSettings();
	if (0!=(windowState()&Qt::WindowFullScreen))
		showNormal();
    writeLayoutSettings(m_nLayoutType,false);
    QMainWindow::closeEvent(event);
}

bool MainWindow::shallUpdateOSGCanvas(OSGCanvas* pCanvas, const QPoint& mousePos)
{
    if (NULL==pCanvas) return false;
    if (!pCanvas->isVisible()) return false;
    QRect globalRect;
    globalRect = QRect(pCanvas->mapToGlobal(pCanvas->rect().topLeft()),
    pCanvas->mapToGlobal(pCanvas->rect().bottomRight()));
    return (globalRect.contains(mousePos));
}

// show_frame is called on timer - necessary for event handling (like active objects recognition on mouse move)
void MainWindow::show_frame()
{
    // when no app window is not active do not perform any updates
    QWidget* activeWindow = QApplication::activeWindow();
    if (NULL==activeWindow) return;

    // do not redraw osg windows in main window when a modal dialog is displayed
    QWidget * pModalWidget=QApplication::activeModalWidget();
    if (pModalWidget)
    {
        //if (Qt::ApplicationModal==pModalWidget->windowModality())
        if (Qt::NonModal!=pModalWidget->windowModality())
            return;
    }

//    Qt::MouseButtons btns=QApplication::mouseButtons ();
    const QPoint mousePos=QCursor::pos(); // in global screen coordiantes
    //if (Qt::NoButton==btns) return;

    int renderingTime = 0;
    if (shallUpdateOSGCanvas(m_3DView,mousePos))
    {
        renderingTime = m_3DView->getLastRenderingTime(); // get time spent on gpu for last frames
        if (renderingTime < m_timer.interval()) // if longer than timer interval, do not perform update
            m_3DView->update();
        //qDebug() << "rt" << renderingTime;
    }
    if (shallUpdateOSGCanvas(m_OrthoXYSlice,mousePos))
        m_OrthoXYSlice->update();
    if (shallUpdateOSGCanvas(m_OrthoXZSlice,mousePos))
        m_OrthoXZSlice->update();
    if (shallUpdateOSGCanvas(m_OrthoYZSlice,mousePos))
        m_OrthoYZSlice->update();
    if (renderingTime>10) // if were able to get rendering time of the 3d scene
    {
        // adjust timer interval
        int timerTime = renderingTime+50;
        //qDebug() << renderingTime;
        if (timerTime<150)
            timerTime=150;
        if (timerTime>1500)
            timerTime=1500;
        m_timer.setInterval(timerTime);
        //qDebug() << "new timer interval " << timerTime;
    }
}

void MainWindow::fixBadSliceSliderPos()
{
    // Workaround for bug https://jic.3dim-laboratory.cz/mantis/view.php?id=9
    // Just invalidate appropriate items to get proper updates
    data::CObjectPtr<data::COrthoSliceXY> spSliceXY( APP_STORAGE.getEntry(data::Storage::SliceXY::Id) );
    APP_STORAGE.invalidate(spSliceXY.getEntryPtr() );
    data::CObjectPtr<data::COrthoSliceXZ> spSliceXZ( APP_STORAGE.getEntry(data::Storage::SliceXZ::Id) );
    APP_STORAGE.invalidate(spSliceXZ.getEntryPtr() );
    data::CObjectPtr<data::COrthoSliceYZ> spSliceYZ( APP_STORAGE.getEntry(data::Storage::SliceYZ::Id) );
    APP_STORAGE.invalidate(spSliceYZ.getEntryPtr() );
}

bool MainWindow::openDICOM()
{
    if (!preOpen())
        return false;

    QSettings settings;
#if QT_VERSION < 0x050000
    QString previousDir=settings.value("DICOMdir",QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString();
#else
    QString previousDir=settings.value("DICOMdir",QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)).toString();
#endif

    // get DICOM data directory
    QString fileName = QFileDialog::getExistingDirectory(this,tr("Load DICOM Data"),previousDir);
    if (fileName.isEmpty())
        return false;

    settings.setValue("DICOMdir",fileName);
    return openDICOM(fileName,fileName);
}

bool MainWindow::openDICOMZIP()
{
    if (!preOpen())
        return false;

    QSettings settings;
#if QT_VERSION < 0x050000
    QString previousDir=settings.value("DICOMdir",QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString();
#else
    QString previousDir=settings.value("DICOMdir",QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)).toString();
#endif

    // get DICOM data directory
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load DICOM Data"),previousDir,tr("ZIP archive (*.zip)"));
    if (fileName.isEmpty())
        return false;

    QFileInfo pathInfo( fileName );
    settings.setValue("DICOMdir",pathInfo.dir().absolutePath());

    return openDICOMZIP(fileName);
}

bool MainWindow::openDICOMZIP(QString fileName)
{
    QString realName = fileName;
    QFileInfo pathInfo( fileName );

    // if the selected item is a zip archive, decompress it to a temp directory
    CZipLoader loader;
    if (loader.setZipArchive(fileName))
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        fileName = loader.decompress();
        QApplication::restoreOverrideCursor();
    }
    
    return openDICOM(fileName,realName);
}

bool MainWindow::openDICOM(const QString& fileName, const QString& realName)
{
    QString fileNameIn(fileName);
    // make sure that we pass in directory name and not a file name
    QFileInfo fi(fileNameIn);
    if (!fi.isDir())
#if(0)
    {
        fileNameIn = fi.dir().absolutePath();
    }
#else    
    {
        std::string ansiName = fi.absolutePath().toStdString();
        int nFrames = 0;
        data::getDicomFileInfo( vpl::sys::tStringConv::fromUtf8(ansiName), fileNameIn.toStdString(), nFrames);
#define DICOM_MULTIFRAME_THRESHOLD  10
        if (nFrames<=DICOM_MULTIFRAME_THRESHOLD)
            fileNameIn = fi.dir().absolutePath();
    }
#endif

    // get ansi name for DCMTK
    // 2012/03/12: Modified by Majkl (Linux compatible code)
//    std::string str = fileNameIn.toUtf8();
    std::string str = fileNameIn.toStdString();
    data::sExtendedTags tags = { };

    {
        // Show simple progress dialog
        CProgress progress(this);
        progress.setLabelText(tr("Scanning the directory for DICOM datasets, please wait..."));
        progress.show();

        APP_STORAGE.reset();

        data::CDicomLoader Loader;
        Loader.registerProgressFunc( vpl::mod::CProgress::tProgressFunc( &progress, &CProgress::Entry ) );

        // preload data from the selected directory
        data::CSeries::tSmartPtr spSeries;
        QFileInfo fi(fileNameIn);
        if (!fi.isDir())
            spSeries = Loader.preLoadFile(vpl::sys::tStringConv::fromUtf8(str));
        else
            spSeries = Loader.preLoadDirectory(vpl::sys::tStringConv::fromUtf8(str));
        if( !spSeries.get() || spSeries->getNumSeries() <= 0 )
        {
            showMessageBox(QMessageBox::Critical,tr("No valid DICOM datasets have been found in a given directory!"));
            return false;
        }

        // let the user select the desired series
        CSeriesSelectionDialog dlg(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
        if (!dlg.setSeries(spSeries))
            return false;
        if (QDialog::Accepted!=dlg.exec())
            return false;
        int iSelection=dlg.getSelection();
        if (iSelection<0)
            return false;
        double ssX, ssY, ssZ;
        dlg.getSubsampling(ssX, ssY, ssZ);

        // Load the data (prepare a new method CExamination::load(spSeries.get()))
        data::CSerieInfo::tSmartPtr current_serie = spSeries->getSerie( iSelection );
        if( !current_serie )
        {
            return false;
        }

        progress.setLabelText(tr("Loading input DICOM dataset, please wait..."));
        vpl::mod::CProgress::tProgressFunc ProgressFunc(&progress, &CProgress::Entry);
        if( !m_Examination.loadDicomData(current_serie.get(),
                                         tags,
                                         ProgressFunc,
                                         data::PATIENT_DATA,
                                         data::CExamination::EST_MANUAL,
                                         vpl::img::CVector3d(ssX, ssY, ssZ)))
        {
            showMessageBox(QMessageBox::Critical,tr("Failed to load the DICOM dataset!"));
            return false;
        }

        // Store the list of loaded files
        data::CSerieInfo::tDicomFileList dicom_files;
        current_serie->getDicomFileList(dicom_files);
        data::CObjectPtr<data::CImageLoaderInfo> spInfo( APP_STORAGE.getEntry(data::Storage::ImageLoaderInfo::Id) );
        spInfo->setList(dicom_files);
        APP_STORAGE.invalidate(spInfo.getEntryPtr());
        spInfo.release();
    }

    // Get the active dataset
    data::CObjectPtr<data::CDensityData> spData( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );    

    // and show the limiter dialog
    CVolumeLimiterDialog vlDlg(this);
    vlDlg.setVolume(spData.get(), true);
    if (QDialog::Accepted==vlDlg.exec())
	{
		const data::SVolumeOfInterest limits = vlDlg.getLimits();
        m_Examination.setLimits(limits, data::PATIENT_DATA);

        // set volume transformation
        {
			osg::Matrix transformationMatrix = osg::Matrix::identity();
			transformationMatrix *= osg::Matrix::translate(-limits.m_MinX * spData->getDX(), -limits.m_MinY * spData->getDY(), -limits.m_MinZ * spData->getDZ());

            data::CObjectPtr<data::CVolumeTransformation> spVolumeTransformation(APP_STORAGE.getEntry(data::Storage::VolumeTransformation::Id));
            spVolumeTransformation->setTransformation(transformationMatrix);
            APP_STORAGE.invalidate(spVolumeTransformation.getEntryPtr());
        }
	}

    // Change the application title
    if (spData->m_sPatientName.empty())
        setWindowTitle(QApplication::applicationName() + " - " + realName);
    else
    {
        QString wsPatientName(spData->m_sPatientName.c_str());
        wsPatientName.replace("^"," ");
            int idxGroup = wsPatientName.indexOf('=');
            if (idxGroup>0)
                wsPatientName = wsPatientName.left(idxGroup);
        setWindowTitle(QApplication::applicationName() + " - " + realName + ", " + wsPatientName);
    }

    postOpen(realName, true);
    return true;
}

bool MainWindow::preOpen()
{
    if (property("SegmentationChanged").toBool())
    {
        if (QMessageBox::Yes!=QMessageBox::question(this,QCoreApplication::applicationName(),tr("You have an unsaved data! All unsaved changes will be lost. Do you want to continue?"),QMessageBox::Yes|QMessageBox::No))
            return false;
        setProperty("SegmentationChanged",false);
    }    
    return true;
}

void MainWindow::postOpen(const QString& filename, bool bDicomData)
{
    undoRedoEnabler();
    fixBadSliceSliderPos();
	ui->actionSave_Original_DICOM_Series->setEnabled(bDicomData);
    ui->actionSave_DICOM_Series->setEnabled(true);
    ui->actionSave_Volumetric_Data->setEnabled(true);
    setProperty("SegmentationChanged",false);
    m_wsProjectPath = filename;
    setProperty("ProjectName",m_wsProjectPath);
    addToRecentFiles(filename);
	m_savedEntriesVersionList = getVersionList();
}

void MainWindow::postSave(const QString& filename)
{
    m_wsProjectPath = filename;
    setProperty("ProjectName",m_wsProjectPath);
    addToRecentFiles(filename);
}

std::vector<int> MainWindow::getVersionList()
{
	int arrWatched[] = { data::Storage::PatientData::Id,
						 data::Storage::RegionData::Id, 
						 data::Storage::BonesModel::Id,
						 data::Storage::ImportedModel::Id};
	const int nEntries = sizeof(arrWatched)/sizeof(arrWatched[0]);
    std::vector<int> res;
    for(int i = 0; i < nEntries ; i++)
	{        
		if (data::Storage::ImportedModel::Id==arrWatched[i])
		{
			for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
				res.push_back( APP_STORAGE.getEntry( data::Storage::ImportedModel::Id+i ).get()->getLatestVersion());
		}
		else
			res.push_back( APP_STORAGE.getEntry( arrWatched[i] ).get()->getLatestVersion());
	}
    return res;
}

bool MainWindow::isDirty()
{
    std::vector<int> curVer = getVersionList();
    const int nEntries = curVer.size();
    if (m_savedEntriesVersionList.size()!=nEntries)
        return true;
    for(int i = 0; i<nEntries; i++)
    {
        if (m_savedEntriesVersionList[i]!=curVer[i])
            return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// Open/Save

// openVLM
bool MainWindow::openVLM()
{
    if (!preOpen())
        return false;

    QSettings settings;
#if QT_VERSION < 0x050000
    QString previousDir=settings.value("VLMdir",QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString();
#else
    QString previousDir=settings.value("VLMdir",QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)).toString();
#endif

    QString fileName = QFileDialog::getOpenFileName(this,tr("Please, choose an input volume data to open..."),previousDir,tr("Volume Data (*.vlm)"));
    if (fileName.isEmpty())
        return false;

    QFileInfo pathInfo( fileName );
    settings.setValue("VLMdir",pathInfo.dir().absolutePath());

    return openVLM(fileName);
}


//! File Channel implementation with seek support
class CFileChannelEx : public vpl::mod::CFileChannelU
{
protected:
	vpl::tSize m_savedPos;
public:
	CFileChannelEx(int Type, const vpl::sys::tString& sFileName) : CFileChannelU(Type,sFileName), m_savedPos(0)	{ }
	vpl::tSize fileSize() {
		if (NULL==m_File)
			return 0;
		size_t pos = ftell(m_File);
		fseek(m_File,0,SEEK_END);
		size_t end = ftell(m_File);
		fseek(m_File,pos,SEEK_SET);
		return end;
	}	
	void seek(vpl::tSize pos, int mode)	{	if (NULL!=m_File) fseek(m_File,pos,mode);	}
	void saveCurPos()	{	m_savedPos = NULL==m_File ? 0 : ftell(m_File);	}
	void restorePos()	{	if (NULL!=m_File) fseek(m_File,m_savedPos,SEEK_SET); }
};


bool MainWindow::openVLM(const QString &wsFileName)
{
    // 2012/03/12: Modified by Majkl (Linux compatible code)
//    std::string ansiName = fileName.toUtf8();
    std::string ansiName = wsFileName.toStdString();

    CProgress progress(this);
    progress.setLabelText(tr("Loading input volumetric data, please wait..."));
    vpl::mod::CProgress::tProgressFunc ProgressFunc(&progress, &CProgress::Entry);
    progress.show();

    // Load the data
    bool bResult = false;
    try 
    {
        APP_STORAGE.reset();        
#if(0)
        bResult = m_Examination.loadDensityDataU(vpl::sys::tStringConv::fromUtf8(ansiName),
                                                ProgressFunc,
                                                data::PATIENT_DATA
                                                );
#else
		// Open input file channel
		CFileChannelEx Channel(vpl::mod::CH_IN, vpl::sys::tStringConv::fromUtf8(ansiName));
		if( Channel.connect() )
		{
			// Try to load the data
			data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::PATIENT_DATA) );
			spVolume->clearDicomData();

			try {
				Channel.saveCurPos();
				const int fileSize = Channel.fileSize();
				bool bNewVlm = false; // recognize newer VLM by search of a DensityData sequence in the last 2kB of file
#define BUF_SIZE	2048
				if (fileSize>BUF_SIZE) 
				{
					Channel.seek(fileSize-BUF_SIZE,SEEK_SET);
					char buffer[BUF_SIZE];
					if (BUF_SIZE == Channel.read(buffer,BUF_SIZE))
					{
						char* start = (char*)memchr(buffer,'D',BUF_SIZE);
						while(!bNewVlm && NULL!=start && start-buffer<BUF_SIZE)
						{
							int rem = BUF_SIZE - (start - buffer);
							bNewVlm = 0==strncmp(start,"DensityData",rem);
							if (bNewVlm)
								break;
							start = (char*)memchr(start+1,'D',rem-1);

						}						
					}
					Channel.restorePos();
				}
#undef BUF_SIZE
				if(!bNewVlm || !vpl::mod::read(*spVolume.get(), Channel, ProgressFunc) ) // try to load vlm with metadata
				{
					Channel.restorePos();
					{
						vpl::img::CDensityVolume VolumeData;
						if( !vpl::mod::read(VolumeData, Channel, ProgressFunc) ) // try to load voxel data only
						{
							return false;
						}					
						else
						{
							spVolume->makeRef(VolumeData);
							spVolume->clearDicomData();
							// Fill the margin
							spVolume->mirrorMargin();
						}					
					}
				}
			}
			catch( const vpl::base::CException Exception )
			{
				return false;	
			}

			APP_STORAGE.invalidate(spVolume.getEntryPtr() );
			m_Examination.onDataLoad( data::PATIENT_DATA );

			// O.K.
			bResult = true;
		}
#endif
    }
    catch( const vpl::base::CFullException& /*Exception*/ )
    {
//        wxString sWhat(Exception.what(), wxConvUTF8);
//        wxLogMessage(_("Caught exception while loading input volumetric data (%s)"), sWhat.c_str());
         bResult = false;
    }
    if (!bResult)
    {
        showMessageBox(QMessageBox::Critical,tr("Failed to read input volumetric data!"));
        return false;
    }
    setWindowTitle(QApplication::applicationName()+" - "+wsFileName);

    postOpen(wsFileName, false);
    return true;
}

// saveVLMAs
bool MainWindow::saveVLMAs()
{
    QSettings settings;
    QString previousDir = getSaveLoadPath("VLMdir");
    previousDir = appendSaveNameHint(previousDir,".vlm");
    QString fileName = QFileDialog::getSaveFileName(this,tr("Choose an output file..."),previousDir,tr("Volume Data (*.vlm)"));
    if (fileName.isEmpty())
        return false;

    QFileInfo pathInfo( fileName );
    settings.setValue("VLMdir",pathInfo.dir().absolutePath());

    // Show simple progress dialog
    CProgress progress(this);
    progress.setLabelText(tr("Saving volumetric data, please wait..."));
    progress.show();

    // 2012/03/12: Modified by Majkl (Linux compatible code)
//    std::string ansiName=fileName.toUtf8();
    std::string ansiName=fileName.toStdString();

    // Save the data
    bool bResult = false;
    try {
         vpl::mod::CProgress::tProgressFunc ProgressFunc(&progress, &CProgress::Entry);
#if(0)	 // saves volumetric data only
         bResult = m_Examination.saveDensityDataU(vpl::sys::tStringConv::fromUtf8(ansiName),
                                                 ProgressFunc,
                                                 data::PATIENT_DATA
                                                 );
#else	 // save volumetric data with metadata (in format compatible with m_Examination.loadDensityDataU)
		 bResult = false;
		 vpl::mod::CFileChannelU Channel(vpl::mod::CH_OUT, vpl::sys::tStringConv::fromUtf8(ansiName));
		 if( Channel.connect() )
		 {
			 data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::PATIENT_DATA) );
			 // because vlm doesn't save data::CVolumeTransformation, we apply it on the tag saved in the patient data
			 const vpl::img::CVector3D pos(spVolume->m_ImagePosition);
			 data::CObjectPtr<data::CVolumeTransformation> spVolumeTransformation(APP_STORAGE.getEntry(data::Storage::VolumeTransformation::Id));
			 osg::Matrix volTransform = spVolumeTransformation->getTransformation();
			 osg::Vec3 offLoad = - volTransform.getTrans();
			 spVolume->m_ImagePosition += vpl::img::CVector3D(offLoad[0],offLoad[1],offLoad[2]);
			 // serialization
			 bResult = vpl::mod::write(*spVolume, Channel, ProgressFunc);
			 // restore original image position value
			 spVolume->m_ImagePosition = pos;
		 }

#endif
    }
    catch( const vpl::base::CFullException& /*Exception*/ )
    {
//		  wxString sWhat(Exception.what(), wxConvUTF8);
//		  wxLogMessage(_("Caught exception while saving volumetric data (%s)"), sWhat.c_str());
        bResult = false;
    }
    // Show the result
    if( !bResult )
    {
        showMessageBox(QMessageBox::Critical,tr("Failed to save volumetric data!"));
        return false;
    }
    else
    {
        postSave(fileName);
    }
    return true;
}


// save dicom series (copy source files to a directory of your choice)
bool MainWindow::saveOriginalDICOM()
{
    QSettings settings;
	QString previousDir = getSaveLoadPath("dstDICOMdir");
    if (previousDir.isEmpty())
        previousDir=settings.value("DICOMdir").toString();

    // get DICOM data destination directory
    QString fileName = QFileDialog::getExistingDirectory(this,tr("Please, choose a directory where to save current DICOM dataset..."),previousDir);
    if (fileName.isEmpty())
        return false;

    settings.setValue("dstDICOMdir",fileName);

	// get list of loaded dicoms
	data::CObjectPtr<data::CImageLoaderInfo> spInfo( APP_STORAGE.getEntry( data::Storage::ImageLoaderInfo::Id ) );
	if ( spInfo->getNumOfFiles() < 1 )
	{
		return false;
	}

	// handle name collisions
	std::vector<QString> fileList;
	for ( int i = 0; i < spInfo->getNumOfFiles(); i++ )
	{
		QString srcName = QString::fromStdString(vpl::sys::tStringConv::toUtf8(spInfo->getList()[i]));
		QFileInfo inputFileInfo( srcName );
		QString dstName = inputFileInfo.baseName() + "." + inputFileInfo.completeSuffix();
		int k = 0;
		for(int j = 0; j < i; j++)
		{
			if (0==dstName.compare(fileList[j],Qt::CaseInsensitive))
			{
				dstName = inputFileInfo.baseName() + QString::number(k) + "." + inputFileInfo.completeSuffix();
				j = 0; k++;
			}
		}
		fileList.push_back(dstName);
	}
	Q_ASSERT(spInfo->getNumOfFiles() == fileList.size());

	bool ok = true;
	// Copy files
	for ( int i = 0; i < spInfo->getNumOfFiles() && ok; i++ )
	{
		QString srcName = QString::fromStdString(vpl::sys::tStringConv::toUtf8(spInfo->getList()[i]));
		QFileInfo inputFileInfo( srcName );
		QString dstName = fileName + "/" + fileList[i]; // (fileName + "/") + inputFileInfo.baseName() + "." + inputFileInfo.completeSuffix();
		while (!QFile::copy(srcName,dstName) && ok)
		{
			if (QMessageBox::No==QMessageBox::question(this,QCoreApplication::applicationName(),tr("Failed to copy file %1. Retry?").arg(inputFileInfo.baseName() + "." + inputFileInfo.completeSuffix()),QMessageBox::Yes,QMessageBox::No))
			{
				ok = false;
				break;
			}
		}
	}
	//
	if (ok)
	{
		postSave(fileName);
	}
	return ok;
}

// save dicom series (copy source files to a directory of your choice)
bool MainWindow::saveDICOM()
{	
	data::CObjectPtr<data::CDensityData> spData( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );    
	const int sizeX = spData->getXSize();
	const int sizeY = spData->getYSize();
	const int sizeZ = spData->getZSize();
	const double dX = spData->getDX();
	const double dY = spData->getDY();
	const double dZ = spData->getDZ();

	QString SeriesID(spData->m_sSeriesUid.c_str());	
	if (SeriesID.isEmpty())
		SeriesID = "cs" + QString::number(spData->getDataCheckSum());
	
	// show dialog with options
    bool bSaveSegmented = false;
	bool bSaveCompressed = false;
	bool bSaveActive = false;
	{
		data::CObjectPtr<data::CRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id));
		const int activeRegion(spColoring->getActiveRegion());
		data::CRegionColoring::tColor color;
		if (activeRegion>=0)
		{
			color = spColoring->getColor( activeRegion );
		}
		QColor  qcolor(color.getR(), color.getG(), color.getB());

        QDialog dlg(NULL,Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
        dlg.setWindowTitle(tr("Save DICOM"));
        //dlg.setMinimumSize(500,300);
		dlg.setSizeGripEnabled(true);
        QGridLayout* pLayout = new QGridLayout();
        dlg.setLayout(pLayout);
		QRadioButton rbCurrent(tr("Save all data"));
		rbCurrent.setChecked(true);
        pLayout->addWidget(&rbCurrent,0,0);
        QRadioButton rbSegmented(tr("Save all segmented areas"));
        pLayout->addWidget(&rbSegmented,1,0);
		QRadioButton rbActiveOnly(tr("Save active region only"));
        pLayout->addWidget(&rbActiveOnly,2,0);
		QLabel labelRegion;
		labelRegion.setMinimumWidth(32);
		labelRegion.setMaximumWidth(32);
		labelRegion.setStyleSheet(QString("background-color: %1").arg(qcolor.name()));
		pLayout->addWidget(&labelRegion,2,1);
		QCheckBox cbCompress(tr("Compress"));
		pLayout->addWidget(&cbCompress,3,0);
		QDialogButtonBox* pButtonBox = new QDialogButtonBox;
		pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        pButtonBox->setCenterButtons(true);
        connect(pButtonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
		connect(pButtonBox, SIGNAL(rejected()), &dlg, SLOT(reject()));
        pLayout->addWidget(pButtonBox,4,0,1,2);
		if (QDialog::Accepted!=dlg.exec())
			return false;
		bSaveSegmented = rbSegmented.isChecked() || rbActiveOnly.isChecked();
		bSaveCompressed = cbCompress.isChecked();
		bSaveActive = rbActiveOnly.isChecked();
	}

	if (bSaveSegmented)
	{
		QObject* pPlugin=findPluginByID("RegionControl");
		if (NULL!=pPlugin)
		{
			PluginLicenseInterface *iPlugin = qobject_cast<PluginLicenseInterface *>(pPlugin);
			if (iPlugin)
			{
				if (!iPlugin->canExport(SeriesID))
				{
					QMessageBox::warning(this,QCoreApplication::applicationName(),tr("Segmented areas can't be exported with the current product license."));
					return false;
				}
			}
		}
	}

    QSettings settings;
	QString previousDir = getSaveLoadPath("dstDICOMdir");
    if (previousDir.isEmpty())
        previousDir=settings.value("DICOMdir").toString();

    // get DICOM data destination directory
    QString fileName = QFileDialog::getExistingDirectory(this,tr("Please, choose a directory where to save current DICOM dataset..."),previousDir);
    if (fileName.isEmpty())
        return false;

	// check directory contents and show warning if it contains any previously saved dicom files
	{
		QDir dir(fileName);
		QStringList extensions;
		extensions.push_back("*.dcm");
		extensions.push_back("*.dicom");
		QStringList lstDicom = dir.entryList(extensions, QDir::Files);
		if (!lstDicom.empty())
		{
			if (QMessageBox::Yes!=QMessageBox::warning(this,QCoreApplication::applicationName(),tr("Specified directory already contains DICOM files! Do you really want to continue?"),QMessageBox::Yes,QMessageBox::No))
				return false;
		}
	}

    settings.setValue("dstDICOMdir",fileName);

    // Show simple progress dialog
    CProgress progress(this);
    progress.setLabelText(tr("Saving DICOM data, please wait..."));
    progress.show();

#ifdef WIN32
	// path in ACP
	std::wstring uniName = (const wchar_t*)fileName.utf16();
	std::string ansiName = wcs2ACP(uniName);
#else // UTF8
	std::string ansiName = fileName.toStdString();
#endif

#ifdef __APPLE__
	std::auto_ptr<qint16> ptr(new qint16[2*sizeX*sizeY]);
#else
    std::unique_ptr<qint16> ptr(new qint16[2*sizeX*sizeY]);
#endif

	// TODO: check whether the top 3 slices are not empty ones (used for volume size alignment)

	char uid[128]={};
	char studyUIDStr[128]={};
	char seriesUIDStr[128]={};
	dcmGenerateUniqueIdentifier(uid, SITE_INSTANCE_UID_ROOT);
	dcmGenerateUniqueIdentifier(studyUIDStr, SITE_STUDY_UID_ROOT);
	dcmGenerateUniqueIdentifier(seriesUIDStr, SITE_SERIES_UID_ROOT);

    DJEncoderRegistration::registerCodecs(); // register JPEG codecs

	bool bOk = true;
	for(int z=0; z<sizeZ && bOk; z++)
	{
		if (!progress.Entry(z,sizeZ))
		{
			bOk = false;
			break;
		}

		// prepare pixel data
        if (bSaveSegmented)
        {			
            data::CObjectPtr< data::CRegionData > rVolume( APP_STORAGE.getEntry( data::Storage::RegionData::Id ) );
            if (rVolume->getSize()==spData->getSize())
            {
				if (bSaveActive)
				{
					data::CObjectPtr<data::CRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id));
					const int activeRegion(spColoring->getActiveRegion());
					qint16* pData = ptr.get();
					for(int y=0;y<sizeY;y++)
					{
						for(int x=0;x<sizeX;x++)
						{
							if (rVolume->at(x,y,z)==activeRegion)
								*pData = std::max(spData->at(x,y,z)+1024,0);
							else
								*pData = 0;
							pData++;
						}
					}
				}
				else
				{
					qint16* pData = ptr.get();
					for(int y=0;y<sizeY;y++)
					{
						for(int x=0;x<sizeX;x++)
						{
							if (rVolume->at(x,y,z)!=0)
								*pData = std::max(spData->at(x,y,z)+1024,0);
							else
								*pData = 0;
							pData++;
						}
					}
				}
            }
        }
        else // all data
        {
			qint16* pData = ptr.get();
			for(int y=0;y<sizeY;y++)
			{
				for(int x=0;x<sizeX;x++)
				{
					*pData=std::max(spData->at(x,y,z)+1024,0);
					pData++;
				}
			}
        }

		// create dicom structures
		DcmFileFormat fileFormat; 
		DcmDataset *pDataset = fileFormat.getDataset();

		{	// set dicom tags		
			pDataset->putAndInsertString(DCM_SOPInstanceUID, uid);
			pDataset->putAndInsertString(DCM_SOPClassUID, UID_SecondaryCaptureImageStorage);

			if (spData->m_sStudyUid.empty())
				pDataset->putAndInsertString(DCM_StudyInstanceUID,studyUIDStr);
			else
				pDataset->putAndInsertString(DCM_StudyInstanceUID, spData->m_sStudyUid.c_str());				

			if (spData->m_sSeriesUid.empty())
				pDataset->putAndInsertString(DCM_SeriesInstanceUID,seriesUIDStr);
			else
				pDataset->putAndInsertString(DCM_SeriesInstanceUID, spData->m_sSeriesUid.c_str());
		
			pDataset->putAndInsertString(DCM_Manufacturer, "3Dim Laboratory");
			{
				app::CProductInfo info=app::getProductInfo();
				std::string product=QString("%1 %2.%3.%4").arg(QCoreApplication::applicationName()).arg(info.getVersion().getMajorNum()).arg(info.getVersion().getMinorNum()).arg(info.getVersion().getBuildNum()).toStdString();
				pDataset->putAndInsertString(DCM_ManufacturerModelName, product.c_str());
			}

			pDataset->putAndInsertString( DCM_PatientName,  spData->m_sPatientName.c_str() );
			pDataset->putAndInsertString( DCM_PatientID,  spData->m_sPatientId.c_str() );
			pDataset->putAndInsertString( DCM_PatientSex,  spData->m_sPatientSex.c_str() );
			pDataset->putAndInsertString( DCM_PatientPosition,  spData->m_sPatientPosition.c_str() );
			pDataset->putAndInsertString( DCM_PatientComments,  spData->m_sPatientDescription.c_str() );
			pDataset->putAndInsertString( DCM_PatientBirthDate,  spData->m_sPatientBirthday.c_str() );

			pDataset->putAndInsertString( DCM_SeriesDescription,  spData->m_sSeriesDescription.c_str() );
			pDataset->putAndInsertString( DCM_StudyDescription,  spData->m_sStudyDescription.c_str() );

			pDataset->putAndInsertString( DCM_SeriesDate,  spData->m_sSeriesDate.c_str() );
			pDataset->putAndInsertString( DCM_SeriesTime,  spData->m_sSeriesTime.c_str() );

			pDataset->putAndInsertString( DCM_StudyDate,  spData->m_sStudyDate.c_str() );

			pDataset->putAndInsertString( DCM_Modality,  spData->m_sModality.c_str() );
			pDataset->putAndInsertString( DCM_ScanOptions,  spData->m_sScanOptions.c_str() );
				
			pDataset->putAndInsertString(DCM_ImageOrientationPatient,"1.000000\0.000000\0.000000\0.000000\1.000000\0.000000");

			std::stringstream spacing;
			spacing << std::fixed << std::setprecision(6) << dX << "\\" << dY;
			std::string sspacing = spacing.str();
			pDataset->putAndInsertString(DCM_PixelSpacing,sspacing.c_str());
			pDataset->putAndInsertUint16(DCM_Rows,sizeY);
			pDataset->putAndInsertUint16(DCM_Columns,sizeX);
			pDataset->putAndInsertUint16(DCM_PixelRepresentation,0); // 1 signed, 0 unsigned
			//pDataset->putAndInsertUint16(DCM_NumberOfFrames,1);
			pDataset->putAndInsertUint16(DCM_SamplesPerPixel,1); // count of channels			
			pDataset->putAndInsertString( DCM_PhotometricInterpretation,  "MONOCHROME2" );
			//pDataset->putAndInsertUint16(DCM_PlanarConfiguration,0);
			pDataset->putAndInsertUint16(DCM_BitsAllocated,16);
			pDataset->putAndInsertUint16(DCM_BitsStored,16);
			pDataset->putAndInsertUint16(DCM_HighBit, 15);								
			pDataset->putAndInsertString(DCM_RescaleIntercept, "-1024");
			pDataset->putAndInsertString(DCM_RescaleSlope, "1");
			//pDataset->putAndInsertString(DCM_WindowCenter, "0");
			//pDataset->putAndInsertString(DCM_WindowWidth, "1000");
			{
				data::CObjectPtr<data::CVolumeTransformation> spVolumeTransformation(APP_STORAGE.getEntry(data::Storage::VolumeTransformation::Id));
				osg::Matrix volTransform = spVolumeTransformation->getTransformation();
				osg::Vec3 offLoad = - volTransform.getTrans();

				std::stringstream pos;				
				pos << std::fixed << std::setprecision(6) << spData->m_ImagePosition.x() + offLoad[0] << "\\" << spData->m_ImagePosition.y() + offLoad[1] << "\\" << spData->m_ImagePosition.z() + offLoad[2] + z * dZ;
				std::string spos = pos.str();
				pDataset->putAndInsertString( DCM_ImagePositionPatient,  spos.c_str() );
			}
			{
				std::stringstream thickness;
				thickness << dZ;
				pDataset->putAndInsertString(DCM_SliceThickness,thickness.str().c_str());				
			}
			pDataset->putAndInsertUint16(DCM_InstanceNumber,z);

			pDataset->putAndInsertUint16Array(DCM_PixelData, (Uint16*)ptr.get(), sizeX*sizeY); 
		}

		// data compression
		E_TransferSyntax xfer = EXS_Unknown;
		if(bSaveCompressed)
        {
            DJ_RPLossless params; // codec parameters, we use the defaults        			
            // this causes the lossless JPEG version of the dataset to be created
        #if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361)
            pDataset->chooseRepresentation(EXS_JPEGProcess14SV1, &params);
        #else
            pDataset->chooseRepresentation(EXS_JPEGProcess14SV1TransferSyntax, &params);
        #endif

            // check if everything went well
        #if defined(PACKAGE_VERSION_NUMBER) && (PACKAGE_VERSION_NUMBER == 361)
            if (pDataset->canWriteXfer(EXS_JPEGProcess14SV1))
			{
				xfer = EXS_JPEGProcess14SV1;
        #else
            if (pDataset->canWriteXfer(EXS_JPEGProcess14SV1TransferSyntax))
			{
				xfer = EXS_JPEGProcess14SV1TransferSyntax;
        #endif
            }
        }
			
		{	// save file
			std::stringstream ss;
			ss << ansiName << "/" << z << ".dcm";
			std::string curFile = ss.str();

			if (xfer == EXS_Unknown)
				xfer = EXS_LittleEndianExplicit;
			OFCondition status = fileFormat.saveFile(curFile.c_str(), xfer);
			if (status.bad())
				bOk = false;
		}
	}
    DJEncoderRegistration::cleanup(); // deregister JPEG codecs

	if (bOk)
	{
		// update license info
		if (bSaveSegmented)
		{
			QObject* pPlugin=findPluginByID("RegionControl");
			if (NULL!=pPlugin)
			{
				PluginLicenseInterface *iPlugin = qobject_cast<PluginLicenseInterface *>(pPlugin);
				if (iPlugin)
				{
					iPlugin->wasExported(SeriesID);
				}
			}
		}
		postSave(fileName);
	}
	return bOk;
}

bool MainWindow::openSTL()
{
    QSettings settings;
#if QT_VERSION < 0x050000
    QString previousDir=settings.value("STLdir",QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString();
#else
    QString previousDir=settings.value("STLdir",QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)).toString();
#endif

    previousDir = getSaveLoadPath("STLdir");
    QString fileName = QFileDialog::getOpenFileName(this,tr("Choose an input binary STL model to load..."),previousDir,tr("Stereo litography model (*.stl);;Polygon file format (*.ply)"));
    if (fileName.isEmpty())
        return false;

    QFileInfo pathInfo( fileName );
    settings.setValue("STLdir",pathInfo.dir().absolutePath());
    return openSTL(fileName);
}

struct sGeodeInfo
{
    osg::Geode* pGeode;
    osg::Matrix matrix;
};

void parseChildren(osg::Node* pNode, const osg::Matrix& matrix, std::vector<sGeodeInfo> &geodes)
{
    if (NULL==pNode) return;
    Q_ASSERT(!matrix.isNaN());
    if (matrix.isNaN()) return;
    // is geode?
    osg::Geode* pGeode = pNode->asGeode();
    if (NULL!=pGeode)
    {
        sGeodeInfo info = {};
        info.pGeode = pGeode;
        info.matrix = matrix;
        geodes.push_back(info);
        return;
    }    
    // is a matrix transform
    osg::MatrixTransform* pTransform = dynamic_cast<osg::MatrixTransform*>(pNode);
    if (NULL!=pTransform)
    {
        osg::Matrix mx = matrix * pTransform->getMatrix();
        for(unsigned int j = 0; j < pTransform->getNumChildren(); ++j)
        {
            if (NULL==pTransform->getChild(j)) continue;
            parseChildren(pTransform->getChild(j),mx,geodes);
        }
        return;
    }
    // is a group?
    osg::Group* pGroup = pNode->asGroup();
    if (NULL!=pGroup)
    {
        for(unsigned int i = 0; i < pGroup->getNumChildren(); ++i)
        {
            if (NULL==pGroup->getChild(i)) continue;
            parseChildren(pGroup->getChild(i),matrix,geodes);
        }
        return;
    }
    Q_ASSERT(false);
}


bool MainWindow::openSTL(const QString& fileName)
{
#ifdef WIN32
    // OpenMesh needs path in ACP
    std::wstring uniName = (const wchar_t*)fileName.utf16();
    std::string ansiName = wcs2ACP(uniName);
#else
    // 2012/03/12: Modified by Majkl (Linux compatible code)
    //std::string ansiName = fileName.toUtf8();
    std::string ansiName = fileName.toStdString();
#endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // get file extension
    std::string extension;
    std::string::size_type pos(ansiName.rfind("."));    
    if (pos != std::string::npos)
    { 
        extension = ansiName.substr(pos+1, ansiName.length()-pos-1);
        std::transform( extension.begin(), extension.end(), extension.begin(), tolower );
    }

    bool result = false;
	int loadedID = 0;

    // don't use osg for stl models as it doesn't merge vertices
    std::string extensions = "stl stla stlb ply obj";
    if (extensions.find(extension) == std::string::npos)
    {
        // Find unused id
        int id(findPossibleModelId());
		if(id < 0)
			return false;

        // see if we can load the file using osg
        osg::ref_ptr<osg::Node> model(osgDB::readNodeFile(ansiName));
        if(model.get())
        {
            // if the node is a group, get all child geodes
            std::vector<sGeodeInfo> geodes;
            parseChildren(model,osg::Matrix::identity(),geodes);            
            int nDrawables = 0;
            for(int g = 0; g < geodes.size(); g++)
            {
                osg::Geode* pGeode = geodes[g].pGeode;                            
                const UniDrawableList& list=getGeodeDrawableList(pGeode);
                nDrawables += list.size();
            }
            if (nDrawables>0)
            {
                data::CObjectPtr< data::CModel > pModel( APP_STORAGE.getEntry( id ) );
                geometry::CMesh *pMesh = new geometry::CMesh;
                pModel->setMesh(pMesh);
				pModel->clearAllProperties();
            }
            // for every geode that we found
            for(int g = 0; g < geodes.size(); g++)
            {
                osg::Geode* pGeode = geodes[g].pGeode;            
                // for every drawable in the geode
                const UniDrawableList& list=getGeodeDrawableList(pGeode);
                for(UniDrawableList::const_iterator it = list.begin(); it!=list.end(); ++it)
                {
                    osg::Geometry* pGeom = it->operator osg::Drawable *()->asGeometry();
                    if (NULL!=pGeom)
                    {                    
                        // has valid data?
                        osg::Vec3Array * vertices = static_cast< osg::Vec3Array* >( pGeom->getVertexArray() );
                        const unsigned int nSets = pGeom->getNumPrimitiveSets();                    
                        const int nVertices = vertices->size();
                        if (nVertices>0 && nSets>0)
                        {
                            // Open model interface
                            data::CObjectPtr< data::CModel > pModel( APP_STORAGE.getEntry( id ) );
                            // get destination mesh
                            geometry::CMesh *pMesh = pModel->getMesh();
                            // get vertices
                            std::vector<geometry::CMesh::VertexHandle> omVertices;
                            omVertices.reserve(nVertices);
                            //std::vector<osg::Vec3> dbgVertices;
                            for(int i = 0; i<nVertices; i++)
                            {
                                osg::Vec3 vert((*vertices)[i]);
                                vert = vert * geodes[g].matrix; // apply transformation matrix
                                //dbgVertices.push_back(vert);
                                Q_ASSERT(!vert.isNaN());
                                geometry::CMesh::Point vx;
                                vx[0] = vert[0];
                                vx[1] = vert[1];
                                vx[2] = vert[2];
                                omVertices.push_back(pMesh->add_vertex(vx));                            
                            }                                                

                            // get primitives
                            for(unsigned int i = 0; i<nSets; i++)
                            {
                                osg::PrimitiveSet* pSet = pGeom->getPrimitiveSet(i);
                                switch (pSet->getMode()) 
                                {
                                case osg::PrimitiveSet::TRIANGLES:
                                    {
                                        const int nIndices = pSet->getNumIndices();
                                        for (int i2=0; i2<nIndices-2; i2+=3) 
                                        {
                                            int idx0 = pSet->index(i2);
                                            int idx1 = pSet->index(i2+1);
                                            int idx2 = pSet->index(i2+2);
                                            //assert(idx0!=idx1 && idx1!=idx2 && idx0!=idx2);
                                            if (idx0!=idx1 && idx1!=idx2 && idx0!=idx2)
                                            {
                                                assert(idx0>=0 && idx0<nVertices);
                                                assert(idx1>=0 && idx1<nVertices);
                                                assert(idx2>=0 && idx2<nVertices);
                                                if (pMesh->point(omVertices[idx0])!=pMesh->point(omVertices[idx1]))
                                                    pMesh->add_face(omVertices[idx0],
                                                                    omVertices[idx1],
                                                                    omVertices[idx2]);
                                            }
		                                }
                                    }
                                    break;
                                case osg::PrimitiveSet::TRIANGLE_STRIP:
                                    {
                                        const int nIndices = pSet->getNumIndices();
                                        int idx[3]={};
                                        idx[0] = pSet->index(0);
                                        idx[1] = pSet->index(1);
                                        idx[2] = pSet->index(2);
                                        //assert(idx[0]!=idx[1] && idx[1]!=idx[2] && idx[0]!=idx[2]);
                                        if (idx[0]!=idx[1] && idx[1]!=idx[2] && idx[0]!=idx[2])
                                            pMesh->add_face(omVertices[idx[0]],
                                                        omVertices[idx[1]],
                                                        omVertices[idx[2]]);
                                        int m = 0;
                                        for (int i2=3; i2<nIndices; i2++) 
                                        {
                                            idx[m] = pSet->index(i2);
                                            m = (m+1)%3;                                        
                                            assert(idx[0]>=0 && idx[0]<nVertices);
                                            assert(idx[1]>=0 && idx[1]<nVertices);
                                            assert(idx[2]>=0 && idx[2]<nVertices);
                                            //assert(idx[0]!=idx[1] && idx[1]!=idx[2] && idx[0]!=idx[2]);
                                            if (idx[0]!=idx[1] && idx[1]!=idx[2] && idx[0]!=idx[2])
                                            {
                                                if (i2%2==1)
                                                {
                                                    pMesh->add_face(omVertices[idx[1]],
                                                                    omVertices[idx[0]],
                                                                    omVertices[idx[2]]);
                                                }
                                                else
                                                {
                                                    pMesh->add_face(omVertices[idx[0]],
                                                                    omVertices[idx[1]],
                                                                    omVertices[idx[2]]);
                                                }
		                                    }
                                        }
                                    }
                                    break;
                                case osg::PrimitiveSet::QUADS:
                                    {
                                        const int nIndices = pSet->getNumIndices();
                                        for (int i2=0; i2<nIndices-3; i2+=4) 
                                        {
                                            int idx0 = pSet->index(i2);
                                            int idx1 = pSet->index(i2+1);
                                            int idx2 = pSet->index(i2+2);
                                            int idx3 = pSet->index(i2+3);
                                            assert(idx0>=0 && idx0<nVertices);
                                            assert(idx1>=0 && idx1<nVertices);
                                            assert(idx2>=0 && idx2<nVertices);
                                            assert(idx3>=0 && idx3<nVertices);
                                            //assert(idx0!=idx3 && idx3!=idx2 && idx0!=idx2);
                                            if (idx0!=idx3 && idx3!=idx2 && idx0!=idx2)
                                                pMesh->add_face(omVertices[idx0],
                                                                omVertices[idx3],
                                                                omVertices[idx2]);
                                            //assert(idx0!=idx1 && idx1!=idx2 && idx0!=idx2);
                                            if (idx0!=idx1 && idx1!=idx2 && idx0!=idx2)
                                                pMesh->add_face(omVertices[idx2],
                                                                omVertices[idx1],
                                                                omVertices[idx0]);
		                                }
                                    }
                                    break;
                                default:             
//                                    int mode = pSet->getMode();
                                    //assert(false);
                                    break;
                                }
                            }   
                        }
                    }
                }
            }
            if (nDrawables>0)
            {
                data::CObjectPtr< data::CModel > pModel( APP_STORAGE.getEntry( id ) );
                geometry::CMesh *pMesh = pModel->getMesh();
                pMesh->delete_isolated_vertices();
                pMesh->garbage_collection();
                if (pMesh->n_faces()>0)
                {
                    //pModel->setMesh(pMesh);
                    pModel->setLabel(m_modelLabel);
					pModel->setColor(m_modelColor);
                    pModel->setVisibility(true);

                    pModel->setTransformationMatrix(osg::Matrix::identity());
                    APP_STORAGE.invalidate(pModel.getEntryPtr());                        

					loadedID = id;
                    result = true;
                }
            }
        }
    }

    if (!result)
    {
        geometry::CMesh *pMesh = new geometry::CMesh;
        OpenMesh::IO::Options ropt;
        ropt += OpenMesh::IO::Options::Binary;
        result=true;
        if (!OpenMesh::IO::read_mesh(*pMesh, ansiName, ropt))
        {
            delete pMesh;
            result = false;
            showMessageBox(QMessageBox::Critical,tr("Failed to load binary STL model!"));
        }
        else
        {
            // Find unused id
            int id(findPossibleModelId());
			if(id < 0)
				return false;
			loadedID = id;
            data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(id) );
            spModel->setMesh(pMesh);
			spModel->setLabel(m_modelLabel);
			spModel->setColor(m_modelColor);
            spModel->setVisibility(true);
			spModel->clearAllProperties();
			spModel->setTransformationMatrix(osg::Matrix::identity());
            APP_STORAGE.invalidate(spModel.getEntryPtr());         			
        }
    }
    if (result)
	{
		QSettings settings;
		const bool bUseDicom = settings.value("STLUseDICOMCoord",false).toBool();
		if (loadedID>0 && bUseDicom)
		{
			data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(loadedID) );
			data::CObjectPtr<data::CVolumeTransformation> spVolumeTransformation(APP_STORAGE.getEntry(data::Storage::VolumeTransformation::Id));
			data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );
			vpl::img::CVector3D pos = spVolume->m_ImagePosition;
			spModel->setTransformationMatrix(osg::Matrix::translate(-pos.getX(),-pos.getY(),-pos.getZ()) * spVolumeTransformation->getTransformation());
			APP_STORAGE.invalidate(spModel.getEntryPtr());         			
		}
        addToRecentFiles(fileName);
	}
    QApplication::restoreOverrideCursor();
    return result;
}

bool MainWindow::saveSTL()
{
    // Try to get selected model id
    int storage_id(m_modelsPanel->getSelectedModelStorageId());
    if(storage_id == -1)
    {
		int nValid = 0;
		int firstID = 0;
		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(data::Storage::ImportedModel::Id + i, data::Storage::NO_UPDATE) );
			if (spModel->hasData()) 
			{
				if (0==firstID)
					firstID = data::Storage::ImportedModel::Id + i;
				nValid++;
			}
		}
		if (1==nValid && firstID>0)
		{
			// use the only existing model
			storage_id = firstID;
		}
		else
		{
			showMessageBox(QMessageBox::Critical, tr("No model selected!"));
			showModelsListPanel(true);
			return false;
		}
    }

    data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(storage_id) );

    // Check the model
    geometry::CMesh *pMesh = spModel->getMesh();
    if (!pMesh || !(pMesh->n_vertices() > 0) )
    {
        showMessageBox(QMessageBox::Critical, tr("No STL data!"));
        return false;
    }

	data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );
	QString SeriesID(spVolume->m_sSeriesUid.c_str());	
	if (SeriesID.isEmpty())
		SeriesID = "cs" + QString::number(spVolume->getDataCheckSum());
	spVolume.release();

	if (!spModel->getProperty("Licensed").empty())
	{
		QObject* pPlugin=findPluginByID("ModelCreate");
		if (NULL!=pPlugin)
		{
			PluginLicenseInterface *iPlugin = qobject_cast<PluginLicenseInterface *>(pPlugin);
			if (iPlugin)
			{
				if (!iPlugin->canExport(SeriesID))
				{
					QMessageBox::warning(this,QCoreApplication::applicationName(),tr("Data can't be exported with the current product license."));
					return false;
				}
			}
		}
	}

    QSettings settings;
	const bool bUseDicom = settings.value("STLUseDICOMCoord",false).toBool();
    QString previousDir = getSaveLoadPath("STLdir");
    previousDir = appendSaveNameHint(previousDir,".stl");
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(this,tr("Please, specify an output file..."),previousDir,tr("Stereo litography model (*.stl);;Binary Polygon file format (*.ply);;ASCII Polygon file format (*.ply)"),&selectedFilter);
    if (fileName.isEmpty())
        return false;

    QFileInfo pathInfo( fileName );
    settings.setValue("STLdir",pathInfo.dir().absolutePath());

#ifdef WIN32
    // OpenMesh needs path in ACP
    std::wstring uniName = (const wchar_t*)fileName.utf16();
    std::string ansiName = wcs2ACP(uniName);
#else
    // 2012/03/12: Modified by Majkl (Linux compatible code)
    //std::string ansiName = fileName.toUtf8();
    std::string ansiName = fileName.toStdString();
#endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    OpenMesh::IO::Options wopt = OpenMesh::IO::Options::Binary;
    if (0==selectedFilter.compare(tr("ASCII Polygon file format (*.ply)"),Qt::CaseInsensitive))
        wopt = OpenMesh::IO::Options::Default;

	data::CObjectPtr<data::CVolumeTransformation> spVolumeTransformation(APP_STORAGE.getEntry(data::Storage::VolumeTransformation::Id));
	osg::Matrix volTransform = spVolumeTransformation->getTransformation();	
	if (bUseDicom)
	{		
		// apply transformation matrix
		data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );
        vpl::img::CVector3D pos = spVolume->m_ImagePosition;
		osg::Matrix modelMx = spModel->getTransformationMatrix();		
		osg::Matrix exportMatrix = modelMx * osg::Matrix::inverse(osg::Matrix::translate(-pos.getX(),-pos.getY(),-pos.getZ()) * volTransform);
		if (!exportMatrix.isIdentity())
		{
			pMesh = new geometry::CMesh(*pMesh);
			for (geometry::CMesh::VertexIter vit = pMesh->vertices_begin(); vit != pMesh->vertices_end(); ++vit)
			{
				geometry::CMesh::Point point = pMesh->point(vit.handle());
				osg::Vec3 vertex(point[0], point[1], point[2]);
				vertex = vertex * exportMatrix;
				point = geometry::CMesh::Point(vertex[0], vertex[1], vertex[2]);
				pMesh->point(vit.handle()) = point;
			}
		}
	}
	else
	{		
		// apply transformation matrix
		osg::Matrix modelMx = spModel->getTransformationMatrix();
		osg::Matrix exportMatrix = modelMx; //* osg::Matrix::inverse(volTransform);
		if (!exportMatrix.isIdentity())
		{
			pMesh = new geometry::CMesh(*pMesh);
			for (geometry::CMesh::VertexIter vit = pMesh->vertices_begin(); vit != pMesh->vertices_end(); ++vit)
			{
				geometry::CMesh::Point point = pMesh->point(vit.handle());
				osg::Vec3 vertex(point[0], point[1], point[2]);
				vertex = vertex * exportMatrix;
				point = geometry::CMesh::Point(vertex[0], vertex[1], vertex[2]);
				pMesh->point(vit.handle()) = point;
			}
		}
	}
	
    bool result = true;
    if (!OpenMesh::IO::write_mesh(*pMesh, ansiName, wopt))
    {
        result = false;
        showMessageBox(QMessageBox::Critical, tr("Failed to save binary STL model!"));
    }
    else
	{
        addToRecentFiles(fileName);

		if (!spModel->getProperty("Licensed").empty())
		{
			// update license info
			QObject* pPlugin=findPluginByID("ModelCreate");
			if (NULL!=pPlugin)
			{
				PluginLicenseInterface *iPlugin = qobject_cast<PluginLicenseInterface *>(pPlugin);
				if (iPlugin)
				{
					iPlugin->wasExported(SeriesID);
				}
			}
		}
	}
	if (pMesh != spModel->getMesh())
		delete pMesh;
    QApplication::restoreOverrideCursor();
    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Show/hide/update toolbars, views...

void MainWindow::showMessageBox(QMessageBox::Icon icon, QString message)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QCoreApplication::applicationName());
    msgBox.setIcon(icon);
    msgBox.setText(message);
    msgBox.exec();
}

void MainWindow::showMainToolBar()
{
    if (ui->mainToolBar->isVisible())
        ui->mainToolBar->hide();
    else
        ui->mainToolBar->show();
}

void MainWindow::showViewsToolBar()
{
    if (ui->viewsToolBar->isVisible())
        ui->viewsToolBar->hide();
    else
        ui->viewsToolBar->show();
}

void MainWindow::showMouseToolBar()
{
    if (ui->mouseToolBar->isVisible())
        ui->mouseToolBar->hide();
    else
        ui->mouseToolBar->show();
}

void MainWindow::showVisibilityToolBar()
{
    if (ui->visibilityToolBar->isVisible())
        ui->visibilityToolBar->hide();
    else
        ui->visibilityToolBar->show();
}

void MainWindow::showPanelsToolBar()
{
    if (ui->panelsToolBar->isVisible())
        ui->panelsToolBar->hide();
    else
        ui->panelsToolBar->show();
}

// because state of toolbars could be altered via standard QMainWindow context menu,
// we have to implement a special enabler for affected items
void MainWindow::toolbarsEnabler()
{
    ui->actionMain_Toolbar->setChecked(ui->mainToolBar->isVisible());
    ui->actionViews_Toolbar->setChecked(ui->viewsToolBar->isVisible());
    ui->actionMouse_Toolbar->setChecked(ui->mouseToolBar->isVisible());
    ui->actionVisibility_Toolbar->setChecked(ui->visibilityToolBar->isVisible());
    ui->actionPanels_Toolbar->setChecked(ui->panelsToolBar->isVisible());
}

void MainWindow::sigModeChanged( scene::CAppMode::tMode mode )
{
    ui->actionDensity_Window_Adjusting->setChecked(scene::CAppMode::MODE_DENSITY_WINDOW==mode);
    ui->actionTrackball_Mode->setChecked(scene::CAppMode::MODE_TRACKBALL==mode);
    ui->actionObject_Manipulation->setChecked(scene::CAppMode::MODE_SLICE_MOVE==mode);
    ui->actionScale_Scene->setChecked(scene::CAppMode::COMMAND_SCENE_ZOOM==mode);
    ui->actionMeasure_Density_Value->setChecked(scene::CAppMode::COMMAND_DENSITY_MEASURE==mode);
    ui->actionMeasure_Distance->setChecked(scene::CAppMode::COMMAND_DISTANCE_MEASURE==mode);
}

void MainWindow::actionsEnabler()
{
    // Views
    if (NULL!=m_3DView && NULL!=m_3DView->parentWidget())
        ui->action3D_View->setChecked(m_3DView->parentWidget()->isVisible());
    if (NULL!=m_OrthoXYSlice && NULL!=m_OrthoXYSlice->parentWidget())
        ui->actionAxial_View->setChecked(m_OrthoXYSlice->parentWidget()->isVisible());
    if (NULL!=m_OrthoXZSlice && NULL!=m_OrthoXZSlice->parentWidget())
        ui->actionCoronal_View->setChecked(m_OrthoXZSlice->parentWidget()->isVisible());
    if (NULL!=m_OrthoYZSlice && NULL!=m_OrthoYZSlice->parentWidget())
        ui->actionSagittal_View->setChecked(m_OrthoYZSlice->parentWidget()->isVisible());

    // Panels
    if (NULL!=m_densityWindowPanel && NULL!=m_densityWindowPanel->parentWidget())
        ui->actionDensity_Window->setChecked(m_densityWindowPanel->parentWidget()->isVisible());
    if (NULL!=m_orthoSlicesPanel && NULL!=m_orthoSlicesPanel->parentWidget())
        ui->actionOrtho_Slices_Panel->setChecked(m_orthoSlicesPanel->parentWidget()->isVisible());
    if (NULL!=m_segmentationPanel && NULL!=m_segmentationPanel->parentWidget())
        ui->actionSegmentation_Panel->setChecked(m_segmentationPanel->parentWidget()->isVisible());
    if (NULL!=m_volumeRenderingPanel && NULL!=m_volumeRenderingPanel->parentWidget())
        ui->actionVolume_Rendering_Panel->setChecked(m_volumeRenderingPanel->parentWidget()->isVisible());
	if (NULL!=m_modelsPanel && NULL!=m_modelsPanel->parentWidget())
        ui->actionModels_List_Panel->setChecked(m_modelsPanel->parentWidget()->isVisible());
}

void   MainWindow::undoRedoEnabler()
{
    // get undo manager
    data::CObjectPtr< data::CUndoManager > ptrManager( APP_STORAGE.getEntry(data::Storage::UndoManager::Id) );
//    int nSteps=ptrManager->getUndoSteps();
    ui->actionUndo->setEnabled(ptrManager->canUndo());
    ui->actionRedo->setEnabled(ptrManager->canRedo());
}

void MainWindow::show3DView(bool bShow)
{
    if (bShow)
    {
        QWidget* pParent=m_3DView->parentWidget();
        pParent->show();
        pParent->raise();
    }
    else
        m_3DView->parentWidget()->hide();
}

void MainWindow::showAxialView(bool bShow)
{
    if (bShow)
    {
        QWidget* pParent=m_OrthoXYSlice->parentWidget();
        pParent->show();
        pParent->raise();
    }
    else
        m_OrthoXYSlice->parentWidget()->hide();
}

void MainWindow::showCoronalView(bool bShow)
{
    if (bShow)
    {
        QWidget* pParent=m_OrthoXZSlice->parentWidget();
        pParent->show();
        pParent->raise();
    }
    else
        m_OrthoXZSlice->parentWidget()->hide();
}

void MainWindow::showSagittalView(bool bShow)
{
    if (bShow)
    {
        QWidget* pParent=m_OrthoYZSlice->parentWidget();
        pParent->show();
        pParent->raise();
    }
    else
        m_OrthoYZSlice->parentWidget()->hide();
}

void MainWindow::showDensityWindowPanel(bool bShow)
{
    Q_ASSERT(NULL!=m_densityWindowPanel);
    if (bShow)
    {
        QWidget* pParent=m_densityWindowPanel->parentWidget();
        pParent->show();
        pParent->raise();
    }
    else
        m_densityWindowPanel->parentWidget()->hide();
}

void MainWindow::showOrthoSlicesPanel(bool bShow)
{
    Q_ASSERT(NULL!=m_orthoSlicesPanel);
    if (bShow)
    {
        QWidget* pParent=m_orthoSlicesPanel->parentWidget();
        pParent->show();
        pParent->raise();
    }
    else
        m_orthoSlicesPanel->parentWidget()->hide();
}

void MainWindow::showSegmentationPanel(bool bShow)
{
    Q_ASSERT(NULL!=m_segmentationPanel);
    if (bShow)
    {
        QWidget* pParent=m_segmentationPanel->parentWidget();
        pParent->show();
        pParent->raise();
    }
    else
        m_segmentationPanel->parentWidget()->hide();
}

void MainWindow::showVRPanel(bool bShow)
{
    Q_ASSERT(NULL!=m_volumeRenderingPanel);
    if (bShow)
    {
        QWidget* pParent=m_volumeRenderingPanel->parentWidget();
        pParent->show();
        pParent->raise();
    }
    else
        m_volumeRenderingPanel->parentWidget()->hide();
}

void MainWindow::showModelsListPanel(bool bShow)
{
    Q_ASSERT(NULL!=m_modelsPanel);
    if (bShow)
    {
        QWidget* pParent=m_modelsPanel->parentWidget();
        pParent->show();
        pParent->raise();
    }
    else
        m_modelsPanel->parentWidget()->hide();
}
///////////////////////////////////////////////////////////////////////////////
// Create OSG scenes

void MainWindow::createOSGStuff()
{
	// 3D view
	{
		m_3DView = new CVolumeRendererWindow();
		m_3DView->hide();
		m_Scene3D = new scene::CScene3D(m_3DView);
		m_Scene3D->setRenderer(&m_3DView->getRenderer());
		m_3DView->setScene(m_Scene3D.get());

		// Initialize the model manager
		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			m_modelVisualizers[i] = new osg::CModelVisualizerEx(data::Storage::ImportedModel::Id + i);

			osg::ref_ptr<osg::MatrixTransform> pModelTransform = new osg::MatrixTransform();
			pModelTransform->addChild(m_modelVisualizers[i]);
    
			m_Scene3D->anchorToScene(pModelTransform.get(), true);
			m_modelVisualizers[i]->setCanvas(m_3DView);
		}
    
		//m_modelVisualizer->setModelVisualization(osg::CModelVisualizerEx::EMV_FLAT);

		if( m_3DView->getRenderer().isError() )
		{
			showMessageBox(QMessageBox::Critical,tr("VR Error!"));
		}

		// Drawing event handler
		m_draw3DEH = new osgGA::CISScene3DEH( m_3DView, m_Scene3D.get() );
		m_3DView->addEventHandler( m_draw3DEH.get() );

		// Window drawing event handler
		m_drawW3DEH = new osgGA::CISWindowEH(m_3DView, m_Scene3D.get());

		// Modify drawing EH osg::StateSet 
		m_drawW3DEH->setLineFlags( osg::CLineGeode::USE_RENDER_BIN | osg::CLineGeode::DISABLE_DEPTH_TEST );
		m_drawW3DEH->setZ( 0.5 );

		m_3DView->addEventHandler( m_drawW3DEH.get() );

		// Measurements EH
		m_measurements3DEH = new scene::CMeasurements3DEH( m_3DView, m_Scene3D.get() );
		m_measurements3DEH->setHandleDensityUnderCursor(true);
		m_3DView->addEventHandler( m_measurements3DEH.get() );
	}

    // XY Slice
	{
		m_OrthoXYSlice = new OSGOrtho2DCanvas();
		m_OrthoXYSlice->hide();
		m_SceneXY = new scene::CSceneXY(m_OrthoXYSlice);
		m_OrthoXYSlice->setScene(m_SceneXY.get());
		m_OrthoXYSlice->centerAndScale();
		m_OrthoXYSlice->getManipulator()->m_sigUpDown.connect( m_SceneXY, &scene::CSceneXY::sliceUpDown );
		// Drawing event handler
		m_drawXYEH = new osgGA::CISSceneXYEH( m_OrthoXYSlice, m_SceneXY.get() );
		m_OrthoXYSlice->addEventHandler( m_drawXYEH.get() );
		// Measurements event handler
		m_measurementsXYEH = new scene::CMeasurementsXYEH( m_OrthoXYSlice, m_SceneXY.get() );
		m_measurementsXYEH->setHandleDensityUnderCursor(true);
		m_OrthoXYSlice->addEventHandler( m_measurementsXYEH.get() );
		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			m_importedModelCutSliceXY[i] = new osg::CModelCutVisualizerSliceXY(data::Storage::ImportedModelCutSliceXY::Id + i, m_OrthoXYSlice);
			m_importedModelCutSliceXY[i]->setVisibility(false);
			m_importedModelCutSliceXY[i]->setColor(osg::Vec4(1.0,1.0,0.0,1.0));
			m_SceneXY->addChild(m_importedModelCutSliceXY[i]);
		}
	}

    // XZ Slice
	{
		m_OrthoXZSlice = new OSGOrtho2DCanvas();
		m_OrthoXZSlice->hide();
		m_SceneXZ = new scene::CSceneXZ(m_OrthoXZSlice);
		m_OrthoXZSlice->setScene(m_SceneXZ.get());
		m_OrthoXZSlice->centerAndScale();
		m_OrthoXZSlice->getManipulator()->m_sigUpDown.connect( m_SceneXZ, &scene::CSceneXZ::sliceUpDown );
		// Drawing event handler
		m_drawXZEH = new osgGA::CISSceneXZEH( m_OrthoXZSlice, m_SceneXZ.get() );
		m_OrthoXZSlice->addEventHandler( m_drawXZEH.get() );
		// Measurements event handler
		m_measurementsXZEH = new scene::CMeasurementsXZEH( m_OrthoXZSlice, m_SceneXZ.get() );
		m_measurementsXZEH->setHandleDensityUnderCursor(true);
		m_OrthoXZSlice->addEventHandler( m_measurementsXZEH.get() );
		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			m_importedModelCutSliceXZ[i] = new osg::CModelCutVisualizerSliceXZ(data::Storage::ImportedModelCutSliceXZ::Id + i, m_OrthoXZSlice);
			m_importedModelCutSliceXZ[i]->setVisibility(false);
			m_importedModelCutSliceXZ[i]->setColor(osg::Vec4(1.0,1.0,0.0,1.0));
			m_SceneXZ->addChild(m_importedModelCutSliceXZ[i]);
		}
	}

    // YZ Slice
	{
		m_OrthoYZSlice = new OSGOrtho2DCanvas();
		m_OrthoYZSlice->hide();
		m_SceneYZ = new scene::CSceneYZ(m_OrthoYZSlice);
		m_OrthoYZSlice->setScene(m_SceneYZ.get());
		m_OrthoYZSlice->centerAndScale();
		m_OrthoYZSlice->getManipulator()->m_sigUpDown.connect( m_SceneYZ, &scene::CSceneYZ::sliceUpDown );
		// Drawing event handler
		m_drawYZEH = new osgGA::CISSceneYZEH( m_OrthoYZSlice, m_SceneYZ.get() );
		m_OrthoYZSlice->addEventHandler( m_drawYZEH.get() );
		// Measurements event handler
		m_measurementsYZEH = new scene::CMeasurementsYZEH( m_OrthoYZSlice, m_SceneYZ.get() );
		m_measurementsYZEH->setHandleDensityUnderCursor(true);
		m_OrthoYZSlice->addEventHandler( m_measurementsYZEH.get() );
		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			m_importedModelCutSliceYZ[i] = new osg::CModelCutVisualizerSliceYZ(data::Storage::ImportedModelCutSliceYZ::Id + i, m_OrthoYZSlice);
			m_importedModelCutSliceYZ[i]->setVisibility(false);
			m_importedModelCutSliceYZ[i]->setColor(osg::Vec4(1.0,1.0,0.0,1.0));
			m_SceneYZ->addChild(m_importedModelCutSliceYZ[i]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::setupDockWindows()
{
    m_wnd3DView.setWindowTitle(tr("3D"));    
    m_wnd3DView.setAllowedAreas(Qt::NoDockWidgetArea);
    m_wnd3DView.setFeatures(0);
    m_wnd3DView.setObjectName("3D View");
    m_wnd3DView.setMinimumSize(200,150);
    m_wnd3DView.setWidget(m_3DView);
    m_wnd3DView.setProperty("Icon",":/icons/icon3d.png");    

    m_wndXYView.setWindowTitle(tr("Axial / XY"));
    m_wndXYView.setAllowedAreas(Qt::NoDockWidgetArea);
    m_wndXYView.setFeatures(0);
    m_wndXYView.setObjectName("Axial View");
    m_wndXYView.setMinimumSize(200,150);
    m_wndXYView.setWidget(m_OrthoXYSlice);
    m_wndXYView.setProperty("Icon",":/icons/iconxy.png");

    m_wndXZView.setWindowTitle(tr("Coronal / XZ"));
    m_wndXZView.setAllowedAreas(Qt::NoDockWidgetArea);
    m_wndXZView.setFeatures(0);
    m_wndXZView.setObjectName("Coronal View");
    m_wndXZView.setMinimumSize(200,150);
    m_wndXZView.setWidget(m_OrthoXZSlice);
    m_wndXZView.setProperty("Icon",":/icons/iconxz.png");

    m_wndYZView.setWindowTitle(tr("Sagittal / YZ"));
    m_wndYZView.setAllowedAreas(Qt::NoDockWidgetArea);
    m_wndYZView.setFeatures(0);
    m_wndYZView.setObjectName("Sagittal View");
    m_wndYZView.setMinimumSize(200,150);
    m_wndYZView.setWidget(m_OrthoYZSlice);
    m_wndYZView.setProperty("Icon",":/icons/iconyz.png");

    connect(&m_wnd3DView, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
    connect(&m_wndXYView, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
    connect(&m_wndXZView, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
    connect(&m_wndYZView, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));

    // connect before docking
    connect(&m_wnd3DView,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea))); 
    connect(&m_wndXYView,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
    connect(&m_wndXZView,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
    connect(&m_wndYZView,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
}

void MainWindow::dockWidgetVisiblityChanged(bool visible)
{
    if (visible)
        updateTabIcons();
    actionsEnabler();
}

void MainWindow::dockLocationChanged ( Qt::DockWidgetArea area )
{
    updateTabIcons();
}

///////////////////////////////////////////////////////////////////////////////
// misc. actions

#ifdef WIN32 // Windows need nonunicode paths to be in ACP
std::string MainWindow::wcs2ACP(const std::wstring &filename)
{
    std::string convFilename;
    {
        // get buffer size
        int size=WideCharToMultiByte(CP_ACP,0,filename.c_str(),filename.size(),0,0,0,0);
        if (size>0)
        {
            // convert
            char* buffer=new char[size+1];
            int sizeC=WideCharToMultiByte(CP_ACP,0,filename.c_str(),filename.size(),buffer,size,0,0);
            if (sizeC>0)
            {
                assert(sizeC<size+1);
                buffer[sizeC]=0;
                convFilename=buffer;
            }
            delete buffer;
        }
    }
    return convFilename;
}
#endif

void MainWindow::mouseModeDensityWindow(bool)
{
    APP_MODE.setAndStore(scene::CAppMode::MODE_DENSITY_WINDOW);
}

void MainWindow::mouseModeTrackball(bool)
{
    APP_MODE.setAndStore(scene::CAppMode::MODE_TRACKBALL);
}

void MainWindow::mouseModeObjectManipulation(bool)
{
    APP_MODE.setAndStore(scene::CAppMode::MODE_SLICE_MOVE);
}

void MainWindow::mouseModeZoom(bool)
{
    APP_MODE.set(scene::CAppMode::COMMAND_SCENE_ZOOM);
}

void MainWindow::performUndo()
{
    try {
         // Get the undo manager
         data::CObjectPtr< data::CUndoManager > ptrManager( APP_STORAGE.getEntry(data::Storage::UndoManager::Id) );

         // to find out whether any operation can be undone use the line below
         // event.Enable( ptrManager->canUndo() );

         // undo
         ptrManager->undo();
    }
    catch( ... )
    {
         // does nothing
    }
}

void MainWindow::performRedo()
{
    try {
         // Get the undo manager
         data::CObjectPtr< data::CUndoManager > ptrManager( APP_STORAGE.getEntry(data::Storage::UndoManager::Id) );

         // redo
         ptrManager->redo();
    }
    catch( ... )
    {
         // does nothing
    }
}

void MainWindow::showMergedVR(bool bShow)
{
    PSVR::PSVolumeRendering &renderer = m_3DView->getRenderer();
    renderer.enable(bShow);
}

void MainWindow::showAxialSlice(bool bShow)
{
    VPL_SIGNAL(SigSetPlaneXYVisibility).invoke(bShow);
    m_3DView->Refresh(false);
}

void MainWindow::showCoronalSlice(bool bShow)
{
    VPL_SIGNAL(SigSetPlaneXZVisibility).invoke(bShow);
    m_3DView->Refresh(false);
}

void MainWindow::showSagittalSlice(bool bShow)
{
    VPL_SIGNAL(SigSetPlaneYZVisibility).invoke(bShow);
    m_3DView->Refresh(false);
}

void MainWindow::showSurfaceModel(bool bShow)
{
    for(size_t i = 0; i < MAX_IMPORTED_MODELS; ++i)
        VPL_SIGNAL(SigSetModelVisibility).invoke(data::Storage::ImportedModel::Id + i, bShow);

    m_3DView->Refresh(false);
}

void MainWindow::createSurfaceModel()
{
    // Find unused id
	int id(findPossibleModelId());
	if(id < 0)
		return;

    CProgress progress(this);
    progress.setLabelText(tr("Creating surface model, please wait..."));
    progress.show();

    {   // if there is already an existing model which is large, free it to save memory
        data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(id) );
        geometry::CMesh* pMesh=spModel->getMesh();
        if (NULL!=pMesh && pMesh->n_vertices() > 1000000)
        {
            spModel->setMesh(new geometry::CMesh());
			spModel->clearAllProperties();
            APP_STORAGE.invalidate( spModel.getEntryPtr() );
        }
    }

    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(m_Examination.getActiveDataSet()) );

    vpl::img::tDensityPixel Low = 500;
    vpl::img::tDensityPixel Hi = 2000;
    if (NULL!=m_segmentationPanel)
    {
        Low = m_segmentationPanel->getLo();
        Hi = m_segmentationPanel->getHi();
    }

    geometry::CMesh* pMesh = new geometry::CMesh;

    CMarchingCubes mc;
    mc.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(&progress, &CProgress::Entry));
    vpl::img::CSize3d voxelSize = vpl::img::CSize3d(spVolume->getDX(), spVolume->getDY(), spVolume->getDZ());
    CThresholdFunctor<vpl::img::CDensityVolume, vpl::img::tDensityPixel> ThresholdFunc(Low, Hi, &(*spVolume), voxelSize);

    if( !mc.generateMesh(*pMesh, &ThresholdFunc, true) )
    {
        delete pMesh;
        return;
    }

    spVolume.release();

    progress.restart(tr("Reducing small areas, please wait..."));

    CSmallSubmeshReducer rc;
    rc.reduce(*pMesh, 500);

    progress.restart(tr("Smoothing model, please wait..."));

    // Smooth model
    CSmoothing sm;
    sm.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(&progress, &CProgress::Entry));
//    double p1 = 0.6073;
//    double p2 = 0.1;
    if (!sm.Smooth(*pMesh, 10))
    {
        delete pMesh;
        return;
    }

    progress.restart(tr("Decimating model, please wait..."));

    // Decimate model
    CDecimator dc;
    dc.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(&progress, &CProgress::Entry));
    int output_mesh_size_tri = ( pMesh->n_faces() * 20 ) / 100;
    int output_mesh_size_vert = 1;
    if( !dc.Reduce( *pMesh, output_mesh_size_vert, output_mesh_size_tri) )
    {
        delete pMesh;
        return;
    }

    data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(id) );
    spModel->setMesh(pMesh);
	spModel->setLabel(m_modelLabel);
	spModel->setColor(m_modelColor);
	spModel->clearAllProperties();
	spModel->setProperty("Created","1");
	spModel->setTransformationMatrix(osg::Matrix::identity());
    spModel->setVisibility(true);
    APP_STORAGE.invalidate(spModel.getEntryPtr());

	// if model-region linking is not enabled, hide previously created models on creation of a new one
	QSettings settings;
	bool bModelsLinked = settings.value("ModelRegionLinkEnabled", QVariant(false)).toBool();
	if (!bModelsLinked)
	{
		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			if (data::Storage::ImportedModel::Id + i == id) continue;
			data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(data::Storage::ImportedModel::Id + i) );
			if (spModel->isShown())
			{
				std::string created = spModel->getProperty("Created");
				if (!created.empty())
					VPL_SIGNAL(SigSetModelVisibility).invoke(data::Storage::ImportedModel::Id + i, false);
			}
		}
	}

    bool bAnyVisible(false);
    for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        if(VPL_SIGNAL(SigGetModelVisibility).invoke2(data::Storage::ImportedModel::Id + i))
        {
            bAnyVisible = true;
            break;
        }

    ui->actionShow_Surface_Model->setChecked(bAnyVisible);
}

///////////////////////////////////////////////////////////////////////////////
// Preferences, Properties, Help

void MainWindow::showPreferencesDialog()
{	
    CPreferencesDialog dlg(m_localeDir, ui->menuBar, this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    if (QDialog::Accepted==dlg.exec())
    {
        if (dlg.colorsChanged())
        {
            QSettings savedSettings;
            QColor color=savedSettings.value("BGColor",DEFAULT_BACKGROUND_COLOR).toUInt();
            osg::Vec4 osgColor(color.redF(), color.greenF(), color.blueF(), 1.0f);

            data::CObjectPtr<data::CAppSettings> appSettings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
            appSettings->setClearColor(osgColor);
            APP_STORAGE.invalidate(appSettings.getEntryPtr());

            m_3DView->setBackgroundColor(osgColor);
            m_OrthoXYSlice->setBackgroundColor(osgColor);
            m_OrthoXZSlice->setBackgroundColor(osgColor);
            m_OrthoYZSlice->setBackgroundColor(osgColor);
        }
		if (dlg.shortcutsChanged())
		{
			saveShortcuts();
		}
        if (dlg.needsRestart())
        {
            showMessageBox(QMessageBox::Information,tr("You must restart the application to apply the changes."));
        }
        //retranslateUi();        
    }
}

void MainWindow::showDataProperties()
{
    CDataInfoDialog dlg(this);
    dlg.exec();    
}

void MainWindow::showAboutPlugins()
{
    CPluginInfoDialog dialog(m_pPlugins, this);
    dialog.exec();
}

void MainWindow::showAbout()
{
    app::CProductInfo info=app::getProductInfo();
    QString product=QString("%1 %2.%3.%4 %5").arg(QCoreApplication::applicationName()).arg(info.getVersion().getMajorNum()).arg(info.getVersion().getMinorNum()).arg(info.getVersion().getBuildNum()).arg(QString::fromStdString(info.getNote()));
    QMessageBox msgBox(this);
    msgBox.setIconPixmap(QPixmap(":/icons/3dim.ico"));
    msgBox.setWindowTitle(tr("About ") + QCoreApplication::applicationName());
    msgBox.setText(product + "\n"
                   + tr("Lightweight 3D DICOM viewer.\n\n")
                   + QObject::trUtf8("Copyright %1 2008-%2 by 3Dim Laboratory s.r.o.").arg(QChar(169)).arg(QDate::currentDate().year()) + "\n" 
                   + tr("http://www.3dim-laboratory.cz/")
                   );
    msgBox.setDetailedText(tr(
        "Licensed under the Apache License, Version 2.0 (the \"License\");\n"
        "you may not use this file except in compliance with the License.\n"
        "You may obtain a copy of the License at\n"
        "\n"
        "  http://www.apache.org/licenses/LICENSE-2.0\n"
        "\n"
        "Unless required by applicable law or agreed to in writing, software\n"
        "distributed under the License is distributed on an \"AS IS\" BASIS,\n"
        "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n"
        "See the License for the specific language governing permissions and\n"
        "limitations under the License."
        ));
    msgBox.exec();
}

void MainWindow::loadHelp()
{
    if (!m_helpView)
		return;
    // is anything loaded?
    QUrl currentUrl=m_helpView->url();
    QString urlPath=currentUrl.path();
    if (!urlPath.isEmpty()) return;
    //
    QSettings settings;
    QString lngFile=settings.value("Language","").toString();
    QString path;
    QString basePath=qApp->applicationDirPath();
    if (!QFileInfo(basePath+"/doc").exists())
    {
        QDir dir(basePath);
        dir.cdUp();
        basePath=dir.absolutePath();
    }

    if (!lngFile.isEmpty())
    {
        path=basePath+"/doc/help_"+QLocale::system().name()+".html";
        QFileInfo pathInfo( path );
        if (!pathInfo.exists())
            path=basePath+"/doc/help.html";
    }
    else
        path=basePath+"/doc/help.html";
    QUrl helpUrl(path);
    helpUrl.setScheme("file");
    m_helpView->load(helpUrl);
}

void MainWindow::showHelp()
{
    Q_ASSERT(m_helpView);

    m_helpView->parentWidget()->show();
    m_helpView->parentWidget()->raise();
}

///////////////////////////////////////////////////////////////////////////////
// Filters

void MainWindow::mixVolumes(vpl::img::CDensityVolume* main, vpl::img::CDensityVolume* temp, int mixing) const // 0-100
{
	if (NULL == main || NULL == temp || main == temp) return;
	const vpl::tSize xSize = main->getXSize();
	const vpl::tSize ySize = main->getYSize();
	const vpl::tSize zSize = main->getZSize();
#pragma omp parallel for
	for(int z = 0; z < zSize; z++)
	{
		for(int y = 0; y < ySize; y++)
		{
			for(int x = 0; x < xSize; x++)
			{
				main->at(x,y,z) = (main->at(x,y,z)*(100-mixing) + temp->at(x,y,z)*mixing)/100;
			}
		}
	}
}

// Slice filtering method for CFilterDialog
void gaussianSliceFilter(int slice, int strength, QImage& result)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	data::CObjectPtr<data::CActiveDataSet> spDataSet( APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id) );
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(spDataSet->getId()) );
	const vpl::tSize xSize=spVolume->getXSize();
	const vpl::tSize ySize=spVolume->getYSize();
	const vpl::tSize zSize=spVolume->getZSize();
	
	QImage img(xSize,ySize, QImage::Format_Indexed8);
	QVector<QRgb> my_table;
	for(int i = 0; i < 256; i++) 
		my_table.push_back(qRgb(i,i,i));
	img.setColorTable(my_table);

	int minDensity = data::CDensityWindow::getMinDensity();
	int maxDensity = data::CDensityWindow::getMaxDensity();
	{
#pragma omp parallel for
		for(int y=0;y<ySize;y++)
		{
			vpl::img::CVolumeGauss3Filter<vpl::img::CDensityVolume> Filter; // getResponse can't run in parallel, therefore instance for each thread
			uchar* scanLine = const_cast<uchar*>(img.constScanLine(y));		// to avoid deep copy
			for(int x=0;x<xSize;x++)
			{
				int val = Filter.getResponse(*spVolume,x,y,slice); 
				if (strength<100)
				{
					int oval = spVolume->at(x,y,slice);
					val = (val * strength + (100-strength) * oval)/100;
				}
				scanLine[x] = std::min(255,std::max(0,(int)val-minDensity)/(std::max(1,maxDensity-minDensity)/255));
			}
		}
	}
	result = img;

	QApplication::restoreOverrideCursor();
}


void MainWindow::filterGaussian()
{
	CFilterDialog dlg(this,tr("Gaussian Filter"));
	dlg.setFilter(&gaussianSliceFilter);
	if (QDialog::Accepted==dlg.exec())
	{
	// Show simple progress dialog
		CProgress progress(this);
		progress.setLabelText(tr("Filtering volumetric data, please wait..."));
		progress.show();

		// Input and output data
		data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(m_Examination.getActiveDataSet()) );
		vpl::img::CDensityVolume::tSmartPtr spFiltered = new vpl::img::CDensityVolume(*spVolume);

		// Median filtering
		vpl::img::CVolumeGauss3Filter<vpl::img::CDensityVolume> Filter;
		Filter.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(&progress, &CProgress::Entry));
		// initialize kernel (before multiple threads kick in)
		float tmpPixel = float(Filter.getResponse(*spVolume.get(), 0, 0, 0));
		spFiltered->at(0, 0, 0) = tmpPixel;
		bool bResult = Filter(*spVolume, *spFiltered);

		// Destroy the progress dialog
		progress.hide();

		// Show the result
		if( bResult )
		{
			// Create reference to the new data
			//spVolume->makeRef(*spFiltered);
			mixVolumes(spVolume.get(),spFiltered.get(),dlg.getStrength());
		}
		else
		{
			showMessageBox(QMessageBox::Critical,tr("Filtering aborted!"));
		}

		// Update the data
		APP_STORAGE.invalidate(spVolume.getEntryPtr());
	}
}

// Slice filtering method for CFilterDialog
void anisotropicSliceFilter(int slice, int strength, QImage& result)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	static const double dKappa = 150.0;
	static const int iNumOfIters = 5.0;

	data::CObjectPtr<data::CActiveDataSet> spDataSet( APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id) );
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(spDataSet->getId()) );
	const vpl::tSize xSize=spVolume->getXSize();
	const vpl::tSize ySize=spVolume->getYSize();
	const vpl::tSize zSize=spVolume->getZSize();
	
	QImage img(xSize,ySize, QImage::Format_Indexed8);
	QVector<QRgb> my_table;
	for(int i = 0; i < 256; i++) 
		my_table.push_back(qRgb(i,i,i));
	img.setColorTable(my_table);

	int minDensity = data::CDensityWindow::getMinDensity();
	int maxDensity = data::CDensityWindow::getMaxDensity();

    vpl::img::CAnisotropicFilter<vpl::img::CDImage> Filter2D(dKappa, iNumOfIters);
	vpl::img::CDImage src(xSize,ySize,5);
	src.fillEntire(0);
	for(int y=0;y<ySize;y++)
		for(int x=0;x<xSize;x++)
			src.at(x,y) = spVolume->at(x,y,slice);
	
    vpl::img::CDImage Filtered(src);
    src.mirrorMargin();
    Filter2D(src, Filtered);

	{
#pragma omp parallel for
		for(int y=0;y<ySize;y++)
		{
			uchar* scanLine = const_cast<uchar*>(img.constScanLine(y));		// to avoid deep copy
			for(int x=0;x<xSize;x++)
			{
				int val = Filtered.at(x,y);
				if (strength<100)
				{
					int oval = spVolume->at(x,y,slice);
					val = (val * strength + (100-strength) * oval)/100;
				}
				scanLine[x] = std::min(255,std::max(0,(int)val-minDensity)/(std::max(1,maxDensity-minDensity)/255));
			}
		}
	}
	result = img;

	QApplication::restoreOverrideCursor();
}


void MainWindow::filterAnisotropic()
{
	CFilterDialog dlg(this,tr("Anisotropic Filter"));
	dlg.setFilter(&anisotropicSliceFilter);
	if (QDialog::Accepted==dlg.exec())
	{
		static const double dKappa = 150.0;
		static const int iNumOfIters = 5.0;

		// Show simple progress dialog
		CProgress progress(this);
		progress.setLabelText(tr("Filtering volumetric data, please wait..."));
		progress.show();

		// Input and output data
		data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(m_Examination.getActiveDataSet()) );
		vpl::img::CDensityVolume::tSmartPtr spFiltered = new vpl::img::CDensityVolume(*spVolume);

		// Anisotropic filtering
		vpl::img::CVolumeAnisotropicFilter<vpl::img::CDensityVolume> Filter(dKappa, iNumOfIters);
		Filter.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(&progress, &CProgress::Entry));
		bool bResult = Filter(*spVolume, *spFiltered);

		// Destroy the progress dialog
		progress.hide();

		// Show the result
		if( bResult )
		{
			//spVolume->makeRef(*spFiltered);
			mixVolumes(spVolume.get(),spFiltered.get(),dlg.getStrength());
		}
		else
		{
			showMessageBox(QMessageBox::Critical,tr("Filtering aborted!"));
		}
		// Update the data
		APP_STORAGE.invalidate(spVolume.getEntryPtr());
	}
}

// Slice filtering method for CFilterDialog
void medianSliceFilter(int slice, int strength, QImage& result)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	data::CObjectPtr<data::CActiveDataSet> spDataSet( APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id) );
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(spDataSet->getId()) );
	const vpl::tSize xSize=spVolume->getXSize();
	const vpl::tSize ySize=spVolume->getYSize();
	const vpl::tSize zSize=spVolume->getZSize();
	
	QImage img(xSize,ySize, QImage::Format_Indexed8);
	QVector<QRgb> my_table;
	for(int i = 0; i < 256; i++) 
		my_table.push_back(qRgb(i,i,i));
	img.setColorTable(my_table);

	int minDensity = data::CDensityWindow::getMinDensity();
	int maxDensity = data::CDensityWindow::getMaxDensity();
	{
#pragma omp parallel for
		for(int y=0;y<ySize;y++)
		{
			// Median filtering
			vpl::img::CVolumeMedianFilter<vpl::img::CDensityVolume> Filter(5); // getResponse can't run in parallel, therefore instance for each thread
			uchar* scanLine = const_cast<uchar*>(img.constScanLine(y));		// to avoid deep copy
			for(int x=0;x<xSize;x++)
			{
				int val = Filter.getResponse(*spVolume,x,y,slice); 
				if (strength<100)
				{
					int oval = spVolume->at(x,y,slice);
					val = (val * strength + (100-strength) * oval)/100;
				}
				scanLine[x] = std::min(255,std::max(0,(int)val-minDensity)/(std::max(1,maxDensity-minDensity)/255));
			}
		}
	}
	result = img;

	QApplication::restoreOverrideCursor();
}

void MainWindow::filterMedian()
{
	CFilterDialog dlg(this, tr("Median Filter"));
	dlg.setFilter(&medianSliceFilter);
	if (QDialog::Accepted==dlg.exec())
	{
	// Show simple progress dialog
		CProgress progress(this);
		progress.setLabelText(tr("Filtering volumetric data, please wait..."));
		progress.show();

		// Input and output data
		data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(m_Examination.getActiveDataSet()) );
		vpl::img::CDensityVolume::tSmartPtr spFiltered = new vpl::img::CDensityVolume(*spVolume);

		// Median filtering
		vpl::img::CVolumeMedianFilter<vpl::img::CDensityVolume> Filter(3);
		Filter.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(&progress, &CProgress::Entry));
		bool bResult = Filter(*spVolume, *spFiltered);

		// Destroy the progress dialog
		progress.hide();

		// Show the result
		if( bResult )
		{
			// Create reference to the new data
			//spVolume->makeRef(*spFiltered);
			mixVolumes(spVolume.get(),spFiltered.get(),dlg.getStrength());
		}
		else
		{
			showMessageBox(QMessageBox::Critical,tr("Filtering aborted!"));
		}

		// Update the data
		APP_STORAGE.invalidate(spVolume.getEntryPtr());
	}
}


// Slice filtering method for CFilterDialog
void sharpenSliceFilter(int slice, int strength, QImage& result)
{
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	data::CObjectPtr<data::CActiveDataSet> spDataSet( APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id) );
    data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(spDataSet->getId()) );
	const vpl::tSize xSize=spVolume->getXSize();
	const vpl::tSize ySize=spVolume->getYSize();
	const vpl::tSize zSize=spVolume->getZSize();
	
	QImage img(xSize,ySize, QImage::Format_Indexed8);
	QVector<QRgb> my_table;
	for(int i = 0; i < 256; i++) 
		my_table.push_back(qRgb(i,i,i));
	img.setColorTable(my_table);

	int minDensity = data::CDensityWindow::getMinDensity();
	int maxDensity = data::CDensityWindow::getMaxDensity();
	{
#pragma omp parallel for
		for(int y=0;y<ySize;y++)
		{
			vpl::img::CVolumeGauss3Filter<vpl::img::CDensityVolume> Filter; // getResponse can't run in parallel, therefore instance for each thread
			uchar* scanLine = const_cast<uchar*>(img.constScanLine(y));		// to avoid deep copy
			for(int x=0;x<xSize;x++)
			{
				int oval = spVolume->at(x,y,slice);
				int val = Filter.getResponse(*spVolume,x,y,slice); 
				val = oval + (oval-val)*3;
				if (abs(val-oval)<10)
					val = oval;
				val = (val * strength + (100-strength) * oval)/100;
				scanLine[x] = std::min(255,std::max(0,(int)val-minDensity)/(std::max(1,maxDensity-minDensity)/255));
			}
		}
	}
	result = img;

	QApplication::restoreOverrideCursor();
}


void MainWindow::filterSharpen()
{
	CFilterDialog dlg(this,tr("Sharpen Filter"));
	dlg.setFilter(&sharpenSliceFilter);
	if (QDialog::Accepted==dlg.exec())
	{
	// Show simple progress dialog
		CProgress progress(this);
		progress.setLabelText(tr("Filtering volumetric data, please wait..."));
		progress.show();

		// Input and output data
		data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(m_Examination.getActiveDataSet()) );
		vpl::img::CDensityVolume::tSmartPtr spFiltered = new vpl::img::CDensityVolume(*spVolume);

		// Median filtering
		vpl::img::CVolumeGauss3Filter<vpl::img::CDensityVolume> Filter;
		Filter.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(&progress, &CProgress::Entry));
		// initialize kernel (before multiple threads kick in)
		float tmpPixel = float(Filter.getResponse(*spVolume.get(), 0, 0, 0));
		spFiltered->at(0, 0, 0) = tmpPixel;
		bool bResult = Filter(*spVolume, *spFiltered);

		// Destroy the progress dialog
		progress.hide();

		// Show the result
		if( bResult )
		{
			const vpl::tSize xSize=spVolume->getXSize();
			const vpl::tSize ySize=spVolume->getYSize();
			const vpl::tSize zSize=spVolume->getZSize();

			int minDensity = data::CDensityWindow::getMinDensity();
			int maxDensity = data::CDensityWindow::getMaxDensity();
			int strength = dlg.getStrength();
		#pragma omp parallel for
			for(int z=0;z<zSize;z++)
			{
				for(int y=0;y<ySize;y++)
				{
					vpl::img::CVolumeGauss3Filter<vpl::img::CDensityVolume> Filter; // getResponse can't run in parallel, therefore instance for each thread
					for(int x=0;x<xSize;x++)
					{
						int oval = spVolume->at(x,y,z);
						int val = spFiltered->at(x,y,z);
						val = oval + (oval-val)*3;
						if (abs(val-oval)<10)
							val = oval;
						val = (val * strength + (100-strength) * oval)/100;
						spVolume->at(x,y,z) = std::min(maxDensity, std::max(minDensity, val));
					}
				}
			}
		}
		else
		{
			showMessageBox(QMessageBox::Critical,tr("Filtering aborted!"));
		}

		// Update the data
		APP_STORAGE.invalidate(spVolume.getEntryPtr());
	}
}

///////////////////////////////////////////////////////////////////////////////
// Plugins

QObject* MainWindow::findPluginByID(QString sPluginName)
{
	if (NULL!=m_pPlugins)	
		return m_pPlugins->findPluginByID(sPluginName);
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Splitter hack

// synchronize splitter position for both horizontal splitters
void  MainWindow::topSplitterMoved( int pos, int index )
{
    if (!m_realCentralWidget) return;
    QSplitter* vSplitter = qobject_cast<QSplitter*>(m_realCentralWidget);
    if (vSplitter)
    {
        CRapedSplitter* tSplitter = qobject_cast<CRapedSplitter*>(vSplitter->widget(0));
        if (tSplitter)
        {
            if (tSplitter->ignoreMove()) return;
            tSplitter->setIgnoreMove(true);
        }
        CRapedSplitter* bSplitter = qobject_cast<CRapedSplitter*>(vSplitter->widget(1));
        if (bSplitter)
            bSplitter->moveSplitter(pos,index);
        if (tSplitter)
            tSplitter->setIgnoreMove(false);
    }
}

// synchronize splitter position for both horizontal splitters
void  MainWindow::bottomSplitterMoved( int pos, int index )
{
    if (!m_realCentralWidget) return;
    QSplitter* vSplitter = qobject_cast<QSplitter*>(m_realCentralWidget);
    if (vSplitter)
    {
        CRapedSplitter* bSplitter = qobject_cast<CRapedSplitter*>(vSplitter->widget(1));
        if (bSplitter)
        {
            if (bSplitter->ignoreMove()) return;
            bSplitter->setIgnoreMove(true);
        }
        CRapedSplitter* tSplitter = qobject_cast<CRapedSplitter*>(vSplitter->widget(0));
        if (tSplitter)
            tSplitter->moveSplitter(pos,index);
        if (bSplitter)
            bSplitter->setIgnoreMove(false);    }
}

///////////////////////////////////////////////////////////////////////////////
// misc. actions

void MainWindow::showInformationWidgets(bool bShow)
{
    // Get scene widgets from the storage
    data::CObjectPtr<data::CSceneWidgetParameters> spWidgets( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );
    spWidgets->show( bShow ); // !spWidgets->widgetsVisible()

    APP_STORAGE.invalidate( spWidgets.getEntryPtr() );
}

void MainWindow::print()
{
    QPrinter printer;
    printer.setFullPage (TRUE);
    QPrintDialog printDialog(&printer, this);
    if (printDialog.exec() == QDialog::Accepted)
    {
        //qApp->processEvents ();
        //glFinish ();
        //qApp->processEvents ();

        QPainter painter (&printer);

        QGLWidget* myWidget=m_3DView;
        double xscale = printer.pageRect().width()/double(myWidget->width());
        double yscale = printer.pageRect().height()/double(myWidget->height());
        double scale = qMin(xscale, yscale);

        QRect destRect(printer.pageRect().x(),
                        printer.pageRect().y(),
                        scale*myWidget->width(),
                        scale*myWidget->height());

        QImage fb=myWidget->grabFrameBuffer();
        painter.drawImage(destRect,fb);
    }
}

// show panel of dataexpress plugin only
void MainWindow::sendDataExpressData()
{
    QObject* pPlugin=findPluginByID("DataExpress");
    if (NULL!=pPlugin)
    {
        PluginInterface *iPlugin = qobject_cast<PluginInterface *>(pPlugin);
        if (iPlugin)
        {
            QWidget* pPanel=iPlugin->getOrCreatePanel();
            if (pPanel)
            {
                QDockWidget* pParentDock = qobject_cast<QDockWidget*>(pPanel->parentWidget());
                if (pParentDock)
                {
                    pParentDock->show();
                    pParentDock->raise();
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// Measurements menu implementation

void MainWindow::triggerPluginAction(const QString& pluginName, const QString& actionName)
{
    QObject* pPlugin=findPluginByID(pluginName);
    if (NULL!=pPlugin)
    {
        PluginInterface *iPlugin = qobject_cast<PluginInterface *>(pPlugin);
        if (iPlugin)
        {
            QAction* pAct=iPlugin->getAction(actionName);
            if (pAct)
                pAct->trigger();
        }
    }
}

//! Measure Density using Gauge plugin
void MainWindow::measureDensity(bool on)
{
    triggerPluginAction("Gauge","MeasureDensity");
}

//! Measure Distance using Gauge plugin
void MainWindow::measureDistance(bool on)
{
    triggerPluginAction("Gauge","MeasureDistance");
}

//! Clear Measurements using Gauge plugin
void MainWindow::clearMeasurements()
{
    triggerPluginAction("Gauge","ClearMeasurements");
}

void MainWindow::loadLookupTables(QSettings &settings, std::map<std::string, CLookupTable> &luts)
{
    settings.beginGroup(QString("LookupTables"));
    int tableCount = settings.value("TableCount").toInt();
    for (int l = 0; l < tableCount; ++l)
    {
        settings.beginGroup(QString("LookupTable%1").arg(l));
        std::string lutID = settings.value("ID").toString().toStdString();
        
        // find lookup table
        CLookupTable *lut = NULL;
        for (std::map<std::string, CLookupTable>::iterator it = luts.begin(); it != luts.end(); ++it)
        {
            if (it->first == lutID)
            {
                lut = &(it->second);
                break;
            }
        }

        // load table if found
        if (lut != NULL)
        {
            lut->clear();
            lut->setName(settings.value("Name").toString().toStdString());
            int componentCount = settings.value("ComponentCount").toInt();

            for (int c = 0; c < componentCount; ++c)
            {
                lut->addComponent();
                settings.beginGroup(QString("Component%1").arg(c));
                lut->setName(c, settings.value("Name").toString().toStdString());
                lut->setAlphaFactor(c, settings.value("AlphaFactor").toDouble());
                int pointCount = settings.value("PointCount").toInt();
                for (int p = 0; p < pointCount; ++p)
                {
                    settings.beginGroup(QString("Point%1").arg(p));
                    double position = settings.value("Position").toDouble();
                    osg::Vec4 color;
                    color.r() = settings.value("ColorR").toFloat();
                    color.g() = settings.value("ColorG").toFloat();
                    color.b() = settings.value("ColorB").toFloat();
                    color.a() = settings.value("ColorA").toFloat();
                    settings.endGroup(); // point

                    lut->addPoint(c, position, color);
                }
                settings.endGroup(); // component
            }
        }

        settings.endGroup(); // table
    }
    settings.endGroup(); // tables
}

void MainWindow::loadAppSettings()
{
    QSettings settings;
    // load volume rendering settings
    PSVR::PSVolumeRendering &renderer = m_3DView->getRenderer();

    // load VR lookup tables
    loadLookupTables(settings, renderer.getLookupTables());
    QString filename = QApplication::applicationDirPath() + "/lut.ini";
    QSettings fileSettings(filename, QSettings::IniFormat);
    loadLookupTables(settings, renderer.getLookupTables());

    renderer.enable(settings.value("VREnabled",false).toBool());
    renderer.setQuality(settings.value("VRQuality", renderer.getQuality()).toInt());

	int savedShader = settings.value("VRShader", renderer.getShader()).toInt();
	if (PSVR::PSVolumeRendering::CUSTOM!=savedShader) // don't restore custom shader (it is used for segmentation data visualization only)
	{
		renderer.setShader(settings.value("VRShader", renderer.getShader()).toInt());
		renderer.setLut(settings.value("VRLUT", renderer.getLut()).toInt());
	}

    VPL_SIGNAL(SigSetPlaneXYVisibility).invoke(settings.value("ShowXYSlice", false /*VPL_SIGNAL(SigGetPlaneXYVisibility).invoke2()*/).toBool());
    VPL_SIGNAL(SigSetPlaneXZVisibility).invoke(settings.value("ShowZXSlice", false /*VPL_SIGNAL(SigGetPlaneXZVisibility).invoke2()*/).toBool());
    VPL_SIGNAL(SigSetPlaneYZVisibility).invoke(settings.value("ShowYZSlice", false /*VPL_SIGNAL(SigGetPlaneYZVisibility).invoke2()*/).toBool());

    ui->actionVolume_Rendering->setChecked(renderer.isEnabled());
}

void MainWindow::saveLookupTables(QSettings &settings, std::map<std::string, CLookupTable> &luts)
{
    settings.beginGroup(QString("LookupTables"));
    settings.setValue("TableCount", (unsigned int)(luts.size()));
    int l = 0;
    for (std::map<std::string, CLookupTable>::iterator it = luts.begin(); it != luts.end(); ++it)
    {
        settings.beginGroup(QString("LookupTable%1").arg(l));
        settings.setValue("ID", QString(it->first.c_str()));
        settings.setValue("Name", QString(it->second.name().c_str()));
        settings.setValue("ComponentCount", it->second.componentCount());
        for (int c = 0; c < it->second.componentCount(); ++c)
        {
            settings.beginGroup(QString("Component%1").arg(c));
            settings.setValue("Name", it->second.name(c).c_str());
            settings.setValue("AlphaFactor", it->second.alphaFactor(c));
            settings.setValue("PointCount", it->second.pointCount(c));
            for (int p = 0; p < it->second.pointCount(c); ++p)
            {
                settings.beginGroup(QString("Point%1").arg(p));
                settings.setValue("Position", it->second.pointPosition(c, p));
                osg::Vec4 color = it->second.pointColor(c, p);
                settings.setValue("ColorR", color.r());
                settings.setValue("ColorG", color.g());
                settings.setValue("ColorB", color.b());
                settings.setValue("ColorA", color.a());
                settings.endGroup(); // point
            }
            settings.endGroup(); // component
        }
        settings.endGroup(); // table
        l++;
    }
    settings.endGroup(); // tables
}

void MainWindow::saveAppSettings()
{
    QSettings settings;
    // save volume rendering settings
    PSVR::PSVolumeRendering &renderer = m_3DView->getRenderer();

    settings.setValue("VREnabled", renderer.isEnabled());
    settings.setValue("VRQuality", renderer.getQuality());
	if (PSVR::PSVolumeRendering::CUSTOM!=renderer.getShader()) // don't save vr shader info when custom shader is selected
	{
		settings.setValue("VRShader", renderer.getShader());
		settings.setValue("VRLUT", renderer.getLut());
	}
    settings.setValue("ShowXYSlice",VPL_SIGNAL(SigGetPlaneXYVisibility).invoke2());
    settings.setValue("ShowZXSlice",VPL_SIGNAL(SigGetPlaneXZVisibility).invoke2());
    settings.setValue("ShowYZSlice",VPL_SIGNAL(SigGetPlaneYZVisibility).invoke2());

    // save VR lookup tables
    saveLookupTables(settings, renderer.getLookupTables());
}

void MainWindow::firstEvent()
{
	{
		app::CProductInfo info=app::getProductInfo();
		const std::string& versionNote = info.getNote();
		if (!versionNote.empty())
		{
			if (0==strcmp(versionNote.c_str(),app::DEV_NOTE.c_str()))
				setWindowTitle(windowTitle() + " - " + tr("ONLY for internal testing and not for distribution!"));       // "This build is for internal testing purposes only and not for distribution!"
			else
				setWindowTitle(windowTitle() + " - " + tr("ONLY for testing!"));            // "This build is for testing purposes only!"
		}
	}
	// open file from command line
    QString wsVLMName = qApp->property("OpenVLM").toString();
    if (!wsVLMName.isEmpty())
    {
        openVLM(wsVLMName);
    }
    QString wsSTLName = qApp->property("OpenSTL").toString();
    if (!wsSTLName.isEmpty())
    {
        openSTL(wsSTLName);
    }
    QString wsDICOMName = qApp->property("OpenDICOM").toString();
    if (!wsDICOMName.isEmpty())
    {
        if (wsDICOMName.endsWith(".zip",Qt::CaseInsensitive))
            openDICOMZIP(wsDICOMName);
        else
            openDICOM(wsDICOMName,wsDICOMName);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// save screenshot

QImage* osgImageToQImage(const osg::Image* p_Img, bool bUpsideDown)
{
    GLenum format = p_Img->getPixelFormat();
    QImage *im = new QImage(p_Img->s(), p_Img->t(), GL_RGBA==format?QImage::Format_ARGB32: QImage::Format_RGB888);
    uchar* bits = im->bits();
    int glRow=p_Img->getRowSizeInBytes();    
    int qtRow=im->bytesPerLine();
    int m=std::min(glRow,qtRow);
    int h=p_Img->t();
    const uchar* glBits=p_Img->data();
    if (!bUpsideDown)
    {
        for(int y=0;y<h;y++)
            memcpy(bits+y*qtRow,glBits+y*glRow,m);
    }
    else
    {
        for(int y=0;y<h;y++)
            memcpy(bits+y*qtRow,glBits+(h-y-1)*glRow,m);
    }
    return im;
}

QImage* MainWindow::canvasScreenShotToQImage(OSGCanvas* pCanvas, int nRenderingSize, bool bIncludeWidgets)
{
    osg::ref_ptr< osg::Image >  p_Img = new osg::Image;
    pCanvas->screenShot( p_Img.get(), nRenderingSize, bIncludeWidgets );
    //osgDB::writeImageFile(*p_Img, "aaa.png");
    if (!p_Img.get()) return NULL;
    // convert from osg image to qimage
    QImage *im = osgImageToQImage(p_Img.get(),true);
    // set metadata
    QDockWidget* pDW=getParentDockWidget(pCanvas);
    if (NULL!=pDW)
    {
        im->setText("Title",pDW->windowTitle());        
        im->setText("View",pDW->objectName());
    }
    // slice position
    if (m_OrthoXYSlice==pCanvas)
    {
        data::CObjectPtr<data::COrthoSliceXY> spSlice( APP_STORAGE.getEntry(data::Storage::SliceXY::Id) );
        int position=spSlice->getPosition() + 1; // position is zero based
        spSlice.release();
        data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(m_Examination.getActiveDataSet()) );
        int cnt = spVolume->getZSize();
        im->setText("Slice",QString("%1/%2").arg(position).arg(cnt));
    }
    if (m_OrthoXZSlice==pCanvas)
    {
        data::CObjectPtr<data::COrthoSliceXZ> spSlice( APP_STORAGE.getEntry(data::Storage::SliceXZ::Id) );
        int position=spSlice->getPosition() + 1;
        spSlice.release();
        data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(m_Examination.getActiveDataSet()) );
        int cnt = spVolume->getYSize();
        im->setText("Slice",QString("%1/%2").arg(position).arg(cnt));
    }
    if (m_OrthoYZSlice==pCanvas)
    {
        data::CObjectPtr<data::COrthoSliceYZ> spSlice( APP_STORAGE.getEntry(data::Storage::SliceYZ::Id) );
        int position=spSlice->getPosition() + 1;
        spSlice.release();
        data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(m_Examination.getActiveDataSet()) );
        int cnt = spVolume->getXSize();
        im->setText("Slice",QString("%1/%2").arg(position).arg(cnt));
    }
    // application
    im->setText("Author",QCoreApplication::applicationName());
    // patient
    data::CObjectPtr<data::CDensityData> spData( APP_STORAGE.getEntry( data::PATIENT_DATA==data::EDataSet(m_Examination.getActiveDataSet()) ? int(data::Storage::PatientData::Id) : int(data::Storage::AuxData::Id) ) );
    im->setText("Patient",spData->m_sPatientName.c_str());
    // physical resolution
    float sx,sy;
    pCanvas->getDistances(sx,sy);
    if (sx>0 && sy>0)
    {
#ifdef _DEBUG
        double sizeX = im->width()*sx;
        double sizeY = im->height()*sy;
#endif
        // convert from mm/px to px/m
        sx=1000/sx;
        sy=1000/sy;
        // multiply by scale factor
        float factor = nRenderingSize/100.0;
        sx*=factor;
        sy*=factor;
        im->setDotsPerMeterX(sx);
        im->setDotsPerMeterY(sy);
    }
    return im;
}

void  MainWindow::saveScreenshot(OSGCanvas* pCanvas)
{
    QImage* pImage = canvasScreenShotToQImage(pCanvas,100,true);
    if (NULL!=pImage)
    {
        QSettings settings;
#if QT_VERSION < 0x050000
        QString previousDir=settings.value("ScreenShotDir",QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString();
#else
        QString previousDir=settings.value("ScreenShotDir",QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)).toString();
#endif
        // if we have a view name then set it as default name
        QString viewName=pImage->text("View");
        if (!viewName.isEmpty())
            previousDir+="/"+viewName+".jpg";
        // open file selection dialog
        QString baseFileName = QFileDialog::getSaveFileName(this,tr("Please specify an output file..."),previousDir,tr("JPEG Image (*.jpg);;PNG Image (*.png);;BMP Image (*.bmp)"));
        if (!baseFileName.isEmpty())
        {
            QFileInfo baseFI( baseFileName );
            settings.setValue("ScreenShotDir",baseFI.dir().absolutePath());
#define SCREENSHOT_SAVE_QUALITY     95
            pImage->save(baseFileName,0,SCREENSHOT_SAVE_QUALITY);
        }
        delete pImage;
    }
}

void  MainWindow::copyScreenshotToClipboard(OSGCanvas* pCanvas)
{
    QImage* pImage = canvasScreenShotToQImage(pCanvas,100,true);
    if (NULL!=pImage)
    {
        QClipboard *clipboard = QApplication::clipboard();
        if (clipboard)
            clipboard->setImage(*pImage);
		delete pImage;
    }
}

void  MainWindow::saveSlice(OSGCanvas* pCanvas, int mode)
{
    QImage* pImage = NULL;
    if (m_OrthoXYSlice==pCanvas)
    {
        data::CObjectPtr<data::COrthoSliceXY> spSlice( APP_STORAGE.getEntry(data::Storage::SliceXY::Id) );
        data::CSlice::tTexture* pTexture = spSlice->getTexturePtr();
        if (NULL!=pTexture)
        {
            const osg::Image* pOsgImage = pTexture->getImage();
            if (NULL!=pOsgImage)
                pImage = osgImageToQImage(pOsgImage,false);
        }
    }
    if (m_OrthoXZSlice==pCanvas)
    {
        data::CObjectPtr<data::COrthoSliceXZ> spSlice( APP_STORAGE.getEntry(data::Storage::SliceXZ::Id) );
        data::CSlice::tTexture* pTexture = spSlice->getTexturePtr();
        if (NULL!=pTexture)
        {
            const osg::Image* pOsgImage = pTexture->getImage();
            if (NULL!=pOsgImage)
                pImage = osgImageToQImage(pOsgImage,false);
        }
    }
    if (m_OrthoYZSlice==pCanvas)
    {
        data::CObjectPtr<data::COrthoSliceYZ> spSlice( APP_STORAGE.getEntry(data::Storage::SliceYZ::Id) );
        data::CSlice::tTexture* pTexture = spSlice->getTexturePtr();
        if (NULL!=pTexture)
        {
            const osg::Image* pOsgImage = pTexture->getImage();
            if (NULL!=pOsgImage)
                pImage = osgImageToQImage(pOsgImage,false);
        }
    }
    // set metadata
    if (NULL!=pImage)
    {
        QDockWidget* pDW=getParentDockWidget(pCanvas);
        if (NULL!=pDW)
        {
            pImage->setText("Title",pDW->windowTitle());        
            pImage->setText("View",pDW->objectName());
        }
    }
    if (NULL!=pImage)
    {
        QSettings settings;
#if QT_VERSION < 0x050000
        QString previousDir=settings.value("ScreenShotDir",QDesktopServices::storageLocation(QDesktopServices::HomeLocation)).toString();
#else
        QString previousDir=settings.value("ScreenShotDir",QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory)).toString();
#endif
        // if we have a view name then set it as default name
        QString viewName=pImage->text("View");
        if (!viewName.isEmpty())
            previousDir+="/"+viewName+".jpg";
        // open file selection dialog
        QString baseFileName = QFileDialog::getSaveFileName(this,tr("Please specify an output file..."),previousDir,tr("JPEG Image (*.jpg);;PNG Image (*.png);;BMP Image (*.bmp)"));
        if (!baseFileName.isEmpty())
        {
            QFileInfo baseFI( baseFileName );
            settings.setValue("ScreenShotDir",baseFI.dir().absolutePath());
#define SCREENSHOT_SAVE_QUALITY     95
            pImage->save(baseFileName,0,SCREENSHOT_SAVE_QUALITY);
        }
        delete pImage;
    }
}
// updates status bar info on density about cursor
void MainWindow::densityMeasureHandler(double value)
{
    m_grayLevelLabel->setText(QString(tr("Density Level: %1").arg(value)));
}

void MainWindow::updateTabIcons()
{    
    // check event filters
    QList<QTabBar*> tabs = findChildren<QTabBar*>();
    foreach (QTabBar * tabBar, tabs)
        tabBar->installEventFilter(&m_tabsEventFilter);
    // search for all tab bars and all dockable widgets
    QList<QDockWidget*> dws = findChildren<QDockWidget*>();
    foreach (QTabBar * tabBar, tabs)
    {
        foreach (QDockWidget * dw, dws)
        {
            for (int i = 0; i < tabBar->count(); i++)
            {
                // match widgets and tabs
                if (tabBar->tabText(i)==dw->windowTitle())
                {
                    // if icon property is available, set tab icon
                    QString iconPath = dw->property("Icon").toString();
                    if (!iconPath.isEmpty())
                        tabBar->setTabIcon(i,QIcon(iconPath));
                }
            }
        }
    }
}

void MainWindow::sigRegionDataChanged( data::CStorageEntry *pEntry )
{
    data::CChangedEntries changes;
    //unsigned int ver = pEntry->getDirtyVersion();
    unsigned int ver = pEntry->getLatestVersion();
    if (ver>0 && pEntry->getChanges(ver-1,changes))
    {        
        if (changes.checkFlagAny(data::StorageEntry::DESERIALIZED))
        {
            setProperty("SegmentationChanged",false);
            return;
        }
    }
    setProperty("SegmentationChanged",true);
}

void MainWindow::sigBonesModelChanged( data::CStorageEntry *pEntry )
{
    bool bAnyVisible(false);
    for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        if(VPL_SIGNAL(SigGetModelVisibility).invoke2(data::Storage::ImportedModel::Id + i))
        {
            bAnyVisible = true;
            break;
        }

    ui->actionShow_Surface_Model->setChecked(bAnyVisible);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// recent files

void MainWindow::addToRecentFiles(const QString& wsFileName)
{
    QFileInfo pathInfo( wsFileName );
    if (pathInfo.exists())
    {
        QString wsStdPath = pathInfo.canonicalFilePath();
        QList<QAction *> actions = ui->menuRecent->actions();
        foreach(QAction* pAct, actions)
        {
            QString path=pAct->property("Path").toString();
            if (0==path.compare(wsStdPath,Qt::CaseInsensitive))
            {
                ui->menuRecent->removeAction(pAct);
                break;
            }
        }
        actions = ui->menuRecent->actions();
        QAction * pAction = new QAction(wsStdPath,this);
        connect(pAction,SIGNAL(triggered()),this,SLOT(onRecentFile()));
        pAction->setProperty("Path",wsStdPath);
        if (actions.isEmpty())
            ui->menuRecent->addAction(pAction);
        else
            ui->menuRecent->insertAction(actions.at(0),pAction);        
    }    
}

void MainWindow::onRecentFile()
{
    QAction* pAction = qobject_cast<QAction*>(sender());
    Q_ASSERT(pAction);
    if (pAction)
    {
        QString path=pAction->property("Path").toString();
        if (!path.isEmpty())
        {
            if (!preOpen())
                return;

            QFileInfo fi(path);
            QString suffix = fi.isDir()?"":fi.suffix().toLower();
            QString modelFormats = "stl stla stlb obj ply 3ds dxf lwo";
            
            if (path.endsWith("vlm",Qt::CaseInsensitive))
                openVLM(path);
            else
                if (!suffix.isEmpty() && modelFormats.contains(suffix,Qt::CaseInsensitive))
                    openSTL(path);
                else
                    if (path.endsWith(".zip",Qt::CaseInsensitive))
                        openDICOMZIP(path);
                    else
                        openDICOM(path,path);
        }
    }
}

void MainWindow::aboutToShowRecentFiles()
{
    QMenu* pMenu = qobject_cast<QMenu*>(sender());
    if (NULL!=pMenu)
    {
        pMenu->clear();
        pMenu->addAction(ui->actionLoad_Patient_Dicom_data);
        pMenu->addAction(ui->actionImport_DICOM_Data_from_ZIP_archive);
        //pMenu->addAction(ui->actionLoad_Patient_VLM_Data);
        QList<QAction *> actions = ui->menuRecent->actions();
        if (actions.size()>0)
        {
            pMenu->addSeparator();
            foreach(QAction* pAct, actions)
                pMenu->addAction(pAct);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//

// show custom context menu on toolbars and dock widgets
QMenu * MainWindow::createPopupMenu ()
{
    //return QMainWindow::createPopupMenu();
    QMenu* pMenu = new QMenu(this);
    QMenu* pPanelsMenu = pMenu->addMenu(tr("Panels"));

    QList<QDockWidget*> dws = MainWindow::getInstance()->findChildren<QDockWidget*>();
    qSort(dws.begin(),dws.end(),dockWidgetNameCompare);
    foreach (QDockWidget * dw, dws)
    {
        if (dw->toggleViewAction()->isEnabled())
            pPanelsMenu->addAction(dw->toggleViewAction());            
    }

    QMenu* pToolbarsMenu = pMenu->addMenu(tr("Toolbars"));
    QList<QToolBar*> toolbars = MainWindow::getInstance()->findChildren<QToolBar*>();
    foreach (QToolBar * toolbar, toolbars)
    {
        if (toolbar->toggleViewAction()->isEnabled())
            pToolbarsMenu->addAction(toolbar->toggleViewAction());
    }

    QMenu *pMouseModeMenu = pMenu->addMenu(tr("Mouse Mode"));
    foreach(QAction *pAction, ui->mouseToolBar->actions())
        pMouseModeMenu->addAction(pAction);    
    return pMenu;
}

//! Handling of menu requests from docking panels
void MainWindow::onPanelContextMenu(const QPoint & pos)
{
    QPoint menuPos = pos;
    QWidget* pSender = qobject_cast<QWidget*>(sender());
    if (pSender)
        menuPos = pSender->mapToGlobal(pos);
    QMenu* pPopupMenu = createPopupMenu();
    if (NULL!=pPopupMenu)
    {
        QAction* pAct = pPopupMenu->exec(menuPos);
        delete pPopupMenu;
    }
}

void MainWindow::showPanelsMenu()
{
	QMenu* pMenu = new QMenu(this);
    QList<QDockWidget*> dws = MainWindow::getInstance()->findChildren<QDockWidget*>();
    qSort(dws.begin(),dws.end(),dockWidgetNameCompare);
    foreach (QDockWidget * dw, dws)
    {
        if (dw->toggleViewAction()->isEnabled())
            pMenu->addAction(dw->toggleViewAction());            
    }
    if (NULL!=pMenu)
    {
        QAction* pAct = pMenu->exec(QCursor::pos());
        delete pMenu;
    }
}


void MainWindow::onDockWidgetToggleView(bool bShown)
{
    if (bShown)
    {
        QDockWidget* pDW = qobject_cast<QDockWidget*>(sender()->parent());
        if (NULL!=pDW)
            pDW->raise();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// drag and drop

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    if (canAcceptEvent(event))
        event->acceptProposedAction();
    else
        event->ignore();
}
 
void MainWindow::dragMoveEvent(QDragMoveEvent* event)
{
    if (canAcceptEvent(event))
        event->acceptProposedAction();
    else
        event->ignore();
}


bool MainWindow::canAcceptEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
    if (mimeData->hasUrls())
    {
        const QString modelFormats = "stl stla stlb obj ply 3ds dxf lwo";
        QList<QUrl> urlList = mimeData->urls();     
        for (int i = 0; i < urlList.size(); ++i)
        {
            QString path = urlList.at(i).toLocalFile();            
            if (path.endsWith(".vlm",Qt::CaseInsensitive) || path.endsWith(".zip",Qt::CaseInsensitive) ||
                path.endsWith(".dcm",Qt::CaseInsensitive) || path.endsWith(".dicom",Qt::CaseInsensitive))
            {
                return true;
            }
            QFileInfo fi(path);
            if (fi.isDir()) // DICOM dir
                return true;
            QString suffix = fi.isDir()?"":fi.suffix().toLower();
            if (!suffix.isEmpty() && modelFormats.contains(suffix,Qt::CaseInsensitive))
                return true;
			{ // check for numeric extension or dicom files without extension
				int i = 0 , len = suffix.length();
				for(; i<len; i++)
				{
					if (!suffix[i].isDigit())
						break;
				}
				if (i==len) 
					return true;
			}
        }
    }
    return false;
}
 
//void MainWindow::dragLeaveEvent(QDragLeaveEvent* event)
//{
//    event->accept();
//}

void MainWindow::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();
     
    // check for our needed mime type, here a file or a list of files
    if (mimeData->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList = mimeData->urls();
     
        // extract the local paths of the files
        const QString modelFormats = "stl stla stlb obj ply 3ds dxf lwo";
        bool bOpenedSomething = false;
        for (int i = 0; i < urlList.size() && i < 32; ++i)
        {
            //pathList.append(urlList.at(i).toLocalFile());
            QString path = urlList.at(i).toLocalFile();
            if (path.endsWith(".vlm",Qt::CaseInsensitive))
            {
                bOpenedSomething |= openVLM(path);
            }
            else
            {
                if (path.endsWith(".dcm",Qt::CaseInsensitive) || path.endsWith(".dicom",Qt::CaseInsensitive))
                {
                    bOpenedSomething |= openDICOM(path,path);
                }
                else
                {
                    QFileInfo fi(path);
                    QString suffix = fi.isDir()?"":fi.suffix().toLower();
                    if (!suffix.isEmpty() && modelFormats.contains(suffix,Qt::CaseInsensitive))
                    {
                        bOpenedSomething |= openSTL(path);
                    }
                    else
                    {
                        if (path.endsWith(".zip",Qt::CaseInsensitive))
                            bOpenedSomething |= openDICOMZIP(path);
                        else
                            bOpenedSomething |= openDICOM(path,path);
                    }
                }
            }
        }
        if (bOpenedSomething)
        {
            event->acceptProposedAction();
            return;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::processSurfaceModelExtern()
{
    int model_id(m_modelsPanel->getSelectedModelStorageId());
    if(model_id < 0)
    {
        showMessageBox(QMessageBox::Critical, tr("No model selected!"));
        return;
    }

    {   // Check the model
        data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(model_id) );        
        geometry::CMesh *pMesh = spModel->getMesh();
        if (!pMesh || !(pMesh->n_vertices() > 0) )
        {
            showMessageBox(QMessageBox::Critical, tr("No STL data!"));
            return;
        }
    }
    //
    QString wsMeshFixPath = QCoreApplication::applicationDirPath();
#ifdef _WIN32
    wsMeshFixPath+="/meshfix.exe";
#else
    wsMeshFixPath+="/meshfix.dmg";
#endif
    if (!QFile::exists(wsMeshFixPath))
        wsMeshFixPath.clear();
    //
    QString wsPolyMenderPath = QCoreApplication::applicationDirPath();
#ifdef _WIN32
    wsPolyMenderPath+="/polymender.exe";
#else
    wsPolyMenderPath+="/polymender.dmg";
#endif
    if (!QFile::exists(wsPolyMenderPath))
        wsPolyMenderPath.clear();

    QString wsIsoOctreePath = QCoreApplication::applicationDirPath();
#ifdef _WIN32
    wsIsoOctreePath+="/IsoOctree.exe";
#else
    wsIsoOctreePath+="/IsoOctree.dmg";
#endif
    if (!QFile::exists(wsIsoOctreePath))
        wsIsoOctreePath.clear();

    QMenu contextMenu;
    QAction* repairMeshFixAct = wsMeshFixPath.isEmpty()?NULL:contextMenu.addAction(tr("Process using %1").arg("MeshFix"));
    QAction* repairPolyMenderAct = wsPolyMenderPath.isEmpty()?NULL:contextMenu.addAction(tr("Process using %1").arg("Polymender"));
    QAction* repairIsoOctreeAct = wsIsoOctreePath.isEmpty()?NULL:contextMenu.addAction(tr("Process using %1").arg("IsoOctree"));

    if (contextMenu.actions().empty())
    {
        showMessageBox(QMessageBox::Critical, tr("There are none known tools available!"));
        return;
    }

    const QPoint pos=QCursor::pos(); // in global screen coordinates
    const QAction* win=contextMenu.exec(pos);
    if (NULL!=win)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(model_id));
        geometry::CMesh* pMesh = spModel->getMesh();

        // will write to temp file
        QString fileName = QDir::tempPath();
        QString fileNameO = QDir::tempPath();
        if(win==repairPolyMenderAct || win==repairIsoOctreeAct)
        {
            fileName+="/3DVTempMesh.ply";
            fileNameO+="/3DVTempMeshO.ply";
        }
        else
        {
            fileName+="/3DVTempMesh.off";
            fileNameO+="/3DVTempMeshO";
        }

        std::string ansiIn = C3DimApplication::getAnsiName(fileName);
        std::string ansiOut = C3DimApplication::getAnsiName(fileNameO);                

        OpenMesh::IO::Options wopt = OpenMesh::IO::Options::Default;
        if (OpenMesh::IO::write_mesh(*pMesh, ansiIn, wopt)) // note: openmesh needs lowercase extension
        {
            QStringList arguments;
            QString app2Start;
            if(win==repairPolyMenderAct)
            {
                arguments << fileName << "9" << "0.9" << fileNameO;
                app2Start = wsPolyMenderPath;
            }
            else
            {
                if (win==repairIsoOctreeAct)
                {
                    arguments << "--in" << fileName << "--out" << fileNameO << "--maxDepth" << "6";
                    app2Start = wsIsoOctreePath;
                }
                else
                {
                    arguments << fileName << "-o" << fileNameO;
                    //arguments << fileName << "-o" << fileNameO << "--shells" << QString::number(std::min(2,pMesh->componentCount()/2)) << "-jc";
                    fileNameO+=".off";
                    ansiOut = C3DimApplication::getAnsiName(fileNameO);                
                    app2Start = wsMeshFixPath;
                }
            }
            QFile::remove(fileNameO);

            QProcess myProcess;
            if (win==repairMeshFixAct)
                myProcess.setWorkingDirectory(QDir::tempPath());
            else
                myProcess.setWorkingDirectory(QCoreApplication::applicationDirPath());
            myProcess.start(app2Start,arguments);
            if (myProcess.waitForFinished(180000)) // 3 minutes
            {
                OpenMesh::IO::Options ropt;
                ropt += OpenMesh::IO::Options::Binary;
                geometry::CMesh *mesh = new geometry::CMesh;
                if (OpenMesh::IO::read_mesh(*mesh, ansiOut, ropt) && mesh->n_vertices()>0)
                {
                    spModel->setMesh(mesh);
					spModel->clearAllProperties();
                    APP_STORAGE.invalidate(spModel.getEntryPtr());
                }
                else
                    delete mesh;
            }
        }
#if(1)
        QFile::remove(fileName);
        QFile::remove(fileNameO);
#endif
        QApplication::restoreOverrideCursor();
    }                
}

void MainWindow::modelVisualizationSmooth()
{
    for(size_t i = 0; i < MAX_IMPORTED_MODELS; ++i)
        m_modelVisualizers[i]->setModelVisualization(osg::CModelVisualizerEx::EMV_SMOOTH);

    m_3DView->Refresh(false);
}

void MainWindow::modelVisualizationFlat()
{
    for(size_t i = 0; i < MAX_IMPORTED_MODELS; ++i)
        m_modelVisualizers[i]->setModelVisualization(osg::CModelVisualizerEx::EMV_FLAT);
    m_3DView->Refresh(false);
}

void MainWindow::modelVisualizationWire()
{
    for(size_t i = 0; i < MAX_IMPORTED_MODELS; ++i)
        m_modelVisualizers[i]->setModelVisualization(osg::CModelVisualizerEx::EMV_WIRE);
//     data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(data::Storage::BonesModel::Id) );
//     APP_STORAGE.invalidate(spModel.getEntryPtr());
    m_3DView->Refresh(false);
}

void MainWindow::sigSceneHit(float x, float y, float z, int EventType)
{
    if (scene::CAppMode::DOUBLECLICK==EventType)
    {
        VPL_SIGNAL(SigSetSliceXY).invoke(z);
        VPL_SIGNAL(SigSetSliceXZ).invoke(y);
        VPL_SIGNAL(SigSetSliceYZ).invoke(x);
    }
}

void MainWindow::fullscreen(bool on)
{
    if (0==(windowState()&Qt::WindowFullScreen))
	{
		this->setProperty("StateFlags4FS",(int)windowState());
        this->showFullScreen();
	}
    else
	{
		this->showNormal();
		if (0!=(this->property("StateFlags4FS").toInt()&Qt::WindowMaximized))			
			this->showMaximized();
	}
}

/**
 * \brief	Searches for the first possible model identifier. As a side-effect stores used region color and name (if model-region link exists).
 *
 * \return	The found possible model identifier.
 */
int MainWindow::findPossibleModelId()
{
	int id(-1);
	m_modelColor = data::CColor4f(1.0, 1.0, 0.0);
	m_modelLabel = std::string("Model");

	QSettings settings;
	if(settings.value("ModelRegionLinkEnabled", QVariant(false)).toBool())
	{
		// Get active region
		data::CObjectPtr<data::CRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id));
		int active_region(spColoring->getActiveRegion());

		if(active_region >= 0 && active_region < MAX_IMPORTED_MODELS)
		{
			// Store id
			id = active_region;

			// Store label and color
			m_modelLabel = spColoring->getRegionInfo(active_region).getName();
			data::CRegionColoring::tColor rc(spColoring->getColor(active_region));
			m_modelColor = data::CColor4f(rc.getR()/255.0f, rc.getG()/255.0f, rc.getB()/255.0f, rc.getA()/255.0f);
		}
	}

	// Id not found yet
	if(id < 0)
	{
		id = 0;
		while(id<MAX_IMPORTED_MODELS)
		{
			data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(data::Storage::ImportedModel::Id+id, data::Storage::NO_UPDATE) );
			if (!spModel->hasData()) 
				break;
			++id;
		}
		// Is it free model id?
		if (id == MAX_IMPORTED_MODELS)
		{
			showMessageBox(QMessageBox::Critical,tr("The maximum number of loadable models reached!"));
			return -1;
		}
	}

	// Finalize id
	id += data::Storage::ImportedModel::Id;

	return id;
}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::setModelCutVisibilitySignal(int id, bool bShow)
{
	if (id>=data::Storage::ImportedModel::Id && id < data::Storage::ImportedModel::Id + MAX_IMPORTED_MODELS)
	{
		m_importedModelCutSliceXY[id - data::Storage::ImportedModel::Id]->setVisibility(bShow, true);
		m_importedModelCutSliceXZ[id - data::Storage::ImportedModel::Id]->setVisibility(bShow, true);
		m_importedModelCutSliceYZ[id - data::Storage::ImportedModel::Id]->setVisibility(bShow, true);
	}
}

bool MainWindow::getModelCutVisibilitySignal(int id)
{
	if (id>=data::Storage::ImportedModel::Id && id < data::Storage::ImportedModel::Id + MAX_IMPORTED_MODELS)
	{
		return m_importedModelCutSliceXY[id - data::Storage::ImportedModel::Id]->isVisible();
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::aboutToShowViewFilterMenu()
{
	QMenu* pMenu = qobject_cast<QMenu*>(sender());
	data::CObjectPtr<data::CAppSettings> settings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
	ui->actionViewEqualize->blockSignals(true);
	ui->actionViewSharpen->blockSignals(true);
	ui->actionViewEqualize->setChecked(data::CAppSettings::Equalize==settings->getFilter());
	ui->actionViewSharpen->setChecked(data::CAppSettings::Sharpen==settings->getFilter());
	ui->actionViewEqualize->blockSignals(false);
	ui->actionViewSharpen->blockSignals(false);
}

void MainWindow::setTextureFilterEqualize(bool on)
{
	data::CObjectPtr<data::CAppSettings> settings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
	settings->setFilter(on?data::CAppSettings::Equalize:data::CAppSettings::NoFilter);
	APP_STORAGE.invalidate(settings.getEntryPtr());	
	// invalidata all slices
	data::CObjectPtr<data::COrthoSliceXY> spSliceXY( APP_STORAGE.getEntry(data::Storage::SliceXY::Id) );
	APP_STORAGE.invalidate(spSliceXY.getEntryPtr(), data::COrthoSlice::MODE_CHANGED );
	data::CObjectPtr<data::COrthoSliceXZ> spSliceXZ( APP_STORAGE.getEntry(data::Storage::SliceXZ::Id) );
	APP_STORAGE.invalidate(spSliceXZ.getEntryPtr(), data::COrthoSlice::MODE_CHANGED );
	data::CObjectPtr<data::COrthoSliceYZ> spSliceYZ( APP_STORAGE.getEntry(data::Storage::SliceYZ::Id) );
	APP_STORAGE.invalidate(spSliceYZ.getEntryPtr(), data::COrthoSlice::MODE_CHANGED );
}


void MainWindow::setTextureFilterSharpen(bool on)
{
	data::CObjectPtr<data::CAppSettings> settings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
	settings->setFilter(on?data::CAppSettings::Sharpen:data::CAppSettings::NoFilter);
	APP_STORAGE.invalidate(settings.getEntryPtr());	
	// invalidata all slices
	data::CObjectPtr<data::COrthoSliceXY> spSliceXY( APP_STORAGE.getEntry(data::Storage::SliceXY::Id) );
	APP_STORAGE.invalidate(spSliceXY.getEntryPtr(), data::COrthoSlice::MODE_CHANGED );
	data::CObjectPtr<data::COrthoSliceXZ> spSliceXZ( APP_STORAGE.getEntry(data::Storage::SliceXZ::Id) );
	APP_STORAGE.invalidate(spSliceXZ.getEntryPtr(), data::COrthoSlice::MODE_CHANGED );
	data::CObjectPtr<data::COrthoSliceYZ> spSliceYZ( APP_STORAGE.getEntry(data::Storage::SliceYZ::Id) );
	APP_STORAGE.invalidate(spSliceYZ.getEntryPtr(), data::COrthoSlice::MODE_CHANGED );
}

///////////////////////////////////////////////////////////////////////////////

void MainWindow::loadShortcuts()
{
	QSettings settings;
	settings.beginGroup("Shortcuts");
	QList<QMenu*> lst = ui->menuBar->findChildren<QMenu*>();
	foreach(QMenu* m, lst)
	{
		if (m->parent()==ui->menuBar)
			loadShortcutsForMenu(m, settings);
	}
}

void MainWindow::loadShortcutsForMenu(QMenu* menu, QSettings& settings)
{
	if (NULL==menu) return;
	if (!menu->actions().empty())
	{
		settings.beginGroup(menu->objectName());
		foreach(QAction* a, menu->actions())
		{
			if (!a->isSeparator())
			{
				if (NULL!=a->menu())
				{
					loadShortcutsForMenu(a->menu(),settings);
				}
				else
				{
					QString src = a->objectName();
					if (src.isEmpty())
					{
						//qDebug() << a->text();
						src = a->text();
					}
					if (!src.isEmpty())
					{
						QVariant res = settings.value(src,QVariant());
						if (res.isValid())
						{
							QKeySequence ks(res.toString()); // TODO: check whether the action is used
							a->setShortcut(ks);
							a->setProperty("Shortcut","Custom");
						}
					}
				}
			}
		}
		settings.endGroup();
	}
}

void MainWindow::saveShortcutsForMenu(QMenu* menu, QSettings& settings)
{
	if (NULL==menu) return;
	if (!menu->actions().empty())
	{
		settings.beginGroup(menu->objectName());
		foreach(QAction* a, menu->actions())
		{
			if (!a->isSeparator())
			{
				if (NULL!=a->menu())
				{
					saveShortcutsForMenu(a->menu(),settings);
				}
				else
				{
					if (0==a->property("Shortcut").toString().compare("Custom",Qt::CaseInsensitive))
					{
						QString key = a->objectName();
						if (key.isEmpty())
							key = a->text();
						if (!key.isEmpty())
							settings.setValue(key,a->shortcut().toString());
					}
				}
			}
		}
		settings.endGroup();
	}
}

void MainWindow::saveShortcuts()
{
	QSettings settings;
	settings.beginGroup("Shortcuts");
	QList<QMenu*> lst = ui->menuBar->findChildren<QMenu*>();
	foreach(QMenu* m, lst)
	{
		if (m->parent()==ui->menuBar)
			saveShortcutsForMenu(m, settings);
	}
}

bool MainWindow::isDockWidgetVisible(QDockWidget* pDW)
{
	if (NULL==pDW)
		return false;
	if (NULL==pDW->widget())
		return pDW->isVisible();
	if (!pDW->widget()->visibleRegion().isEmpty())
		return true;
	// test above can fail even when the dock widget is visible, therefore we
	// enumerate all children and test their visibility
	QList<QWidget*> children = pDW->findChildren<QWidget*>();
	bool bAnyChildVisible = false;
	foreach(QWidget* pChild, children)
	{
		if (!pChild->visibleRegion().isEmpty())
		{
			bAnyChildVisible  = true;
			break;
		}
	}
	return bAnyChildVisible;
}

QDockWidget *MainWindow::getActivePanel()
{
	QWidget* pWidget = QApplication::focusWidget();
	if (NULL==pWidget)
		pWidget = QApplication::activeWindow();
	if (NULL!=pWidget)
	{
		QDockWidget* pDW=getParentDockWidget(pWidget);		
		if (NULL!=pDW)
		{			
			if (!isDockWidgetVisible(pDW))
			{
				//qDebug() << "active widget not visible " << pDW->windowTitle();
				QList<QDockWidget*> tabbed = tabifiedDockWidgets(pDW);
				pDW = NULL;
				foreach(QDockWidget* pDWT, tabbed)
					if (isDockWidgetVisible(pDWT))
					{
						//qDebug() << "replacing by visible " << pDWT->windowTitle();
						pDW = pDWT;
						break;
					}
			}
			if (NULL!=pDW)
			{
                if (NULL!=pDW->widget() && NULL!=qobject_cast<OSGCanvas*>(pDW->widget()))
				{
					// find dockwidget to close
				}
				else
				{
					return pDW;
				}
			}
		}
	}
	// enumerate all dockwidgets and check their visibility
	QList<QDockWidget *> dockWidgets = findChildren<QDockWidget *>();
	std::list<QDockWidget*> lstVisible;
	foreach(QDockWidget* pDW, dockWidgets)
	{
        if (isDockWidgetVisible(pDW) && !(NULL!=pDW->widget() && NULL!=qobject_cast<OSGCanvas*>(pDW->widget())))
			lstVisible.push_back(pDW);
	}
	if (lstVisible.size()==1)
	{
		//qDebug() << "found one visible";
		return lstVisible.front();
	}
	else if (lstVisible.size()>0)
	{
		//qDebug() << "found more visible, use cursor pos";
		QPoint mousePos = QCursor::pos();
		for(auto it = lstVisible.begin(); it!=lstVisible.end(); ++it)
		{								
			QRect globalRect;
			globalRect = QRect((*it)->mapToGlobal((*it)->rect().topLeft()),
			(*it)->mapToGlobal((*it)->rect().bottomRight()));
			if (globalRect.contains(mousePos))
			{
				return (*it);
			}
		}
	}
	return NULL;
}

void	MainWindow::prevPanel()
{
	QDockWidget* pActive = getActivePanel();
	if (NULL!=pActive)
	{
		QList<QTabBar*> tabs = findChildren<QTabBar*>();
		foreach (QTabBar * tabBar, tabs)
		{
            for (int i = 0; i < tabBar->count(); i++)
            {
                // match widgets and tabs
                if (tabBar->tabText(i)==pActive->windowTitle())
                {
					i = (i-1+tabBar->count())%tabBar->count();
					tabBar->setCurrentIndex(i);
					return;
				}
			}
		}
	}
}

void	MainWindow::nextPanel()
{
	QDockWidget* pActive = getActivePanel();
	if (NULL!=pActive)
	{
		QList<QTabBar*> tabs = findChildren<QTabBar*>();
		foreach (QTabBar * tabBar, tabs)
		{
            for (int i = 0; i < tabBar->count(); i++)
            {
                // match widgets and tabs
                if (tabBar->tabText(i)==pActive->windowTitle())
                {
					i = (i+1)%tabBar->count();
					tabBar->setCurrentIndex(i);
					return;
				}
			}
		}
	}
}

void MainWindow::closeActivePanel()
{
	QDockWidget* pActive = getActivePanel();
	if (NULL!=pActive)
		pActive->hide();
}

///////////////////////////////////////////////////////////////////////////////