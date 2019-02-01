///////////////////////////////////////////////////////////////////////////////
// 
// Copyright 2008-2016 3Dim Laboratory s.r.o.
//

#include "cgaugepanel.h"
#include "ui_cgaugepanel.h"

#include <data/CActiveDataSet.h>
#include <data/CDensityData.h>

#include <QSettings>

CGaugePanel::CGaugePanel(CAppBindings *pBindings, QWidget *parent) :
    QWidget(parent), CAppBindings(pBindings),
    ui(new Ui::CGaugePanel)
{
    ui->setupUi(this);
    ui->comboBoxMeasuringMode->setCurrentIndex(1); // default mode is average

    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox);
    SETUP_COLLAPSIBLE_GROUPBOX(ui->groupBox_2);

    QSettings settings;
    settings.beginGroup("GaugePanel");

    if (settings.value("PackGroup1", false).toBool())
    {
        ui->groupBox->setChecked(false);
        packGroupBox(ui->groupBox, false);
    }
    if (settings.value("PackGroup2", false).toBool())
    {
        ui->groupBox_2->setChecked(false);
        packGroupBox(ui->groupBox_2, false);
    }

    settings.endGroup();

	m_Connection = PLUGIN_APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).connect(this, &CGaugePanel::onNewDensityData);
}

CGaugePanel::~CGaugePanel()
{
    QSettings settings;
    settings.beginGroup("GaugePanel");

    settings.setValue("PackGroup1", !ui->groupBox->isChecked());
    settings.setValue("PackGroup2", !ui->groupBox_2->isChecked());

    settings.endGroup();

	PLUGIN_APP_STORAGE.getEntrySignal(data::Storage::ActiveDataSet::Id).disconnect(m_Connection);

    delete ui;
}

void CGaugePanel::packGroupBox(bool checked)
{
    QGroupBox* pWidget = qobject_cast<QGroupBox*>(sender());
    packGroupBox(pWidget, checked);
}

void CGaugePanel::packGroupBox(QGroupBox* pWidget, bool checked)
{
    if (NULL == pWidget) return;
    int height = pWidget->property("ProperHeight").toInt();
    if (checked)
        pWidget->setMaximumHeight(height);
    else
        pWidget->setMaximumHeight(GROUPBOX_HEIGHT);
}

void CGaugePanel::updateButtons(scene::CAppMode::tMode mode)
{
	ui->pushButtonDensity->blockSignals(true);
	ui->pushButtonDistance->blockSignals(true);
	ui->pushButtonDensity->setChecked(scene::CAppMode::COMMAND_DENSITY_MEASURE == mode);
	ui->pushButtonDistance->setChecked(scene::CAppMode::COMMAND_DISTANCE_MEASURE == mode);
	ui->pushButtonDensity->blockSignals(false);
	ui->pushButtonDistance->blockSignals(false);
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

void CGaugePanel::onNewDensityData(data::CStorageEntry *pEntry)
{
	data::CObjectPtr<data::CDensityData> spData(PLUGIN_APP_STORAGE.getEntry(data::Storage::PatientData::Id));
	QString dataType = QString::fromStdString(spData->m_sModality);

	QList<QAction *> actions;
	actions.append(m_pPlugin->getAction("measure_density"));
	actions.append(m_pPlugin->getAction("measure_distance"));
	actions.append(m_pPlugin->getAction("clear_measurements"));

	foreach(QAction *a, actions)
	{
		a->setToolTip((dataType == "MR") ? a->toolTip().replace("density", "intensity") : a->toolTip().replace("intensity", "density"));
		a->setStatusTip((dataType == "MR") ? a->toolTip().replace("density", "intensity") : a->toolTip().replace("intensity", "density"));
		a->setToolTip((dataType == "MR") ? a->toolTip().replace("Density", "Intensity") : a->toolTip().replace("Intensity", "Density"));
		a->setStatusTip((dataType == "MR") ? a->toolTip().replace("Density", "Intensity") : a->toolTip().replace("Intensity", "Density"));
		a->setText((dataType == "MR") ? a->text().replace("density", "intensity") : a->text().replace("intensity", "density"));
		a->setText((dataType == "MR") ? a->text().replace("Density", "Intensity") : a->text().replace("Intensity", "Density"));
	}	
}

void CGaugePanel::on_pushButtonDensity_toggled(bool checked)
{
	m_pPlugin->getAction("measure_density")->trigger();
	/*if (checked)
		getAppMode()->storeAndSet(scene::CAppMode::COMMAND_DENSITY_MEASURE);
	else
		getAppMode()->restore();*/
}

void CGaugePanel::on_pushButtonDistance_toggled(bool checked)
{
	m_pPlugin->getAction("measure_distance")->trigger();
	/*if (checked)
		getAppMode()->storeAndSet(scene::CAppMode::COMMAND_DISTANCE_MEASURE);
	else
		getAppMode()->restore();*/
}

void CGaugePanel::on_pushButtonClear_clicked()
{
	m_pPlugin->getAction("clear_measurements")->trigger();
}
