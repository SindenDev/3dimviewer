///////////////////////////////////////////////////////////////////////////////
// $Id$
//
// 3DimViewer
// Lightweight 3D DICOM viewer.
//
// Copyright 2008-2016 3Dim Laboratory s.r.o.
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

#include <C3DimApplication.h>

#include <QtGui>
#include <QDockWidget>
#include <QLayout>
#include <QPrintDialog>
#include <QStylePainter>
#include <QPrinter>
#include <QFileDialog>
#include <QDesktopServices>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QKeyEvent>
#include <QStackedWidget>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QHostInfo>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    #include <QUrlQuery>
#endif

#if QT_VERSION >= 0x050000
    #include <QStandardPaths>
#endif
#include <QMap>
#include <QCheckBox>
#include <QRadioButton>
#include <data/CDicomLoader.h>
#include <data/CAppSettings.h>
#include <data/CUndoManager.h>
#include <data/CRegionData.h>
#include <data/CRegionColoring.h>
#include <data/CMultiClassRegionColoring.h>
#include <data/CImageLoaderInfo.h>
#include <data/CVolumeTransformation.h>
#include <data/CVolumeOfInterestData.h>
#include <data/CPivot.h>

#include <VPL/Module/Progress.h>
#include <VPL/Module/Serialization.h>
#include <VPL/Image/VolumeFiltering.h>
#include <VPL/Image/VolumeFunctions.h>
#include <VPL/Image/Filters/Anisotropic.h>

#include <render/PSVRosg.h>

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
#include <CFilterDialog.h>
#include <CModelVisualizerEx.h>

#include <cinfodialog.h>

#include <data/CCustomData.h>

#include <zlib.h>
#include <zip/unzip.h>

#include <osg/OSGTreeAnalyser.h>
#include <osg/CPseudoMaterial.h>

//#define DUMP_OSG_TREE


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

#if defined( TRIDIM_USE_GDCM )

#include <data/CDicomSaverGDCM.h>

#else

#include <data/CDicomSaverDCTk.h>
#include <data/CDicomDCTk.h>
#include <CDicomTransferDialog.h>

#endif

#include <osg/NoteSubtree.h>
#if defined(PLUGIN_VIEWER_3DIM)
    #include <notesplugin.h>
    #include <notespluginpanel.h>
#endif
#include <CZipLoader.h>

#include <3dim/geometry/base/CTMReader.h>
#include <3dim/geometry/base/CTMWriter.h>

#ifdef ENABLE_PYTHON
#include <qtpython/pyconfigure.h>
#include <qtpython/interpret.h>
#include "qtpython/settings.h"
#endif

#include <CSysInfo.h>

#define DEFAULT_PING_URL "https://licreq.t3d.team/flying_feather.php"

#undef interface

//#undef USE_PSVR

bool dockWidgetNameCompare(QDockWidget* left, QDockWidget* right)
{
    return left->windowTitle().compare(right->windowTitle())<0;
}

///////////////////////////////////////////////////////////////////////////////
// MainWindow

MainWindow* MainWindow::m_pMainWindow = NULL;

MainWindow::MainWindow(QWidget *parent, CPluginManager* pPlugins) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
	m_pPlugins(pPlugins),
	m_segmentationPluginsLoaded(false),
    m_eventFilter(this),
    m_region3DPreviewVisible(false),
    m_myViewVisibilityChange(false)
{    
    this->setObjectName("MainWindow");

	Q_ASSERT(NULL!=m_pPlugins);
    m_pMainWindow = this;
    m_nLayoutType = 0;
    setProperty("SegmentationChanged",false);

    m_3DView = NULL;
    m_OrthoXYSlice = NULL;
    m_OrthoXZSlice = NULL;
    m_OrthoYZSlice = NULL;
    m_ArbitrarySlice = NULL;

    for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        m_modelVisualizers[i] = NULL;

    m_region3DPreviewVisualizer = NULL;

    m_densityWindowPanel=NULL;
    m_orthoSlicesPanel=NULL;
    m_segmentationPanel=NULL;
    m_volumeRenderingPanel=NULL;
    m_realCentralWidget = NULL;
	m_modelsPanel = NULL;
    // we have to create a permanent central widget because of some sizing issues
    // when switching workspaces without central widget
    m_centralWidget = new QWidget();
    QLayout* pCentralLayout = new QHBoxLayout(); // just some layout
    pCentralLayout->setContentsMargins(0,0,0,0);
    pCentralLayout->setSpacing(0);
    m_centralWidget->setLayout(pCentralLayout);

    // setup UI
    ui->setupUi(this);

    installEventFilter(this);

    data::CObjectPtr<data::CRegionData> spOldRegionData(APP_STORAGE.getEntry(data::Storage::RegionData::Id));
    spOldRegionData->makeDummy(true);

    //setDockOptions(QMainWindow::VerticalTabs);
	setDockOptions(QMainWindow::AllowTabbedDocks | QMainWindow::AllowNestedDocks | QMainWindow::VerticalTabs);
    setAcceptDrops(true);
    
    // set up status bar
    m_grayLevelLabel = new QLabel();
    m_grayLevelLabel->setMinimumWidth(200);
    ui->statusBar->addPermanentWidget(m_grayLevelLabel);

    // set blue background color
    data::CObjectPtr<data::CAppSettings> spSettings( APP_STORAGE.getEntry(data::Storage::AppSettings::Id) );
	spSettings->setClearColor(osg::Vec4(0.2f, 0.2f, 0.4f, 1.0f));

	setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
    setCentralWidget(m_centralWidget);

    // connect actions to slots
    connectActions();

    // add actions to toolbars
    createToolBars();

    // create OSG windows
    createOSGStuff();

    /*m_SceneXY->setArbSliceVisibility(false);
    m_SceneXZ->setArbSliceVisibility(false);
    m_SceneYZ->setArbSliceVisibility(false);*/

	VPL_SIGNAL(SigSetContoursVisibility).connect(this, &MainWindow::setContoursVisibility);
	VPL_SIGNAL(SigGetContoursVisibility).connect(this, &MainWindow::getContoursVisibility);

	VPL_SIGNAL(SigSetVolumeOfInterestVisibility).connect(this, &MainWindow::setVOIVisibility);
	VPL_SIGNAL(SigGetVolumeOfInterestVisibility).connect(this, &MainWindow::getVOIVisibility);

	VPL_SIGNAL(SigEnableRegionColoring).connect(this, &MainWindow::setRegionsVisibility);
    VPL_SIGNAL(SigEnableMultiClassRegionColoring).connect(this, &MainWindow::setRegionsVisibility);

	VPL_SIGNAL(SigShowVolumeOfInterestDialog).connect(this, &MainWindow::limitVolume);

    VPL_SIGNAL(SigRegionDataLoaded).connect(this, &MainWindow::moveRegionDataToMulticlass);
    VPL_SIGNAL(SigRemoveAllModels).connect(this, &MainWindow::removeAllModels);

    VPL_SIGNAL(SigGetModelManager).connect(this, &MainWindow::getModelManager);

    VPL_SIGNAL(SigChangeVRVisibility).connect(this, &MainWindow::changeVRVisibility);
    VPL_SIGNAL(SigVRVisible).connect(this, &MainWindow::isVRVisible);
    VPL_SIGNAL(SigVRChanged).connect(this, &MainWindow::onSigVRChanged);
    VPL_SIGNAL(SigModelsVisible).connect(this, &MainWindow::areModelsVisible);
    VPL_SIGNAL(SigChangeModelsVisibility).connect(this, &MainWindow::changeModelsVisibility);

    VPL_SIGNAL(SigDrawingInProgress).connect(this, &MainWindow::onDrawingInProgress);


    VPL_SIGNAL(SigStrokeOnOffWithCheck).connect(this, &MainWindow::setStrokeOnOff);

    VPL_SIGNAL(SigMakeWindowFloating).connect(this, &MainWindow::makeWndFloating);
    VPL_SIGNAL(SigShowViewInWindow).connect(this, &MainWindow::showViewInWindow);

    VPL_SIGNAL(SigPythonVolumeLoaded).connect(this, &MainWindow::postOpenActions);

    VPL_SIGNAL(SigPluginLog).connect(this, &MainWindow::pluginLog);

    VPL_SIGNAL(SigSaveScreenshot).connect(this, &MainWindow::saveScreenshot);


	QSettings settings;

    setupDockWindows();

    // load application settings
    int nThreadingMode=0;
    {   // read info on basic layout		
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
            m_ArbitrarySlice->setBackgroundColor(osgColor);
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

    // setup working space
    m_viewsToShow.clear();

    switch (m_nLayoutType)
    {
        case WorkspaceTabs:
        {
            m_viewsToShow.push_back(sWindowInfo(View3D, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewXY, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewXZ, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewYZ, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewARB, false, true, true));
        }
        break;
        case WorkspaceGrid:
        {
            m_viewsToShow.push_back(sWindowInfo(ViewXY, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(View3D, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewXZ, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewYZ, false, true, true));
        }
        break;
        case WorkspaceSlices:
        {
            m_viewsToShow.push_back(sWindowInfo(ViewXY, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewARB, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewXZ, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewYZ, false, true, true));
        }
        break;
        case Workspace3D:
        default:
        {
            m_viewsToShow.push_back(sWindowInfo(ViewXY, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewXZ, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewYZ, false, true, true));
            m_viewsToShow.push_back(sWindowInfo(ViewARB, false, true, true));
        }
        break;
    }
    
    setUpWorkspace();

    m_3DView->show();
    m_OrthoXYSlice->show();
    m_OrthoXZSlice->show();
    m_OrthoYZSlice->show();
    m_ArbitrarySlice->show();

    // create panels
    createPanels();

	std::map<int, vpl::base::CObject*>& vplSignalsTable = m_pPlugins->getVplSignalsTable();
	vplSignalsTable[VPL_SIGNAL(SigShowPreviewDialog).getId()] = &(VPL_SIGNAL(SigShowPreviewDialog));
	vplSignalsTable[VPL_SIGNAL(SigPreviewDialogClosed).getId()] = &(VPL_SIGNAL(SigPreviewDialogClosed));
	vplSignalsTable[VPL_SIGNAL(SigShowInfoDialog).getId()] = &(VPL_SIGNAL(SigShowInfoDialog));
	vplSignalsTable[VPL_SIGNAL(SigNumberOfRegionsChanged).getId()] = &(VPL_SIGNAL(SigNumberOfRegionsChanged));
	vplSignalsTable[VPL_SIGNAL(SigVRChanged).getId()] = &(VPL_SIGNAL(SigVRChanged));
	vplSignalsTable[VPL_SIGNAL(SigManSeg3DChanged).getId()] = &(VPL_SIGNAL(SigManSeg3DChanged));
    vplSignalsTable[VPL_SIGNAL(SigUpdateVR).getId()] = &(VPL_SIGNAL(SigUpdateVR));
    vplSignalsTable[VPL_SIGNAL(SigRemoveMeasurements).getId()] = &(VPL_SIGNAL(SigRemoveMeasurements));
    vplSignalsTable[VPL_SIGNAL(SigDrawingDone).getId()] = &(VPL_SIGNAL(SigDrawingDone));
    vplSignalsTable[VPL_SIGNAL(SigRegionDataLoaded).getId()] = &(VPL_SIGNAL(SigRegionDataLoaded));
    vplSignalsTable[VPL_SIGNAL(SigRemoveAllModels).getId()] = &(VPL_SIGNAL(SigRemoveAllModels));
    vplSignalsTable[VPL_SIGNAL(SigPluginAuxRegionsLoaded).getId()] = &(VPL_SIGNAL(SigPluginAuxRegionsLoaded));
    vplSignalsTable[VPL_SIGNAL(SigScrollPerformed).getId()] = &(VPL_SIGNAL(SigScrollPerformed));
    vplSignalsTable[VPL_SIGNAL(SigSetColoring).getId()] = &(VPL_SIGNAL(SigSetColoring));
    vplSignalsTable[VPL_SIGNAL(SigEnableRegionColoring).getId()] = &(VPL_SIGNAL(SigEnableRegionColoring));
    vplSignalsTable[VPL_SIGNAL(SigEnableMultiClassRegionColoring).getId()] = &(VPL_SIGNAL(SigEnableMultiClassRegionColoring));
    vplSignalsTable[VPL_SIGNAL(SigUndoOnColoringPerformed).getId()] = &(VPL_SIGNAL(SigUndoOnColoringPerformed));
    vplSignalsTable[VPL_SIGNAL(SigGetModelManager).getId()] = &(VPL_SIGNAL(SigGetModelManager));
    vplSignalsTable[VPL_SIGNAL(SigActiveRegionChanged).getId()] = &(VPL_SIGNAL(SigActiveRegionChanged));
    vplSignalsTable[VPL_SIGNAL(SigNewRegionSelected).getId()] = &(VPL_SIGNAL(SigNewRegionSelected));
    vplSignalsTable[VPL_SIGNAL(SigSetActiveRegion3DPreviewVisibility).getId()] = &(VPL_SIGNAL(SigSetActiveRegion3DPreviewVisibility));
    vplSignalsTable[VPL_SIGNAL(SigChangeVRVisibility).getId()] = &(VPL_SIGNAL(SigChangeVRVisibility));
    vplSignalsTable[VPL_SIGNAL(SigVRVisible).getId()] = &(VPL_SIGNAL(SigVRVisible));
    vplSignalsTable[VPL_SIGNAL(SigModelsVisible).getId()] = &(VPL_SIGNAL(SigModelsVisible));
    vplSignalsTable[VPL_SIGNAL(SigChangeModelsVisibility).getId()] = &(VPL_SIGNAL(SigChangeModelsVisibility));
    vplSignalsTable[VPL_SIGNAL(SigSetRegion3DPreviewInterval).getId()] = &(VPL_SIGNAL(SigSetRegion3DPreviewInterval));
    vplSignalsTable[VPL_SIGNAL(SigClearAllLandmarkAnnotationDrawables).getId()] = &(VPL_SIGNAL(SigClearAllLandmarkAnnotationDrawables));
    vplSignalsTable[VPL_SIGNAL(SigClearLandmarkAnnotationDrawable).getId()] = &(VPL_SIGNAL(SigClearLandmarkAnnotationDrawable));
    vplSignalsTable[VPL_SIGNAL(SigCreateLandmarkAnnotationDrawable).getId()] = &(VPL_SIGNAL(SigCreateLandmarkAnnotationDrawable));
    vplSignalsTable[VPL_SIGNAL(SigSetLandmarkAnnotationDrawablesVisibility).getId()] = &(VPL_SIGNAL(SigSetLandmarkAnnotationDrawablesVisibility));
    vplSignalsTable[VPL_SIGNAL(SigGetModelDraggerModelId).getId()] = &(VPL_SIGNAL(SigGetModelDraggerModelId));
    vplSignalsTable[VPL_SIGNAL(SigSetModelDraggerModelId).getId()] = &(VPL_SIGNAL(SigSetModelDraggerModelId));
    vplSignalsTable[VPL_SIGNAL(SigGetModelDraggerVisibility).getId()] = &(VPL_SIGNAL(SigGetModelDraggerVisibility));
    vplSignalsTable[VPL_SIGNAL(SigSetModelDraggerVisibility).getId()] = &(VPL_SIGNAL(SigSetModelDraggerVisibility));
    vplSignalsTable[VPL_SIGNAL(SigGetActiveConvObject).getId()] = &(VPL_SIGNAL(SigGetActiveConvObject));
    vplSignalsTable[VPL_SIGNAL(SigStrokeOnOffWithCheck).getId()] = &(VPL_SIGNAL(SigStrokeOnOffWithCheck));
    vplSignalsTable[VPL_SIGNAL(SigStrokeOnOff).getId()] = &(VPL_SIGNAL(SigStrokeOnOff));
    vplSignalsTable[VPL_SIGNAL(SigPythonVolumeLoaded).getId()] = &(VPL_SIGNAL(SigPythonVolumeLoaded));
    vplSignalsTable[VPL_SIGNAL(SigPluginLog).getId()] = &(VPL_SIGNAL(SigPluginLog));
    vplSignalsTable[VPL_SIGNAL(SigRegion3DPreviewVisible).getId()] = &(VPL_SIGNAL(SigRegion3DPreviewVisible));
    vplSignalsTable[VPL_SIGNAL(SigHasCnnModels).getId()] = &(VPL_SIGNAL(SigHasCnnModels));
    vplSignalsTable[VPL_SIGNAL(SigHasLandmarksDefinition).getId()] = &(VPL_SIGNAL(SigHasLandmarksDefinition));


#ifdef ENABLE_PYTHON
	data::CObjectPtr<data::Interpret> interpret(APP_STORAGE.getEntry(data::Storage::InterpretData::Id));
	interpret->initialize();
    settings.setValue("isPythonEnabled", true);
#else
    settings.setValue("isPythonEnabled", false);
#endif

    // load plugins
    Q_ASSERT(m_densityWindowPanel && m_densityWindowPanel->parentWidget());
    QDockWidget* dockPluginsTo=qobject_cast<QDockWidget*>(m_densityWindowPanel->parentWidget());
	if (NULL!=m_pPlugins)
		m_pPlugins->loadPlugins(this,&m_localeDir,dockPluginsTo);	

	if (findPluginByID("AutoSegAndFiltering"))
	{
		m_segmentationPluginsLoaded = true;
	}

    VPL_SIGNAL(SigPluginAuxRegionsLoaded).invoke(findPluginByID("SegToSmallRegions") != NULL);

    // load last known layout
    readLayoutSettings(false);
    setUpWorkspace();

    // set up mouse mode change monitoring
    m_ConnectionModeChanged = APP_MODE.getModeChangedSignal().connect( this, &MainWindow::sigModeChanged );
    sigModeChanged(APP_MODE.get()); // first call has to be explicit

    APP_MODE.getSceneHitSignal().connect(this, &MainWindow::sigSceneHit);

    APP_MODE.getContinuousDensityMeasureSignal().connect(this, &MainWindow::densityMeasureHandler);

	VPL_SIGNAL(SigSetModelCutVisibility).connect(this, &MainWindow::setModelCutVisibilitySignal);
	VPL_SIGNAL(SigGetModelCutVisibility).connect(this, &MainWindow::getModelCutVisibilitySignal);
    
    VPL_SIGNAL(SigGetModelDraggerModelId).connect(this, &MainWindow::getModelDraggerModelId);
    VPL_SIGNAL(SigSetModelDraggerModelId).connect(this, &MainWindow::setModelDraggerModelId);
    VPL_SIGNAL(SigGetModelDraggerVisibility).connect(this, &MainWindow::getModelDraggerVisibility);
    VPL_SIGNAL(SigSetModelDraggerVisibility).connect(this, &MainWindow::setModelDraggerVisibility);
    VPL_SIGNAL(SigAlignmentDraggerMove).connect(this, &MainWindow::onDraggerMove);


    // set threading mode
    if (1==nThreadingMode)
    {
        VPL_LOG_INFO("Enabling multithreaded OpenGL");
        m_3DView->enableMultiThreaded();
        m_OrthoXYSlice->enableMultiThreaded();
        m_OrthoXZSlice->enableMultiThreaded();
        m_OrthoYZSlice->enableMultiThreaded();
        m_ArbitrarySlice->enableMultiThreaded();
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
        /*if (cntOther>0)
        {
            ui->menuModel->addAction(tr("Process Model"),this,SLOT(processSurfaceModelExtern()));
        }*/
#endif
        /*QMenu* pMenu = ui->menuModel->addMenu(tr("Visualization"));
        QAction* pSmooth = pMenu->addAction(tr("Smooth"),this,SLOT(modelVisualizationSmooth()));
		pSmooth->setObjectName("smoothShading");
        QAction* pFlat = pMenu->addAction(tr("Flat"),this,SLOT(modelVisualizationFlat()));
		pFlat->setObjectName("flatShading");
        QAction* pWire = pMenu->addAction(tr("Wire"),this,SLOT(modelVisualizationWire()));
		pWire->setObjectName("noShading");*/
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
    ptrManager->setMaxSize(1024 * 1024 * (long)500); // set max space for undo
    ptrManager->getUndoChangedSignal().connect(this, &MainWindow::undoRedoEnabler);

    m_conRegionData = APP_STORAGE.getEntrySignal(data::Storage::MultiClassRegionData::Id).connect(this, &MainWindow::sigRegionDataChanged);
    m_conRegionColoring = APP_STORAGE.getEntrySignal(data::Storage::MultiClassRegionColoring::Id).connect(this, &MainWindow::sigRegionColoringChanged);

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

	connect(ui->menuData,SIGNAL(aboutToShow()),this,SLOT(aboutToShowViewFilterMenu()));
	connect(ui->actionViewEqualize, SIGNAL(toggled(bool)), this, SLOT(setTextureFilterEqualize(bool)));
	connect(ui->actionViewSharpen, SIGNAL(toggled(bool)), this, SLOT(setTextureFilterSharpen(bool)));

	loadShortcuts();

    // timer for OSG widget repaint - needed for eventqueue to work properly
    m_timer.setInterval(150);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(show_frame()));
    m_timer.start();	

	// first event will open files from command line etc
    QTimer::singleShot(0, this, SLOT(firstEvent()));

    connectActionsToEventFilter();
	m_eventFilter.processSettings();

    VPL_SIGNAL(SigActiveRegionChanged).connect(this, &MainWindow::onActiveRegionChanged);

	VPL_SIGNAL(SigShowPreviewDialog).connect(this, &MainWindow::showPreviewDialog);
	VPL_SIGNAL(SigShowInfoDialog).connect(this, &MainWindow::showInfoDialog);

	VPL_SIGNAL(SigSaveModelExt).connect(this, &MainWindow::saveSTLById);

	data::CObjectPtr<data::CMultiClassRegionData> spRegionData(APP_STORAGE.getEntry(data::Storage::MultiClassRegionData::Id, data::Storage::NO_UPDATE));

	ui->actionRegions_Visible->setChecked(spRegionData->isColoringEnabled());

	if (!findPluginByID("DataExpress"))
	{
		ui->actionSend_Data->setVisible(false);
	}

    setUpRegion3DPreview();

    ui->menuSegmentation->menuAction()->setVisible(false);

	if (!m_segmentationPluginsLoaded)
	{
		ui->menuSegmentation->menuAction()->setVisible(false);
		ui->actionRegions_Contours_Visible->setVisible(false);
		ui->actionRegions_Visible->setVisible(false);
        m_modelsPanel->setEnabled(true);
	}
	else
	{
		ui->actionModels_List_Panel->setEnabled(false);
		ui->actionModels_List_Panel->setVisible(false);
		m_modelsPanel->close();
		m_modelsPanel->setEnabled(false);
		m_modelsPanel->setVisible(false);
		m_modelsPanel->parentWidget()->setEnabled(false);
		m_modelsPanel->parentWidget()->setVisible(false);
        m_modelsPanel->setEnabled(false);

        ui->actionSegmentation_Panel->setEnabled(false);
        ui->actionSegmentation_Panel->setVisible(false);
        m_segmentationPanel->close();
        m_segmentationPanel->setEnabled(false);
        m_segmentationPanel->setVisible(false);
        m_segmentationPanel->parentWidget()->setEnabled(false);
        m_segmentationPanel->parentWidget()->setVisible(false);
	}

	//ui->actionRegions_Contours_Visible->setChecked(!settings.value("ContoursVisibility", true).toBool());
    ui->actionRegions_Contours_Visible->setChecked(true);
	ui->actionRegions_Contours_Visible->trigger();  
    ui->actionRegions_Contours_Visible->setVisible(false);

	m_Connection = APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &MainWindow::onNewDensityData);
    m_Connection2DCanvasLeave = APP_MODE.getWindowEnterLeaveSignal().connect(this, &MainWindow::sig2DCanvasLeave);

	// show the feedback request
	settings.beginGroup("Feedback");
	int starts = settings.value("NumberOfStarts", 0).toInt();

	if (starts == 10 || (starts > 10 && starts % 50 == 0))
		QTimer::singleShot(1500, this, SLOT(showFeedbackRequestDialog()));

	settings.setValue("NumberOfStarts", starts + 1);
	settings.endGroup();

	// load all other app settings
	loadAppSettings();

	//m_volumeRenderingPanel->setVRMode();

    // NotesPluginPanel will be able to receive events of this object.
    // It needs them for line highlighting and knowing when camera moves
#if defined(PLUGIN_VIEWER_3DIM)
    CNotesPlugin *plug = (CNotesPlugin *)findPluginByID("NotesPlugin");

    if (plug != nullptr) 
    {
        CNotesPluginPanel *panel = (CNotesPluginPanel *)plug->getOrCreatePanel();
        m_3DView->installEventFilter(panel);

    }
#endif

    // just for translations
    QString regName(QObject::tr("New Region"));
    QString regName2(QObject::tr("No Region"));

    ui->actionVolume_Rendering_View->setVisible(false);

    data::CObjectPtr< data::CDrawingOptions > spOptions(APP_STORAGE.getEntry(data::Storage::DrawingOptions::Id));
    spOptions->setDrawingMode(data::CDrawingOptions::DRAW_NOTHING);
    APP_STORAGE.invalidate(spOptions.getEntryPtr());
}

MainWindow::~MainWindow()
{

#ifdef DUMP_OSG_TREE
    osg::OSGTreeAnalyser an;
    m_Scene3D->accept(an);
#endif
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

    APP_STORAGE.getEntrySignal(data::Storage::MultiClassRegionData::Id).disconnect(m_conRegionData);
    APP_STORAGE.getEntrySignal(data::Storage::MultiClassRegionColoring::Id).disconnect(m_conRegionColoring);
    APP_MODE.getModeChangedSignal().disconnect(m_ConnectionModeChanged);
    APP_MODE.getModeChangedSignal().disconnect(m_Connection2DCanvasLeave);
    {   // clear undo before plugin unload because it can contain data allocated by plugins
        data::CObjectPtr< data::CUndoManager > ptrManager(APP_STORAGE.getEntry(data::Storage::UndoManager::Id));
        ptrManager->clear();
    }
	if (NULL!=m_pPlugins)
		m_pPlugins->disconnectPlugins();

    for (size_t i = 0; i < m_wndViews.size(); ++i)
    {
        delete m_wndViews[i];
        delete m_wndViewsSubstitute[i];
    }

    delete ui;

    destroyRegion3DPreview();
}

// Just a hack. When pressing ALT, menu bar gets focus and key release doesn't appear in OSGCanvas,
// so the mouse mode stays active, which is not the behavior we want
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QKeyEvent::KeyRelease)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);

        if (ke->key() == Qt::Key_Alt)
        {
            if (APP_MODE.isTempMode())
            {
                APP_MODE.restore();
                if (0 != (APP_MODE.get() & scene::CAppMode::COMMAND_MODE))
                {
                    APP_MODE.setDefault();
                }
                APP_MODE.enableHighlight(true);
            }

            QKeyEvent *escEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
            QCoreApplication::postEvent(ui->menuBar, escEvent);

            return true;
        }
    }
    else if (event->type() == QKeyEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);

        if (ke->key() == Qt::Key_Alt)
        {
            if (APP_MODE.isTempMode())
            {
                APP_MODE.restore();
                if (0 != (APP_MODE.get() & scene::CAppMode::COMMAND_MODE))
                {
                    APP_MODE.setDefault();
                }
                APP_MODE.enableHighlight(true);
            }

            APP_MODE.storeAndSet(scene::CAppMode::MODE_DENSITY_WINDOW);

            return true;
        }
    }

    return QObject::eventFilter(object, event);
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
    connect(ui->actionSave_STL_Model_in_DICOM_Coordinates, SIGNAL(triggered()), this, SLOT(saveSTLinDicomCoords()));
    connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(print()));
    connect(ui->actionShow_Data_Properties, SIGNAL(triggered()), this, SLOT(showDataProperties()));
    connect(ui->actionSend_Data, SIGNAL(triggered()), this, SLOT(sendDataExpressData()));
	connect(ui->actionReceive_DICOM_Data, SIGNAL(triggered()), this, SLOT(receiveDicomData()));

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
	connect(ui->actionTutorials, SIGNAL(triggered()), this, SLOT(showTutorials()));
	connect(ui->actionFeedback, SIGNAL(triggered()), this, SLOT(showFeedback()));

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
    connect(ui->actionArbitrary_View, SIGNAL(triggered(bool)), this, SLOT(showArbitraryView(bool)));

    // Panels
    connect(ui->actionDensity_Window, SIGNAL(triggered(bool)), this, SLOT(showDensityWindowPanel(bool)));
    connect(ui->actionOrtho_Slices_Panel, SIGNAL(triggered(bool)), this, SLOT(showOrthoSlicesPanel(bool)));
    connect(ui->actionSegmentation_Panel, SIGNAL(triggered(bool)), this, SLOT(showSegmentationPanel(bool)));
    connect(ui->actionVolume_Rendering_Panel, SIGNAL(triggered(bool)), this, SLOT(showVRPanel(bool)));
	connect(ui->actionModels_List_Panel, SIGNAL(triggered(bool)), this, SLOT(showModelsListPanel(bool)));
	connect(ui->actionClose_Active_Panel, SIGNAL(triggered()), this, SLOT(closeActivePanel()));
	connect(ui->actionPrevious_Panel, SIGNAL(triggered()), this, SLOT(prevPanel()));
	connect(ui->actionNext_Panel, SIGNAL(triggered()), this, SLOT(nextPanel()));
    connect(ui->actionTabify_Panels, SIGNAL(triggered()), this, SLOT(autoTabPanels()));

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
    connect(ui->actionArbitrary_Slice, SIGNAL(triggered(bool)), this, SLOT(showArbitrarySlice(bool)));
    connect(ui->actionVolume_Rendering, SIGNAL(triggered(bool)), this, SLOT(showMergedVR(bool)));

    // Workspace switching
    connect(ui->actionWorkspace3DView, SIGNAL(triggered()), this, SLOT(setUpWorkSpace3D()));
    connect(ui->actionWorkspaceTabs, SIGNAL(triggered()), this, SLOT(setUpWorkSpaceTabs()));
    connect(ui->actionWorkspaceGrid, SIGNAL(triggered()), this, SLOT(setUpWorkSpaceGrid()));
    connect(ui->actionWorkspaceSlices, SIGNAL(triggered()), this, SLOT(setUpWorkSpaceSlices()));

    connect(ui->actionFloating3D, SIGNAL(triggered(bool)), this, SLOT(makeFloating3D(bool)));
    connect(ui->actionFloatingAxial, SIGNAL(triggered(bool)), this, SLOT(makeFloatingAxial(bool)));
    connect(ui->actionFloatingCoronal, SIGNAL(triggered(bool)), this, SLOT(makeFloatingCoronal(bool)));
    connect(ui->actionFloatingSagittal, SIGNAL(triggered(bool)), this, SLOT(makeFloatingSagittal(bool)));
    connect(ui->actionFloatingArbitrary, SIGNAL(triggered(bool)), this, SLOT(makeFloatingArbitrary(bool)));

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

	// Segmentation
	connect(ui->actionLimit_Volume, SIGNAL(triggered()), this, SLOT(limitVolume()));
	connect(ui->actionReset, SIGNAL(triggered()), this, SLOT(resetLimit()));
	connect(ui->actionRegions_Visible, SIGNAL(triggered(bool)), this, SLOT(onMenuRegionsVisibleToggled(bool)));
	connect(ui->actionRegions_Contours_Visible, SIGNAL(triggered(bool)), this, SLOT(onMenuContoursVisibleToggled(bool)));
	connect(ui->actionVolume_of_Interest_Visible, SIGNAL(triggered(bool)), this, SLOT(onMenuVOIVisibleToggled(bool)));

	//Model
	connect(ui->actionSmooth, SIGNAL(triggered()), this, SLOT(modelVisualizationSmooth()));
	connect(ui->actionFlat, SIGNAL(triggered()), this, SLOT(modelVisualizationFlat()));
	connect(ui->actionWire, SIGNAL(triggered()), this, SLOT(modelVisualizationWire()));
	connect(ui->actionShow_Surface_Model, SIGNAL(triggered(bool)), this, SLOT(showSurfaceModel(bool)));

    // other signals
    VPL_SIGNAL(SigVREnabledChange).connect(this, &MainWindow::onVREnabledChange);
}

void MainWindow::limitVolume()
{
	// Get the active dataset
	//data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(data::Storage::PatientData::Id));

	data::CObjectPtr< data::CActiveDataSet > ptrDataset(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
	data::CObjectPtr< data::CDensityData > pVolume(APP_STORAGE.getEntry(ptrDataset->getId()));

	data::SVolumeOfInterest initLimits;

	data::CObjectPtr<data::CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(data::Storage::VolumeOfInterestData::Id));

	initLimits.m_MinX = spVOI->getMinX();
	initLimits.m_MinY = spVOI->getMinY();
	initLimits.m_MinZ = spVOI->getMinZ();
	initLimits.m_MaxX = spVOI->getMaxX();
	initLimits.m_MaxY = spVOI->getMaxY();
	initLimits.m_MaxZ = spVOI->getMaxZ();

	// and show the limiter dialog
	CVolumeLimiterDialog vlDlg(this);
	vlDlg.setVolume(pVolume.get(), false);
	vlDlg.setLimits(initLimits);

	if (QDialog::Accepted == vlDlg.exec())
	{
		const data::SVolumeOfInterest limits = vlDlg.getLimits();

		spVOI->setLimits(limits.m_MinX, limits.m_MinY, limits.m_MinZ, limits.m_MaxX, limits.m_MaxY, limits.m_MaxZ);

		VPL_SIGNAL(SigVolumeOfInterestChanged).invoke(true);

		setVOIVisibility(true);
	}
	else
	{
		VPL_SIGNAL(SigVolumeOfInterestChanged).invoke(false);
		setVOIVisibility(true);
	}
}

void MainWindow::resetLimit()
{
	// Get the active dataset
	//data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(data::Storage::PatientData::Id));
	data::CObjectPtr<data::CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(data::Storage::VolumeOfInterestData::Id));

	data::CObjectPtr< data::CActiveDataSet > ptrDataset(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
	data::CObjectPtr< data::CDensityData > pVolume(APP_STORAGE.getEntry(ptrDataset->getId()));

	spVOI->setLimits(0, 0, 0, pVolume->getXSize() - 1, pVolume->getYSize() - 1, pVolume->getZSize() - 1);

	VPL_SIGNAL(SigVolumeOfInterestChanged).invoke(false);

	setVOIVisibility(false);
}

void MainWindow::connectActionsToEventFilter()
{
	QList<QAction*> actions = this->findChildren<QAction*>();

	foreach(QAction *a, actions)
	{
		if (!a->objectName().isEmpty())
		{
			connect(a, SIGNAL(triggered()), &m_eventFilter, SLOT(catchMenuAction()));

			if (!a->shortcut().isEmpty())
				connect(a, SIGNAL(triggered()), this, SLOT(resetAppMode()));
		}
	}
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
    ui->viewsToolBar->addAction(ui->actionArbitrary_View);

    ui->mouseToolBar->addAction(ui->actionDensity_Window_Adjusting);
    ui->mouseToolBar->addAction(ui->actionTrackball_Mode);
    ui->mouseToolBar->addAction(ui->actionObject_Manipulation);
    ui->mouseToolBar->addAction(ui->actionScale_Scene);

    ui->visibilityToolBar->addAction(ui->actionShow_Surface_Model);
    ui->visibilityToolBar->addAction(ui->actionVolume_Rendering);
    ui->visibilityToolBar->addAction(ui->actionAxial_Slice);
    ui->visibilityToolBar->addAction(ui->actionCoronal_Slice);
    ui->visibilityToolBar->addAction(ui->actionSagittal_Slice);
    ui->visibilityToolBar->addAction(ui->actionArbitrary_Slice);

	QAction *pPanels = ui->panelsToolBar->addAction(QIcon(":/svg/svg/page.svg"),tr("Panels"),this, SLOT(showPanelsMenu()));
	pPanels->setToolTip(tr("Change panel visibility"));
    ui->panelsToolBar->addAction(ui->actionDensity_Window);
    ui->panelsToolBar->addAction(ui->actionOrtho_Slices_Panel);
    ui->panelsToolBar->addAction(ui->actionVolume_Rendering_Panel);

    if (!m_segmentationPluginsLoaded)
    {
        ui->panelsToolBar->addAction(ui->actionModels_List_Panel);
        ui->panelsToolBar->addAction(ui->actionSegmentation_Panel);
    }
    
    QAction* pFullScreen = ui->appToolBar->addAction(QIcon(":/svg/svg/fullscreen.svg"),tr("Fullscreen"));
	pFullScreen->setObjectName("actionFullScreen");
    pFullScreen->setCheckable(true);
    connect(pFullScreen,SIGNAL(triggered(bool)),this,SLOT(fullscreen(bool)));
}

void MainWindow::workspacesEnabler()
{
    ui->actionWorkspace3DView->blockSignals(true);
    ui->actionWorkspaceGrid->blockSignals(true);
    ui->actionWorkspaceTabs->blockSignals(true);
    ui->actionWorkspaceSlices->blockSignals(true);
    ui->actionWorkspace3DView->setChecked(Workspace3D==m_nLayoutType);
    ui->actionWorkspaceGrid->setChecked(WorkspaceGrid==m_nLayoutType);
    ui->actionWorkspaceTabs->setChecked(WorkspaceTabs==m_nLayoutType);
    ui->actionWorkspaceSlices->setChecked(WorkspaceSlices==m_nLayoutType);
    ui->actionWorkspace3DView->blockSignals(false);
    ui->actionWorkspaceGrid->blockSignals(false);
    ui->actionWorkspaceTabs->blockSignals(false);
    ui->actionWorkspaceSlices->blockSignals(false);
}

// setups windows, saves and loads visibility and positions of dock windows from QSettings
void MainWindow::setUpWorkSpace3D()
{
    if (m_nLayoutType == Workspace3D)
    {
        return;
    }

    saveFloatingWindowsGeometry();

    makeWndFloating(View3D, false, false);
    makeWndFloating(ViewXY, false, false);
    makeWndFloating(ViewXZ, false, false);
    makeWndFloating(ViewYZ, false, false);
    makeWndFloating(ViewARB, false, false);

    setUpWorkspace();

    m_viewsToShow.clear();
    m_viewsToShow.push_back(sWindowInfo(ViewXY, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewXZ, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewYZ, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewARB, false, true, true));

    writeLayoutSettings(m_nLayoutType,true);
    m_nLayoutType=Workspace3D;
    setUpdatesEnabled(false); // prevent screen flicker
    setUpWorkspace();
    readLayoutSettings(true);
    //loadFloatingWindowsGeometry();
    setUpdatesEnabled(true);
}

void MainWindow::setUpWorkSpaceTabs()
{
    if (m_nLayoutType == WorkspaceTabs)
    {
        return;
    }

    saveFloatingWindowsGeometry();

    makeWndFloating(View3D, false, false);
    makeWndFloating(ViewXY, false, false);
    makeWndFloating(ViewXZ, false, false);
    makeWndFloating(ViewYZ, false, false);
    makeWndFloating(ViewARB, false, false);

    setUpWorkspace();

    m_viewsToShow.clear();
    m_viewsToShow.push_back(sWindowInfo(View3D, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewXY, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewXZ, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewYZ, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewARB, false, true, true));

    writeLayoutSettings(m_nLayoutType,true);
    m_nLayoutType=WorkspaceTabs;
    setUpdatesEnabled(false); // prevent screen flicker
    setUpWorkspace();
    readLayoutSettings(true);
    //loadFloatingWindowsGeometry();
    setUpdatesEnabled(true);
}

void MainWindow::setUpWorkSpaceGrid()
{
    if (m_nLayoutType == WorkspaceGrid)
    {
        return;
    }

    saveFloatingWindowsGeometry();

    makeWndFloating(View3D, false, false);
    makeWndFloating(ViewXY, false, false);
    makeWndFloating(ViewXZ, false, false);
    makeWndFloating(ViewYZ, false, false);
    makeWndFloating(ViewARB, false, false);

    setUpWorkspace();

    m_viewsToShow.clear();
    m_viewsToShow.push_back(sWindowInfo(ViewXY, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(View3D, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewXZ, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewYZ, false, true, true));

    writeLayoutSettings(m_nLayoutType,true);
    m_nLayoutType=WorkspaceGrid;
    setUpdatesEnabled(false); // prevent screen flicker
    setUpWorkspace();
    readLayoutSettings(true);
    //loadFloatingWindowsGeometry();
    setUpdatesEnabled(true);
}

void MainWindow::setUpWorkSpaceSlices()
{
    if (m_nLayoutType == WorkspaceSlices)
    {
        return;
    }

    saveFloatingWindowsGeometry();

    makeWndFloating(View3D, false, false);
    makeWndFloating(ViewXY, false, false);
    makeWndFloating(ViewXZ, false, false);
    makeWndFloating(ViewYZ, false, false);
    makeWndFloating(ViewARB, false, false);

    setUpWorkspace();

    m_viewsToShow.clear();
    m_viewsToShow.push_back(sWindowInfo(ViewXY, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewARB, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewXZ, false, true, true));
    m_viewsToShow.push_back(sWindowInfo(ViewYZ, false, true, true));

    writeLayoutSettings(m_nLayoutType, true);
    m_nLayoutType = WorkspaceSlices;
    setUpdatesEnabled(false); // prevent screen flicker
    setUpWorkspace();
    readLayoutSettings(true);
    //loadFloatingWindowsGeometry();
    setUpdatesEnabled(true);
}

void MainWindow::makeFloating3D(bool bFloating)
{
    makeWndFloating(View3D, bFloating);
}

void MainWindow::makeFloatingAxial(bool bFloating)
{
    makeWndFloating(ViewXY, bFloating);
}

void MainWindow::makeFloatingCoronal(bool bFloating)
{
    makeWndFloating(ViewXZ, bFloating);
}

void MainWindow::makeFloatingSagittal(bool bFloating)
{
    makeWndFloating(ViewYZ, bFloating);
}

void MainWindow::makeFloatingArbitrary(bool bFloating)
{
    makeWndFloating(ViewARB, bFloating);
}

int MainWindow::getIndexOfWndInLayout(unsigned int wnd)
{
    int index = -1;
    for (size_t i = 0; i < m_viewsToShow.size(); ++i)
    {
        if (m_viewsToShow[i].windowType == wnd)
        {
            index = i;
            break;
        }
    }

    return index;
}

void MainWindow::makeWndFloating(unsigned int wnd, bool bFloating, bool updateWorkspace)
{
    if (wnd < 0)
    {
        return;
    }

    switch (wnd)
    {
        case View3D:
        {
            ui->actionFloating3D->blockSignals(true);
            ui->actionFloating3D->setChecked(bFloating);
            ui->actionFloating3D->blockSignals(false);
        }
        break;
        case ViewXY:
        {
            ui->actionFloatingAxial->blockSignals(true);
            ui->actionFloatingAxial->setChecked(bFloating);
            ui->actionFloatingAxial->blockSignals(false);
        }
        break;
        case ViewXZ:
        {
            ui->actionFloatingCoronal->blockSignals(true);
            ui->actionFloatingCoronal->setChecked(bFloating);
            ui->actionFloatingCoronal->blockSignals(false);
        }
        break;
        case ViewYZ:
        {
            ui->actionFloatingSagittal->blockSignals(true);
            ui->actionFloatingSagittal->setChecked(bFloating);
            ui->actionFloatingSagittal->blockSignals(false);
        }
        break;
        case ViewARB:
        default:
        {
            ui->actionFloatingArbitrary->blockSignals(true);
            ui->actionFloatingArbitrary->setChecked(bFloating);
            ui->actionFloatingArbitrary->blockSignals(false);
        }
        break;
    }

    saveFloatingWindowsGeometry();

    int index = getIndexOfWndInLayout(wnd);

    if (index >= 0 && index < m_viewsToShow.size())
    {
        if (m_viewsToShow[index].bShowSubstitute)
        {
            m_viewsToShow[index].windowType = wnd;
            m_viewsToShow[index].bFloating = bFloating;
            m_viewsToShow[index].bDirty = true;
        }
        else
        {
            m_viewsToShow.erase(m_viewsToShow.begin() + index);
        }
    }
    else
    {
        m_viewsToShow.push_back(sWindowInfo(wnd, bFloating, false, true));
    }

    if (updateWorkspace)
    {
        setUpWorkspace();

        loadFloatingWindowsGeometry();
    }
}

void MainWindow::saveFloatingWindowsGeometry()
{
    std::vector<bool> floating;
    for (size_t i = 0; i < m_wndViews.size(); ++i)
    {
        floating.push_back(false);
    }

    for (size_t i = 0; i < m_viewsToShow.size(); ++i)
    {
        if (m_viewsToShow[i].bFloating)
        {
            floating[m_viewsToShow[i].windowType] = true;
        }
    }

    if (floating[View3D])
    {
        QRect geometry(50, 50, 700, 600);
        bool maximized = false;
        bool minimized = false;
        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[View3D]->parent());
        if (parent)
        {
            geometry = parent->geometry();
            maximized = parent->isMaximized();
            minimized = parent->isMinimized();
        }

        QSettings settings;
        settings.beginGroup("Floating3D");
        settings.setValue("geometry", geometry);
        settings.setValue("maximized", maximized);
        settings.setValue("minimized", minimized);
        settings.endGroup();
    }

    if (floating[ViewXY])
    {
        QRect geometry(50, 50, 700, 600);
        bool maximized = false;
        bool minimized = false;
        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[ViewXY]->parent());
        if (parent)
        {
            geometry = parent->geometry();
            maximized = parent->isMaximized();
            minimized = parent->isMinimized();
        }

        QSettings settings;
        settings.beginGroup("FloatingXY");
        settings.setValue("geometry", geometry);
        settings.setValue("maximized", maximized);
        settings.setValue("minimized", minimized);
        settings.endGroup();
    }

    if (floating[ViewXZ])
    {
        QRect geometry(50, 50, 700, 600);
        bool maximized = false;
        bool minimized = false;
        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[ViewXZ]->parent());
        if (parent)
        {
            geometry = parent->geometry();
            maximized = parent->isMaximized();
            minimized = parent->isMinimized();
        }

        QSettings settings;
        settings.beginGroup("FloatingXZ");
        settings.setValue("geometry", geometry);
        settings.setValue("maximized", maximized);
        settings.setValue("minimized", minimized);
        settings.endGroup();
    }

    if (floating[ViewYZ])
    {
        QRect geometry(50, 50, 700, 600);
        bool maximized = false;
        bool minimized = false;
        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[ViewYZ]->parent());
        if (parent)
        {
            geometry = parent->geometry();
            maximized = parent->isMaximized();
            minimized = parent->isMinimized();

        }
        QSettings settings;
        settings.beginGroup("FloatingYZ");
        settings.setValue("geometry", geometry);
        settings.setValue("maximized", maximized);
        settings.setValue("minimized", minimized);
        settings.endGroup();
    }

    if (floating[ViewARB])
    {
        QRect geometry(50, 50, 700, 600);
        bool maximized = false;
        bool minimized = false;
        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[ViewARB]->parent());
        if (parent)
        {
            geometry = parent->geometry();
            maximized = parent->isMaximized();
            minimized = parent->isMinimized();

        }
        QSettings settings;
        settings.beginGroup("FloatingARB");
        settings.setValue("geometry", geometry);
        settings.setValue("maximized", maximized);
        settings.setValue("minimized", minimized);
        settings.endGroup();
    }
}

void MainWindow::loadFloatingWindowsGeometry()
{
    std::vector<bool> floating;
    for (size_t i = 0; i < m_wndViews.size(); ++i)
    {
        floating.push_back(false);
    }

    for (size_t i = 0; i < m_viewsToShow.size(); ++i)
    {
        if (m_viewsToShow[i].bFloating)
        {
            floating[m_viewsToShow[i].windowType] = true;
        }
    }

    if (floating[View3D])
    {
        QSettings settings;
        settings.beginGroup("Floating3D");
        QRect geometry = settings.value("geometry", QRect(50, 50, 700, 600)).toRect();
        bool maximized = settings.value("maximized", false).toBool();
        bool minimized = settings.value("minimized", false).toBool();
        settings.endGroup();

        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[View3D]->parent());
        if (parent)
        {
            parent->setGeometry(geometry);

            if (maximized)
            {
                parent->showMaximized();
            }
            if (minimized)
            {
                parent->showMinimized();
            }
        }
    }

    if (floating[ViewXY])
    {
        QSettings settings;
        settings.beginGroup("FloatingXY");
        QRect geometry = settings.value("geometry", QRect(50, 50, 700, 600)).toRect();
        bool maximized = settings.value("maximized", false).toBool();
        bool minimized = settings.value("minimized", false).toBool();
        settings.endGroup();

        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[ViewXY]->parent());
        if (parent)
        {
            parent->setGeometry(geometry);

            if (maximized)
            {
                parent->showMaximized();
            }
            if (minimized)
            {
                parent->showMinimized();
            }
        }
    }

    if (floating[ViewXZ])
    {
        QSettings settings;
        settings.beginGroup("FloatingXZ");
        QRect geometry = settings.value("geometry", QRect(50, 50, 700, 600)).toRect();
        bool maximized = settings.value("maximized", false).toBool();
        bool minimized = settings.value("minimized", false).toBool();
        settings.endGroup();

        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[ViewXZ]->parent());
        if (parent)
        {
            parent->setGeometry(geometry);

            if (maximized)
            {
                parent->showMaximized();
            }
            if (minimized)
            {
                parent->showMinimized();
            }
        }
    }

    if (floating[ViewYZ])
    {
        QSettings settings;
        settings.beginGroup("FloatingYZ");
        QRect geometry = settings.value("geometry", QRect(50, 50, 700, 600)).toRect();
        bool maximized = settings.value("maximized", false).toBool();
        bool minimized = settings.value("minimized", false).toBool();
        settings.endGroup();

        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[ViewYZ]->parent());
        if (parent)
        {
            parent->setGeometry(geometry);

            if (maximized)
            {
                parent->showMaximized();
            }
            if (minimized)
            {
                parent->showMinimized();
            }
        }
    }

    if (floating[ViewARB])
    {
        QSettings settings;
        settings.beginGroup("FloatingARB");
        QRect geometry = settings.value("geometry", QRect(50, 50, 700, 600)).toRect();
        bool maximized = settings.value("maximized", false).toBool();
        bool minimized = settings.value("minimized", false).toBool();
        settings.endGroup();

        QMainWindow* parent = qobject_cast<QMainWindow*>(m_wndViews[ViewARB]->parent());
        if (parent)
        {
            parent->setGeometry(geometry);

            if (maximized)
            {
                parent->showMaximized();
            }
            if (minimized)
            {
                parent->showMinimized();
            }
        }
    }
}

static void setSliceDockWidgetFeatures(QDockWidget* widget)
{
    Q_ASSERT(NULL!=widget);
    widget->setAllowedAreas(Qt::NoDockWidgetArea);
    widget->setFeatures(0);
    //widget->setAllowedAreas(Qt::AllDockWidgetAreas);
    //widget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    widget->setMinimumSize(200,150);
}

void MainWindow::removeViewsParentWidget(QWidget* view)
{
    if (NULL==view) return;
    QDockWidget *pDockW = NULL;
    QMainWindow* window = NULL;
    QWidget     *pParent = view->parentWidget();
    // find window parent which is a dock window
    while(NULL!=pParent)
    {
        if (pDockW == NULL)
        {
            pDockW = qobject_cast<QDockWidget*>(pParent);
        }

        if (window == NULL)
        {
            window = qobject_cast<QMainWindow*>(pParent);
        }

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
    }

    if (NULL != window)
    {
        if (window->objectName() == "FloatingWindow")
        {
            window->setProperty("StayFloating", true);
            window->close();
        }
    }

    Q_ASSERT(pDockW);
}

void MainWindow::removeDockWidgetAndWindow(QDockWidget* dockWidget)
{
    if (NULL == dockWidget)
    {
        return;
    }

    QMainWindow* window = NULL;
    QWidget     *pParent = dockWidget->parentWidget();
    // find window parent which is a dock window
    while (NULL != pParent)
    {
        if (window == NULL)
        {
            window = qobject_cast<QMainWindow*>(pParent);
        }

        pParent = pParent->parentWidget();
    }

    if (NULL != dockWidget)
    {
        // undock dock widget and hide it        
        removeDockWidget(dockWidget);
        dockWidget->hide();
        // remove any custom title bar
        QWidget* pCustomTitle = dockWidget->titleBarWidget();
        if (NULL != pCustomTitle)
        {
            dockWidget->setTitleBarWidget(NULL);
            delete pCustomTitle;
        }
    }

    if (NULL != window)
    {
        if (window->objectName() == "FloatingWindow")
        {
            window->setProperty("StayFloating", true);
            window->close();
        }
    }

    Q_ASSERT(dockWidget);
}

QDockWidget* MainWindow::getParentDockWidget(QWidget* view)
{
    if (NULL==view) return nullptr;
    QDockWidget *pDockW = NULL;
    QWidget* pParent = view->parentWidget();
    // find window parent which is a dock window
    while (NULL != pParent)
    {
        pDockW = qobject_cast<QDockWidget*>(pParent);
        if (pDockW)
            break;
        pParent = pParent->parentWidget();
    }
    return pDockW;
}

static void removeQLayoutChildren(QLayout* layout)
{
    QLayoutItem* child;
    while (layout->count() != 0)
    {
        child = layout->takeAt(0);
        if (child->layout() != 0)
        {
            removeQLayoutChildren(child->layout());
        }
        else if (child->widget() != 0)
        {
            delete child->widget();
        }

        delete child;
    }
}

CCustomDockWidgetTitle* MainWindow::createTitleBar(unsigned int wnd, unsigned int commonButton, bool showSlider)
{
    unsigned int sliderType = DWT_SLIDER_VR;

    switch (wnd)
    {
    case ViewARB:
    {
        sliderType = DWT_SLIDER_ARB;
    }
    break;
    case ViewXY:
    {
        sliderType = DWT_SLIDER_XY;
    }
    break;
    case ViewXZ:
    {
        sliderType = DWT_SLIDER_XZ;
    }
    break;
    case ViewYZ:
    {
        sliderType = DWT_SLIDER_YZ;
    }
    break;
    case View3D:
    default:
    {
        sliderType = DWT_SLIDER_VR;
    }
    break;
    }

    return new CCustomDockWidgetTitle(wnd, showSlider ? commonButton | sliderType : commonButton);
}

void MainWindow::showViewInWindow(int view, int window)
{
    if (view == window)
    {
        makeWndFloating(view, false);
    }
    else
    {
        // backup
        std::vector<sWindowInfo> viewToShowBackup = m_viewsToShow;

        saveFloatingWindowsGeometry();

        /*makeWndFloating(View3D, false, false);
        makeWndFloating(ViewXY, false, false);
        makeWndFloating(ViewXZ, false, false);
        makeWndFloating(ViewYZ, false, false);
        makeWndFloating(ViewARB, false, false);*/

        makeWndFloating(view, false, false);

        setUpWorkspace();

        m_viewsToShow.clear();

        sWindowInfo infoView(-1, false, false, false);
        sWindowInfo infoWindow(-1, false, false, false);

        bool addReplacedView = true;
        bool addViewWithoutSubstitute = false;

        for (int i = 0; i < viewToShowBackup.size(); ++i)
        {
            if (viewToShowBackup[i].windowType == view)
            {
                infoView = viewToShowBackup[i];
            }
            else if (viewToShowBackup[i].windowType == window)
            {
                infoWindow = viewToShowBackup[i];
            }
        }

        if (infoView.windowType == -1)
        {
            infoView.windowType = view;
            infoView.bFloating = false;
            infoView.bShowSubstitute = true;
            infoView.bDirty = true;

            if (infoWindow.bFloating)
            {
                addViewWithoutSubstitute = true;
            }
        }
        else
        {
            if (!infoView.bShowSubstitute)
            {
                addReplacedView = false;

                if (infoWindow.bFloating)
                {
                    addViewWithoutSubstitute = true;
                }
            }

            infoView.bFloating = false;
            infoView.bShowSubstitute = true;
            infoView.bDirty = true;
        }

        /*if (infoWindow.windowType != -1)
        {
            if (infoWindow.bFloating)
            {
                infoWindow.bShowSubstitute = false;
            }
        }*/

        for (int i = 0; i < viewToShowBackup.size(); ++i)
        {
            if (viewToShowBackup[i].windowType == view)
            {
                if (addReplacedView)
                {
                    m_viewsToShow.push_back(infoWindow);
                }
            }
            else if (viewToShowBackup[i].windowType == window)
            {
                m_viewsToShow.push_back(infoView);
            }
            else
            {
                m_viewsToShow.push_back(viewToShowBackup[i]);
            }
        }

        if (addViewWithoutSubstitute && infoWindow.bFloating)
        {
            infoWindow.bShowSubstitute = false;
            m_viewsToShow.push_back(infoWindow);
        }

        //writeLayoutSettings(m_nLayoutType, true);
        setUpdatesEnabled(false); // prevent screen flicker
        setUpWorkspace();
        //readLayoutSettings(true);
        loadFloatingWindowsGeometry();
        setUpdatesEnabled(true);
    }
}

void MainWindow::setUpWorkspace()
{
    assert(m_wndViews.size() == m_wndViewsSubstitute.size());

    bool bTimer = m_timer.isActive();

    if (bTimer)
    {
        m_timer.stop();
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    Q_ASSERT(NULL != m_3DView);
    Q_ASSERT(NULL != m_OrthoXYSlice);
    Q_ASSERT(NULL != m_OrthoXZSlice);
    Q_ASSERT(NULL != m_OrthoYZSlice);
    Q_ASSERT(NULL != m_ArbitrarySlice);

    ui->actionFloating3D->setVisible(true);

    QWidget* oldCentralWidget = m_realCentralWidget;

    // remove/hide all existing windows
    /*removeViewsParentWidget(m_3DView);
    removeViewsParentWidget(m_OrthoXYSlice);
    removeViewsParentWidget(m_OrthoXZSlice);
    removeViewsParentWidget(m_OrthoYZSlice);
    removeViewsParentWidget(m_ArbitrarySlice);*/

    std::vector<bool> viewVisible;

    size_t wndCnt = m_wndViews.size();

    for (size_t i = 0; i < wndCnt; ++i)
    {
        sWindowInfo info(i, false, false, false);
        auto it = std::find(m_viewsToShow.begin(), m_viewsToShow.end(), info);
        if (it != m_viewsToShow.end())
        {
            if (it->bDirty)
            {
                removeDockWidgetAndWindow(m_wndViews[i]);
                m_wndViews[i]->setParent(0);
                setSliceDockWidgetFeatures(m_wndViews[i]);
            }
        }
        else
        {
            removeDockWidgetAndWindow(m_wndViews[i]);
            m_wndViews[i]->setParent(0);
            setSliceDockWidgetFeatures(m_wndViews[i]);
        }

        removeDockWidget(m_wndViewsSubstitute[i]);
        setSliceDockWidgetFeatures(m_wndViewsSubstitute[i]);
        viewVisible.push_back(false);
        m_wndViewsSubstitute[i]->setParent(0);

        QLayout* layout = m_wndViewsSubstitute[i]->widget()->layout();
        removeQLayoutChildren(layout);
    }

    bool floating3DView = ui->actionFloating3D->isChecked();
    bool floatingXYView = ui->actionFloatingAxial->isChecked();
    bool floatingXZView = ui->actionFloatingCoronal->isChecked();
    bool floatingYZView = ui->actionFloatingSagittal->isChecked();
    bool floatingArbitraryView = ui->actionFloatingArbitrary->isChecked();

    if (m_realCentralWidget)
    {
        m_centralWidget->layout()->removeWidget(m_realCentralWidget);
    }

    if (oldCentralWidget && oldCentralWidget != m_wndViews[View3D])
    {
        delete oldCentralWidget;
    }

    // recreate dock widgets for all views
    switch(m_nLayoutType)
    {
    case WorkspaceTabs:
        {
            QTabWidget* newCenter = new QTabWidget();
            newCenter->setTabPosition(QTabWidget::South);
            newCenter->setIconSize(QSize(16, 16));

            for (size_t i = 0; i < m_viewsToShow.size(); ++i)
            {
                const unsigned int viewIndex = m_viewsToShow[i].windowType;
                const bool viewFloating = m_viewsToShow[i].bFloating;
                const bool showSubstitute = m_viewsToShow[i].bShowSubstitute;
                const bool dirty = m_viewsToShow[i].bDirty;

                if (viewFloating)
                {
                    if (dirty)
                    {
                        m_wndViews[viewIndex]->setAllowedAreas(Qt::AllDockWidgetAreas);

                        CFloatingMainWindow* w = new CFloatingMainWindow(viewIndex, viewIndex != View3D);
                        w->setObjectName("FloatingWindow");
                        w->addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, m_wndViews[viewIndex]);
                        m_wndViews[viewIndex]->setParent(w);
                        m_wndViews[viewIndex]->show();
                        w->show();

                        CCustomDockWidgetTitle* title = createTitleBar(viewIndex);
                        title->setPinned(false);
                        m_wndViews[viewIndex]->setTitleBarWidget(title);
                    }

                    m_viewsToShow[i].bDirty = false;

                    if (showSubstitute)
                    {
                        viewVisible[viewIndex] = true;

                        CCustomDockWidgetTitle* title = createTitleBar(viewIndex, DWT_BUTTONS_VIEW, false);
                        m_wndViewsSubstitute[viewIndex]->setTitleBarWidget(title);

                        newCenter->addTab(m_wndViewsSubstitute[viewIndex], m_wndViewsSubstitute[viewIndex]->windowTitle());
                    }
                }
                else
                {
                    viewVisible[viewIndex] = true;

                    newCenter->addTab(m_wndViews[viewIndex], m_wndViews[viewIndex]->windowTitle());

                    CCustomDockWidgetTitle* title = createTitleBar(viewIndex, DWT_BUTTONS_PIN | DWT_BUTTONS_VIEW);
                    title->setPinned(true);
                    m_wndViews[viewIndex]->setTitleBarWidget(title);
                }
            }

            m_centralWidget->layout()->addWidget(newCenter);
            m_realCentralWidget = newCenter;
        }
        break;
    case WorkspaceGrid:
    case WorkspaceSlices:
        {
            assert(m_viewsToShow.size() >= 4);

            QSplitter* vertical = new QSplitter(Qt::Vertical);
            CSyncedSplitter* horizontalTop = new CSyncedSplitter(Qt::Horizontal);
            CSyncedSplitter* horizontalBottom = new CSyncedSplitter(Qt::Horizontal);

            for (size_t i = 0; i < m_viewsToShow.size(); ++i)
            {
                const unsigned int viewIndex = m_viewsToShow[i].windowType;
                const bool viewFloating = m_viewsToShow[i].bFloating;
                const bool showSubstitute = m_viewsToShow[i].bShowSubstitute;
                const bool dirty = m_viewsToShow[i].bDirty;

                if (viewFloating)
                {
                    if (dirty)
                    {
                        m_wndViews[viewIndex]->setAllowedAreas(Qt::AllDockWidgetAreas);

                        CFloatingMainWindow* w = new CFloatingMainWindow(viewIndex, viewIndex != View3D);
                        w->setObjectName("FloatingWindow");
                        w->addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, m_wndViews[viewIndex]);
                        m_wndViews[viewIndex]->setParent(w);
                        m_wndViews[viewIndex]->show();
                        w->show();

                        CCustomDockWidgetTitle* title = createTitleBar(viewIndex);
                        title->setPinned(false);
                        m_wndViews[viewIndex]->setTitleBarWidget(title);

                        m_viewsToShow[i].bDirty = false;
                    }

                    if (showSubstitute)
                    {
                        if (i < 2)
                        {
                            viewVisible[viewIndex] = true;
                            horizontalTop->addWidget(m_wndViewsSubstitute[viewIndex]);
                        }
                        else if (i < 4)
                        {
                            viewVisible[viewIndex] = true;
                            horizontalBottom->addWidget(m_wndViewsSubstitute[viewIndex]);
                        }


                        CCustomDockWidgetTitle* title = createTitleBar(viewIndex, DWT_BUTTONS_VIEW, false);
                        m_wndViewsSubstitute[viewIndex]->setTitleBarWidget(title);

                        m_wndViewsSubstitute[viewIndex]->show();
                    }
                }
                else
                {
                    viewVisible[viewIndex] = true;

                    if (i < 2)
                    {
                        horizontalTop->addWidget(m_wndViews[viewIndex]);
                    }
                    else
                    {
                        horizontalBottom->addWidget(m_wndViews[viewIndex]);
                    }

                    m_wndViews[viewIndex]->show();

                    CCustomDockWidgetTitle* title = createTitleBar(viewIndex, DWT_BUTTONS_MAXIMIZE | DWT_BUTTONS_PIN | DWT_BUTTONS_VIEW);
                    title->setPinned(true);
                    m_wndViews[viewIndex]->setTitleBarWidget(title);
                }
            }

            horizontalTop->setStretchFactor(0, 1);
            horizontalTop->setStretchFactor(1, 1);
            horizontalBottom->setStretchFactor(0, 1);
            horizontalBottom->setStretchFactor(1, 1);
            vertical->addWidget(horizontalTop);
            vertical->addWidget(horizontalBottom);
            QList<int> sizes;
            sizes.append(1);
            sizes.append(1);
            horizontalTop->setSizes(sizes);
            horizontalBottom->setSizes(sizes);
            vertical->setStretchFactor(0, 5);
            vertical->setStretchFactor(1, 5);

            m_centralWidget->layout()->addWidget(vertical);
            m_realCentralWidget = vertical;

            connect(horizontalTop, SIGNAL(splitterMoved(int, int)), this, SLOT(topSplitterMoved(int, int)));
            connect(horizontalBottom, SIGNAL(splitterMoved(int, int)), this, SLOT(bottomSplitterMoved(int, int)));

            vertical->handle(1)->setAttribute(Qt::WA_Hover);
            horizontalTop->handle(1)->setAttribute(Qt::WA_Hover);
            horizontalBottom->handle(1)->setAttribute(Qt::WA_Hover);
            
        }
        break;
    case Workspace3D:
    default:
        {
            ui->actionFloating3D->setVisible(false);

            {
                viewVisible[View3D] = true;

                /*if (floating3DView)
                {
                    m_wndViews[View3D]->setAllowedAreas(Qt::AllDockWidgetAreas);

                    m_wndViewsSubstitute[View3D]->setAllowedAreas(Qt::AllDockWidgetAreas);
                    m_wndViewsSubstitute[View3D]->setFeatures(QDockWidget::DockWidgetMovable);

                    m_centralWidget->layout()->addWidget(m_wndViewsSubstitute[View3D]);
                    m_realCentralWidget = m_wndViewsSubstitute[View3D];
                    m_wndViewsSubstitute[View3D]->show();

                    CFloatingMainWindow* w = new CFloatingMainWindow(View3D);
                    w->setObjectName("FloatingWindow");
                    w->addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, m_wndViews[View3D]);
                    m_wndViews[View3D]->setParent(w);
                    m_wndViews[View3D]->show();
                    w->show();

                    CCustomDockWidgetTitle* title = createTitleBar(View3D);
                    title->setPinned(true);
                    m_wndViews[View3D]->setTitleBarWidget(title);
                }
                else*/
                {
                    m_wndViews[View3D]->setAllowedAreas(Qt::AllDockWidgetAreas);
                    m_wndViews[View3D]->setFeatures(QDockWidget::DockWidgetMovable);

                    m_centralWidget->layout()->addWidget(m_wndViews[View3D]);
                    m_realCentralWidget = m_wndViews[View3D];
                    m_wndViews[View3D]->show();

                    CCustomDockWidgetTitle* title = createTitleBar(View3D, 0);
                    title->setPinned(true);
                    m_wndViews[View3D]->setTitleBarWidget(title);
                }
            }

            for (size_t i = 0; i < m_viewsToShow.size(); ++i)
            {
                const unsigned int viewIndex = m_viewsToShow[i].windowType;
                const bool viewFloating = m_viewsToShow[i].bFloating;
                const bool viewSubstitute = m_viewsToShow[i].bShowSubstitute;
                const bool dirty = m_viewsToShow[i].bDirty;

                if (viewFloating)
                {
                    m_wndViews[viewIndex]->setAllowedAreas(Qt::AllDockWidgetAreas);

                    if (viewSubstitute)
                    {
                        viewVisible[viewIndex] = true;

                        m_wndViewsSubstitute[viewIndex]->setAllowedAreas(Qt::AllDockWidgetAreas);
                        m_wndViewsSubstitute[viewIndex]->setFeatures(QDockWidget::DockWidgetMovable);

                        CCustomDockWidgetTitle* title = createTitleBar(viewIndex, DWT_BUTTONS_VIEW, false);
                        title->removeActionFromViewButton(View3D);
                        m_wndViewsSubstitute[viewIndex]->setTitleBarWidget(title);

                        addDockWidget(Qt::LeftDockWidgetArea, m_wndViewsSubstitute[viewIndex]);
                        m_wndViewsSubstitute[viewIndex]->show();
                    }

                    if (dirty)
                    {
                        CFloatingMainWindow* w = new CFloatingMainWindow(viewIndex, viewIndex != View3D);
                        w->setObjectName("FloatingWindow");
                        w->addDockWidget(Qt::DockWidgetArea::TopDockWidgetArea, m_wndViews[viewIndex]);
                        m_wndViews[viewIndex]->setParent(w);
                        m_wndViews[viewIndex]->show();
                        w->show();

                        CCustomDockWidgetTitle* title = createTitleBar(viewIndex);
                        title->setPinned(false);
                        m_wndViews[viewIndex]->setTitleBarWidget(title);

                        m_viewsToShow[i].bDirty = false;
                    }
                }
                else
                {
                    viewVisible[viewIndex] = true;

                    m_wndViews[viewIndex]->setAllowedAreas(Qt::AllDockWidgetAreas);
                    m_wndViews[viewIndex]->setFeatures(QDockWidget::DockWidgetMovable);

                    addDockWidget(Qt::LeftDockWidgetArea, m_wndViews[viewIndex]);
                    m_wndViews[viewIndex]->show();

                    CCustomDockWidgetTitle* title = createTitleBar(viewIndex, DWT_BUTTONS_PIN | DWT_BUTTONS_VIEW);
                    title->setPinned(true);
                    title->removeActionFromViewButton(View3D);
                    m_wndViews[viewIndex]->setTitleBarWidget(title);
                }
            }
        }
        break;
    }

    /*if (floating3DView)
    {
        QLayout* layout = m_wndViewsSubstitute[View3D]->widget()->layout();
        removeQLayoutChildren(layout);

        QVBoxLayout* vbLayout = qobject_cast<QVBoxLayout*>(layout);
        vbLayout->setAlignment(Qt::AlignCenter);
        if (NULL != vbLayout)
        {
            QPushButton* show3D = new QPushButton();
            show3D->setText("Show 3D View");
            show3D->setIcon(QIcon(":/svg/svg/icon3d.svg"));
            show3D->setIconSize(QSize(34, 34));
            show3D->setProperty("View", View3D);
            show3D->setProperty("Parent", View3D);
            show3D->setMinimumHeight(50);
            show3D->setMinimumWidth(150);
            QObject::connect(show3D, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(show3D);

            if (!viewVisible[ViewXY])
            {
                QPushButton* showXY = new QPushButton();
                showXY->setText("Show Axial View");
                showXY->setIcon(QIcon(":/svg/svg/iconxy.svg"));
                showXY->setIconSize(QSize(34, 34));
                showXY->setProperty("View", ViewXY);
                showXY->setProperty("Parent", View3D);
                showXY->setMinimumHeight(50);
                QObject::connect(showXY, SIGNAL(clicked()), this, SLOT(showSelectedView()));

                vbLayout->addWidget(showXY);
            }

            if (!viewVisible[ViewXZ])
            {
                QPushButton* showXZ = new QPushButton();
                showXZ->setText("Show Coronal View");
                showXZ->setIcon(QIcon(":/svg/svg/iconxz.svg"));
                showXZ->setIconSize(QSize(34, 34));
                showXZ->setProperty("View", ViewXZ);
                showXZ->setProperty("Parent", View3D);
                showXZ->setMinimumHeight(50);
                QObject::connect(showXZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

                vbLayout->addWidget(showXZ);
            }

            if (!viewVisible[ViewYZ])
            {
                QPushButton* showYZ = new QPushButton();
                showYZ->setText("Show Sagittal View");
                showYZ->setIcon(QIcon(":/svg/svg/iconyz.svg"));
                showYZ->setIconSize(QSize(34, 34));
                showYZ->setProperty("View", ViewYZ);
                showYZ->setProperty("Parent", View3D);
                showYZ->setMinimumHeight(50);
                QObject::connect(showYZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

                vbLayout->addWidget(showYZ);
            }

            if (!viewVisible[ViewARB])
            {
                QPushButton* showArbitrary = new QPushButton();
                showArbitrary->setText("Show Arbitrary View");
                showArbitrary->setIcon(QIcon(":/svg/svg/iconarb.svg"));
                showArbitrary->setIconSize(QSize(34, 34));
                showArbitrary->setProperty("View", ViewARB);
                showArbitrary->setProperty("Parent", View3D);
                showArbitrary->setMinimumHeight(50);
                QObject::connect(showArbitrary, SIGNAL(clicked()), this, SLOT(showSelectedView()));

                vbLayout->addWidget(showArbitrary);
            }
        }
    }

    if (floatingXYView)
    {
        QLayout* layout = m_wndViewsSubstitute[ViewXY]->widget()->layout();
        removeQLayoutChildren(layout);

        QVBoxLayout* vbLayout = qobject_cast<QVBoxLayout*>(layout);
        vbLayout->setAlignment(Qt::AlignCenter);
        if (NULL != vbLayout)
        {
            QPushButton* showXY = new QPushButton();
            showXY->setText("Show Axial View");
            showXY->setIcon(QIcon(":/svg/svg/iconxy.svg"));
            showXY->setIconSize(QSize(34, 34));
            showXY->setProperty("View", ViewXY);
            showXY->setProperty("Parent", ViewXY);
            showXY->setMinimumHeight(50);
            showXY->setMinimumWidth(150);
            QObject::connect(showXY, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showXY);
        }

        if (!viewVisible[View3D])
        {
            QPushButton* show3D = new QPushButton();
            show3D->setText("Show 3D View");
            show3D->setIcon(QIcon(":/svg/svg/icon3d.svg"));
            show3D->setIconSize(QSize(34, 34));
            show3D->setProperty("View", View3D);
            show3D->setProperty("Parent", ViewXY);
            show3D->setMinimumHeight(50);
            QObject::connect(show3D, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(show3D);
        }

        if (!viewVisible[ViewXZ])
        {
            QPushButton* showXZ = new QPushButton();
            showXZ->setText("Show Coronal View");
            showXZ->setIcon(QIcon(":/svg/svg/iconxz.svg"));
            showXZ->setIconSize(QSize(34, 34));
            showXZ->setProperty("View", ViewXZ);
            showXZ->setProperty("Parent", ViewXY);
            showXZ->setMinimumHeight(50);
            QObject::connect(showXZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showXZ);
        }

        if (!viewVisible[ViewYZ])
        {
            QPushButton* showYZ = new QPushButton();
            showYZ->setText("Show Sagittal View");
            showYZ->setIcon(QIcon(":/svg/svg/iconyz.svg"));
            showYZ->setIconSize(QSize(34, 34));
            showYZ->setProperty("View", ViewYZ);
            showYZ->setProperty("Parent", ViewXY);
            showYZ->setMinimumHeight(50);
            QObject::connect(showYZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showYZ);
        }

        if (!viewVisible[ViewARB])
        {
            QPushButton* showArbitrary = new QPushButton();
            showArbitrary->setText("Show Arbitrary View");
            showArbitrary->setIcon(QIcon(":/svg/svg/iconarb.svg"));
            showArbitrary->setIconSize(QSize(34, 34));
            showArbitrary->setProperty("View", ViewARB);
            showArbitrary->setProperty("Parent", ViewXY);
            showArbitrary->setMinimumHeight(50);
            QObject::connect(showArbitrary, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showArbitrary);
        }
    }

    if (floatingXZView)
    {
        QLayout* layout = m_wndViewsSubstitute[ViewXZ]->widget()->layout();
        removeQLayoutChildren(layout);

        QVBoxLayout* vbLayout = qobject_cast<QVBoxLayout*>(layout);
        vbLayout->setAlignment(Qt::AlignCenter);
        if (NULL != vbLayout)
        {
            QPushButton* showXZ = new QPushButton();
            showXZ->setText("Show Coronal View");
            showXZ->setIcon(QIcon(":/svg/svg/iconxz.svg"));
            showXZ->setIconSize(QSize(34, 34));
            showXZ->setProperty("View", ViewXZ);
            showXZ->setProperty("Parent", ViewXZ);
            showXZ->setMinimumHeight(50);
            showXZ->setMinimumWidth(150);
            QObject::connect(showXZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showXZ);
        }

        if (!viewVisible[View3D])
        {
            QPushButton* show3D = new QPushButton();
            show3D->setText("Show 3D View");
            show3D->setIcon(QIcon(":/svg/svg/icon3d.svg"));
            show3D->setIconSize(QSize(34, 34));
            show3D->setProperty("View", View3D);
            show3D->setProperty("Parent", ViewXZ);
            show3D->setMinimumHeight(50);
            QObject::connect(show3D, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(show3D);
        }

        if (!viewVisible[ViewXY])
        {
            QPushButton* showXY = new QPushButton();
            showXY->setText("Show Axial View");
            showXY->setIcon(QIcon(":/svg/svg/iconxy.svg"));
            showXY->setIconSize(QSize(34, 34));
            showXY->setProperty("View", ViewXY);
            showXY->setProperty("Parent", ViewXZ);
            showXY->setMinimumHeight(50);
            QObject::connect(showXY, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showXY);
        }

        if (!viewVisible[ViewYZ])
        {
            QPushButton* showYZ = new QPushButton();
            showYZ->setText("Show Sagittal View");
            showYZ->setIcon(QIcon(":/svg/svg/iconyz.svg"));
            showYZ->setIconSize(QSize(34, 34));
            showYZ->setProperty("View", ViewYZ);
            showYZ->setProperty("Parent", ViewXZ);
            showYZ->setMinimumHeight(50);
            QObject::connect(showYZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showYZ);
        }

        if (!viewVisible[ViewARB])
        {
            QPushButton* showArbitrary = new QPushButton();
            showArbitrary->setText("Show Arbitrary View");
            showArbitrary->setIcon(QIcon(":/svg/svg/iconarb.svg"));
            showArbitrary->setIconSize(QSize(34, 34));
            showArbitrary->setProperty("View", ViewARB);
            showArbitrary->setProperty("Parent", ViewXZ);
            showArbitrary->setMinimumHeight(50);
            QObject::connect(showArbitrary, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showArbitrary);
        }
    }

    if (floatingYZView)
    {
        QLayout* layout = m_wndViewsSubstitute[ViewYZ]->widget()->layout();
        removeQLayoutChildren(layout);

        QVBoxLayout* vbLayout = qobject_cast<QVBoxLayout*>(layout);
        vbLayout->setAlignment(Qt::AlignCenter);
        if (NULL != vbLayout)
        {
            QPushButton* showYZ = new QPushButton();
            showYZ->setText("Show Sagittal View");
            showYZ->setIcon(QIcon(":/svg/svg/iconyz.svg"));
            showYZ->setIconSize(QSize(34, 34));
            showYZ->setProperty("View", ViewYZ);
            showYZ->setProperty("Parent", ViewYZ);
            showYZ->setMinimumHeight(50);
            showYZ->setMinimumWidth(150);
            QObject::connect(showYZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showYZ);
        }

        if (!viewVisible[View3D])
        {
            QPushButton* show3D = new QPushButton();
            show3D->setText("Show 3D View");
            show3D->setIcon(QIcon(":/svg/svg/icon3d.svg"));
            show3D->setIconSize(QSize(34, 34));
            show3D->setProperty("View", View3D);
            show3D->setProperty("Parent", ViewYZ);
            show3D->setMinimumHeight(50);
            QObject::connect(show3D, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(show3D);
        }

        if (!viewVisible[ViewXY])
        {
            QPushButton* showXY = new QPushButton();
            showXY->setText("Show Axial View");
            showXY->setIcon(QIcon(":/svg/svg/iconxy.svg"));
            showXY->setIconSize(QSize(34, 34));
            showXY->setProperty("View", ViewXY);
            showXY->setProperty("Parent", ViewYZ);
            showXY->setMinimumHeight(50);
            QObject::connect(showXY, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showXY);
        }

        if (!viewVisible[ViewXZ])
        {
            QPushButton* showXZ = new QPushButton();
            showXZ->setText("Show Coronal View");
            showXZ->setIcon(QIcon(":/svg/svg/iconxz.svg"));
            showXZ->setIconSize(QSize(34, 34));
            showXZ->setProperty("View", ViewXZ);
            showXZ->setProperty("Parent", ViewYZ);
            showXZ->setMinimumHeight(50);
            QObject::connect(showXZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showXZ);
        }

        if (!viewVisible[ViewARB])
        {
            QPushButton* showArbitrary = new QPushButton();
            showArbitrary->setText("Show Arbitrary View");
            showArbitrary->setIcon(QIcon(":/svg/svg/iconarb.svg"));
            showArbitrary->setIconSize(QSize(34, 34));
            showArbitrary->setProperty("View", ViewARB);
            showArbitrary->setProperty("Parent", ViewYZ);
            showArbitrary->setMinimumHeight(50);
            QObject::connect(showArbitrary, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showArbitrary);
        }
    }

    if (floatingArbitraryView)
    {
        QLayout* layout = m_wndViewsSubstitute[ViewARB]->widget()->layout();
        removeQLayoutChildren(layout);

        QVBoxLayout* vbLayout = qobject_cast<QVBoxLayout*>(layout);
        vbLayout->setAlignment(Qt::AlignCenter);
        if (NULL != vbLayout)
        {
            QPushButton* showArbitrary = new QPushButton();
            showArbitrary->setText("Show Arbitrary View");
            showArbitrary->setIcon(QIcon(":/svg/svg/iconarb.svg"));
            showArbitrary->setIconSize(QSize(34, 34));
            showArbitrary->setProperty("View", ViewARB);
            showArbitrary->setProperty("Parent", ViewARB);
            showArbitrary->setMinimumHeight(50);
            showArbitrary->setMinimumWidth(150);
            QObject::connect(showArbitrary, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showArbitrary);
        }

        if (!viewVisible[View3D])
        {
            QPushButton* show3D = new QPushButton();
            show3D->setText("Show 3D View");
            show3D->setIcon(QIcon(":/svg/svg/icon3d.svg"));
            show3D->setIconSize(QSize(34, 34));
            show3D->setProperty("View", View3D);
            show3D->setProperty("Parent", ViewARB);
            show3D->setMinimumHeight(50);
            QObject::connect(show3D, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(show3D);
        }

        if (!viewVisible[ViewXY])
        {
            QPushButton* showXY = new QPushButton();
            showXY->setText("Show Axial View");
            showXY->setIcon(QIcon(":/svg/svg/iconxy.svg"));
            showXY->setIconSize(QSize(34, 34));
            showXY->setProperty("View", ViewXY);
            showXY->setProperty("Parent", ViewARB);
            showXY->setMinimumHeight(50);
            QObject::connect(showXY, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showXY);
        }

        if (!viewVisible[ViewXZ])
        {
            QPushButton* showXZ = new QPushButton();
            showXZ->setText("Show Coronal View");
            showXZ->setIcon(QIcon(":/svg/svg/iconxz.svg"));
            showXZ->setIconSize(QSize(34, 34));
            showXZ->setProperty("View", ViewXZ);
            showXZ->setProperty("Parent", ViewARB);
            showXZ->setMinimumHeight(50);
            QObject::connect(showXZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showXZ);
        }

        if (!viewVisible[ViewYZ])
        {
            QPushButton* showYZ = new QPushButton();
            showYZ->setText("Show Sagittal View");
            showYZ->setIcon(QIcon(":/svg/svg/iconyz.svg"));
            showYZ->setIconSize(QSize(34, 34));
            showYZ->setProperty("View", ViewYZ);
            showYZ->setProperty("Parent", ViewARB);
            showYZ->setMinimumHeight(50);
            QObject::connect(showYZ, SIGNAL(clicked()), this, SLOT(showSelectedView()));

            vbLayout->addWidget(showYZ);
        }
    }*/

    // update UI
    afterWorkspaceChange(); // disables some actions in ui for some configurations
    workspacesEnabler(); // update checked states

    QApplication::restoreOverrideCursor();

    if (bTimer)
    {
        m_timer.start();
    }
}

void MainWindow::showSelectedView()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (NULL == button)
    {
        return;
    }

    const unsigned int view = button->property("View").toUInt();
    const unsigned int parent = button->property("Parent").toUInt();

    int index = getIndexOfWndInLayout(parent);

    if (index < 0 || index >= m_viewsToShow.size())
    {
        return;
    }

    int toRemove = -1;
    for (size_t i = index + 1; i < m_viewsToShow.size(); ++i)
    {
        if (m_viewsToShow[i].windowType == view)
        {
            toRemove = i;
            break;
        }
    }

    if (toRemove > 0)
    {
        m_viewsToShow.erase(m_viewsToShow.begin() + toRemove);
    }

    sWindowInfo old = m_viewsToShow[index];

    saveFloatingWindowsGeometry();

    m_viewsToShow[index].windowType = view;
    m_viewsToShow[index].bFloating = false;
    m_viewsToShow[index].bShowSubstitute = true;

    if (old.windowType != view && old.bFloating)
    {
        old.bShowSubstitute = false;
        m_viewsToShow.push_back(old);
    }

    makeWndFloating(view, false);
}

void MainWindow::createPanels()
{
    m_densityWindowPanel = new CDensityWindowWidget();
    QDockWidget *dockDWP = new QDockWidget(tr("Brightness / Contrast"), this);
    dockDWP->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockDWP->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable); // QDockWidget::DockWidgetFloatable
    dockDWP->setObjectName("Brightness / Contrast Panel");
    dockDWP->setWidget(m_densityWindowPanel);
	dockDWP->setProperty("Icon",":/svg/svg/density_window_dock.svg");
    addDockWidget(Qt::RightDockWidgetArea, dockDWP);

    m_orthoSlicesPanel = new COrthoSlicesWidget();
    QDockWidget *dockOrtho = new QDockWidget(tr("2D Slices"), this);
    dockOrtho->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockOrtho->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    dockOrtho->setObjectName("2D Slices Panel");
    dockOrtho->setWidget(m_orthoSlicesPanel);
	dockOrtho->setProperty("Icon",":/svg/svg/ortho_slices_window_dock.svg");
    tabifyDockWidget(dockDWP, dockOrtho);

    m_segmentationPanel = new CSegmentationWidget();
    QDockWidget *dockSeg = new QDockWidget(tr("Quick Tissue Model Creation"), this);
    dockSeg->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockSeg->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
    dockSeg->setObjectName("Quick Tissue Model Creation Panel");
    dockSeg->setWidget(m_segmentationPanel);
	dockSeg->setProperty("Icon",":/svg/svg/segmentation_window_dock.svg");
    tabifyDockWidget(dockDWP, dockSeg);

    m_volumeRenderingPanel = new CVolumeRenderingWidget();
    m_volumeRenderingPanel->setRenderer(&m_3DView->getRenderer());
    QDockWidget *dockVR = new QDockWidget(tr("Volume Rendering"), this);
    dockVR->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockVR->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
    dockVR->setObjectName("Volume Rendering Panel");
    dockVR->setWidget(m_volumeRenderingPanel);
	dockVR->setProperty("Icon",":/svg/svg/volume_rendering_window2_dock.svg");
    dockVR->hide();
    tabifyDockWidget(dockDWP, dockVR);

    m_modelsPanel = new CModelsWidget();
    QDockWidget *dockModels = new QDockWidget(tr("Models List"), this);
    dockModels->setAllowedAreas(Qt::AllDockWidgetAreas);
    dockModels->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable);
    dockModels->setObjectName("Models Panel");
    dockModels->setWidget(m_modelsPanel);
	dockModels->setProperty("Icon",":/svg/svg/models_dock.svg");
    tabifyDockWidget(dockDWP, dockModels);

    connect(dockDWP, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
    connect(dockOrtho, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
    connect(dockSeg, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
    connect(dockVR, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
	connect(dockModels, SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));

    connect(dockDWP,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea))); 
    connect(dockOrtho,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
    connect(dockSeg,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
    connect(dockVR,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
	connect(dockModels,SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),this,SLOT(dockLocationChanged(Qt::DockWidgetArea)));
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
        settings.setValue("geometryX", saveGeometry().toBase64());
        settings.setValue("windowStateX", saveState().toBase64());
        QSizeF relSize = getRelativeSize(centralWidget());
        settings.setValue("centralWidth", relSize.width());
        settings.setValue("centralHeight", relSize.height());        
        settings.endGroup();
    }
    if (WorkspaceTabs==nLayoutType)
    {
        settings.beginGroup("WorkspaceTabs");
        settings.setValue("geometryX", saveGeometry().toBase64());
        settings.setValue("windowStateX", saveState().toBase64());
        QSizeF relSize = getRelativeSize(centralWidget());
        settings.setValue("centralWidth", relSize.width());
        settings.setValue("centralHeight", relSize.height());
        settings.endGroup();
    }
    if (WorkspaceGrid==nLayoutType)
    {
        settings.beginGroup("WorkspaceGrid");
        settings.setValue("geometryX", saveGeometry().toBase64());
        settings.setValue("windowStateX", saveState().toBase64());
        QSizeF relSize = getRelativeSize(centralWidget());
        settings.setValue("centralWidth", relSize.width());
        settings.setValue("centralHeight", relSize.height());
        settings.endGroup();
    }
    if (WorkspaceSlices == nLayoutType)
    {
        settings.beginGroup("WorkspaceSlices");
        settings.setValue("geometryX", saveGeometry().toBase64());
        settings.setValue("windowStateX", saveState().toBase64());
        QSizeF relSize = getRelativeSize(centralWidget());
        settings.setValue("centralWidth", relSize.width());
        settings.setValue("centralHeight", relSize.height());
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
    case WorkspaceSlices:
        settings.beginGroup("WorkspaceSlices");
        break;
    default:
        Q_ASSERT(false);
        return;
    }
    if (!bInnerLayoutOnly)
    {
        QByteArray geomX = settings.value("geometryX").toByteArray();
        if (geomX.isEmpty())
            restoreGeometry(settings.value("geometry").toByteArray());
        else
            restoreGeometry(QByteArray::fromBase64(geomX));
    }
    QByteArray wsX = settings.value("windowStateX").toByteArray();
    if (wsX.isEmpty())
		restoreState(settings.value("windowState").toByteArray());
    else
		restoreState(QByteArray::fromBase64(wsX));
    if (bInnerLayoutOnly)
    {
        double width = settings.value("centralWidth").toDouble();
        double height = settings.value("centralHeight").toDouble();
        if (0==width || 0==height)
        {
            QVariant val = settings.value("centralSize");
            if (QVariant::SizeF==val.type())
            {
                QSizeF sizew=val.toSizeF();
                width = sizew.width();
                height = sizew.height();
            }
        }
        if (0!=width && 0!=height)
        {
            QSize sizeM = size();
            centralWidget()->resize(sizeM.width()*width,sizeM.height()*height);
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
    case WorkspaceSlices:
        ui->actionAxial_View->setEnabled(false);
        ui->actionCoronal_View->setEnabled(false);
        ui->actionSagittal_View->setEnabled(false);
        ui->actionArbitrary_View->setEnabled(false);
        ui->viewsToolBar->hide();
        break;
    case Workspace3D:
        ui->actionAxial_View->setEnabled(true);
        ui->actionCoronal_View->setEnabled(true);
        ui->actionSagittal_View->setEnabled(true);
        ui->actionArbitrary_View->setEnabled(true);
        ui->viewsToolBar->show();
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

    QByteArray wsX = settings.value("windowStateX").toByteArray();
    if (wsX.isEmpty())
		restoreState(settings.value("windowState").toByteArray());
    else
		restoreState(QByteArray::fromBase64(wsX));
    {
        double width = settings.value("centralWidth").toDouble();
        double height = settings.value("centralHeight").toDouble();
        if (0==width || 0==height)
        {
            QVariant val = settings.value("centralSize");
            if (QVariant::SizeF==val.type())
            {
                QSizeF sizew=val.toSizeF();
                width = sizew.width();
                height = sizew.height();
            }
        }
        if (0!=width && 0!=height)
        {
            QSize sizeM = size();
            centralWidget()->resize(sizeM.width()*width,sizeM.height()*height);
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
    ui->actionArbitrary_Slice->setChecked(VPL_SIGNAL(SigGetPlaneARBVisibility).invoke2());

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
        if (property("SegmentationChanged").toBool())
        {
            if (QMessageBox::Yes != QMessageBox::question(this, QCoreApplication::applicationName(),
                tr("You have an unsaved segmentation data! All unsaved changes will be lost. Do you want to continue?"),
                QMessageBox::Yes | QMessageBox::No))
            {
                event->ignore();
                return;
            }
        }
    }

    makeWndFloating(View3D, false, false);
    makeWndFloating(ViewXY, false, false);
    makeWndFloating(ViewXZ, false, false);
    makeWndFloating(ViewYZ, false, false);
    makeWndFloating(ViewARB, false, false);

    setUpWorkspace();

    saveAppSettings();
	if (0!=(windowState()&Qt::WindowFullScreen))
		showNormal();
    writeLayoutSettings(m_nLayoutType,false);

    QMainWindow::closeEvent(event);
}

void MainWindow::enterEvent(QEvent* event)
{
    //QApplication::setActiveWindow(this);

    /*QPoint posBkp = QApplication::desktop()->cursor().pos();
    QPoint point = this->pos();
    int height = this->height(); 
    int width = this->width();

    setCursor(Qt::BlankCursor);

    QApplication::desktop()->cursor().setPos(point.x() + width * 0.5, point.y() + height - 100);
    mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 1, 1, 0, 0);
    QApplication::desktop()->cursor().setPos(posBkp);

    setCursor(Qt::ArrowCursor);*/

    //QApplication::processEvents();
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
    if (shallUpdateOSGCanvas(m_ArbitrarySlice, mousePos))
        m_ArbitrarySlice->update();
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

	setVOIVisibility(false);

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

    settings.setValue("DICOMdir", fileName);
    return openDICOM(fileName, fileName, true);
}

bool MainWindow::openDICOMZIP()
{
    if (!preOpen())
        return false;

	setVOIVisibility(false);

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
    settings.setValue("DICOMdir", pathInfo.dir().absolutePath());

    return openDICOMZIP(fileName);
}

bool MainWindow::openDICOMZIP(QString fileName)
{
	setVOIVisibility(false);

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
    
    return openDICOM(fileName,realName, false);
}

bool MainWindow::openDICOM(const QString& fileName, const QString& realName, bool fromFolder)
{
    m_region3DPreviewVisualizer->setVisibility(false);
    m_3DView->Refresh(false);

	setVOIVisibility(false);

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
        //TODO Why is this here? is it neccessary? It's imposible not to select dir
#if !defined( TRIDIM_USE_GDCM )
                std::string ansiName = fi.absolutePath().toStdString();
        int nFrames = 0;
        data::CDicomDCTk::getDicomFileInfo( vpl::sys::tStringConv::fromUtf8(ansiName), fileNameIn.toStdString(), nFrames);
#define DICOM_MULTIFRAME_THRESHOLD  10
        if (nFrames<=DICOM_MULTIFRAME_THRESHOLD)
            fileNameIn = fi.dir().absolutePath();
#endif
    }
#endif

    // get ansi name for DCMTK
    // 2012/03/12: Modified by Majkl (Linux compatible code)
//    std::string str = fileNameIn.toUtf8();
	std::string str = fileNameIn.toStdString();

    data::sExtendedTags tags = { };

    QSettings settings;

    {
        // Show simple progress dialog
        CProgress progress(this);
        progress.setLabelText(tr("Scanning the directory for DICOM datasets, please wait..."));

        APP_STORAGE.reset();

        progress.show();

        data::CDicomLoader Loader;
        Loader.registerProgressFunc( vpl::mod::CProgress::tProgressFunc( &progress, &CProgress::Entry ) );

        // preload data from the selected directory
        data::CSeries::tSmartPtr spSeries;
        fi = QFileInfo(fileNameIn);
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
		CSeriesSelectionDialog dlg(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
		if (!dlg.setSeries(spSeries))
		{
			QApplication::restoreOverrideCursor();
			return false;
		}

        progress.hide();

        settings.beginGroup("SeriesSelectionWindow");

        QRect geometry = settings.value("geometry", QRect(100, 100, 500, 500)).toRect();
        dlg.setGeometry(geometry);

        if (QDialog::Accepted!=dlg.exec())
		{
			QApplication::restoreOverrideCursor();
			return false;
		}

        settings.setValue("geometry", dlg.geometry());
        settings.endGroup();

        int iSelection=dlg.getSelection();
        if (iSelection<0)
		{
			QApplication::restoreOverrideCursor();
			return false;
		}
		QApplication::restoreOverrideCursor();
        double ssX, ssY, ssZ;
        dlg.getSubsampling(ssX, ssY, ssZ);

        dlg.hidePreview();

        // Load the data (prepare a new method CExamination::load(spSeries.get()))
        data::CSerieInfo::tSmartPtr current_serie = spSeries->getSerie( iSelection );
        if( !current_serie )
        {
            return false;
        }

        progress.show();
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
    CVolumeLimiterDialog vlDlg(this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowStaysOnTopHint);
    vlDlg.setVolume(spData.get(), true);

    settings.beginGroup("VolumeLimiterWindow");

    QRect geometry = settings.value("geometry", QRect(100, 100, 500, 500)).toRect();
    vlDlg.setGeometry(geometry);

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

    settings.setValue("geometry", vlDlg.geometry());
    settings.endGroup();

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

    postOpen(realName, fromFolder);

    return true;
}

bool MainWindow::preOpen()
{
    if (property("SegmentationChanged").toBool())
    {
        if (QMessageBox::Yes!=QMessageBox::question(this,QCoreApplication::applicationName(),tr("You have an unsaved segmentation data! All unsaved changes will be lost. Do you want to continue?"),QMessageBox::Yes|QMessageBox::No))
            return false;
        setProperty("SegmentationChanged",false);
    }   

    m_region3DPreviewManager->stop();

    return true;
}
void MainWindow::pluginLog(int type, const std::string& whatInvoke, const std::string& message)
{
    switch(type)
    {
    case LL_TRACE:
        VPL_LOG_TRACE(whatInvoke + ": " + message);
        break;
    case LL_DEBUG:
        VPL_LOG_DEBUG(whatInvoke + ": " + message);
        break;
    case LL_INFO:
        VPL_LOG_INFO(whatInvoke + ": " + message);
        break;
    case LL_WARN:
        VPL_LOG_WARN(whatInvoke + ": " + message);
        break;
    case LL_ERROR:
        VPL_LOG_ERROR(whatInvoke + ": " + message);
        break;
    case LL_FATAL:
        VPL_LOG_FATAL(whatInvoke + ": " + message);
        break;
    }
}
void MainWindow::postOpenActions()
{
    data::CObjectPtr<data::CMultiClassRegionData> spRegionData(APP_STORAGE.getEntry(data::Storage::MultiClassRegionData::Id));
    if (spRegionData->isDummy())
    {
        spRegionData->disableDummyMode();
        APP_STORAGE.invalidate(spRegionData.getEntryPtr());
    }

    data::CObjectPtr< data::CActiveDataSet > ptrDataset(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
    data::CObjectPtr< data::CDensityData > pVolume(APP_STORAGE.getEntry(ptrDataset->getId()));
    vpl::img::CSize3d voxelSize(pVolume->getDX(), pVolume->getDY(), pVolume->getDZ());

    m_region3DPreviewManager->init(voxelSize, 0);
    geometry::Vec3Array vertices;
    std::vector<int> indicies;
    m_region3DPreviewVisualizer->setData(vertices, indicies);
    update3DPreview();

    if (m_region3DPreviewVisible)
    {
        m_region3DPreviewManager->run();
        m_region3DPreviewVisualizer->setVisibility(true);
        m_3DView->Refresh(false);
    }

    undoRedoEnabler();
    fixBadSliceSliderPos();
    resetLimit();

}
void MainWindow::postOpen(const QString& filename, bool bDicomData)
{
    postOpenActions();
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
						 data::Storage::MultiClassRegionData::Id,
						 data::Storage::BonesModel::Id,
						 data::Storage::ImportedModel::Id};
	const int nEntries = sizeof(arrWatched)/sizeof(arrWatched[0]);
    std::vector<int> res;
    for (int i = 0; i < nEntries; i++)
    {
        if (data::Storage::ImportedModel::Id == arrWatched[i])
        {
            for (int x = 0; x < MAX_IMPORTED_MODELS; ++x)
            {
                res.push_back(APP_STORAGE.getEntry(data::Storage::ImportedModel::Id + x).get()->getLatestVersion());
            }
        }
        else
        {
            res.push_back(APP_STORAGE.getEntry(arrWatched[i]).get()->getLatestVersion());
        }
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

	setVOIVisibility(false);

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
    m_region3DPreviewVisualizer->setVisibility(false);
    m_3DView->Refresh(false);

	setVOIVisibility(false);

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
							enableMenuActions();
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
			catch( const vpl::base::CException& Exception )
			{
				enableMenuActions();
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
		enableMenuActions();
        return false;
    }
    setWindowTitle(QApplication::applicationName()+" - "+wsFileName);

    postOpen(wsFileName, false);
	enableMenuActions();
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
		enableMenuActions();
        return false;
    }
    else
    {
        postSave(fileName);
    }
	enableMenuActions();
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
		srcName.replace("//", "/");
		QFileInfo inputFileInfo( srcName );
		QString dstName = fileName + "/" + fileList[i]; // (fileName + "/") + inputFileInfo.baseName() + "." + inputFileInfo.completeSuffix();
		if (!QFile::copy(srcName,dstName))
		{
			if (QMessageBox::critical(this,QCoreApplication::applicationName(),tr("Failed to copy file %1. Make sure, that the destination directory doesn't include a file with the same name.").arg(inputFileInfo.baseName() + "." + inputFileInfo.completeSuffix())))
			{
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
    bool bSaveSegmented = false;
	bool bSaveCompressed = false;
	bool bSaveActive = false;
	bool bSaveVOI = false;
	bool bAnonymize = false;
	QString anonymString = "Anonymous";
	QString anonymID = "";
	{
		data::CObjectPtr<data::CMultiClassRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id));
		const int activeRegion(spColoring->getActiveRegion());
		data::CMultiClassRegionColoring::tColor color;
		if (activeRegion>=0)
		{
			color = spColoring->getColor( activeRegion );
		}
		QColor  qcolor(color.getR(), color.getG(), color.getB());

        QDialog dlg(this,Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
        dlg.setWindowTitle(tr("Save / Anonymize DICOM"));
        //dlg.setMinimumSize(500,300);
		dlg.setSizeGripEnabled(true);
        QGridLayout* pLayout = new QGridLayout();
        dlg.setLayout(pLayout);

		QGroupBox *box1 = new QGroupBox();
		box1->setTitle(tr("Image Data Manipulation"));
		QGridLayout *gl1 = new QGridLayout();

		if (m_segmentationPluginsLoaded)
		{
			pLayout->addWidget(box1, 0, 0);
		}

		QRadioButton rbCurrent(tr("Save All Data"));
		rbCurrent.setChecked(true);
		gl1->addWidget(&rbCurrent, 0, 0);
        QRadioButton rbSegmented(tr("Save All Segmented Areas"));
		gl1->addWidget(&rbSegmented, 1, 0);
		QRadioButton rbActiveOnly(tr("Save Active Region Only"));
		gl1->addWidget(&rbActiveOnly, 2, 0);
		QLabel labelRegion;
		labelRegion.setMinimumWidth(32);
		labelRegion.setMaximumWidth(32);
		labelRegion.setStyleSheet(QString("background-color: %1").arg(qcolor.name()));
		gl1->addWidget(&labelRegion, 2, 1);
		QRadioButton rbVOI(tr("Save Data in Volume of Interest"));
		gl1->addWidget(&rbVOI, 3, 0);
		box1->setLayout(gl1);

		QGroupBox *box2 = new QGroupBox();
		box2->setTitle(tr("DICOM Properties"));
		QGridLayout *gl2 = new QGridLayout();
		pLayout->addWidget(box2, 2, 0);
		QCheckBox cbCompress(tr("Compress"));
		gl2->addWidget(&cbCompress, 0, 0);
		box2->setLayout(gl2);

		QGroupBox *box3 = new QGroupBox();
		box3->setTitle(tr("Anonymize As"));
		box3->setCheckable(true);
		box3->setChecked(false);
		QGridLayout *gl3 = new QGridLayout();
		pLayout->addWidget(box3, 1, 0);
		QLabel nameLabel(tr("Name:"));
		QLineEdit anonymEdit;
		anonymEdit.setText(tr("Anonymous"));
		gl3->addWidget(&nameLabel, 0, 0);
		gl3->addWidget(&anonymEdit, 0, 1);
		QLabel idLabel(tr("Id:"));
		QLineEdit anonymIDEdit;
		anonymIDEdit.setText("");
		gl3->addWidget(&idLabel, 1, 0);
		gl3->addWidget(&anonymIDEdit, 1, 1);		
		box3->setLayout(gl3);

		QDialogButtonBox* pButtonBox = new QDialogButtonBox;
		pButtonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        pButtonBox->setCenterButtons(true);
        connect(pButtonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
		connect(pButtonBox, SIGNAL(rejected()), &dlg, SLOT(reject()));
        pLayout->addWidget(pButtonBox,5,0,1,2);

		if (QDialog::Accepted!=dlg.exec())
			return false;

		bSaveSegmented = rbSegmented.isChecked() || rbActiveOnly.isChecked();
		bSaveCompressed = cbCompress.isChecked();
		bSaveActive = rbActiveOnly.isChecked();
		bSaveVOI = rbVOI.isChecked();
		bAnonymize = box3->isChecked();
		anonymString = anonymEdit.text();
		anonymID = anonymIDEdit.text();
	}
    
    QSettings settings;
	QString previousDir = getSaveLoadPath("dstDICOMdir");
    if (previousDir.isEmpty())
        previousDir=settings.value("DICOMdir").toString();

    // get DICOM data destination directory
    QString fileName = QFileDialog::getExistingDirectory(this,tr("Please, choose a directory where to save current DICOM dataset..."),previousDir);
    if (fileName.isEmpty())
        return false;

	fileName = QString(fileName.toUtf8());

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

    //pointer do data storage
    data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(data::Storage::PatientData::Id));

    QString SeriesID(spData->m_sSeriesUid.c_str());
    if (SeriesID.isEmpty())
        SeriesID = "cs" + QString::number(spData->getDataCheckSum());

    if (bSaveSegmented)
    {
        QObject* pPlugin = MainWindow::getInstance()->findPluginByID("RegionAndModelControl");

        if (NULL != pPlugin)
        {
            PluginLicenseInterface *iPlugin = qobject_cast<PluginLicenseInterface *>(pPlugin);
            if (iPlugin)
            {
                if (!iPlugin->canExport(SeriesID))
                {
                    QMessageBox::warning(nullptr, QCoreApplication::applicationName(), "Segmented areas can't be exported with the current product license.");
                    return false;
                }
            }
        }
    }
    //// Show simple progress dialog
    CProgress progress(this);
    progress.setLabelText(tr("Saving DICOM data, please wait..."));
    progress.show();



    bool bOk = false ;

#ifdef WIN32
	std::wstring uniName = (const wchar_t*)fileName.utf16();
	std::string dirName = wcs2ACP(uniName);
#else
	std::string dirName = fileName.toStdString();
#endif

    vpl::mod::CProgress::tProgressFunc ProgressFunc(&progress, &CProgress::Entry);
    try
    {
#if defined( TRIDIM_USE_GDCM )
        
            data::CDicomSaverGDCM saver;
			bOk = saver.saveSerie(dirName, bSaveSegmented, bSaveCompressed, bSaveActive, 
                bSaveVOI, bAnonymize, anonymString.toStdString(), anonymID.toStdString(), ProgressFunc);   
        
#else        
            data::CDicomSaverDCTk saver;
			bOk = saver.saveSerie(dirName, bSaveSegmented, bSaveCompressed, bSaveActive,
                bSaveVOI, bAnonymize, anonymString.toStdString(), anonymID.toStdString(), ProgressFunc);
#endif

    }
    catch (std::exception &e)
    {
        showMessageBox(QMessageBox::Critical, tr(e.what()));
		enableMenuActions();
        return false;
    }


    // update license info
    if (bOk)
    {
        // update license info
        if (bSaveSegmented)
        {
            QObject* pPlugin = MainWindow::getInstance()->findPluginByID("RegionAndModelControl");
            if (NULL != pPlugin)
            {
                PluginLicenseInterface *iPlugin = qobject_cast<PluginLicenseInterface *>(pPlugin);
                if (iPlugin)
                {
                    iPlugin->wasExported(SeriesID);
                }
            }
        }
    }

    postSave(fileName);

	enableMenuActions();

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


    int defaultFilter = settings.value("ExportedModelsFormat").toInt();
    QString selectedFilter;

    switch (defaultFilter)
    {
    case 0:
        selectedFilter = "Stereo litography model (*.stl)";
        previousDir = appendSaveNameHint(previousDir, ".stl");
        break;
    case 1:
        selectedFilter = "Binary Polygon file format (*.ply)";
        previousDir = appendSaveNameHint(previousDir, ".ply");
        break;
    case 2:
        selectedFilter = "ASCII Polygon file format (*.ply)";
        previousDir = appendSaveNameHint(previousDir, ".ply");
        break;
    case 3:
        selectedFilter = "OpenCTM compressed file format (*.ctm)";
        previousDir = appendSaveNameHint(previousDir, ".ctm");
        break;
    default:
        selectedFilter = "Stereo litography model (*.stl)";
        previousDir = appendSaveNameHint(previousDir, ".stl");
        break;
    }

	QString filePath = QFileDialog::getOpenFileName(this, tr("Choose an input polygonal model to load..."), previousDir, tr("Stereo litography model (*.stl);;Binary Polygon file format (*.ply);;ASCII Polygon file format (*.ply);;OpenCTM compressed file format (*.ctm)"), &selectedFilter);
	if (filePath.isEmpty())
        return false;

	QFileInfo pathInfo(filePath);
    settings.setValue("STLdir",pathInfo.dir().absolutePath());
	return openSTL(filePath, pathInfo.fileName());
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


bool MainWindow::openSTL(const QString& filePath, const QString& fileName)
{
#ifdef WIN32
    // OpenMesh needs path in ACP
	std::wstring uniName = (const wchar_t*)filePath.utf16();
    std::string ansiName = wcs2ACP(uniName);
#else
    // 2012/03/12: Modified by Majkl (Linux compatible code)
    //std::string ansiName = filePath.toUtf8();
	std::string ansiName = filePath.toStdString();
#endif

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // get file extension
    std::string extension;
    std::string::size_type dotPos(ansiName.rfind("."));    
    if (dotPos != std::string::npos)
    { 
        extension = ansiName.substr(dotPos +1, ansiName.length()- dotPos -1);
        std::transform( extension.begin(), extension.end(), extension.begin(), tolower );
    }

    bool result = false;
	int loadedID = 0;

    // don't use osg for stl models as it doesn't merge vertices
	std::string extensions = "stl stla stlb ply obj ctm";
	if (extensions.find(extension) == std::string::npos)
    {
        // Find unused id
        int id(findPossibleModelId());
		if (id < 0)
		{
			QApplication::restoreOverrideCursor();
			return false;
		}

        data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(id));
        data::CSnapshot *snapshot = m_ModelManager.getSnapshot(NULL);
        data::CSnapshot *snapshotModel = spModel->getSnapshot(NULL);
        snapshot->addSnapshot(snapshotModel);
        VPL_SIGNAL(SigUndoSnapshot).invoke(snapshot);

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
                            geometry::CMesh *pMesh = pModel->getMesh(true);
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
                geometry::CMesh *pMesh = pModel->getMesh(false);
                pMesh->delete_isolated_vertices();
                pMesh->garbage_collection();
                if (pMesh->n_faces()>0)
                {
                    //pModel->setMesh(pMesh);
                    pModel->setLabel(fileName.toStdString());
					pModel->setColor(m_modelColor);
                    pModel->setVisibility(true);
					pModel->setRegionId(-1);
					pModel->setLinkedWithRegion(false);

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

		//const OpenMesh::IO::_CTMReader_ &CTMReader = OpenMesh::IO::_CTMReader_();

        if (!OpenMesh::IO::read_mesh(*pMesh, ansiName, ropt))
        {
            delete pMesh;
            result = false;
			QApplication::restoreOverrideCursor();
            showMessageBox(QMessageBox::Critical,tr("Failed to load polygonal model!"));
        }
        else
        {
            // Find unused id
            int id(findPossibleModelId());
			if (id < 0)
			{
				QApplication::restoreOverrideCursor();
				return false;
			}
			loadedID = id;
            data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(id) );
            data::CSnapshot *snapshot = m_ModelManager.getSnapshot(NULL);
            data::CSnapshot *snapshotModel = spModel->getSnapshot(NULL);
            snapshot->addSnapshot(snapshotModel);
            VPL_SIGNAL(SigUndoSnapshot).invoke(snapshot);

            spModel->setMesh(pMesh);
			spModel->setLabel(fileName.toStdString());
			spModel->setColor(m_modelColor);
            spModel->setVisibility(true);
			spModel->setRegionId(-1);
			spModel->setLinkedWithRegion(false);
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
        addToRecentFiles(filePath);
	}
    QApplication::restoreOverrideCursor();
    return result;
}

bool MainWindow::saveSTL()
{
	// Try to get selected model id
	int storage_id((!m_segmentationPluginsLoaded) ? m_modelsPanel->getSelectedModelStorageId() : VPL_SIGNAL(SigGetSelectedModelId).invoke2());

	return saveSTLById(storage_id, false);
}

bool MainWindow::saveSTLinDicomCoords()
{
    // Try to get selected model id
    int storage_id((!m_segmentationPluginsLoaded) ? m_modelsPanel->getSelectedModelStorageId() : VPL_SIGNAL(SigGetSelectedModelId).invoke2());

    return saveSTLById(storage_id, true);
}

bool MainWindow::saveSTLById(int storage_id, bool useDicom)
{
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
			//showModelsListPanel(true);
			return false;
		}
    }

    data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(storage_id) );

    // Check the model
    geometry::CMesh *pMesh = spModel->getMesh(false);
    if (!pMesh || !(pMesh->n_vertices() > 0) )
    {
        showMessageBox(QMessageBox::Critical, tr("No data to save!"));
        return false;
    }

    QString SeriesID("");
    {
        data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(data::Storage::PatientData::Id));
        SeriesID = spVolume->m_sSeriesUid.c_str();
        if (SeriesID.isEmpty())
        {
            SeriesID = "cs" + QString::number(spVolume->getDataCheckSum());
        }
    }

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
	//const bool bUseDicom = settings.value("STLUseDICOMCoord",false).toBool();
    QString previousDir = getSaveLoadPath("STLdir");
    int defaultFilter = settings.value("ExportedModelsFormat").toInt();
    QString selectedFilter;

    switch (defaultFilter)
    {
        case 0:
            selectedFilter = "Stereo litography model (*.stl)";
            previousDir = appendSaveNameHint(previousDir, ".stl");
            break;
        case 1:
            selectedFilter = "Binary Polygon file format (*.ply)";
            previousDir = appendSaveNameHint(previousDir, ".ply");
            break;
        case 2:
            selectedFilter = "ASCII Polygon file format (*.ply)";
            previousDir = appendSaveNameHint(previousDir, ".ply");
            break;
        case 3:
            selectedFilter = "OpenCTM compressed file format (*.ctm)";
            previousDir = appendSaveNameHint(previousDir, ".ctm");
            break;
        default:
            selectedFilter = "Stereo litography model (*.stl)";
            previousDir = appendSaveNameHint(previousDir, ".stl");
            break;
    }

	QString fileName = QFileDialog::getSaveFileName(this, tr("Please, specify an output file..."), previousDir, tr("Stereo litography model (*.stl);;Binary Polygon file format (*.ply);;ASCII Polygon file format (*.ply);;OpenCTM compressed file format (*.ctm)"), &selectedFilter);
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
	if (useDicom)
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

	if (selectedFilter.compare(tr("OpenCTM compressed  file format (*.ctm)"), Qt::CaseInsensitive) == 0 && !pMesh->has_vertex_normals()) {
		pMesh->request_vertex_normals();	//writer needs per vertex normals..
	}

	const OpenMesh::IO::_CTMWriter_ &CTMWriter = OpenMesh::IO::_CTMWriter_();

    bool result = true;
    if (!OpenMesh::IO::write_mesh(*pMesh, ansiName, wopt))
    {
        result = false;
        showMessageBox(QMessageBox::Critical, tr("Failed to save polygonal model!"));
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

    m_3DView->Refresh(false);
}

void MainWindow::actionsEnabler()
{
    if (!m_myViewVisibilityChange)
    {
        // Views
        /*if (NULL!=m_3DView && NULL!=m_3DView->parentWidget())
            ui->action3D_View->setChecked(m_3DView->parentWidget()->isVisible());
        if (NULL!=m_OrthoXYSlice && NULL!=m_OrthoXYSlice->parentWidget())
            ui->actionAxial_View->setChecked(m_OrthoXYSlice->parentWidget()->isVisible());
        if (NULL!=m_OrthoXZSlice && NULL!=m_OrthoXZSlice->parentWidget())
            ui->actionCoronal_View->setChecked(m_OrthoXZSlice->parentWidget()->isVisible());
        if (NULL!=m_OrthoYZSlice && NULL!=m_OrthoYZSlice->parentWidget())
            ui->actionSagittal_View->setChecked(m_OrthoYZSlice->parentWidget()->isVisible());*/

        ui->action3D_View->setChecked(m_wndViews[View3D]->isVisible() || m_wndViewsSubstitute[View3D]->isVisible());
        ui->actionAxial_View->setChecked(m_wndViews[ViewXY]->isVisible() || m_wndViewsSubstitute[ViewXY]->isVisible());
        ui->actionCoronal_View->setChecked(m_wndViews[ViewXZ]->isVisible() || m_wndViewsSubstitute[ViewXZ]->isVisible());
        ui->actionSagittal_View->setChecked(m_wndViews[ViewYZ]->isVisible() || m_wndViewsSubstitute[ViewYZ]->isVisible());
        ui->actionArbitrary_View->setChecked(m_wndViews[ViewARB]->isVisible() || m_wndViewsSubstitute[ViewARB]->isVisible());
    }

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

void MainWindow::showAnyView(bool bShow, QWidget* pView)
{
    if (NULL == pView) return;
    QDockWidget *pDockW = NULL;
    QWidget     *pParent = pView->parentWidget();
    Q_ASSERT(pParent); // parent missing!
                       // find window parent which is a dock window
    while (NULL != pParent)
    {
        pDockW = qobject_cast<QDockWidget*>(pParent);
        if (pDockW)
            break;
        pParent = pParent->parentWidget();
    }
    if (!pDockW) return;

    if (bShow && pDockW->isVisible() || !bShow && !pDockW->isVisible())
    {
        return;
    }

    if (bShow)
    {
        pDockW->show();
        pDockW->raise();
    }
    else
        pDockW->hide();
}

void MainWindow::show3DView(bool bShow)
{
    m_myViewVisibilityChange = true;

    makeWndFloating(View3D, false); // adds all hidden windows, so we need to hide them again

    showAnyView(bShow, m_3DView);
    showAnyView(ui->actionAxial_View->isChecked(), m_OrthoXYSlice);
    showAnyView(ui->actionCoronal_View->isChecked(), m_OrthoXZSlice);
    showAnyView(ui->actionSagittal_View->isChecked(), m_OrthoYZSlice);
    showAnyView(ui->actionArbitrary_View->isChecked(), m_ArbitrarySlice);

    m_myViewVisibilityChange = false;
}

void MainWindow::showAxialView(bool bShow)
{
    m_myViewVisibilityChange = true;

    makeWndFloating(ViewXY, false); // adds all hidden windows, so we need to hide them again

    showAnyView(bShow, m_OrthoXYSlice);
    showAnyView(ui->action3D_View->isChecked(), m_3DView);
    showAnyView(ui->actionCoronal_View->isChecked(), m_OrthoXZSlice);
    showAnyView(ui->actionSagittal_View->isChecked(), m_OrthoYZSlice);
    showAnyView(ui->actionArbitrary_View->isChecked(), m_ArbitrarySlice);

    m_myViewVisibilityChange = false;
}

void MainWindow::showCoronalView(bool bShow)
{
    m_myViewVisibilityChange = true;

    makeWndFloating(ViewXZ, false); // adds all hidden windows, so we need to hide them again

    showAnyView(bShow, m_OrthoXZSlice);
    showAnyView(ui->actionAxial_View->isChecked(), m_OrthoXYSlice);
    showAnyView(ui->action3D_View->isChecked(), m_3DView);
    showAnyView(ui->actionSagittal_View->isChecked(), m_OrthoYZSlice);
    showAnyView(ui->actionArbitrary_View->isChecked(), m_ArbitrarySlice);

    m_myViewVisibilityChange = false;
}

void MainWindow::showSagittalView(bool bShow)
{
    m_myViewVisibilityChange = true;

    makeWndFloating(ViewYZ, false); // adds all hidden windows, so we need to hide them again

    showAnyView(bShow, m_OrthoYZSlice);
    showAnyView(ui->actionAxial_View->isChecked(), m_OrthoXYSlice);
    showAnyView(ui->actionCoronal_View->isChecked(), m_OrthoXZSlice);
    showAnyView(ui->action3D_View->isChecked(), m_3DView);
    showAnyView(ui->actionArbitrary_View->isChecked(), m_ArbitrarySlice);

    m_myViewVisibilityChange = false;
}

void MainWindow::showArbitraryView(bool bShow)
{
    m_myViewVisibilityChange = true;

    makeWndFloating(ViewARB, false); // adds all hidden windows, so we need to hide them again

    showAnyView(bShow, m_ArbitrarySlice);
    showAnyView(ui->actionAxial_View->isChecked(), m_OrthoXYSlice);
    showAnyView(ui->actionCoronal_View->isChecked(), m_OrthoXZSlice);
    showAnyView(ui->actionSagittal_View->isChecked(), m_OrthoYZSlice);
    showAnyView(ui->action3D_View->isChecked(), m_3DView);

    m_myViewVisibilityChange = false;
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
	{
		double dpiFactor = 1;
		QDesktopWidget* pDesktop=QApplication::desktop();
		if (NULL!=pDesktop)
			dpiFactor = std::max(1.0,pDesktop->logicalDpiX()/96.0);
		data::CObjectPtr<data::CSceneWidgetParameters> spWidgets( APP_STORAGE.getEntry(data::Storage::SceneWidgetsParameters::Id) );
        dpiFactor = 1 + (dpiFactor - 1)/2; // reduce upscale
		spWidgets->setDefaultWidgetsScale(dpiFactor);
		spWidgets->setWidgetsScale(dpiFactor);
		APP_STORAGE.invalidate( spWidgets.getEntryPtr() );
	}

    QSettings settings;

    // Transfer QSurfaceFormat setting into osg::DisplaySettings
    {
        QSurfaceFormat format = QSurfaceFormat::defaultFormat();

        bool debugContext = format.options() & QSurfaceFormat::DebugContext;

        osg::DisplaySettings::instance()->setGLContextVersion(std::to_string(format.majorVersion()) + "." + std::to_string(format.minorVersion()));
        osg::DisplaySettings::instance()->setNumMultiSamples(format.samples());
        osg::DisplaySettings::instance()->setGLContextFlags(GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT | (debugContext ? GL_CONTEXT_FLAG_DEBUG_BIT : 0));
        osg::DisplaySettings::instance()->setGLContextProfileMask(GL_CONTEXT_CORE_PROFILE_BIT);
    }

    

        m_ArbitrarySlice = new OSGOrtho2DCanvas(nullptr);
        m_ArbitrarySlice->hide();

        m_SceneArb = new scene::CArbitrarySliceScene(m_ArbitrarySlice);

        m_ArbitrarySlice->setScene(m_SceneArb.get());
        m_ArbitrarySlice->centerAndScale();
        m_ArbitrarySlice->getManipulator()->m_sigUpDown.connect(m_SceneArb, &scene::CArbitrarySliceScene::sliceUpDown);
        // Drawing event handler
        m_drawARBEH = new osgGA::CISSceneARBEH(m_ArbitrarySlice, m_SceneArb.get());
        m_ArbitrarySlice->addEventHandler(m_drawARBEH.get());
        /*// Measurements event handler
        m_measurementsXYEH = new scene::CMeasurementsXYEH(m_ArbitrarySlice, m_SceneXY.get());
        m_measurementsXYEH->setHandleDensityUnderCursor(true);
        m_ArbitrarySlice->addEventHandler(m_measurementsXYEH.get());

        for (int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        {
        m_importedModelCutSliceXY[i] = new osg::CModelCutVisualizerSliceXY(data::Storage::ImportedModelCutSliceXY::Id + i, m_ArbitrarySlice);
        m_importedModelCutSliceXY[i]->setVisibility(false);
        m_importedModelCutSliceXY[i]->setColor(osg::Vec4(1.0, 1.0, 0.0, 1.0));
        m_SceneXY->addChild(m_importedModelCutSliceXY[i]);
        }

        //m_xyVizualizer = new osg::CRegionsVisualizerForSliceXY(osg::Matrix::identity(), data::Storage::SliceXY::Id, m_ArbitrarySlice);
        //m_SceneXY->addChild(m_xyVizualizer);
        m_xyVizualizer = new osg::CMultiClassRegionsVisualizerForSliceXY(osg::Matrix::identity(), data::Storage::SliceXY::Id, m_ArbitrarySlice);
        m_SceneXY->addChild(m_xyVizualizer);*/

        //m_arbVOIVizualizer = new osg::CVolumeOfInterestVisualizerForSliceXY(osg::Matrix::identity(), data::Storage::SliceXY::Id, m_ArbitrarySlice);
        //m_sceneArb->addChild(m_arbVOIVizualizer);

	// 3D view
	{
		m_3DView = new CVolumeRendererWindow(nullptr);
        m_Scene3D = new scene::CScene3D(m_3DView);
        m_Scene3D->setRenderer(&m_3DView->getRenderer());
        m_3DView->hide();
        m_3DView->setScene(m_Scene3D.get());

        //m_3DViewSubstitute = new OSGOrtho2DCanvas(nullptr);
        //m_3DViewSubstitute->hide();

        m_arbSliceVisualizer = new osg::CArbitrarySliceVisualizer(m_3DView, data::Storage::ArbitrarySlice::Id);
        m_Scene3D->anchorToScene(m_arbSliceVisualizer.get(), false);
        m_arbSliceVisualizer->setCanvas(m_3DView);

		// Initialize the model manager
		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			m_modelVisualizers[i] = new osg::CModelVisualizerEx(data::Storage::ImportedModel::Id + i);
            m_modelVisualizers[i]->m_materialRegular->applySingleLightSetup();
            m_modelVisualizers[i]->m_materialRegular->uniform("Shininess")->set(20.0f);
            m_modelVisualizers[i]->m_materialRegular->uniform("Specularity")->set(0.5f);

			osg::ref_ptr<osg::MatrixTransform> pModelTransform = new osg::MatrixTransform();
			pModelTransform->addChild(m_modelVisualizers[i]);

			m_Scene3D->anchorToScene(pModelTransform.get(), true);
			m_modelVisualizers[i]->setCanvas(m_3DView);
		}

#ifdef ENABLE_DEEPLEARNING
        // Initialize Reference Anatomical Landmark Model Visualizer
        {
            m_referenceAnatomicalLandmarkModelVisualizer = new osg::CModelVisualizerEx(data::Storage::ReferenceAnatomicalLandmarkModel::Id);
            m_referenceAnatomicalLandmarkModelVisualizer->m_materialRegular->applySingleLightSetup();
            m_referenceAnatomicalLandmarkModelVisualizer->m_materialRegular->uniform("Shininess")->set(20.0f);
            m_referenceAnatomicalLandmarkModelVisualizer->m_materialRegular->uniform("Specularity")->set(0.5f);

            osg::ref_ptr<osg::MatrixTransform> pModelTransform = new osg::MatrixTransform();
            pModelTransform->addChild(m_referenceAnatomicalLandmarkModelVisualizer);

            m_Scene3D->anchorToScene(pModelTransform.get(), true);
            m_referenceAnatomicalLandmarkModelVisualizer->setCanvas(m_3DView);

            m_referenceAnatomicalLandmarkModelVisualizer->setModelVisualization(osg::CModelVisualizerEx::EMV_SMOOTH);
        }
#endif

        // dragger for STL model
        {
            osg::Vec3 sceneSize;
            osg::Matrix modelPos;
            data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(data::Storage::PatientData::Id));
            sceneSize = osg::Vec3
            (spVolume->getXSize() * spVolume->getDX(),
                spVolume->getYSize() * spVolume->getDY(),
                spVolume->getZSize() * spVolume->getDZ()
            );
            spVolume.release();
            modelPos.makeTranslate(osg::Vec3(sceneSize.x() / 2, sceneSize.y() / 2, sceneSize.z() / 2));

            m_draggerPivot = new osg::CPivotDraggerHolder();
            m_draggerModel = new osg::C3DModelDraggerHolder(data::Storage::ReferenceAnatomicalLandmarkModel::Id, modelPos, sceneSize, true, true);
            m_draggerModel->setPivotDragger(m_draggerPivot);
#ifdef ENABLE_DEEPLEARNING
            m_draggerModel->addTransformUpdating(m_referenceAnatomicalLandmarkModelVisualizer->getModelTransform().get());
#endif
            m_switchDraggerModel = new osg::COnOffNode();
            m_switchDraggerModel->setName("Model dragger");
            m_switchDraggerModel->addChild(m_draggerModel.get());
            m_switchDraggerModel->addChild(m_draggerPivot.get());
            m_switchDraggerModel->hide();
            m_Scene3D->anchorToScene(m_switchDraggerModel.get(), true);
        }

        // Set implant draggers higher priority
        //m_3DView->addEventHandler(new scene::CDraggerEventHandler(true));
        m_3DView->addEventHandler(new scene::CDraggerEventHandler(m_3DView));

        // Region preview visualizer
        {
            m_region3DPreviewVisualizer = new osg::CRegion3DPreviewVisualizer;

            osg::ref_ptr<osg::MatrixTransform> pPreviewTransform = new osg::MatrixTransform();
            pPreviewTransform->addChild(m_region3DPreviewVisualizer);
            m_Scene3D->anchorToScene(pPreviewTransform.get(), true);
        }

        {
            //add subtree for display of drawn notes
            osg::ref_ptr<osg::CNoteDrawerNode> noteSubtree = new osg::CNoteDrawerNode();
            m_Scene3D->anchorToScene(noteSubtree, false);
        }

		//m_modelVisualizer->setModelVisualization(osg::CModelVisualizerEx::EMV_FLAT);

		if( m_3DView->getRenderer().isError() )
		{
			showMessageBox(QMessageBox::Critical,tr("VR Error!"));
		}

        m_3DView->addEventHandler(new scene::CDraggerEventHandler(m_3DView));

		// Drawing event handler
		m_draw3DEH = new osgGA::CISScene3DEH( m_3DView, m_Scene3D.get() );
		m_3DView->addEventHandler( m_draw3DEH.get() );

		// Initialize the model manager
		for (int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			m_draw3DEH->AddNode(m_modelVisualizers[i]);
		}
#ifdef ENABLE_DEEPLEARNING
        m_draw3DEH->AddNode(m_referenceAnatomicalLandmarkModelVisualizer);
#endif

		// Window drawing event handler
		m_drawW3DEH = new osgGA::CISWindowEH(m_3DView, m_Scene3D.get());

		// Modify drawing EH osg::StateSet 
        m_drawW3DEH->setLineFlags(osg::CLineGeode::USE_RENDER_BIN | osg::CLineGeode::DISABLE_DEPTH_TEST | osg::CLineGeode::ENABLE_TRANSPARENCY );
		m_drawW3DEH->setZ( 0.5 );

		//m_3DView->addEventHandler( m_drawW3DEH.get() );

		// Measurements EH
		m_measurements3DEH = new scene::CMeasurements3DEH( m_3DView, m_Scene3D.get() );
		m_measurements3DEH->setHandleDensityUnderCursor(true);
		m_3DView->addEventHandler( m_measurements3DEH.get() );

#ifdef ENABLE_DEEPLEARNING
        // Landmark annotations EH
        m_landmarkAnnotations3DEH = new scene::CLandmarkAnnotations3DEH(m_3DView, m_Scene3D.get());
        m_3DView->addEventHandler(m_landmarkAnnotations3DEH.get());
#endif

	}

    // XY Slice
	{
		m_OrthoXYSlice = new OSGOrtho2DCanvas(nullptr);
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
#ifdef ENABLE_DEEPLEARNING
        // Landmark annotations event handler
        m_landmarkAnnotationsXYEH = new scene::CLandmarkAnnotationsXYEH(m_OrthoXYSlice, m_SceneXY.get());
        m_OrthoXYSlice->addEventHandler(m_landmarkAnnotationsXYEH.get());
#endif

        //m_OrthoXYSliceSubstitute = new OSGOrtho2DCanvas(nullptr);
        //m_OrthoXYSliceSubstitute->hide();

		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			m_importedModelCutSliceXY[i] = new osg::CModelCutVisualizerSliceXY(data::Storage::ImportedModelCutSliceXY::Id + i, m_OrthoXYSlice);
			m_importedModelCutSliceXY[i]->setVisibility(false);
			m_importedModelCutSliceXY[i]->setColor(osg::Vec4(1.0,1.0,0.0,1.0));
			m_SceneXY->addChild(m_importedModelCutSliceXY[i]);
		}

		//m_xyVizualizer = new osg::CRegionsVisualizerForSliceXY(osg::Matrix::identity(), data::Storage::SliceXY::Id, m_OrthoXYSlice);
		//m_SceneXY->addChild(m_xyVizualizer);
        m_xyVizualizer = new osg::CMultiClassRegionsVisualizerForSliceXY(osg::Matrix::identity(), data::Storage::SliceXY::Id, m_OrthoXYSlice);
        m_SceneXY->addChild(m_xyVizualizer);

		m_xyVOIVizualizer = new osg::CVolumeOfInterestVisualizerForSliceXY(osg::Matrix::identity(), data::Storage::SliceXY::Id, m_OrthoXYSlice);
		//m_SceneXY->addChild(m_xyVOIVizualizer);
	}

    // XZ Slice
	{
		m_OrthoXZSlice = new OSGOrtho2DCanvas(nullptr);
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
#ifdef ENABLE_DEEPLEARNING
        // Landmark annotations event handler
        m_landmarkAnnotationsXZEH = new scene::CLandmarkAnnotationsXZEH(m_OrthoXZSlice, m_SceneXZ.get());
        m_OrthoXZSlice->addEventHandler(m_landmarkAnnotationsXZEH.get());
#endif

        //m_OrthoXZSliceSubstitute = new OSGOrtho2DCanvas(nullptr);
        //m_OrthoXZSliceSubstitute->hide();

		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			m_importedModelCutSliceXZ[i] = new osg::CModelCutVisualizerSliceXZ(data::Storage::ImportedModelCutSliceXZ::Id + i, m_OrthoXZSlice);
			m_importedModelCutSliceXZ[i]->setVisibility(false);
			m_importedModelCutSliceXZ[i]->setColor(osg::Vec4(1.0,1.0,0.0,1.0));
			m_SceneXZ->addChild(m_importedModelCutSliceXZ[i]);
		}

		//m_xzVizualizer = new osg::CRegionsVisualizerForSliceXZ(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(1.0, 0.0, 0.0)), data::Storage::SliceXZ::Id, m_OrthoXZSlice);
		//m_SceneXZ->addChild(m_xzVizualizer);
        m_xzVizualizer = new osg::CMultiClassRegionsVisualizerForSliceXZ(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(1.0, 0.0, 0.0)), data::Storage::SliceXZ::Id, m_OrthoXZSlice);
        m_SceneXZ->addChild(m_xzVizualizer);

		m_xzVOIVizualizer = new osg::CVolumeOfInterestVisualizerForSliceXZ(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(1.0, 0.0, 0.0)), data::Storage::SliceXZ::Id, m_OrthoXZSlice);
		//m_SceneXZ->addChild(m_xzVOIVizualizer);
	}

    // YZ Slice
	{
		m_OrthoYZSlice = new OSGOrtho2DCanvas(nullptr);
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
#ifdef ENABLE_DEEPLEARNING
        // Landmark annotations event handler
        m_landmarkAnnotationsYZEH = new scene::CLandmarkAnnotationsYZEH(m_OrthoYZSlice, m_SceneYZ.get());
        m_OrthoYZSlice->addEventHandler(m_landmarkAnnotationsYZEH.get());
#endif

        //m_OrthoYZSliceSubstitute = new OSGOrtho2DCanvas(nullptr);
        //m_OrthoYZSliceSubstitute->hide();

		for(int i = 0; i < MAX_IMPORTED_MODELS; ++i)
		{
			m_importedModelCutSliceYZ[i] = new osg::CModelCutVisualizerSliceYZ(data::Storage::ImportedModelCutSliceYZ::Id + i, m_OrthoYZSlice);
			m_importedModelCutSliceYZ[i]->setVisibility(false);
			m_importedModelCutSliceYZ[i]->setColor(osg::Vec4(1.0,1.0,0.0,1.0));
			m_SceneYZ->addChild(m_importedModelCutSliceYZ[i]);
		}

		//m_yzVizualizer = new osg::CRegionsVisualizerForSliceYZ(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(1.0, 0.0, 0.0)) * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 1.0, 0.0)), data::Storage::SliceYZ::Id, m_OrthoYZSlice);
		//m_SceneYZ->addChild(m_yzVizualizer);
        m_yzVizualizer = new osg::CMultiClassRegionsVisualizerForSliceYZ(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(1.0, 0.0, 0.0)) * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 1.0, 0.0)), data::Storage::SliceYZ::Id, m_OrthoYZSlice);
        m_SceneYZ->addChild(m_yzVizualizer);

		m_yzVOIVizualizer = new osg::CVolumeOfInterestVisualizerForSliceYZ(osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(1.0, 0.0, 0.0)) * osg::Matrix::rotate(osg::DegreesToRadians(-90.0), osg::Vec3(0.0, 1.0, 0.0)), data::Storage::SliceYZ::Id, m_OrthoYZSlice);
		//m_SceneYZ->addChild(m_yzVOIVizualizer);
	}

	m_contoursVisible = true;
	m_VOIVisible = true;

	setVOIVisibility(false);
}

///////////////////////////////////////////////////////////////////////////////

// Whenever the widget or a parent of it gets reparented so that the top-level window becomes different, 
// the widget's associated context is destroyed and a new one is created. This is then followed by a call 
// to initializeGL() where all OpenGL resources must get reinitialized. 
QWidget* createGlWindowContainer(QWidget* pChild, int nIter = 1)
{
    if (nIter == 0)
        return pChild;
    QFrame* pContainer = new QFrame();
    pContainer->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    pContainer->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    QGridLayout * layout = new QGridLayout();
    layout->setSpacing(1);
    layout->setContentsMargins(0, 0, 0, 0);
    pContainer->setLayout(layout);
    QWidget * pW = createGlWindowContainer(pChild, nIter - 1);
    layout->addWidget(pW,0,0);     
    return pContainer;
}

void MainWindow::setupDockWindows()
{
    QSettings settings;
    QColor color = settings.value("BGColor", DEFAULT_BACKGROUND_COLOR).toUInt();

    QPalette pal = palette();
    pal.setColor(QPalette::Background, color);

    /************************ 3D ********************************/
    CCustomDockWidget* wnd3DView = new CCustomDockWidget(View3D);
    wnd3DView->setWindowTitle(tr("3D"));    
    wnd3DView->setAllowedAreas(Qt::NoDockWidgetArea);
    wnd3DView->setFeatures(0);
    wnd3DView->setObjectName("3D View");
    wnd3DView->setMinimumSize(200, 150);
    //wnd3DView->setWidget(createGlWindowContainer(m_3DView));
    wnd3DView->setProperty("Icon",":/svg/svg/icon3d.svg");
    wnd3DView->setProperty("View", View3D);

    QWidget* inner3D = new QWidget(wnd3DView);
    QVBoxLayout* layout3D = new QVBoxLayout();
    layout3D->setMargin(0);
    layout3D->setSpacing(0);
    QWidget* container3D = createGlWindowContainer(m_3DView);
    container3D->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout3D->addWidget(container3D);
    inner3D->setLayout(layout3D);
    wnd3DView->setWidget(inner3D);

    QDockWidget* wnd3DViewSubstitute = new QDockWidget();
    //wnd3DViewSubstitute->setWindowTitle(tr("3D"));
    wnd3DViewSubstitute->setAllowedAreas(Qt::NoDockWidgetArea);
    wnd3DViewSubstitute->setFeatures(0);
    wnd3DViewSubstitute->setObjectName("3D View Substitute");
    wnd3DViewSubstitute->setMinimumSize(200, 150);
    //wnd3DViewSubstitute->setProperty("Icon", ":/svg/svg/icon3d.svg");

    QWidget* inner3DSubstitute = new QWidget();

    inner3DSubstitute->setAutoFillBackground(true);
    inner3DSubstitute->setPalette(pal);

    QVBoxLayout* layout3DSubstitute = new QVBoxLayout();
    inner3DSubstitute->setLayout(layout3DSubstitute);
    wnd3DViewSubstitute->setWidget(inner3DSubstitute);

    /************************************************************/

    /************************ XY ********************************/
    CCustomDockWidget* wndXYView = new CCustomDockWidget(ViewXY);
    wndXYView->setWindowTitle(tr("Axial / XY"));
    wndXYView->setAllowedAreas(Qt::NoDockWidgetArea);
    wndXYView->setFeatures(0);
    wndXYView->setObjectName("Axial View");
    wndXYView->setMinimumSize(200,150);
    //wndXYView->setWidget(createGlWindowContainer(m_OrthoXYSlice));
    wndXYView->setProperty("Icon",":/svg/svg/iconxy.svg");
    wndXYView->setProperty("View", ViewXY);

    QWidget* innerXY = new QWidget(wndXYView);
    QVBoxLayout* layoutXY = new QVBoxLayout();
    layoutXY->setMargin(0);
    layoutXY->setSpacing(0);
    QWidget* containerXY = createGlWindowContainer(m_OrthoXYSlice);
    containerXY->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layoutXY->addWidget(containerXY);
    innerXY->setLayout(layoutXY);
    wndXYView->setWidget(innerXY);

    QDockWidget* wndXYViewSubstitute = new QDockWidget();
    //wndXYViewSubstitute->setWindowTitle(tr("Axial / XY"));
    wndXYViewSubstitute->setAllowedAreas(Qt::NoDockWidgetArea);
    wndXYViewSubstitute->setFeatures(0);
    wndXYViewSubstitute->setObjectName("Axial View Substitute");
    wndXYViewSubstitute->setMinimumSize(200, 150);
    //wndXYViewSubstitute->setProperty("Icon", ":/svg/svg/iconxy.svg");

    QWidget* innerXYSubstitute = new QWidget();

    innerXYSubstitute->setAutoFillBackground(true);
    innerXYSubstitute->setPalette(pal);

    QVBoxLayout* layoutXYSubstitute = new QVBoxLayout();
    innerXYSubstitute->setLayout(layoutXYSubstitute);
    wndXYViewSubstitute->setWidget(innerXYSubstitute);

    /************************************************************/

    /************************ XZ ********************************/
    CCustomDockWidget* wndXZView = new CCustomDockWidget(ViewXZ);
    wndXZView->setWindowTitle(tr("Coronal / XZ"));
    wndXZView->setAllowedAreas(Qt::NoDockWidgetArea);
    wndXZView->setFeatures(0);
    wndXZView->setObjectName("Coronal View");
    wndXZView->setMinimumSize(200,150);
    //wndXZView->setWidget(createGlWindowContainer(m_OrthoXZSlice));
    wndXZView->setProperty("Icon",":/svg/svg/iconxz.svg");
    wndXZView->setProperty("View", ViewXZ);

    QWidget* innerXZ = new QWidget(wndXZView);
    QVBoxLayout* layoutXZ = new QVBoxLayout();
    layoutXZ->setMargin(0);
    layoutXZ->setSpacing(0);
    QWidget* containerXZ = createGlWindowContainer(m_OrthoXZSlice);
    containerXZ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layoutXZ->addWidget(containerXZ);
    innerXZ->setLayout(layoutXZ);
    wndXZView->setWidget(innerXZ);

    QDockWidget* wndXZViewSubstitute = new QDockWidget();
    //wndXZViewSubstitute->setWindowTitle(tr("Coronal / XZ"));
    wndXZViewSubstitute->setAllowedAreas(Qt::NoDockWidgetArea);
    wndXZViewSubstitute->setFeatures(0);
    wndXZViewSubstitute->setObjectName("Coronal View Substitute");
    wndXZViewSubstitute->setMinimumSize(200, 150);
    //wndXZViewSubstitute->setProperty("Icon", ":/svg/svg/iconxz.svg");

    QWidget* innerXZSubstitute = new QWidget();

    innerXZSubstitute->setAutoFillBackground(true);
    innerXZSubstitute->setPalette(pal);

    QVBoxLayout* layoutXZSubstitute = new QVBoxLayout();
    innerXZSubstitute->setLayout(layoutXZSubstitute);
    wndXZViewSubstitute->setWidget(innerXZSubstitute);

    /************************************************************/

    /************************ YZ ********************************/
    CCustomDockWidget* wndYZView = new CCustomDockWidget(ViewYZ);
    wndYZView->setWindowTitle(tr("Sagittal / YZ"));
    wndYZView->setAllowedAreas(Qt::NoDockWidgetArea);
    wndYZView->setFeatures(0);
    wndYZView->setObjectName("Sagittal View");
    wndYZView->setMinimumSize(200,150);
    //wndYZView->setWidget(createGlWindowContainer(m_OrthoYZSlice));
    wndYZView->setProperty("Icon",":/svg/svg/iconyz.svg");
    wndYZView->setProperty("View", ViewYZ);

    QWidget* innerYZ = new QWidget(wndYZView);
    QVBoxLayout* layoutYZ = new QVBoxLayout();
    layoutYZ->setMargin(0);
    layoutYZ->setSpacing(0);
    QWidget* containerYZ = createGlWindowContainer(m_OrthoYZSlice);
    containerYZ->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layoutYZ->addWidget(containerYZ);
    innerYZ->setLayout(layoutYZ);
    wndYZView->setWidget(innerYZ);

    QDockWidget* wndYZViewSubstitute = new QDockWidget();
    //wndYZViewSubstitute->setWindowTitle(tr("Sagittal / YZ"));
    wndYZViewSubstitute->setAllowedAreas(Qt::NoDockWidgetArea);
    wndYZViewSubstitute->setFeatures(0);
    wndYZViewSubstitute->setObjectName("Sagittal View Substitute");
    wndYZViewSubstitute->setMinimumSize(200, 150);
    //wndYZViewSubstitute->setProperty("Icon", ":/svg/svg/iconyz.svg");

    QWidget* innerYZSubstitute = new QWidget();

    innerYZSubstitute->setAutoFillBackground(true);
    innerYZSubstitute->setPalette(pal);

    QVBoxLayout* layoutYZSubstitute = new QVBoxLayout();
    innerYZSubstitute->setLayout(layoutYZSubstitute);
    wndYZViewSubstitute->setWidget(innerYZSubstitute);

    /************************************************************/

    /********************* Arbitrary ****************************/
    CCustomDockWidget* wndArbitraryView = new CCustomDockWidget(ViewARB);
    wndArbitraryView->setWindowTitle(tr("Arbitrary"));
    wndArbitraryView->setAllowedAreas(Qt::NoDockWidgetArea);
    wndArbitraryView->setFeatures(0);
    wndArbitraryView->setObjectName("Arbitrary View");
    wndArbitraryView->setMinimumSize(200, 150);
    //wndArbitraryView->setWidget(createGlWindowContainer(m_ArbitrarySlice));
    wndArbitraryView->setProperty("Icon", ":/svg/svg/iconarb.svg");
    wndArbitraryView->setProperty("View", ViewARB);

    QWidget* innerArbitrary = new QWidget(wndArbitraryView);
    QVBoxLayout* layoutArbitrary = new QVBoxLayout();
    layoutArbitrary->setMargin(0);
    layoutArbitrary->setSpacing(0);
    QWidget* containerArbitrary = createGlWindowContainer(m_ArbitrarySlice);
    containerArbitrary->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layoutArbitrary->addWidget(containerArbitrary);
    innerArbitrary->setLayout(layoutArbitrary);
    wndArbitraryView->setWidget(innerArbitrary);

    QDockWidget* wndArbitraryViewSubstitute = new QDockWidget();
    //wndArbitraryViewSubstitute->setWindowTitle(tr("Arbitrary"));
    wndArbitraryViewSubstitute->setAllowedAreas(Qt::NoDockWidgetArea);
    wndArbitraryViewSubstitute->setFeatures(0);
    wndArbitraryViewSubstitute->setObjectName("Arbitrary View Substitute");
    wndArbitraryViewSubstitute->setMinimumSize(200, 150);
    //wndArbitraryViewSubstitute->setProperty("Icon", ":/svg/svg/iconarb.svg");

    QWidget* innerArbitrarySubstitute = new QWidget();

    innerArbitrarySubstitute->setAutoFillBackground(true);
    innerArbitrarySubstitute->setPalette(pal);

    QVBoxLayout* layoutArbitrarySubstitute = new QVBoxLayout();
    innerArbitrarySubstitute->setLayout(layoutArbitrarySubstitute);
    wndArbitraryViewSubstitute->setWidget(innerArbitrarySubstitute);

    /************************************************************/

    m_wndViews.push_back(wnd3DView);
    m_wndViews.push_back(wndXYView);
    m_wndViews.push_back(wndXZView);
    m_wndViews.push_back(wndYZView);
    m_wndViews.push_back(wndArbitraryView);

    m_wndViewsSubstitute.push_back(wnd3DViewSubstitute);
    m_wndViewsSubstitute.push_back(wndXYViewSubstitute);
    m_wndViewsSubstitute.push_back(wndXZViewSubstitute);
    m_wndViewsSubstitute.push_back(wndYZViewSubstitute);
    m_wndViewsSubstitute.push_back(wndArbitraryViewSubstitute);

    assert(m_wndViews.size() == m_wndViewsSubstitute.size());

    for (size_t i = 0; i < m_wndViews.size(); ++i)
    {
        connect(m_wndViews[i], SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
        connect(m_wndViews[i], SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
        //connect(m_wndViews[i], SIGNAL(topLevelChanged(bool)), this, SLOT(dockWidget_topLevelChanged(bool)));
        //m_wndViews[i]->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);

        connect(m_wndViewsSubstitute[i], SIGNAL(visibilityChanged(bool)), this, SLOT(dockWidgetVisiblityChanged(bool)));
        connect(m_wndViewsSubstitute[i], SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
        //connect(m_wndViewsSubstitute[i], SIGNAL(topLevelChanged(bool)), this, SLOT(dockWidget_topLevelChanged(bool)));
        //m_wndViewsSubstitute[i]->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    }
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

void MainWindow::dockWidget_topLevelChanged(bool changed)
{
    CCustomDockWidget* dw = static_cast<CCustomDockWidget*>(QObject::sender());

    unsigned int view = dw->property("View").toUInt();

    int index = getIndexOfWndInLayout(view);

    if (index < 0 || index >= m_viewsToShow.size())
    {
        return;
    }

    if (m_viewsToShow[index].bFloating && m_viewsToShow[index].bShowSubstitute)
    {
        dw->setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
        dw->show();
    }
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
            delete [] buffer;
        }
    }
    return convFilename;
}
#endif

bool MainWindow::getModelDraggerVisibility()
{
    return m_switchDraggerModel->isVisible();
}

void MainWindow::setModelDraggerVisibility(bool visible)
{
    m_switchDraggerModel->setOnOffState(visible);
}

int MainWindow::getModelDraggerModelId()
{
    return m_draggerModel->getModelID();
}

void MainWindow::setModelDraggerModelId(int modelId, bool on)
{
    if (-1 == modelId && !on && m_switchDraggerModel.get())
    {
        if (!m_switchDraggerModel->isVisible())
            return;
    }

    // unplug dragger from previous models
    for (int i = 0; i < MAX_IMPORTED_MODELS; i++)
    {
        m_draggerModel->removeTransformUpdating(m_modelVisualizers[i]->getModelTransform().get());
    }
#ifdef ENABLE_DEEPLEARNING
    m_draggerModel->removeTransformUpdating(m_referenceAnatomicalLandmarkModelVisualizer->getModelTransform().get());
#endif

    // hide dragger
    if (!on)
        m_switchDraggerModel->hide();

    if (on && modelId >= 0)
    {
        data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(modelId, data::Storage::NO_UPDATE));
        const geometry::CMesh* pMesh = spModel->getMesh();
        if (pMesh && pMesh->n_vertices() > 0)
        {
            bool setPivot = true;
            m_draggerModel->setModelID(modelId, setPivot);

            switch (modelId)
            {
#ifdef ENABLE_DEEPLEARNING
            case data::Storage::ReferenceAnatomicalLandmarkModel::Id:
                m_draggerModel->addTransformUpdating(m_referenceAnatomicalLandmarkModelVisualizer->getModelTransform().get());
                break;
#endif

            default:
                Q_ASSERT(modelId >= data::Storage::ImportedModel::Id && modelId<data::Storage::ImportedModel::Id + MAX_IMPORTED_MODELS);
                m_draggerModel->addTransformUpdating(m_modelVisualizers[modelId - data::Storage::ImportedModel::Id]->getModelTransform().get());
                break;
            }

            m_switchDraggerModel->show();
        }
    }

    // refresh 3D window
    m_3DView->Refresh(false);
}

void MainWindow::onDraggerMove(int eventType, bool setMatrix)
{
    // store undo record
    if (scene::CAppMode::PUSH == eventType)
    {
        int idModel = m_draggerModel->getModelID();
        data::CObjectPtr<data::CPivot> spPivot(APP_STORAGE.getEntry(data::Storage::ModelPivot::Id));
        std::vector<int> modelList;
        if (idModel>0)
            modelList.push_back(idModel);
        m_ModelManager.createAndStoreSnapshot(modelList, false, spPivot->getSnapshot(NULL));
    }

    // invalidate model to update cuts
    if ((setMatrix) && (scene::CAppMode::RELEASE == eventType || scene::CAppMode::DRAG == eventType))
    {
        if (!m_switchDraggerModel->isVisible())
            return;
        int idModel = m_draggerModel->getModelID();
        data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(idModel, data::Storage::NO_UPDATE));
        if (spModel->getMesh())
        {
            // update model matrix
#ifdef ENABLE_DEEPLEARNING
            if (data::Storage::ReferenceAnatomicalLandmarkModel::Id == idModel)
                spModel->setTransformationMatrix(m_referenceAnatomicalLandmarkModelVisualizer->getModelTransform()->getMatrix());
#endif
            if (idModel >= data::Storage::ImportedModel::Id)
                spModel->setTransformationMatrix(m_modelVisualizers[idModel - data::Storage::ImportedModel::Id]->getModelTransform()->getMatrix());
            // invalidate model
            APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::POSITION_CHANGED);
        }
    }
}

void MainWindow::mouseModeDensityWindow(bool checked)
{
    if (checked)
    {
        APP_MODE.setAndStore(scene::CAppMode::MODE_DENSITY_WINDOW);
        //APP_MODE.storeAndSet(scene::CAppMode::MODE_DENSITY_WINDOW);
        //APP_MODE.set(scene::CAppMode::MODE_DENSITY_WINDOW);
    }
    else
    {
        APP_MODE.setAndStore(scene::CAppMode::MODE_TRACKBALL);
    }
}

void MainWindow::mouseModeTrackball(bool checked)
{
    if (checked)
    {
        APP_MODE.setAndStore(scene::CAppMode::MODE_TRACKBALL);
        //APP_MODE.storeAndSet(scene::CAppMode::MODE_TRACKBALL);
        //APP_MODE.set(scene::CAppMode::MODE_TRACKBALL);
    }
}

void MainWindow::mouseModeObjectManipulation(bool checked)
{
    if (checked)
    {
        APP_MODE.setAndStore(scene::CAppMode::MODE_SLICE_MOVE);
        //APP_MODE.storeAndSet(scene::CAppMode::MODE_SLICE_MOVE);
        //APP_MODE.set(scene::CAppMode::MODE_SLICE_MOVE);
    }
    else
    {
        APP_MODE.setAndStore(scene::CAppMode::MODE_TRACKBALL);
    }
}

void MainWindow::mouseModeZoom(bool checked)
{
    if (checked)
    {
        APP_MODE.set(scene::CAppMode::COMMAND_SCENE_ZOOM);
    }
    else
    {
        APP_MODE.setAndStore(scene::CAppMode::MODE_TRACKBALL);
    }
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

void MainWindow::showArbitrarySlice(bool bShow)
{
    VPL_SIGNAL(SigSetPlaneARBVisibility).invoke(bShow);
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
    if (id < 0)
        return;

    CProgress *progress = new CProgress(this);
    progress->setLabelText(tr("Reducing small submeshes, please wait..."));
    progress->show();
    progress->setLoopsCnt(4);

    progress->setCurrentLoopIndex(0);
    progress->setLabelText(tr("Creating model, please wait..."));
    progress->show();

    {   // if there is already an existing model which is large, free it to save memory
        data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(id));
        const geometry::CMesh* pMesh = spModel->getMesh();
        if (NULL != pMesh && pMesh->n_vertices() > 1000000)
        {
            spModel->setMesh(new geometry::CMesh());
            spModel->clearAllProperties();
            APP_STORAGE.invalidate(spModel.getEntryPtr());
        }
    }

    data::CObjectPtr<data::CDensityData> spVolume(APP_STORAGE.getEntry(m_Examination.getActiveDataSet()));

    vpl::img::tDensityPixel Low = 500;
    vpl::img::tDensityPixel Hi = 2000;
    if (NULL != m_segmentationPanel)
    {
        Low = m_segmentationPanel->getLo();
        Hi = m_segmentationPanel->getHi();
    }

    geometry::CMesh* pMesh = new geometry::CMesh;

    CMarchingCubes mc;
    mc.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(progress, &CProgress::Entry));
    vpl::img::CSize3d voxelSize = vpl::img::CSize3d(spVolume->getDX(), spVolume->getDY(), spVolume->getDZ());
    CThresholdFunctor<vpl::img::CDensityVolume, vpl::img::tDensityPixel> ThresholdFunc(Low, Hi, &(*spVolume), voxelSize);

    if (!mc.generateMesh(*pMesh, &ThresholdFunc, true))
    {
        delete pMesh;
        delete progress;
        return;
    }

    spVolume.release();

    progress->setCurrentLoopIndex(1);
    progress->setLabelText(tr("Reducing small areas, please wait..."));
    progress->show();

    CSmallSubmeshReducer rc;
    rc.reduce(*pMesh, 500);

    progress->setCurrentLoopIndex(2);
    progress->setLabelText(tr("Smoothing model, please wait..."));
    progress->show();

    // Smooth model
    CSmoothing sm;
    sm.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(progress, &CProgress::Entry));
    //    double p1 = 0.6073;
    //    double p2 = 0.1;
    if (!sm.Smooth(*pMesh, 10))
    {
        delete pMesh;
        delete progress;
        return;
    }

    progress->setCurrentLoopIndex(3);
    progress->setLabelText(tr("Decimating model, please wait..."));
    progress->show();

    // Decimate model
    CDecimator dc;
    dc.registerProgressFunc(vpl::mod::CProgress::tProgressFunc(progress, &CProgress::Entry));
    int output_mesh_size_tri = (pMesh->n_faces() * 20) / 100;
    int output_mesh_size_vert = 1;
    if (!dc.Reduce(*pMesh, output_mesh_size_vert, output_mesh_size_tri))
    {
        delete pMesh;
        delete progress;
        return;
    }

    delete progress;

    data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(id));
    spModel->setMesh(pMesh);
    spModel->setLabel(m_modelLabel);
    spModel->setColor(m_modelColor);
    spModel->clearAllProperties();
    spModel->setProperty("Created", "1");
    spModel->setTransformationMatrix(osg::Matrix::identity());
    spModel->setVisibility(true);
    spModel->setLinkedWithRegion(false);
    spModel->setRegionId(-1);
    APP_STORAGE.invalidate(spModel.getEntryPtr());

    // if model-region linking is not enabled, hide previously created models on creation of a new one
    QSettings settings;
    bool bModelsLinked = settings.value("ModelRegionLinkEnabled", QVariant(DEFAULT_MODEL_REGION_LINK)).toBool();
    if (!bModelsLinked)
    {
        for (int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        {
            if (data::Storage::ImportedModel::Id + i == id) continue;
            data::CObjectPtr<data::CModel> spModel2(APP_STORAGE.getEntry(data::Storage::ImportedModel::Id + i));
            if (spModel2->isVisible())
            {
                std::string created = spModel2->getProperty("Created");
                if (!created.empty())
                    VPL_SIGNAL(SigSetModelVisibility).invoke(data::Storage::ImportedModel::Id + i, false);
            }
        }
    }

    bool bAnyVisible(false);
    for (int i = 0; i < MAX_IMPORTED_MODELS; ++i)
        if (VPL_SIGNAL(SigGetModelVisibility).invoke2(data::Storage::ImportedModel::Id + i))
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
	CPreferencesDialog dlg(m_localeDir, ui->menuBar, m_eventFilter, this, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
    if (QDialog::Accepted==dlg.exec())
    {
        m_eventFilter.processSettings();

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
            m_ArbitrarySlice->setBackgroundColor(osgColor);
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
	msgBox.setStandardButtons(QMessageBox::Ok);
	QPushButton* feedbackButton = msgBox.addButton(tr("Give Us Your Feedback..."), QMessageBox::AcceptRole);
	QObject::connect(feedbackButton, SIGNAL(clicked()), this, SLOT(showFeedback()));
	msgBox.setDefaultButton(feedbackButton);
	msgBox.setEscapeButton(QMessageBox::Ok);
    msgBox.setIconPixmap(QPixmap(":/svg/svg/3dim.svg"));
    msgBox.setWindowTitle(tr("About ") + QCoreApplication::applicationName());
    msgBox.setText(product + "\n"
                   + tr("Lightweight 3D DICOM viewer.\n")
#ifdef TRIDIM_USE_GDCM
                   + tr("GDCM version.\n\n")
#else
                   + tr("DCMTK version.\n\n")
#endif
                   + QObject::trUtf8("Copyright %1 2008-%2 by TESCAN 3DIM s.r.o.").arg(QChar(169)).arg(QDate::currentDate().year()) + "\n" 
                   + tr("https://www.tescan3dim.com/")
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

void MainWindow::showFeedback()
{
	QDesktopServices::openUrl(QUrl("https://goo.gl/forms/bs1va1HIvubCeA5S2"));
}

void MainWindow::showTutorials()
{
	QDesktopServices::openUrl(QUrl("http://www.3dim-laboratory.cz/en/software/3dimviewer/tutorials/"));
}

void MainWindow::showHelp()
{
	// help moved to pdf file

	QSettings settings;
	QString lngFile = settings.value("Language", "").toString();
	QString path;
	QString basePath = qApp->applicationDirPath();
	if (!QFileInfo(basePath + "/doc").exists())
	{
		QDir dir(basePath);
		dir.cdUp();
		basePath = dir.absolutePath();
	}

	if (!lngFile.isEmpty())
	{
		path = basePath + "/doc/help_" + QLocale::system().name() + ".pdf";
		QFileInfo pathInfo(path);
		if (!pathInfo.exists())
			path = basePath + "/doc/help.pdf";
	}
	else
		path = basePath + "/doc/help.pdf";

	if (!path.isEmpty())
	{
		QFile file(path);
		if (file.exists())
		{
			if (!QDesktopServices::openUrl(QUrl::fromLocalFile(file.fileName())))
				QMessageBox::critical(NULL, QCoreApplication::applicationName(), tr("A PDF viewer is required to view help!"));
		}
		else
			QMessageBox::critical(NULL, QCoreApplication::applicationName(), tr("Help file is missing!"));
	}    
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
		if (QMessageBox::Yes != QMessageBox::warning(this, QCoreApplication::applicationName(), tr("By applying the filter you will modify and ovewrite the original density data! Do you want to proceed?"), QMessageBox::Yes, QMessageBox::No))
			return;

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
		if (QMessageBox::Yes != QMessageBox::warning(this, QCoreApplication::applicationName(), tr("By applying the filter you will modify and ovewrite the original density data! Do you want to proceed?"), QMessageBox::Yes, QMessageBox::No))
			return;

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
		if (QMessageBox::Yes != QMessageBox::warning(this, QCoreApplication::applicationName(), tr("By applying the filter you will modify and ovewrite the original density data! Do you want to proceed?"), QMessageBox::Yes, QMessageBox::No))
			return;

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
		if (QMessageBox::Yes != QMessageBox::warning(this, QCoreApplication::applicationName(), tr("By applying the filter you will modify and ovewrite the original density data! Do you want to proceed?"), QMessageBox::Yes, QMessageBox::No))
			return;

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
			for(vpl::tSize z=0;z<zSize;z++)
			{
				for(vpl::tSize y=0;y<ySize;y++)
				{
					for(vpl::tSize x=0;x<xSize;x++)
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
        CSyncedSplitter* tSplitter = qobject_cast<CSyncedSplitter*>(vSplitter->widget(0));
        if (tSplitter)
        {
            if (tSplitter->ignoreMove()) return;
            tSplitter->setIgnoreMove(true);
        }
        CSyncedSplitter* bSplitter = qobject_cast<CSyncedSplitter*>(vSplitter->widget(1));
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
        CSyncedSplitter* bSplitter = qobject_cast<CSyncedSplitter*>(vSplitter->widget(1));
        if (bSplitter)
        {
            if (bSplitter->ignoreMove()) return;
            bSplitter->setIgnoreMove(true);
        }
        CSyncedSplitter* tSplitter = qobject_cast<CSyncedSplitter*>(vSplitter->widget(0));
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

        QOpenGLWidget* myWidget=m_3DView;
        double xscale = printer.pageRect().width()/double(myWidget->width());
        double yscale = printer.pageRect().height()/double(myWidget->height());
        double scale = qMin(xscale, yscale);

        QRect destRect(printer.pageRect().x(),
                        printer.pageRect().y(),
                        scale*myWidget->width(),
                        scale*myWidget->height());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        QImage fb;
        QOpenGLWidget* pQt5GlWidget = dynamic_cast<QOpenGLWidget*>(myWidget);
        if (NULL != pQt5GlWidget)
            fb = pQt5GlWidget->grabFramebuffer();
        /*QGLWidget* pQt4GlWidget = dynamic_cast<QGLWidget*>(myWidget);
        if (NULL != pQt4GlWidget)
            fb = pQt4GlWidget->grabFrameBuffer();*/
#else
        QImage fb=myWidget->grabFrameBuffer();
#endif
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

void MainWindow::receiveDicomData()
{
    //TODO show dialog to notify user, that this function is not implemented when using GDMC?
#if !defined(TRIDIM_USE_GDCM)
    CDicomTransferDialog dlg(this);
    dlg.exec();
    if (!dlg.m_openDicomFolder.isEmpty())
    {
		openDICOM(dlg.m_openDicomFolder, dlg.m_openDicomFolder, false);
    }
#else
    QMessageBox::critical(this, QCoreApplication::applicationName(),tr("Not implemented"),QMessageBox::Ok);
#endif

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
    triggerPluginAction("Gauge","measure_density");
}

//! Measure Distance using Gauge plugin
void MainWindow::measureDistance(bool on)
{
    triggerPluginAction("Gauge","measure_distance");
}

//! Clear Measurements using Gauge plugin
void MainWindow::clearMeasurements()
{
    triggerPluginAction("Gauge","clear_measurements");
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
                    double positionOrig = settings.value("Position").toDouble();
                    double positionX = settings.value("PositionX", positionOrig).toDouble();
                    double positionY = settings.value("PositionY", 0.0).toDouble();
                    double density = settings.value("Density", true).toBool();
                    double gradient = settings.value("Gradient", false).toBool();
                    double radius = settings.value("Radius", 0.0).toDouble();
                    osg::Vec4 color;
                    color.r() = settings.value("ColorR").toFloat();
                    color.g() = settings.value("ColorG").toFloat();
                    color.b() = settings.value("ColorB").toFloat();
                    color.a() = settings.value("ColorA").toFloat();
                    settings.endGroup(); // point

                    lut->addPoint(c, osg::Vec2d(positionX, positionY), color, density, gradient, radius);
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
	renderer.setShader(static_cast<PSVR::PSVolumeRendering::EShaders>(settings.value("VRShader", static_cast<int>(PSVR::PSVolumeRendering::EShaders::SURFACE)).toInt()));
	renderer.setLut(static_cast<PSVR::PSVolumeRendering::ELookups>(settings.value("VRLUT", static_cast<int>(renderer.getLut())).toInt()));

    VPL_SIGNAL(SigSetPlaneXYVisibility).invoke(settings.value("ShowXYSlice", false /*VPL_SIGNAL(SigGetPlaneXYVisibility).invoke2()*/).toBool());
    VPL_SIGNAL(SigSetPlaneXZVisibility).invoke(settings.value("ShowZXSlice", false /*VPL_SIGNAL(SigGetPlaneXZVisibility).invoke2()*/).toBool());
    VPL_SIGNAL(SigSetPlaneYZVisibility).invoke(settings.value("ShowYZSlice", false /*VPL_SIGNAL(SigGetPlaneYZVisibility).invoke2()*/).toBool());
    VPL_SIGNAL(SigSetPlaneARBVisibility).invoke(settings.value("ShowARBSlice", false /*VPL_SIGNAL(SigGetPlaneARBVisibility).invoke2()*/).toBool());

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
                settings.setValue("PositionX", it->second.pointPosition(c, p).x());
                settings.setValue("PositionY", it->second.pointPosition(c, p).y());
                osg::Vec4 color = it->second.pointColor(c, p);
                settings.setValue("ColorR", (double)color.r());
                settings.setValue("ColorG", (double)color.g());
                settings.setValue("ColorB", (double)color.b());
                settings.setValue("ColorA", (double)color.a());
                settings.setValue("Density", it->second.pointDensity(c, p));
                settings.setValue("Gradient", it->second.pointGradient(c, p));
                settings.setValue("Radius", it->second.pointRadius(c, p));
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
	if (PSVR::PSVolumeRendering::EShaders::CUSTOM != renderer.getShader()) // don't save vr shader info when custom shader is selected
	{
		settings.setValue("VRShader", static_cast<int>(renderer.getShader()));
		settings.setValue("VRLUT", static_cast<int>(renderer.getLut()));
	}
    settings.setValue("ShowXYSlice",VPL_SIGNAL(SigGetPlaneXYVisibility).invoke2());
    settings.setValue("ShowZXSlice",VPL_SIGNAL(SigGetPlaneXZVisibility).invoke2());
    settings.setValue("ShowYZSlice",VPL_SIGNAL(SigGetPlaneYZVisibility).invoke2());
    settings.setValue("ShowARBSlice",VPL_SIGNAL(SigGetPlaneARBVisibility).invoke2());

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
        openSTL(wsSTLName, "Model");
    }
    QString wsDICOMName = qApp->property("OpenDICOM").toString();
    if (!wsDICOMName.isEmpty())
    {
        if (wsDICOMName.endsWith(".zip",Qt::CaseInsensitive))
            openDICOMZIP(wsDICOMName);
        else
            openDICOM(wsDICOMName, wsDICOMName, true);
    }

    //QSettings settings;
    //bool bPing = settings.value("Ping", QVariant(true)).toBool();
    //if (bPing)
        pingOurServer();
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

void MainWindow::saveScreenshot(int viewType)
{
    switch (viewType)
    {
        case ViewARB:
        {
            saveScreenshotOfCanvas(m_ArbitrarySlice);
        }
        break;
        case ViewXY:
        {
            saveScreenshotOfCanvas(m_OrthoXYSlice);
        }
        break;
        case ViewXZ:
        {
            saveScreenshotOfCanvas(m_OrthoXZSlice);
        }
        break;
        case ViewYZ:
        {
            saveScreenshotOfCanvas(m_OrthoYZSlice);
        }
        break;
        case View3D:
        {
            saveScreenshotOfCanvas(m_3DView);
        }
        default:
        {
        }
        break;
    }
}

void  MainWindow::saveScreenshotOfCanvas(OSGCanvas* pCanvas)
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
            if (!pImage->save(baseFileName, 0, SCREENSHOT_SAVE_QUALITY))
                QMessageBox::critical(this, QCoreApplication::applicationName(), tr("Save failed! Please check available diskspace and try again."), QMessageBox::Ok);
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
	data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(data::Storage::PatientData::Id));

	QString dataType = QString::fromStdString(spData->m_sModality);

    m_grayLevelLabel->setText(QString((dataType == "MR") ? tr("Intensity Level: %1") : tr("Density Level: %1")).arg(value));
}

void MainWindow::updateTabIcons()
{   
    double sizeFactor = C3DimApplication::getDpiFactor();
    // check event filters and update icon size
    QList<QTabBar*> tabs = findChildren<QTabBar*>();
    foreach (QTabBar * tabBar, tabs)
    {
        tabBar->installEventFilter(&m_tabsEventFilter);
        if (sizeFactor>1)
            tabBar->setIconSize(QSize(20*sizeFactor,20*sizeFactor));
    }

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
        if (changes.checkFlagAny(data::StorageEntry::DESERIALIZED) || changes.checkFlagAny(data::Storage::STORAGE_RESET))
        {
            setProperty("SegmentationChanged",false);
            return;
        }
        else
        {
            m_region3DPreviewManager->regionDataChanged();
        }

    }
    setProperty("SegmentationChanged",true);
}

void MainWindow::sigRegionColoringChanged(data::CStorageEntry *pEntry)
{
    data::CChangedEntries changes;
    unsigned int ver = pEntry->getLatestVersion();
    if (ver > 0 && pEntry->getChanges(ver - 1, changes))
    {
        if (changes.checkFlagAny(data::StorageEntry::DESERIALIZED) || changes.checkFlagAny(data::Storage::STORAGE_RESET))
        {
            return;
        }
        else
        {
            m_region3DPreviewManager->regionDataChanged();
        }
    }
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
		pAction->setObjectName("actionRecent");
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
            QFileInfo fi(path);
            QString suffix = fi.isDir()?"":fi.suffix().toLower();
            QString modelFormats = "stl stla stlb obj ply 3ds dxf lwo";

            if (!(!suffix.isEmpty() && modelFormats.contains(suffix, Qt::CaseInsensitive)))
                if (!preOpen())
                return;
            
            if (path.endsWith("vlm",Qt::CaseInsensitive))
                openVLM(path);
            else
                if (!suffix.isEmpty() && modelFormats.contains(suffix,Qt::CaseInsensitive))
                    openSTL(path, fi.fileName());
                else
                    if (path.endsWith(".zip",Qt::CaseInsensitive))
                        openDICOMZIP(path);
                    else
                        openDICOM(path, path, true);
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
		if (dw->objectName() == "Models Panel" && m_segmentationPluginsLoaded)
		{
			continue;
		}

        if (dw->objectName() == "Quick Tissue Model Creation Panel" && m_segmentationPluginsLoaded)
        {
            continue;
        }

        if (dw->toggleViewAction()->isEnabled())
            pPanelsMenu->addAction(dw->toggleViewAction());            
    }
    pPanelsMenu->addSeparator();
    pPanelsMenu->addAction(ui->actionTabify_Panels);

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
        qDebug() << dw->objectName();

        if (dw->objectName() == "Models Panel" && m_segmentationPluginsLoaded)
        {
            continue;
        }

        if (dw->objectName() == "Quick Tissue Model Creation Panel" && m_segmentationPluginsLoaded)
        {
            continue;
        }

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
        for (int s = 0; s < urlList.size(); ++s)
        {
            QString path = urlList.at(s).toLocalFile();            
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
                    bOpenedSomething |= openDICOM(path, path, true);
                }
                else
                {
                    QFileInfo fi(path);
                    QString suffix = fi.isDir()?"":fi.suffix().toLower();
                    if (!suffix.isEmpty() && modelFormats.contains(suffix,Qt::CaseInsensitive))
                    {
                        bOpenedSomething |= openSTL(path, fi.fileName());
                    }
                    else
                    {
                        if (path.endsWith(".zip",Qt::CaseInsensitive))
                            bOpenedSomething |= openDICOMZIP(path);
                        else
                            bOpenedSomething |= openDICOM(path, path, true);
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
	int model_id((!m_segmentationPluginsLoaded) ? m_modelsPanel->getSelectedModelStorageId() : VPL_SIGNAL(SigGetSelectedModelId).invoke2());

    if(model_id < 0)
    {
        showMessageBox(QMessageBox::Critical, tr("No model selected!"));
        return;
    }

    {   // Check the model
        data::CObjectPtr<data::CModel> spModel( APP_STORAGE.getEntry(model_id) );        
        const geometry::CMesh *pMesh = spModel->getMesh();
        if (!pMesh || !(pMesh->n_vertices() > 0) )
        {
            showMessageBox(QMessageBox::Critical, tr("No data!"));
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
        const geometry::CMesh* pMesh = spModel->getMesh();

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

	/*QSettings settings;
	if (settings.value("ModelRegionLinkEnabled", QVariant(DEFAULT_MODEL_REGION_LINK)).toBool())
	{
		// Get active region
		data::CObjectPtr<data::CRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id));
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
	}*/

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
			QApplication::restoreOverrideCursor();
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
							connect(a, SIGNAL(triggered()), this, SLOT(resetAppMode()));
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

void MainWindow::onMenuContoursVisibleToggled(bool visible)
{
	VPL_SIGNAL(SigSetContoursVisibility).invoke(visible);
}

void MainWindow::onMenuRegionsVisibleToggled(bool visible)
{
	VPL_SIGNAL(SigEnableRegionColoring).invoke(visible);
    VPL_SIGNAL(SigEnableMultiClassRegionColoring).invoke(visible);
}

void MainWindow::onMenuVOIVisibleToggled(bool visible)
{
	VPL_SIGNAL(SigSetVolumeOfInterestVisibility).invoke(visible);
}

void MainWindow::setRegionsVisibility(bool visible)
{
	ui->actionRegions_Visible->blockSignals(true);
	ui->actionRegions_Visible->setChecked(visible);
	ui->actionRegions_Visible->blockSignals(false);
}

void MainWindow::setContoursVisibility(bool visible)
{
	QSettings settings;
	settings.setValue("ContoursVisibility", visible);

	ui->actionRegions_Contours_Visible->blockSignals(true);
	ui->actionRegions_Contours_Visible->setChecked(visible);
	ui->actionRegions_Contours_Visible->blockSignals(false);

	if (visible && !m_contoursVisible)
	{
		m_SceneXY->addChild(m_xyVizualizer);
		m_SceneXZ->addChild(m_xzVizualizer);
		m_SceneYZ->addChild(m_yzVizualizer);

		m_contoursVisible = true;
	}
	else if (!visible && m_contoursVisible)
	{
		m_SceneXY->removeChild(m_xyVizualizer);
		m_SceneXZ->removeChild(m_xzVizualizer);
		m_SceneYZ->removeChild(m_yzVizualizer);

		m_contoursVisible = false;
	}

	data::CObjectPtr<data::CMultiClassRegionColoring> spColoring(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id));

	APP_STORAGE.invalidate(spColoring.getEntryPtr());
}

bool MainWindow::getContoursVisibility()
{
	return m_contoursVisible;
}

bool MainWindow::getVOIVisibility()
{
	return m_VOIVisible;
}

void MainWindow::showPreviewDialog(const QString& title, CPreviewDialogData& dialogData, int dataType)
{
	CPreviewDialog dlg(this, title, dialogData);

	bool accepted = false;

	if (QDialog::Accepted == dlg.exec())
	{
		accepted = true;
	}
	else
	{
		accepted = false;
	}

	VPL_SIGNAL(SigPreviewDialogClosed).invoke(accepted, dialogData, dataType);
}

void MainWindow::showInfoDialog(const QString& title, const QString& rowsTitles, const QString& rowsValues, const QColor& color)
{
	CInfoDialog dlg(this, title);

	QStringList titles = rowsTitles.split(';');
	QStringList values = rowsValues.split(';');

	Q_ASSERT(!titles.empty() && titles.size() == values.size());

	dlg.addRow(titles[0], values[0], color);

	for (int i = 1; i < titles.size(); ++i)
	{
		dlg.addRow(titles[i], values[i]);
	}

	dlg.exec();
}

void MainWindow::setVOIVisibility(bool visible)
{
	ui->actionVolume_of_Interest_Visible->blockSignals(true);
	ui->actionVolume_of_Interest_Visible->setChecked(visible);
	ui->actionVolume_of_Interest_Visible->blockSignals(false);

	data::CObjectPtr<data::CVolumeOfInterestData> spVOI(APP_STORAGE.getEntry(data::Storage::VolumeOfInterestData::Id));

	if (visible && !m_VOIVisible)
	{
		m_SceneXY->addChild(m_xyVOIVizualizer);
		m_SceneXZ->addChild(m_xzVOIVizualizer);
		m_SceneYZ->addChild(m_yzVOIVizualizer);

		m_VOIVisible = true;
		spVOI->setSetFlag(true);
	}
	else if (!visible && m_VOIVisible)
	{
        if (m_SceneXY->getChildIndex(m_xyVOIVizualizer) != m_SceneXY->getNumChildren())
        {
            m_SceneXY->removeChild(m_xyVOIVizualizer);
        }

        if (m_SceneXZ->getChildIndex(m_xzVOIVizualizer) != m_SceneXZ->getNumChildren())
        {
            m_SceneXZ->removeChild(m_xzVOIVizualizer);
        }

        if (m_SceneYZ->getChildIndex(m_yzVOIVizualizer) != m_SceneYZ->getNumChildren())
        {
            m_SceneYZ->removeChild(m_yzVOIVizualizer);
        }

		m_VOIVisible = false;
		spVOI->setSetFlag(false);
	}

	APP_STORAGE.invalidate(spVOI.getEntryPtr());

	data::CObjectPtr< data::COrthoSliceXY > pXY(APP_STORAGE.getEntry(data::Storage::SliceXY::Id));
	data::CObjectPtr< data::COrthoSliceXZ > pXZ(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id));
	data::CObjectPtr< data::COrthoSliceYZ > pYZ(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id));

	APP_STORAGE.invalidate(pXY.getEntryPtr());
	APP_STORAGE.invalidate(pXZ.getEntryPtr());
	APP_STORAGE.invalidate(pYZ.getEntryPtr());
}

void MainWindow::onNewDensityData(data::CStorageEntry *pEntry)
{
	m_volumeRenderingPanel->setVRMode();

	data::CObjectPtr<data::CDensityData> spData(APP_STORAGE.getEntry(data::Storage::PatientData::Id));
	QString dataType = QString::fromStdString(spData->m_sModality);

	{
		QList<QAction *> actions = this->findChildren<QAction *>();

		foreach(QAction *a, actions)
		{
			a->setToolTip((dataType == "MR") ? a->toolTip().replace("densit", "intensit") : a->toolTip().replace("intensit", "densit"));
			a->setStatusTip((dataType == "MR") ? a->toolTip().replace("densit", "intensit") : a->toolTip().replace("intensit", "densit"));
			a->setToolTip((dataType == "MR") ? a->toolTip().replace("Densit", "Intensit") : a->toolTip().replace("Intensit", "Densit"));
			a->setStatusTip((dataType == "MR") ? a->toolTip().replace("Densit", "Intensit") : a->toolTip().replace("Intensit", "Densit"));
			a->setText((dataType == "MR") ? a->text().replace("densit", "intensit") : a->text().replace("intensit", "densit"));
			a->setText((dataType == "MR") ? a->text().replace("Densit", "Intensit") : a->text().replace("Intensit", "Densit"));
			a->setToolTip((dataType == "MR") ? a->toolTip().replace("denzit", "intenzit") : a->toolTip().replace("intenzit", "denzit"));
			a->setStatusTip((dataType == "MR") ? a->toolTip().replace("denzit", "intenzit") : a->toolTip().replace("intenzit", "denzit"));
			a->setToolTip((dataType == "MR") ? a->toolTip().replace("Denzit", "Intenzit") : a->toolTip().replace("Intenzit", "Denzit"));
			a->setStatusTip((dataType == "MR") ? a->toolTip().replace("Denzit", "Intenzit") : a->toolTip().replace("Intenzit", "Denzit"));
			a->setText((dataType == "MR") ? a->text().replace("denzit", "intenzit") : a->text().replace("intenzit", "denzit"));
			a->setText((dataType == "MR") ? a->text().replace("Denzit", "Intenzit") : a->text().replace("Intenzit", "Denzit"));
		}
	}

	{
		QList<QSpinBox *> spinBoxes = this->findChildren<QSpinBox *>();

		foreach(QSpinBox *s, spinBoxes)
		{
			s->setToolTip((dataType == "MR") ? s->toolTip().replace("densit", "intensit") : s->toolTip().replace("intensit", "densit"));
			s->setStatusTip((dataType == "MR") ? s->toolTip().replace("densit", "intensit") : s->toolTip().replace("intensit", "densit"));
			s->setToolTip((dataType == "MR") ? s->toolTip().replace("Densit", "Intensit") : s->toolTip().replace("Intensit", "Densit"));
			s->setStatusTip((dataType == "MR") ? s->toolTip().replace("Densit", "Intensit") : s->toolTip().replace("Intensit", "Densit"));
			s->setToolTip((dataType == "MR") ? s->toolTip().replace("denzit", "intenzit") : s->toolTip().replace("intenzit", "denzit"));
			s->setStatusTip((dataType == "MR") ? s->toolTip().replace("denzit", "intenzit") : s->toolTip().replace("intenzit", "denzit"));
			s->setToolTip((dataType == "MR") ? s->toolTip().replace("Denzit", "Intenzit") : s->toolTip().replace("Intenzit", "Denzit"));
			s->setStatusTip((dataType == "MR") ? s->toolTip().replace("Denzit", "Intenzit") : s->toolTip().replace("Intenzit", "Denzit"));
		}
	}

	{
		QList<QPushButton *> pushButtons = this->findChildren<QPushButton *>();

		foreach(QPushButton *b, pushButtons)
		{
			b->setToolTip((dataType == "MR") ? b->toolTip().replace("densit", "intensit") : b->toolTip().replace("intensit", "densit"));
			b->setStatusTip((dataType == "MR") ? b->toolTip().replace("densit", "intensit") : b->toolTip().replace("intensit", "densit"));
			b->setToolTip((dataType == "MR") ? b->toolTip().replace("Densit", "Intensit") : b->toolTip().replace("Intensit", "Densit"));
			b->setStatusTip((dataType == "MR") ? b->toolTip().replace("Densit", "Intensit") : b->toolTip().replace("Intensit", "Densit"));
			b->setText((dataType == "MR") ? b->text().replace("densit", "intensit") : b->text().replace("intensit", "densit"));
			b->setText((dataType == "MR") ? b->text().replace("Densit", "Intensit") : b->text().replace("Intensit", "Densit"));
			b->setToolTip((dataType == "MR") ? b->toolTip().replace("denzit", "intenzit") : b->toolTip().replace("intenzit", "denzit"));
			b->setStatusTip((dataType == "MR") ? b->toolTip().replace("denzit", "intenzit") : b->toolTip().replace("intenzit", "denzit"));
			b->setToolTip((dataType == "MR") ? b->toolTip().replace("Denzit", "Intenzit") : b->toolTip().replace("Intenzit", "Denzit"));
			b->setStatusTip((dataType == "MR") ? b->toolTip().replace("Denzit", "Intenzit") : b->toolTip().replace("Intenzit", "Denzit"));
			b->setText((dataType == "MR") ? b->text().replace("denzit", "intenzit") : b->text().replace("intenzit", "denzit"));
			b->setText((dataType == "MR") ? b->text().replace("Denzit", "Intenzit") : b->text().replace("Intenzit", "Denzit"));
		}
	}

	{
		QList<QCheckBox *> checkBoxes = this->findChildren<QCheckBox *>();

		foreach(QCheckBox *c, checkBoxes)
		{
			c->setToolTip((dataType == "MR") ? c->toolTip().replace("densit", "intensit") : c->toolTip().replace("intensit", "densit"));
			c->setStatusTip((dataType == "MR") ? c->toolTip().replace("densit", "intensit") : c->toolTip().replace("intensit", "densit"));
			c->setToolTip((dataType == "MR") ? c->toolTip().replace("Densit", "Intensit") : c->toolTip().replace("Intensit", "Densit"));
			c->setStatusTip((dataType == "MR") ? c->toolTip().replace("Densit", "Intensit") : c->toolTip().replace("Intensit", "Densit"));
			c->setText((dataType == "MR") ? c->text().replace("densit", "intensit") : c->text().replace("intensit", "densit"));
			c->setText((dataType == "MR") ? c->text().replace("Densit", "Intensit") : c->text().replace("Intensit", "Densit"));
			c->setToolTip((dataType == "MR") ? c->toolTip().replace("denzit", "intenzit") : c->toolTip().replace("intenzit", "denzit"));
			c->setStatusTip((dataType == "MR") ? c->toolTip().replace("denzit", "intenzit") : c->toolTip().replace("intenzit", "denzit"));
			c->setToolTip((dataType == "MR") ? c->toolTip().replace("Denzit", "Intenzit") : c->toolTip().replace("Intenzit", "Denzit"));
			c->setStatusTip((dataType == "MR") ? c->toolTip().replace("Denzit", "Intenzit") : c->toolTip().replace("Intenzit", "Denzit"));
			c->setText((dataType == "MR") ? c->text().replace("denzit", "intenzit") : c->text().replace("intenzit", "denzit"));
			c->setText((dataType == "MR") ? c->text().replace("Denzit", "Intenzit") : c->text().replace("Intenzit", "Denzit"));
		}
	}

	{
		QList<QLabel *> labels = this->findChildren<QLabel *>();

		foreach(QLabel *l, labels)
		{
			l->setToolTip((dataType == "MR") ? l->toolTip().replace("densit", "intensit") : l->toolTip().replace("intensit", "densit"));
			l->setStatusTip((dataType == "MR") ? l->toolTip().replace("densit", "intensit") : l->toolTip().replace("intensit", "densit"));
			l->setToolTip((dataType == "MR") ? l->toolTip().replace("Densit", "Intensit") : l->toolTip().replace("Intensit", "Densit"));
			l->setStatusTip((dataType == "MR") ? l->toolTip().replace("Densit", "Intensit") : l->toolTip().replace("Intensit", "Densit"));
			l->setText((dataType == "MR") ? l->text().replace("densit", "intensit") : l->text().replace("intensit", "densit"));
			l->setText((dataType == "MR") ? l->text().replace("Densit", "Intensit") : l->text().replace("Intensit", "Densit"));
			l->setToolTip((dataType == "MR") ? l->toolTip().replace("denzit", "intenzit") : l->toolTip().replace("intenzit", "denzit"));
			l->setStatusTip((dataType == "MR") ? l->toolTip().replace("denzit", "intenzit") : l->toolTip().replace("intenzit", "denzit"));
			l->setToolTip((dataType == "MR") ? l->toolTip().replace("Denzit", "Intenzit") : l->toolTip().replace("Intenzit", "Denzit"));
			l->setStatusTip((dataType == "MR") ? l->toolTip().replace("Denzit", "Intenzit") : l->toolTip().replace("Intenzit", "Denzit"));
			l->setText((dataType == "MR") ? l->text().replace("denzit", "intenzit") : l->text().replace("intenzit", "denzit"));
			l->setText((dataType == "MR") ? l->text().replace("Denzit", "Intenzit") : l->text().replace("Intenzit", "Denzit"));
		}
	}

	{
		QList<QGroupBox *> qroupBoxes = this->findChildren<QGroupBox *>();

		foreach(QGroupBox *g, qroupBoxes)
		{
			g->setTitle((dataType == "MR") ? g->title().replace("densit", "intensit") : g->title().replace("intensit", "densit"));
			g->setTitle((dataType == "MR") ? g->title().replace("Densit", "Intensit") : g->title().replace("Intensit", "Densit"));
			g->setTitle((dataType == "MR") ? g->title().replace("denzit", "intenzit") : g->title().replace("intenzit", "denzit"));
			g->setTitle((dataType == "MR") ? g->title().replace("Denzit", "Intenzit") : g->title().replace("Intenzit", "Denzit"));
		}
	}

	{
		QList<QLineEdit *> edits = this->findChildren<QLineEdit *>();

		foreach(QLineEdit *e, edits)
		{
			e->setToolTip((dataType == "MR") ? e->toolTip().replace("densit", "intensit") : e->toolTip().replace("intensit", "densit"));
			e->setStatusTip((dataType == "MR") ? e->toolTip().replace("densit", "intensit") : e->toolTip().replace("intensit", "densit"));
			e->setToolTip((dataType == "MR") ? e->toolTip().replace("Densit", "Intensit") : e->toolTip().replace("Intensit", "Densit"));
			e->setStatusTip((dataType == "MR") ? e->toolTip().replace("Densit", "Intensit") : e->toolTip().replace("Intensit", "Densit"));
			e->setToolTip((dataType == "MR") ? e->toolTip().replace("denzit", "intenzit") : e->toolTip().replace("intenzit", "denzit"));
			e->setStatusTip((dataType == "MR") ? e->toolTip().replace("denzit", "intenzit") : e->toolTip().replace("intenzit", "denzit"));
			e->setToolTip((dataType == "MR") ? e->toolTip().replace("Denzit", "Intenzit") : e->toolTip().replace("Intenzit", "Denzit"));
			e->setStatusTip((dataType == "MR") ? e->toolTip().replace("Denzit", "Intenzit") : e->toolTip().replace("Intenzit", "Denzit"));
		}
	}

	{
		QList<QComboBox *> combos = this->findChildren<QComboBox *>();

		foreach(QComboBox *c, combos)
		{
			c->setToolTip((dataType == "MR") ? c->toolTip().replace("densit", "intensit") : c->toolTip().replace("intensit", "densit"));
			c->setStatusTip((dataType == "MR") ? c->toolTip().replace("densit", "intensit") : c->toolTip().replace("intensit", "densit"));
			c->setToolTip((dataType == "MR") ? c->toolTip().replace("Densit", "Intensit") : c->toolTip().replace("Intensit", "Densit"));
			c->setStatusTip((dataType == "MR") ? c->toolTip().replace("Densit", "Intensit") : c->toolTip().replace("Intensit", "Densit"));
			c->setToolTip((dataType == "MR") ? c->toolTip().replace("denzit", "intenzit") : c->toolTip().replace("intenzit", "denzit"));
			c->setStatusTip((dataType == "MR") ? c->toolTip().replace("denzit", "intenzit") : c->toolTip().replace("intenzit", "denzit"));
			c->setToolTip((dataType == "MR") ? c->toolTip().replace("Denzit", "Intenzit") : c->toolTip().replace("Intenzit", "Denzit"));
			c->setStatusTip((dataType == "MR") ? c->toolTip().replace("Denzit", "Intenzit") : c->toolTip().replace("Intenzit", "Denzit"));
		}
	}

	{
		QList<QSlider *> combos = this->findChildren<QSlider *>();

		foreach(QSlider *s, combos)
		{
			s->setToolTip((dataType == "MR") ? s->toolTip().replace("densit", "intensit") : s->toolTip().replace("intensit", "densit"));
			s->setStatusTip((dataType == "MR") ? s->toolTip().replace("densit", "intensit") : s->toolTip().replace("intensit", "densit"));
			s->setToolTip((dataType == "MR") ? s->toolTip().replace("Densit", "Intensit") : s->toolTip().replace("Intensit", "Densit"));
			s->setStatusTip((dataType == "MR") ? s->toolTip().replace("Densit", "Intensit") : s->toolTip().replace("Intensit", "Densit"));
			s->setToolTip((dataType == "MR") ? s->toolTip().replace("denzit", "intenzit") : s->toolTip().replace("intenzit", "denzit"));
			s->setStatusTip((dataType == "MR") ? s->toolTip().replace("denzit", "intenzit") : s->toolTip().replace("intenzit", "denzit"));
			s->setToolTip((dataType == "MR") ? s->toolTip().replace("Denzit", "Intenzit") : s->toolTip().replace("Intenzit", "Denzit"));
			s->setStatusTip((dataType == "MR") ? s->toolTip().replace("Denzit", "Intenzit") : s->toolTip().replace("Intenzit", "Denzit"));
		}
	}
}

void MainWindow::showFeedbackRequestDialog()
{
	//app::CProductInfo info = app::getProductInfo();
	//QString product = QString("%1 %2.%3.%4 %5").arg(QCoreApplication::applicationName()).arg(info.getVersion().getMajorNum()).arg(info.getVersion().getMinorNum()).arg(info.getVersion().getBuildNum()).arg(QString::fromStdString(info.getNote()));
	QMessageBox msgBox(this);
	msgBox.setEscapeButton(QMessageBox::Cancel);
	QPushButton* feedbackButton = msgBox.addButton(tr("Give Us Your Feedback..."), QMessageBox::YesRole);
	QObject::connect(feedbackButton, SIGNAL(clicked()), this, SLOT(showFeedback()));
	msgBox.setDefaultButton(feedbackButton);
	msgBox.setIconPixmap(QPixmap(":/svg/svg/3dim.svg"));
	msgBox.setWindowTitle(QCoreApplication::applicationName());
	msgBox.setText(tr("We are surveying users about their experience and satisfaction with 3DimViewer. We appreciate your response. Thank you."));

	msgBox.exec();
}

void MainWindow::resetAppMode() // actions activated via shortcuts with Ctrl/Shift leave mouse mode in a temporary mode, we need to reset it
{
    QAction* pAction = qobject_cast<QAction*>(sender());
    if (NULL!=pAction)
    {
        QKeySequence ks = pAction->shortcut();
        QString kss = ks.toString();
        // restore mouse mode only when a modifier could be used - todo: detect whether a shortcut was really used, this is just a workaround
        if (!ks.isEmpty() && (kss.contains("Meta") || kss.contains("Alt") || kss.contains("Ctrl") || kss.contains("Shift"))) 
	        APP_MODE.restore();
    }
}

// after some actions (like saving dicom series), menu actions remained grey, until the mouse cursor went over them
// this function handles these situations
void MainWindow::enableMenuActions()
{
	QList<QMenu *> menus = this->findChildren<QMenu *>();

	foreach(QMenu *m, menus)
	{
		m->setEnabled(false);
		m->setEnabled(true);
	}
}

//! Autotab all panels in all widget areas
void MainWindow::autoTabPanels()
{
    std::vector<QDockWidget*> dwa[4];
    QList<QDockWidget*> dws = findChildren<QDockWidget*>();
    foreach(QDockWidget* dw,dws)
    {
        Qt::DockWidgetArea area = dockWidgetArea(dw);
        switch(area)
        {
        case Qt::LeftDockWidgetArea: dwa[0].push_back(dw); break;
        case Qt::RightDockWidgetArea: dwa[1].push_back(dw); break;
        case Qt::TopDockWidgetArea: dwa[2].push_back(dw); break;
        case Qt::BottomDockWidgetArea: dwa[3].push_back(dw); break;
        }
    }
    for(int i = 0;i < 4; i++)
    {
        if (dwa[i].empty()) continue;
        QList<QDockWidget*> tabbed = tabifiedDockWidgets(dwa[i][0]);
        //std::remove_if doesn't erase anything from the vector, since it doesn't have access to it. 
        // Instead, it moves the elements you want to keep to the start of the range, leaving the 
        // remaining elements in a valid but unspecified state, and returns the new end.
        // You can use the "erase-remove" idiom to actually erase them from the vector:
        dwa[i].erase(std::remove_if(dwa[i].begin(),dwa[i].end(),[&](QDockWidget* dw) -> bool { 
            return tabbed.contains(dw);
        }),dwa[i].end());
        // tabify only untabbed widgets to preserve order of the already tabbed ones
        for(int j = 1; j<dwa[i].size();j++)
        {
            tabifyDockWidget(dwa[i][0],dwa[i][j]);
        }
    }
}


void MainWindow::moveRegionDataToMulticlass()
{
    data::CObjectPtr<data::CRegionData> spRegionData(APP_STORAGE.getEntry(data::Storage::RegionData::Id));
    data::CObjectPtr<data::CRegionColoring> spRegionColoring(APP_STORAGE.getEntry(data::Storage::RegionColoring::Id));

    if (spRegionData->hasData())
    {
        data::CObjectPtr<data::CMultiClassRegionData> spMultiClassRegionData(APP_STORAGE.getEntry(data::Storage::MultiClassRegionData::Id));
        data::CObjectPtr<data::CMultiClassRegionColoring> spMultiClassRegionColoring(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id));

        vpl::img::CVolSize vSize = spRegionData->getSize();

        const vpl::tSize sx = vSize.x();
        const vpl::tSize sy = vSize.y();
        const vpl::tSize sz = vSize.z();

        std::set<int> labels;

        for (vpl::tSize k = 0; k < sz; ++k)
        {
            for (vpl::tSize j = 0; j < sy; ++j)
            {
                for (vpl::tSize i = 0; i < sx; ++i)
                {
                    data::CRegionData::tVoxel val = spRegionData->at(i, j, k);

                    if (val > 0)
                    {
                        labels.insert(val);
                    }
                }
            }
        }

        spMultiClassRegionData->fillEntire(0);
        spMultiClassRegionColoring->init();
        spMultiClassRegionColoring->resize(labels.size());

        auto it = labels.begin();
        for (int i = 0; i < spMultiClassRegionColoring->getNumOfRegions(); ++i, ++it)
        {
            data::CRegionInfo oldInfo = spRegionColoring->getRegionInfo(*it);
            data::CMultiClassRegionInfo multiClassInfo(oldInfo.getName());

            spMultiClassRegionColoring->setRegionInfo(i, multiClassInfo);
            spMultiClassRegionColoring->setColor(i, spRegionColoring->getColor(*it));
        }

        spMultiClassRegionData->fillFromVolume(*spRegionData.get());

        spRegionData->init();
        spRegionColoring->init();

        VPL_SIGNAL(SigEnableMultiClassRegionColoring).invoke(true);

        APP_STORAGE.invalidate(spRegionData.getEntryPtr());
        APP_STORAGE.invalidate(spRegionColoring.getEntryPtr());
        APP_STORAGE.invalidate(spMultiClassRegionData.getEntryPtr());
        APP_STORAGE.invalidate(spMultiClassRegionColoring.getEntryPtr());
    }
}

void MainWindow::removeAllModels()
{
    for (int i = 0; i < MAX_IMPORTED_MODELS; ++i)
    {
        data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(data::Storage::ImportedModel::Id + i));
        if (spModel->hasData())
        {
            spModel->init();
            APP_STORAGE.invalidate(spModel.getEntryPtr());
        }
    }
}

void MainWindow::setActiveRegion3DPreviewVisibility(bool visible)
{
    if (visible)
    {
        data::CObjectPtr< data::CActiveDataSet > ptrDataset(APP_STORAGE.getEntry(data::Storage::ActiveDataSet::Id));
        data::CObjectPtr< data::CDensityData > pVolume(APP_STORAGE.getEntry(ptrDataset->getId()));
        vpl::img::CSize3d voxelSize(pVolume->getDX(), pVolume->getDY(), pVolume->getDZ());

        data::CObjectPtr<data::CMultiClassRegionColoring> spMultiClassRegionColoring(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id));
        data::CMultiClassRegionData::tVoxel activeRegion = spMultiClassRegionColoring->getActiveRegion();

        m_region3DPreviewManager->init(voxelSize, activeRegion);
        m_region3DPreviewManager->run();
    }
    else
    {
        m_region3DPreviewManager->stop();
    }

    m_region3DPreviewVisualizer->setVisibility(visible);
    m_3DView->Refresh(false);

    m_region3DPreviewVisible = visible;
}

bool MainWindow::getActiveRegion3DPreviewVisibility()
{
    return m_region3DPreviewVisible;
}

void MainWindow::setActiveRegion3DPreviewInterval(int value)
{
    m_region3DPreviewManager->setRedrawInterval(value);
}

void MainWindow::setUpRegion3DPreview()
{
    VPL_SIGNAL(SigSetActiveRegion3DPreviewVisibility).connect(this, &MainWindow::setActiveRegion3DPreviewVisibility);
    VPL_SIGNAL(SigSetRegion3DPreviewInterval).connect(this, &MainWindow::setActiveRegion3DPreviewInterval);
    VPL_SIGNAL(SigRegion3DPreviewVisible).connect(this, &MainWindow::getActiveRegion3DPreviewVisibility);

    m_region3DPreviewManager = new data::CRegion3DPreviewManager;

    m_on3DPreviewStart = new vpl::base::CFunctor<void>(this, &MainWindow::on3DPreviewStart);
    m_on3DPreviewUpdate = new vpl::base::CFunctor<void>(this, &MainWindow::on3DPreviewUpdate);
    m_on3DPreviewStop = new vpl::base::CFunctor<void, geometry::CTrianglesContainer&>(this, &MainWindow::on3DPreviewStop);

    m_region3DPreviewManager->onStart = m_on3DPreviewStart;
    m_region3DPreviewManager->onUpdate = m_on3DPreviewUpdate;
    m_region3DPreviewManager->onStop = m_on3DPreviewStop;
}

void MainWindow::destroyRegion3DPreview()
{
    delete m_region3DPreviewManager;
    delete m_on3DPreviewStart;
    delete m_on3DPreviewUpdate;
    delete m_on3DPreviewStop;
}

void MainWindow::on3DPreviewStart()
{

}

void MainWindow::on3DPreviewUpdate()
{

}

void MainWindow::on3DPreviewStop(geometry::CTrianglesContainer& container)
{
    m_region3DPreviewVisualizer->setData(container.getVertices(), container.getIndicies());

    QMetaObject::invokeMethod(this, "update3DPreview");
}

void MainWindow::update3DPreview()
{
    data::CObjectPtr<data::CMultiClassRegionColoring> spMultiClassRegionColoring(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id));
    data::CMultiClassRegionData::tVoxel activeRegion = spMultiClassRegionColoring->getActiveRegion();
    data::CMultiClassRegionColoring::tColor rc = spMultiClassRegionColoring->getColor(activeRegion);

    m_region3DPreviewVisualizer->update(osg::Vec4(rc.getR() / 255.0f, rc.getG() / 255.0f, rc.getB() / 255.0f, rc.getA() / 255.0f));
    m_3DView->Refresh(false);
}

void MainWindow::onActiveRegionChanged()
{
    data::CObjectPtr<data::CMultiClassRegionColoring> spMultiClassRegionColoring(APP_STORAGE.getEntry(data::Storage::MultiClassRegionColoring::Id));
    data::CMultiClassRegionData::tVoxel activeRegion = spMultiClassRegionColoring->getActiveRegion();

    m_region3DPreviewManager->setRegionIndex(activeRegion);
}

void MainWindow::changeVRVisibility(bool visible)
{
    ui->actionVolume_Rendering->blockSignals(true);
    ui->actionVolume_Rendering->setChecked(visible);
    ui->actionVolume_Rendering->blockSignals(false);
    showMergedVR(visible);
}

void MainWindow::changeModelsVisibility(bool visible)
{
    ui->actionShow_Surface_Model->blockSignals(true);
    ui->actionShow_Surface_Model->setChecked(visible);
    ui->actionShow_Surface_Model->blockSignals(false);
    //showSurfaceModel(visible);

    if (!visible)
    {
        m_visibleModelsBefore3Dseg.clear();
        for (size_t i = 0; i < MAX_IMPORTED_MODELS; ++i)
        {
            data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(data::Storage::ImportedModel::Id + i, data::Storage::NO_UPDATE));
            if (spModel->hasData() && spModel->isVisible())
            {
                m_visibleModelsBefore3Dseg.push_back(i);
                spModel->hide();
                APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::VISIBILITY_CHANGED);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < m_visibleModelsBefore3Dseg.size(); ++i)
        {
            data::CObjectPtr<data::CModel> spModel(APP_STORAGE.getEntry(data::Storage::ImportedModel::Id + m_visibleModelsBefore3Dseg.at(i), data::Storage::NO_UPDATE));
            if (spModel->hasData() && !spModel->isVisible())
            {
                spModel->show();
                APP_STORAGE.invalidate(spModel.getEntryPtr(), data::CModel::VISIBILITY_CHANGED);
            }
        }
    }

    m_3DView->Refresh(false);
}

bool MainWindow::isVRVisible()
{
    return ui->actionVolume_Rendering->isChecked();
}

void MainWindow::onSigVRChanged()
{
    if (m_3DView->getRenderer().getShader() == PSVR::PSVolumeRendering::EShaders::CUSTOM)
    {
        m_3DView->addEventHandler(m_drawW3DEH.get());
    }
    else
    {
        m_3DView->removeEventHandler(m_drawW3DEH.get());
    }
}

bool MainWindow::areModelsVisible()
{
    return ui->actionShow_Surface_Model->isChecked();
}

void MainWindow::onDrawingInProgress(bool drawingInProgress)
{
    m_region3DPreviewManager->setCanUpdate(!drawingInProgress);

    if (!drawingInProgress)
    {
        data::CObjectPtr< data::COrthoSliceXY > ptrSliceXY(APP_STORAGE.getEntry(data::Storage::SliceXY::Id));
        data::CObjectPtr< data::COrthoSliceXZ > ptrSliceXZ(APP_STORAGE.getEntry(data::Storage::SliceXZ::Id));
        data::CObjectPtr< data::COrthoSliceYZ > ptrSliceYZ(APP_STORAGE.getEntry(data::Storage::SliceYZ::Id));
        data::CObjectPtr< data::CArbitrarySlice > ptrSliceARB(APP_STORAGE.getEntry(data::Storage::ArbitrarySlice::Id));
        ptrSliceXY->setUpdatesEnabled(true);
        ptrSliceXZ->setUpdatesEnabled(true);
        ptrSliceYZ->setUpdatesEnabled(true);
        ptrSliceARB->setUpdatesEnabled(true);
        APP_STORAGE.invalidate(ptrSliceXY.getEntryPtr());
        APP_STORAGE.invalidate(ptrSliceXZ.getEntryPtr());
        APP_STORAGE.invalidate(ptrSliceYZ.getEntryPtr());
        APP_STORAGE.invalidate(ptrSliceARB.getEntryPtr());
    }
}

void MainWindow::sig2DCanvasLeave(OSGCanvas* pCanvas, bool bLeave)
{
    data::CObjectPtr< data::CDrawingOptions > ptrOptions(APP_STORAGE.getEntry(data::Storage::DrawingOptions::Id));
    if (APP_MODE.isContinuousDrawingEnabled() && ptrOptions->getDrawingMode() == data::CDrawingOptions::DRAW_STROKE)
    {
        if (bLeave)
        {
            if (pCanvas == m_OrthoXYSlice || pCanvas == m_OrthoXZSlice || pCanvas == m_OrthoYZSlice)
            {
                VPL_SIGNAL(SigDrawingInProgress).invoke(false);
            }
        }
    }
}

bool MainWindow::setStrokeOnOff(bool on, int region)
{
    if (!findPluginByID("ManualSeg"))
    {
        return false;
    }

    VPL_SIGNAL(SigStrokeOnOff).invoke(on, region);

    return true;
}

QString MainWindow::QStringFromWString(const std::wstring & wsString)
{
#ifdef _MSC_VER
    QString qtString = QString::fromUtf16((const ushort *)wsString.c_str());
#else
    QString qtString = QString::fromStdWString(wsString);
#endif
    //qDebug() << qtString;
    return qtString;
}

QString MainWindow::getMacAddress()
{
    foreach(QNetworkInterface networkInterface, QNetworkInterface::allInterfaces())
    {
        // Return only the first non-loopback MAC Address
        if (!(networkInterface.flags() & QNetworkInterface::IsLoopBack))
        {
            QString wsAddr = networkInterface.hardwareAddress();
            if (wsAddr.isEmpty())
                continue;
            return wsAddr;
        }
    }
    return QString();
}

quint32 MainWindow::getIPAddress()
{
    QNetworkConfigurationManager mgr;
    QNetworkConfiguration nconfig = mgr.defaultConfiguration();
    QNetworkSession session(nconfig);
    QNetworkInterface ninter = session.interface();

    quint32 result = 0;
    QHostInfo hostInfo = QHostInfo::fromName(QHostInfo::localHostName());
    QList<QHostAddress> hostNameLookupAddressList = hostInfo.addresses();
    QList<QHostAddress> interfaceAddressList = QNetworkInterface::allAddresses();
    QString hostIpStr;

    foreach(QHostAddress addr, hostNameLookupAddressList)
    {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && interfaceAddressList.contains(addr))
        {
            qDebug() << addr.toString();
            result = addr.toIPv4Address();
            return result;
        }
    }

    // this provides two ip addresses (1 ipv4 and 1 ipv6) at least on my machine
    QList<QNetworkAddressEntry> laddr = ninter.addressEntries();
    foreach(QNetworkAddressEntry addr, laddr)
    {
        QHostAddress ha = addr.ip();
        if (QAbstractSocket::IPv4Protocol == ha.protocol())
        {
            qDebug() << ha << endl;
            result = ha.toIPv4Address();
            break;
        }
    }

    return result;
}

#ifdef _WIN32
std::wstring MainWindow::getHardDiskSerialNumber()
{
    // Result
    std::wstring wsSerialAddr;

    // Obtain the initial locator to WMI
    IWbemLocator *pLoc = NULL;

    HRESULT hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID *)&pLoc
    );
    if (FAILED(hres))
    {
        return wsSerialAddr;
    }

    // Connect to WMI through the IWbemLocator::ConnectServer method
    IWbemServices *pSvc = NULL;

    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"root\\cimv2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (e.g. Kerberos)
        0,                       // Context object 
        &pSvc                    // pointer to IWbemServices proxy
    );
    if (FAILED(hres))
    {
        pLoc->Release();
        return wsSerialAddr;
    }

    // Set security levels on the proxy
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name 
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities 
    );
    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        return wsSerialAddr;
    }

    // Use the IWbemServices pointer to make requests of WMI
    IEnumWbemClassObject* pEnumerator = NULL;

    {
        // Get serial number of the first physical disk
        hres = pSvc->ExecQuery(
            bstr_t("WQL"),
            bstr_t("SELECT MediaType, DeviceID FROM Win32_DiskDrive"),
            WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
            NULL,
            &pEnumerator
        );

        if (FAILED(hres))
        {
            pSvc->Release();
            pLoc->Release();
            return wsSerialAddr;
        }

        // Get the data from the query
        IWbemClassObject *pclsObj = 0;
        ULONG uReturn = 0;

        std::wstring wsDiskSerial;
        while (pEnumerator)
        {
            HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
            if (uReturn == 0)
            {
                break;
            }

            VARIANT vtProp;

            // Get the MediaType property
            hr = pclsObj->Get(L"MediaType", 0, &vtProp, 0, 0);
            if (FAILED(hr) || vtProp.vt == VT_NULL)
            {
                pclsObj->Release();
                continue;
            }
            std::wstring wsMedia = vtProp.bstrVal;
            VariantClear(&vtProp);

            // Check if the first disk was found
            if (wsMedia.find(L"Fixed") == std::wstring::npos
                || wsMedia.find(L"hard") == std::wstring::npos)
                //                || wsMedia.find(L"media") == std::wstring::npos )
            {
                pclsObj->Release();
                continue;
            }

            // Get the SerialNumber property
/*            hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
            if( FAILED(hr) || vtProp.vt == VT_NULL )
            {
                pclsObj->Release();
                continue;
            }
            std::wstring wsSerial = vtProp.bstrVal;
            VariantClear(&vtProp);*/

            // Get the DeviceID
            hr = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
            if (FAILED(hr) || vtProp.vt == VT_NULL)
            {
                pclsObj->Release();
                continue;
            }

            // Prepare query for the SerialNumber
            std::wstring wsDeviceIDQuery = L"ASSOCIATORS OF {Win32_DiskDrive.DeviceID='";
            wsDeviceIDQuery += vtProp.bstrVal;
            wsDeviceIDQuery += L"'} WHERE ResultClass=Win32_PhysicalMedia";
            VariantClear(&vtProp);

            // "join" Win32_DiskDrive to Win32_PhysicalMedia
            IEnumWbemClassObject* pEnumerator2 = NULL;
            hres = pSvc->ExecQuery(
                bstr_t("WQL"),
                bstr_t(wsDeviceIDQuery.c_str()),
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pEnumerator2
            );
            if (FAILED(hres))
            {
                pclsObj->Release();
                continue;
            }

            // Get the data from the query
            IWbemClassObject *pclsObj2 = 0;
            ULONG uReturn2 = 0;

            while (pEnumerator2)
            {
                HRESULT hr = pEnumerator2->Next(WBEM_INFINITE, 1, &pclsObj2, &uReturn2);
                if (uReturn2 == 0)
                {
                    break;
                }

                VARIANT vtProp2;
                hr = pclsObj2->Get(L"SerialNumber", 0, &vtProp2, 0, 0);
                if (FAILED(hr) || vtProp2.vt == VT_NULL)
                {
                    pclsObj2->Release();
                    continue;
                }

                std::wstring wsSerial = vtProp2.bstrVal;
                VariantClear(&vtProp2);
                pclsObj2->Release();

                wsDiskSerial = wsSerial;
                if (!wsSerialAddr.empty())
                {
                    wsSerialAddr += L", ";
                }
                wsSerialAddr += wsDiskSerial;
                break;
            }

            pclsObj->Release();

            // Return the found address
            if (!wsDiskSerial.empty())
            {
                break;
            }
        }

        pEnumerator->Release();
    }

    // Cleanup
    pSvc->Release();
    pLoc->Release();

    return wsSerialAddr;
}
#else

#include <IOKit/IOKitLib.h>

std::wstring CF2W(CFStringRef str)
{
    CFIndex length = CFStringGetLength(str);
    if (!length) return L"";
    std::vector<UniChar> buffer(length);
    CFRange range = {0,length};
    CFStringGetCharacters(str,range,&buffer[0]);
    return std::wstring(buffer.begin(),buffer.end());
}

std::wstring MainWindow::getHardDiskSerialNumber()
{
    // on MAC we get platform uuid instead
    std::wstring result;
    io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
    CFStringRef uuidCf = (CFStringRef) IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
    IOObjectRelease(ioRegistryRoot);
    result = CF2W(uuidCf);
    CFRelease(uuidCf);
    
    return result;
}

#endif

void MainWindow::pingOurServer()
{
    QSettings settings;
    // check only once per day
    QDate lastCheck = settings.value("LastPing").toDate();
    if (lastCheck < QDate::currentDate())
    {
        settings.setValue("LastPing", QDate::currentDate());
        m_networkManager.disconnect(this);
        { // ping
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
            QUrlQuery params;
#else
            QUrl params;
#endif
            QString software = CSysInfo::instance()->getApplicationName("");
            QString operatingSystemString = CSysInfo::instance()->m_sOperatingSystem;
            QString wsMacAddress = getMacAddress();
            quint32 ipAdr = getIPAddress();
            QString wsDiskID = QStringFromWString(getHardDiskSerialNumber());

            QSettings settings;
            QString requestUrl = settings.value("PingURL", DEFAULT_PING_URL).toString();
            settings.beginGroup("License");
            QString licNum = settings.value("RegNum").toString();

            params.addQueryItem("form_id", "flying_feather_sw");
            params.addQueryItem("software", software);
            params.addQueryItem("platform", operatingSystemString);
            params.addQueryItem("ip_address", QHostAddress(ipAdr).toString());
            params.addQueryItem("mac_address", wsMacAddress);
            params.addQueryItem("disk_id", wsDiskID);

            if (!licNum.isEmpty())
            {
                params.addQueryItem("lic_num", licNum);
            }

            QNetworkRequest request(requestUrl);
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

            m_networkManager.disconnect(this);
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
            QNetworkReply *pReply = m_networkManager.post(request, params.query(QUrl::FullyEncoded).toUtf8());
            qDebug() << requestUrl << " " << params.query(QUrl::FullyEncoded).toUtf8();
#else
            QNetworkReply *pReply = m_networkManager.post(request, params.encodedQuery());
#endif
            connect(&m_networkManager, SIGNAL(finished(QNetworkReply*)), SLOT(pingFinished(QNetworkReply*)));
            connect(&m_networkManager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> &)), this, SLOT(sslErrorHandler(QNetworkReply*, const QList<QSslError> &)));
        }
    }
}

void MainWindow::pingFinished(QNetworkReply* reply)
{
    QByteArray downloadedData;

    QNetworkReply::NetworkError err = reply->error();
    if (err == QNetworkReply::NoError)
    {
        int httpstatuscode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt();
        if (reply->isReadable())
        {
            downloadedData = reply->readAll();
        }
    }
    reply->deleteLater();
    if (!downloadedData.isEmpty())
    {
        qDebug() << downloadedData;
    }
}

void MainWindow::sslErrorHandler(QNetworkReply* reply, const QList<QSslError> & errlist)
{
    reply->ignoreSslErrors();
}
