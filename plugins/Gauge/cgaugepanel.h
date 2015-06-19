///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2015 3Dim Laboratory s.r.o.
//

#ifndef CGAUGEPANEL_H
#define CGAUGEPANEL_H

#include <QWidget>

#include <qtplugin/PluginInterface.h>

namespace Ui {
class CGaugePanel;
}

class CGaugePanel : public QWidget, public CAppBindings
{
    Q_OBJECT
    
public:
    explicit CGaugePanel(CAppBindings* pBindings, QWidget *parent = 0);
    ~CGaugePanel();

    //! set last read density to panel
    void setDensity(int nValue);

    //! set last read distance to panel
    void setDistance(double fValue);
    
private slots:
    void on_comboBoxMeasuringMode_currentIndexChanged(int index);

private:
    Ui::CGaugePanel *ui;
};

#endif // CGAUGEPANEL_H
