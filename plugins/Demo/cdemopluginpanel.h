///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2015 3Dim Laboratory s.r.o.
//

#ifndef CDEMOPLUGINPANEL_H
#define CDEMOPLUGINPANEL_H

#include <QWidget>

#include <qtplugin/PluginInterface.h>

#include <controls/ccolorcombobox.h>

namespace Ui {
class CDemoPluginPanel;
}

class CDemoPlugin;

class CDemoPluginPanel : public QWidget, public CAppBindings
{
    Q_OBJECT
    
public:
    explicit CDemoPluginPanel(CAppBindings* pBindings, QWidget *parent = 0);
    ~CDemoPluginPanel();

protected:
    //! Point selected signal handler
    void OnPointPicked( float x, float y, float z, int EventType );

    //! Set drawing options
    void setDrawingOptions( const data::CDrawingOptions::EDrawingMode mode, const osg::Vec4 & color, bool bConnect = true );

    //! Handle drawing
    void handleDrawing( const osg::Vec3Array * points, const int handlerType, const int mouseButton );

    //! Handle stroke
    void handleStroke( const osg::Vec3Array * points, const int handlerType );

    //! Handle polygon
    void handlePolygon( const osg::Vec3Array * points, const int handlerType );

    //! Connect drawing signals
    void connectSignals();

    //! Current drawisq ng mode
    data::CDrawingOptions::EDrawingMode m_drawingMode;

    //! Drawing mode changed signal
    typedef vpl::mod::CSignal< void, int > tSigDrawingModeChanged;

    //! Drawing mode changed signal
    tSigDrawingModeChanged m_sigDrawingModeChanged;

    //! Drawing callback connection
    vpl::mod::tSignalConnection m_conDrawingDone, m_conDrawingModeChanged;

    //! Get current drawing mode
    data::CDrawingOptions::EDrawingMode getMode() { return m_drawingMode; }

    //! Returns the current region id
    int getCurrentRegion();

    //! Mouse mode changed signal handler
    void OnDrawingModeChanged( int Mode );

    //! uncheck buttons
    void updateButtons(QWidget *letBe);

    //! update "show regions" checkbox
    void updateShowRegions(bool bEnable);

    //! Have I focus?
    bool m_haveFocus;

    //! Color combobox for regions
    CColorComboBox m_colorComboBox;

private slots:
    void on_pushButtonPickValue_clicked();

    void on_pushButtonPolygon_toggled(bool checked);

    void on_pushButtonStroke_toggled(bool checked);

    void on_comboBoxRegion_currentIndexChanged(int index);

    void on_checkBoxShowRegions_toggled(bool checked);

private:
    Ui::CDemoPluginPanel *ui;

    friend class CDemoPlugin;
};

#endif // CGAUGEPANEL_H
