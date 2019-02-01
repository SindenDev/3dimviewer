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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//! Includes
#include <configure.h>

#include <QMainWindow>
#include <QLabel>
#include <QtCore/QTimer>
#include <QProxyStyle>
#include <QDir>
#include <QApplication>
#include <QSplitter>
#include <QMessageBox>
#include <QToolButton>
#include <QSettings>
#include <QDockWidget>
#include <QMouseEvent>
#include <QTabBar>
#include <QSlider>
#include <QEvent>
#include <QNetworkReply>

//#include <data/CExamination.h>
#include <CAppExamination.h>
#include <data/CModelManager.h>

#include <osg/OSGCanvas.h>
#include <osg/OSGOrtho2DCanvas.h>
#include <osg/CSceneOSG.h>
#include <osg/CMeasurementsEH.h>
#ifdef ENABLE_DEEPLEARNING
    #include <graphmedi_viewer/osg/CLandmarkAnnotationsEH.h>
#endif
#include <drawing/CISEventHandlerEx.h>
#include <osg/CModelCutVisualizer.h>

#include <CRegion3DPreviewVisualizer.h>
#include <CRegion3DPreviewManager.h>
#include <geometry/base/CTrianglesContainer.h>

#ifdef USE_PSVR
   #include <render/CSceneVolumeRendering.h>
   #include <render/cvolumerendererwindow.h>
#endif // USE_PSVR

#include <densitywindowwidget.h>
#include <orthosliceswidget.h>
#include <segmentationwidget.h>
#include <volumerenderingwidget.h>
#include <modelswidget.h>
#include <cpreferencesdialog.h>
#include <CPluginManager.h>
#include <CCustomUI.h>
#include <Signals.h>
#include <data/CModelCut.h>
#include <CPreviewDialog.h>
#include <CPreviewDialogData.h>

#include <actlog/ceventfilter.h>

#include <data/CVolumeOfInterest.h>

#include <data/CRegionsVisualizer.h>
#include <data/CMultiClassRegionsVisualizer.h>
#include <data/CVolumeOfInterestVisualizer.h>

#include <osg/CArbitrarySliceVisualizer.h>

#include <osg/CLimiterSceneOSG.h>

#include <osg/CModelDragger3D.h>

#if defined(__APPLE__) &&  !defined(_LIBCPP_VERSION)
#include <tr1/array>
#else
#include <array>
#endif

/////////////////////////////////////////////////////
class CDockWidgetEx : public QDockWidget
{
    Q_OBJECT
public:
    //! Constructor
    CDockWidgetEx(QWidget* pParent = NULL) : QDockWidget(pParent) 
    { 
        
    }
    //! Destructor
    ~CDockWidgetEx() { }
};

/////////////////////////////////////////////////////

namespace osg
{
    class CModelVisualizerEx;
}

namespace Ui {
class MainWindow;
}

//! MainWindow class declaration
class MainWindow : public QMainWindow, public IHasEventFilter
{
    Q_OBJECT
    
private:
    static MainWindow* m_pMainWindow;

public:
    explicit MainWindow(QWidget *parent, CPluginManager	*pPlugins);
    ~MainWindow();

    //! Returns pointer to the only instance of MainWindow
    static MainWindow* getInstance() { return m_pMainWindow; } 

    //! Returns pointer to model manager
    data::CModelManager* getModelManager() { return &m_ModelManager; }

    //! Implementing interface for accessing event filter
    virtual const CEventFilter &getEventFilter() const { return m_eventFilter; }

    //! Find plugin by name
    QObject*                findPluginByID(QString sPluginName);

#ifdef WIN32 // Windows need nonunicode paths to be in ACP
    static std::string     wcs2ACP(const std::wstring &filename);
#endif

    //! Get path for save/load operation of segmentation data, model, etc
    static QString getSaveLoadPath(const QString& key)
    {
        QSettings settings;
        int savePathMode = settings.value("SavePathMode",DEFAULT_SAVE_PATH_MODE).toInt();
        QString lastUsed = settings.value(key).toString();
        if (1==savePathMode)
        {
            MainWindow* pMainWindow = MainWindow::getInstance();
            if (NULL!=pMainWindow)
            {
                QFileInfo inf(pMainWindow->m_wsProjectPath);
				QString absPath = pMainWindow->m_wsProjectPath;
				if (!inf.isDir())
					absPath = inf.dir().absolutePath();
                if (!absPath.isEmpty())
                    lastUsed = absPath;
            }
        }
        return lastUsed;
    }

    static QString appendSaveNameHint(QString dir, const QString& extension)
    {
        data::CObjectPtr<data::CDensityData> spVolume( APP_STORAGE.getEntry(data::Storage::PatientData::Id) );
        QString wsPatientName(spVolume->m_sPatientName.c_str());

		QSettings settings;
		int savedFilesNameMode = settings.value("SavedFilesNameMode").toInt();

		if (savedFilesNameMode == 1)
		{
			QDir d(dir);
			wsPatientName = d.dirName();
		}

        if (wsPatientName.isEmpty())
        {
            MainWindow* pMainWindow = MainWindow::getInstance();
            if (NULL!=pMainWindow)
            {
                QFileInfo inf(pMainWindow->m_wsProjectPath);
                wsPatientName = inf.baseName();
            }
        }
        if (!wsPatientName.isEmpty())
        {
            wsPatientName.replace("^"," ");
			wsPatientName.replace("/"," ");
			wsPatientName.replace("\\"," ");
            int idxGroup = wsPatientName.indexOf('=');
            if (idxGroup>0)
                wsPatientName = wsPatientName.left(idxGroup);
            if (!dir.isEmpty() && !dir.endsWith('/') && !dir.endsWith('\\'))
                dir+='/';
            dir += wsPatientName.trimmed() + extension;
        }
        return dir;
    }

    //! Get renderer pointer
    PSVR::PSVolumeRendering* getRenderer() { return NULL==m_3DView?NULL:&m_3DView->getRenderer(); } ;

    //! Workspace layouts definitions
    enum
    {
        Workspace3D=0,
        WorkspaceTabs=1,
        WorkspaceGrid=2,
        WorkspaceSlices=3
    };

    enum
    {
        View3D = 0,
        ViewXY = 1,
        ViewXZ = 2,
        ViewYZ = 3,
        ViewARB = 4
    };

private:
    //! UI created in designer
    Ui::MainWindow *ui;

    //! Event filter, which handles ui events recording
    CEventFilter m_eventFilter;

    //! Current basic layout
    int                                         m_nLayoutType;  // Workspace	

    //! OSG window with 3D scene and VR
    CVolumeRendererWindow*          m_3DView;
    //! OSG window with Axial slice
    OSGOrtho2DCanvas*               m_OrthoXYSlice;
    //! OSG window with Coronal slice
    OSGOrtho2DCanvas*               m_OrthoXZSlice;
    //! OSG window with Sagittal slice
    OSGOrtho2DCanvas*               m_OrthoYZSlice;

    OSGOrtho2DCanvas*               m_ArbitrarySlice;

    //! Dock widgets with osg windows
    /*QDockWidget                     m_wnd3DView,
                                    m_wndXYView,
                                    m_wndXZView,
                                    m_wndYZView,
                                    m_wndArbitraryView;*/

    std::vector<CCustomDockWidget*> m_wndViews;

    /*QDockWidget                     m_wnd3DViewSubstitute,
                                    m_wndXYViewSubstitute,
                                    m_wndXZViewSubstitute,
                                    m_wndYZViewSubstitute,
                                    m_wndArbitraryViewSubstitute;*/

    std::vector<QDockWidget*> m_wndViewsSubstitute;

    /*CCustomDockWidgetTitle*       m_wnd3DViewCustomToolbar;
    CCustomDockWidgetTitle*       m_wndXYViewCustomToolbar;
    CCustomDockWidgetTitle*       m_wndXZViewCustomToolbar;
    CCustomDockWidgetTitle*       m_wndYZViewCustomToolbar;
    CCustomDockWidgetTitle*       m_wndArbitraryViewCustomToolbar;*/

    struct sWindowInfo
    {
        unsigned int windowType;
        bool bFloating;
        bool bShowSubstitute;
        bool bDirty;

        sWindowInfo(unsigned int type, bool floating, bool showSubstitute, bool dirty)
            : windowType(type)
            , bFloating(floating)
            , bShowSubstitute(showSubstitute)
            , bDirty(dirty)
        {}

        inline bool operator==(const sWindowInfo& info)
        {
            return windowType == info.windowType;
        }
    };

    std::vector<sWindowInfo> m_viewsToShow;

    int getIndexOfWndInLayout(unsigned int wnd);

    // helpers
    QTimer                          m_timer;

	//! Signal connection for volume data change
	vpl::mod::tSignalConnection m_Connection, m_Connection2DCanvasLeave;

    //! Network access manager
    QNetworkAccessManager m_networkManager;

    void sig2DCanvasLeave(OSGCanvas* pCanvas, bool bLeave);

	//! Called on volume data change.
	void onNewDensityData(data::CStorageEntry *pEntry);

// true data
    //! Examination managing all data.
    data::CAppExamination           m_Examination;

    //! Model manager.
    data::CModelManager             m_ModelManager;

	//! Used model color
	data::CColor4f					m_modelColor;

	//! Used model label
	std::string						m_modelLabel;
    //! Model visualizers
#if defined(__APPLE__) &&  !defined(_LIBCPP_VERSION)
    std::tr1::array
#else
    std::array
#endif
    <osg::CModelVisualizerEx*, MAX_IMPORTED_MODELS>      m_modelVisualizers;

#ifdef ENABLE_DEEPLEARNING
    //! Used to render the reference model for landmark annotation
    osg::ref_ptr<osg::CModelVisualizerEx>                m_referenceAnatomicalLandmarkModelVisualizer;
    //! Stores reference landmarks attached to the reference landmark annotation visualizer so they can be deleted from it on request
    osg::NodeList                                        m_referenceAnatomicalLandmarkModelAnnotations;
#endif

    //! STL model dragger
    osg::ref_ptr<osg::CPivotDraggerHolder>      m_draggerPivot;
    osg::ref_ptr<osg::C3DModelDraggerHolder>    m_draggerModel;
    osg::ref_ptr<osg::COnOffNode>               m_switchDraggerModel;

    //! OSG scenes.
    osg::ref_ptr<scene::CScene3D>   m_Scene3D;
    osg::ref_ptr<scene::CSceneXY>   m_SceneXY;
    osg::ref_ptr<scene::CSceneXZ>   m_SceneXZ;
    osg::ref_ptr<scene::CSceneYZ>   m_SceneYZ;
    osg::ref_ptr<scene::CArbitrarySliceScene> m_SceneArb;
    osg::ref_ptr<osg::CArbitrarySliceVisualizer> m_arbSliceVisualizer;

	/*osg::ref_ptr<osg::CRegionsVisualizerForSliceXY> m_xyVizualizer;
	osg::ref_ptr<osg::CRegionsVisualizerForSliceXZ> m_xzVizualizer;
	osg::ref_ptr<osg::CRegionsVisualizerForSliceYZ> m_yzVizualizer;*/
    osg::ref_ptr<osg::CMultiClassRegionsVisualizerForSliceXY> m_xyVizualizer;
    osg::ref_ptr<osg::CMultiClassRegionsVisualizerForSliceXZ> m_xzVizualizer;
    osg::ref_ptr<osg::CMultiClassRegionsVisualizerForSliceYZ> m_yzVizualizer;

	osg::ref_ptr<osg::CVolumeOfInterestVisualizerForSliceXY> m_xyVOIVizualizer;
	osg::ref_ptr<osg::CVolumeOfInterestVisualizerForSliceXZ> m_xzVOIVizualizer;
	osg::ref_ptr<osg::CVolumeOfInterestVisualizerForSliceYZ> m_yzVOIVizualizer;

	bool m_contoursVisible;

	bool m_VOIVisible;

	bool m_segmentationPluginsLoaded;

    //! 3D window drawing handler
    osg::ref_ptr<osgGA::CISWindowEH> m_drawW3DEH;

    //! 3D scene drawing event handler
    osg::ref_ptr< osgGA::CISScene3DEH > m_draw3DEH;

    //! XY scene drawing event handler
    osg::ref_ptr< osgGA::CISSceneXYEH > m_drawXYEH;

    //! XZ scene drawing event handler
    osg::ref_ptr< osgGA::CISSceneXZEH > m_drawXZEH;

    //! YZ scene drawing event handler
    osg::ref_ptr< osgGA::CISSceneYZEH > m_drawYZEH;

    //! ARB scene drawing event handler
    osg::ref_ptr< osgGA::CISSceneARBEH > m_drawARBEH;

    //! Event handlers - measurement on 3D scene
    osg::ref_ptr< scene::CMeasurements3DEH > m_measurements3DEH;

    //! Event handlers - measurement on XY scene
    osg::ref_ptr< scene::CMeasurementsXYEH > m_measurementsXYEH;

    //! Event handlers - measurement on XZ scene
    osg::ref_ptr< scene::CMeasurementsXZEH > m_measurementsXZEH;

    //! Event handlers - measurement on YZ scene
    osg::ref_ptr< scene::CMeasurementsYZEH > m_measurementsYZEH;

#ifdef ENABLE_DEEPLEARNING
    //! Event handlers - landmark annotation on 3D scene
    osg::ref_ptr< scene::CLandmarkAnnotations3DEH > m_landmarkAnnotations3DEH;

    //! Event handlers - landmark annotation on XY scene
    osg::ref_ptr< scene::CLandmarkAnnotationsXYEH > m_landmarkAnnotationsXYEH;

    //! Event handlers - landmark annotation XZ scene
    osg::ref_ptr< scene::CLandmarkAnnotationsXZEH > m_landmarkAnnotationsXZEH;

    //! Event handlers - landmark annotation YZ scene
    osg::ref_ptr< scene::CLandmarkAnnotationsYZEH > m_landmarkAnnotationsYZEH;
#endif

	//! Cuts through model
	osg::ref_ptr<osg::CModelCutVisualizerSliceXY> m_importedModelCutSliceXY[MAX_IMPORTED_MODELS];
	osg::ref_ptr<osg::CModelCutVisualizerSliceXZ> m_importedModelCutSliceXZ[MAX_IMPORTED_MODELS];
	osg::ref_ptr<osg::CModelCutVisualizerSliceYZ> m_importedModelCutSliceYZ[MAX_IMPORTED_MODELS];

    //! HACK: We need a permanent placeholder and parent for windows in central area for proper sizes during workspace switching
    QWidget*                m_centralWidget;
    //! "Real" central widget, ie the one that we care of (got this pointer for easier cleanup only)
    QWidget*                m_realCentralWidget;
    //! Standard docking panels
    CDensityWindowWidget*   m_densityWindowPanel;
    COrthoSlicesWidget*     m_orthoSlicesPanel;
    CSegmentationWidget*    m_segmentationPanel;
    CVolumeRenderingWidget* m_volumeRenderingPanel;
    CModelsWidget*          m_modelsPanel;

    //! Event filter for tabs
    TabBarMouseFunctionalityEx  m_tabsEventFilter;

    //! Status bar density label
    QLabel*                 m_grayLevelLabel;

    //! Density under mouse cursor handler
    void                    densityMeasureHandler(double value);

// Plugins
    CPluginManager			*m_pPlugins;

// Translations
    //! Translations base directory
    QDir                    m_localeDir;

    bool m_myViewVisibilityChange;

    std::vector<int> m_visibleModelsBefore3Dseg;

 // Region 3D Preview
    osg::CRegion3DPreviewVisualizer *m_region3DPreviewVisualizer;
    bool m_region3DPreviewVisible;
    data::CRegion3DPreviewManager *m_region3DPreviewManager;

    vpl::base::CFunctor<void> *m_on3DPreviewStart;
    vpl::base::CFunctor<void> *m_on3DPreviewUpdate;
    vpl::base::CFunctor<void, geometry::CTrianglesContainer&> *m_on3DPreviewStop;

    void on3DPreviewStart();
    void on3DPreviewUpdate();
    void on3DPreviewStop(geometry::CTrianglesContainer& container);

    void setUpRegion3DPreview();
    void destroyRegion3DPreview();

    void onDrawingInProgress(bool drawingInProgress);

    bool setStrokeOnOff(bool on, int region);

// mouse mode handling
    //! Signal connection for mouse mode change monitoring
    vpl::mod::tSignalConnection m_ConnectionModeChanged;

    //! updates UI according to current mouse mode
    void            sigModeChanged( scene::CAppMode::tMode mode );

    //! Updates UI on scene hit
    void            sigSceneHit(float x, float y, float z, int EventType);

//! Project path (also in property "ProjectName" for better compatibility with plugins)
    QString         m_wsProjectPath;

// region data monitoring
    void            sigRegionDataChanged(data::CStorageEntry *pEntry);
    vpl::mod::tSignalConnection m_conRegionData;

    void sigRegionColoringChanged(data::CStorageEntry *pEntry);
    vpl::mod::tSignalConnection m_conRegionColoring;

// model monitoring
    void            sigBonesModelChanged(data::CStorageEntry *pEntry);
    vpl::mod::tSignalConnection m_conBonesModel;

// creation
    //! Connect actions to slots
    void            connectActions();

	//! Connect actions to slot in event filter
    void connectActionsToEventFilter();

    //! Create toolbars
    void            createToolBars();
    //! Create OSG scenes
    void            createOSGStuff();
    //! Create standard panels and associated dock widgets
    void            createPanels();
    //! Set up workspace, create dockwidgets for OSG scenes
    void            setUpWorkspace();
    //! Update workspaces actions checked state
    void            workspacesEnabler();
    //! Setup dock widgets for osg scenes
    void            setupDockWindows();

	bool			getContoursVisibility();

    CCustomDockWidgetTitle* createTitleBar(unsigned int wnd, unsigned int commonButton = DWT_BUTTONS_PIN, bool showSlider = true);

// model cut
	//! Set Model Cut Visibility
	void			setModelCutVisibilitySignal(int id, bool bShow);
	//! Get Model Cut Visibility
	bool			getModelCutVisibilitySignal(int id);

// settings
    //! load application settings (excluding workspace related)
    void            loadAppSettings();
    //! save application settings (excluding workspace related)
    void            saveAppSettings();
    //! load lut settings
    void            loadLookupTables(QSettings &settings, std::map<std::string, CLookupTable> &luts);
    //! save lut settings
    void            saveLookupTables(QSettings &settings, std::map<std::string, CLookupTable> &luts);
    //! save windows relative size
    QSizeF          getRelativeSize(QWidget* widget);
    //! write layout settings
    void            writeLayoutSettings(int nLayoutType, bool bInnerLayoutOnly);
    //! read layout settings
    void            readLayoutSettings(bool bInnerLayoutOnly);
    //! enablers after workspace change
    void            afterWorkspaceChange();

    void saveFloatingWindowsGeometry();
    void loadFloatingWindowsGeometry();

// helpers
    //! detaches widgets from their parent dock windows
    void            removeViewsParentWidget(QWidget *view);
    void removeDockWidgetAndWindow(QDockWidget* dockWidget);
    //! returns parent dock widget
    QDockWidget*    getParentDockWidget(QWidget* view);
    //! Show Message Box
    void            showMessageBox(QMessageBox::Icon icon, QString message);
    //! Fix bad slice slider position after load
    void            fixBadSliceSliderPos();
    //! Post open action for DICOM and VLM
    void            postOpen(const QString& filename, bool bDicomData);
    //! Post open action for DICOM and VLM, initialize segmentation volume, undo
    void            postOpenActions();
    //! Post save action for DICOM and VLM
    void            postSave(const QString& filename);
    //! Action before project open (return false to cancel)
    bool            preOpen();

    //! List of version of entries for the last save
    std::vector<int> m_savedEntriesVersionList;
    //! Get current saved entries version list
    std::vector<int> getVersionList();
    //! Any of saved entries changed since last save?
    bool            isDirty();
	// Find usable model storage id
	int findPossibleModelId();

// undo
    //! Enabler for undo/redo
    void            undoRedoEnabler();

	void enableMenuActions();

    void setActiveRegion3DPreviewVisibility(bool visible);
    void setActiveRegion3DPreviewInterval(int value);
    bool getActiveRegion3DPreviewVisibility();

    void changeVRVisibility(bool visible);
    bool isVRVisible();
    void onSigVRChanged();
    bool areModelsVisible();
    void changeModelsVisibility(bool visible);
    void pluginLog(int type, const std::string& whatInvoke, const std::string& message);


    bool eventFilter(QObject *object, QEvent *event) override;

    //! notify our server that the license lives
    void pingOurServer();
    //! Get mac address
    QString getMacAddress();
    //! Get ip4 address
    quint32 getIPAddress();
    //! Convert std::wstring to QString
    QString QStringFromWString(const std::wstring & wsString);
    //! Retrieve serial number of hard disk
    std::wstring getHardDiskSerialNumber();

// events
protected:
    //! handle show event to restore layout
    void            showEvent(QShowEvent *event);
    //! handle close event to save layout
    void            closeEvent(QCloseEvent *event);

    void enterEvent(QEvent* event);

	void showPreviewDialog(const QString& title, CPreviewDialogData& dialogData, int dataType);

	//! function gets the title of the window and two QStrings, each of them represents rows titles/values separated by ';' and color of the first row
	void showInfoDialog(const QString& title, const QString& rowsTitles, const QString& rowsValues, const QColor& color);

// drag and drop support
protected:
    //! Mouse drag event handling
    virtual void dragEnterEvent(QDragEnterEvent* event);
    virtual void dragMoveEvent(QDragMoveEvent* event);
    //virtual void dragLeaveEvent(QDragLeaveEvent* event);
    //! Mouse drop event handling
    virtual void dropEvent(QDropEvent* event);
    bool canAcceptEvent(QDropEvent* event);

protected:
    void onVREnabledChange(bool value);

public slots:
    void update3DPreview();

// slots
private slots:
    //! Actions that need to be performed after app start
    void            firstEvent();
// load and save
    //! Load volumetric data
    bool            openVLM();
    //! Load volumetric data
    bool            openVLM(const QString &wsFileName);
    //! Load DICOM dataset
    bool            openDICOM();
    //! Load DICOM dataset from ZIP
    bool            openDICOMZIP();
    //! Load DICOM dataset from ZIP
    bool            openDICOMZIP(QString fileName);
    //! Load DICOM dataset
    bool            openDICOM(const QString& fileName, const QString& realName, bool fromFolder);
    //! Load STL model (doesn't drop other data)
    bool            openSTL();
    //! Load STL model (doesn't drop other data)
	bool            openSTL(const QString &wsFilePath, const QString &FileName);
    //! Save DICOM series (copy files)
    bool            saveOriginalDICOM();
	//! Save DICOM series (save loaded volume)
    bool            saveDICOM();
    //! Save volumetric data
    bool            saveVLMAs();
    //! Save STL model
    bool            saveSTL();

    bool            saveSTLinDicomCoords();

	bool            saveSTLById(int storage_id, bool useDicom);

    //! Print method
    void            print();

	//! Receive DICOM data
	void			receiveDicomData();

// Plugins
    //! Triggers action from a specified plugin, if available
    void            triggerPluginAction(const QString& pluginName, const QString& actionName);

    //! Send Data using DataExpress service (plugin required)
    void            sendDataExpressData();

    //! Measure Density using Gauge plugin
    void            measureDensity(bool);

    //! Measure Distance using Gauge plugin
    void            measureDistance(bool);

    //! Clear Measurements using Gauge plugin
    void            clearMeasurements();

// Dialogs
    //! Show Preferences Dialog
    void            showPreferencesDialog();

    //! Show properties of current data set
    void            showDataProperties();

    //! Show Help window
    void            showHelp();

	void            showTutorials();

	void            showFeedback();

    //! Show application's "About" window
    void            showAbout();

    //! Show basic information on loaded plugins
    void            showAboutPlugins();

// enablers and other crap
    //! Updates actions in toolbars and menus when some view or panel is shown/hidden
    void            actionsEnabler();

    //! Updates actions for visibility of toolbars when some toolbar is shown/hidden
    void            toolbarsEnabler();

    //! Checks mouse position against canvas rect
    bool            shallUpdateOSGCanvas(OSGCanvas* pCanvas, const QPoint& mousePos);

    //! update for osg canvases called on timer (necessary for OSG event queue to work properly)
    void            show_frame(); // OSG animation

// Basic show/hide implementations
    //! Show/hide toolbars
    void            showMainToolBar();
    void            showViewsToolBar();
    void            showMouseToolBar();
    void            showVisibilityToolBar();
    void            showPanelsToolBar();

    //! Show/hide views
    void            showAnyView(bool bShow, QWidget* pView);
    void            show3DView(bool);
    void            showAxialView(bool);
    void            showCoronalView(bool);
    void            showSagittalView(bool);
    void            showArbitraryView(bool);

    //! Show/hide panels
    void            showDensityWindowPanel(bool);
    void            showOrthoSlicesPanel(bool);
    void            showSegmentationPanel(bool);
    void            showVRPanel(bool);
	void			showModelsListPanel(bool);

    //! Show/hide slices in 3D scene
    void            showAxialSlice(bool bShow);
    void            showCoronalSlice(bool bShow);
    void            showSagittalSlice(bool bShow);
    void            showArbitrarySlice(bool bShow);

    //! Show VR in 3D scene
    void            showMergedVR(bool bShow);

    //! Show/hide surface model in 3D scene
    void            showSurfaceModel(bool bShow);
    //! Process surface model using extern application
    void            processSurfaceModelExtern();
    //! Change model visualization mode
    void            modelVisualizationSmooth();
    void            modelVisualizationFlat();
    void            modelVisualizationWire();

    //! Show/hide information widgets in OSG windows
    void            showInformationWidgets(bool bShow);

// set up workspaces
    //! Set workspace to a layout where 3D scene is the main window
    void            setUpWorkSpace3D();
    //! Set workspace to a tabbed layout where 3D scene and slices are in the tabs
    void            setUpWorkSpaceTabs();
    //! Set workspace to a layout where 3D scene and slices are in a grid
    void            setUpWorkSpaceGrid();

    void            setUpWorkSpaceSlices();

    void makeFloating3D(bool bFloating);
    void makeFloatingAxial(bool bFloating);
    void makeFloatingCoronal(bool bFloating);
    void makeFloatingSagittal(bool bFloating);
    void makeFloatingArbitrary(bool bFloating);

    void makeWndFloating(unsigned int wnd, bool bFloating, bool updateWorkspace = true);
    void showViewInWindow(int view, int window);

    //! Save user perspective (workspace layout)
    void            saveUserPerspective();
    //! Load user perspective (workspace layout)
    void            loadUserPerspective();
    //! Load default perspective (workspace layout)
    void            loadDefaultPerspective();

    //! Return model visible state
    bool getModelDraggerVisibility();

    //! Shows or hides model dragger
    void setModelDraggerVisibility(bool visible);

    //! Returns the model id of the model connected to the dragger
    int getModelDraggerModelId();

    //! Connects the model dragger for the model with the given id or disables it
    void setModelDraggerModelId(int modelId, bool on);

    //! Update model in storage which is connected to the drager based on dragger move
    void onDraggerMove(int eventType, bool setMatrix);

    // mouse modes
    //! Switch mouse mode to density window adjustment mode
    void            mouseModeDensityWindow(bool);
    //! Switch mouse mode to scene manipulation mode
    void            mouseModeTrackball(bool);
    //! Switch mouse mode to object manipulation mode (slices, implants,...)
    void            mouseModeObjectManipulation(bool);
    //! Switch mouse mode to zoom
    void            mouseModeZoom(bool);

// filters
    //! Perform 3D Gaussian filtering of volumetric data
    void            filterGaussian();
    //! Perform 3D median filtering of volumetric data
    void            filterMedian();
    //! Perform 3D anisotropic filtering of volumetric data
    void            filterAnisotropic();
	//! Perform sharpening filtering of volumetric data
    void            filterSharpen();
	//! Helper volume mixing method
	void			mixVolumes(vpl::img::CDensityVolume* main, vpl::img::CDensityVolume* temp, int mixing) const; // 0-100

// segmentation
	//! Volume limiter
	void limitVolume();

	void resetLimit();

	void setContoursVisibility(bool visible);

	void onMenuContoursVisibleToggled(bool visible);

	void onMenuRegionsVisibleToggled(bool visible);

	void setRegionsVisibility(bool visible);

	void onMenuVOIVisibleToggled(bool visible);

	void setVOIVisibility(bool visible);

	bool getVOIVisibility();

    void moveRegionDataToMulticlass();

    void removeAllModels();

// undo and redo
    //! Undo last undoable action
    void            performUndo();
    //! Redo last action
    void            performRedo();

// splitter hack - because we have nested splitters and want them synchronized
    //! Adjust bottom splitter position when the top one has moved
    void            topSplitterMoved( int pos, int index );
    //! Adjust top splitter position when the bottom one has moved
    void            bottomSplitterMoved( int pos, int index );

// screenshots
    QImage*         canvasScreenShotToQImage(OSGCanvas* pCanvas, int nRenderingSize, bool bIncludeWidgets);
    void            saveScreenshotOfCanvas(OSGCanvas* pCanvas);
    void            saveScreenshot(int viewType);
	void			copyScreenshotToClipboard(OSGCanvas* pCanvas);
    void            saveSlice(OSGCanvas* pCanvas, int mode);

// tab icons
    void            updateTabIcons();
    void            dockWidgetVisiblityChanged(bool visible);
    void            dockLocationChanged ( Qt::DockWidgetArea area );
    void            dockWidget_topLevelChanged(bool changed);

// recent files
    void            addToRecentFiles(const QString& wsFileName);
    void            onRecentFile();
    void            aboutToShowRecentFiles();

// panels context menu
    QMenu *         createPopupMenu ();
    void            onPanelContextMenu(const QPoint & pos);
    void            onDockWidgetToggleView(bool);
	void			showPanelsMenu();
	void			fullscreen(bool);	

    //! Autotab all panels in all widget areas
    void            autoTabPanels();

// texture filters
	void			setTextureFilterEqualize(bool);
	void			setTextureFilterSharpen(bool);
	void			aboutToShowViewFilterMenu();

// keyboard shortcuts
	void			loadShortcuts();
	void			saveShortcuts();
	void			loadShortcutsForMenu(QMenu* menu, QSettings& settings);
	void			saveShortcutsForMenu(QMenu* menu, QSettings& settings);

	//! Detect dock widget visibility by examining its children visibility
	bool			isDockWidgetVisible(QDockWidget* pDW);
	//! find active panel;
	QDockWidget *	getActivePanel();
	//! Close active panel
	void			closeActivePanel();
	//! activate previous panel
	void			prevPanel();
	//! activate next panel
	void			nextPanel();

	void showFeedbackRequestDialog();

	void resetAppMode();

    void onActiveRegionChanged();

    void showSelectedView();

    //! server ping handling
    void pingFinished(QNetworkReply* reply);

    //! ssl error handling
    void sslErrorHandler(QNetworkReply* reply, const QList<QSslError> & errlist);

public slots:
	//! Create surface model
    void            createSurfaceModel();

};


///////////////////////////////////////////////////////////////////////////////

#endif // MAINWINDOW_H
