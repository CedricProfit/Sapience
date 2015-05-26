#include "optionspage.h"
#include "ui_optionspage.h"
#include <QSettings>

extern bool bStakingUserEnabled;
extern bool bPlumeUserEnabled;

OptionsPage::OptionsPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionsPage)
{
    ui->setupUi(this);

    QSettings settings;
    ui->enableStakingCheckBox->setChecked(settings.value("bStakingEnabled", true).toBool());
    ui->voiceEnabledCheckBox->setChecked(settings.value("bVoiceEnabled", true).toBool());
    ui->dataPlumesCheckBox->setChecked(settings.value("bPlumeUserEnabled", true).toBool());
    bStakingUserEnabled = ui->enableStakingCheckBox->isChecked();
    bPlumeUserEnabled = ui->dataPlumesCheckBox->isChecked();
    connect(ui->enableStakingCheckBox, SIGNAL(stateChanged(int)), this, SLOT(enableStakingCheckBox_stateChanged(int)));
    connect(ui->voiceEnabledCheckBox, SIGNAL(stateChanged(int)), this, SLOT(voiceEnabledCheckBox_stateChanged(int)));
    connect(ui->dataPlumesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(dataPlumesEnabledCheckBox_stateChanged(int)));
}

void OptionsPage::enableStakingCheckBox_stateChanged(int state)
{
    QSettings settings;
    settings.setValue("bStakingEnabled", ui->enableStakingCheckBox->isChecked());
    bStakingUserEnabled = ui->enableStakingCheckBox->isChecked();
}

void OptionsPage::voiceEnabledCheckBox_stateChanged(int state)
{
    QSettings settings;
    settings.setValue("bVoiceEnabled", ui->voiceEnabledCheckBox->isChecked());
}

void OptionsPage::dataPlumesEnabledCheckBox_stateChanged(int state)
{
    QSettings settings;
    settings.setValue("bPlumeUserEnabled", ui->dataPlumesCheckBox->isChecked());
    bPlumeUserEnabled = ui->dataPlumesCheckBox->isChecked();
}

OptionsPage::~OptionsPage()
{
    delete ui;
}
