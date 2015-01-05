///////////////////////////////////////////////////////////////////////////////
// $Id$
//

#include "cgaugepanel.h"
#include "ui_cgaugepanel.h"

CGaugePanel::CGaugePanel(CAppBindings *pBindings, QWidget *parent) :
    QWidget(parent), CAppBindings(pBindings),
    ui(new Ui::CGaugePanel)
{
    ui->setupUi(this);
    ui->comboBoxMeasuringMode->setCurrentIndex(1); // default mode is average
}

CGaugePanel::~CGaugePanel()
{
    delete ui;
}

void CGaugePanel::setDensity(int nValue)
{
    ui->editDensity->setText(QString::number(nValue));
}

void CGaugePanel::setDistance(double fValue)
{
    ui->editDistance->setText(QString::number(fValue,'f',2));
}

void CGaugePanel::on_comboBoxMeasuringMode_currentIndexChanged(int index)
{
    // Modify measuring mode
    PLUGIN_APP_MODE.getMeasuringParametersSignal().invoke( 0, index );
}
