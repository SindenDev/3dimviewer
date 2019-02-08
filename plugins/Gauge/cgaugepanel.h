///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//

#ifndef CGAUGEPANEL_H
#define CGAUGEPANEL_H

#include <QWidget>

#include <qtplugin/PluginInterface.h>

#include <controls/ccollapsiblegroupbox.h>

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

	//! Called on volume data change.
	void onNewDensityData(data::CStorageEntry *pEntry);

	void updateButtons(scene::CAppMode::tMode mode);

private slots:
    void on_comboBoxMeasuringMode_currentIndexChanged(int index);
	void on_pushButtonDensity_toggled(bool checked);
	void on_pushButtonDistance_toggled(bool checked);
	void on_pushButtonClear_clicked();

    void packGroupBox(bool checked);
    void packGroupBox(QGroupBox* pWidget, bool checked);

private:
    Ui::CGaugePanel *ui;

	//! Signal connection for volume data change
	vpl::mod::tSignalConnection m_Connection;
};

#endif // CGAUGEPANEL_H
