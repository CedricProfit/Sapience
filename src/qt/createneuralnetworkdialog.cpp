#include "createneuralnetworkdialog.h"
#include "ui_createneuralnetworkdialog.h"
#include "selectplumedialog.h"

#include "util.h"
#include "ailib/ailib.h"
#include "plume/plumeapi.h"
#include "plume/plumecore.h"

#include <QMessageBox>

CreateNeuralNetworkDialog::CreateNeuralNetworkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateNeuralNetworkDialog)
{
    ui->setupUi(this);


}

CreateNeuralNetworkDialog::~CreateNeuralNetworkDialog()
{
    delete ui;
}


void CreateNeuralNetworkDialog::on_okButton_clicked()
{
    // validate first
    if(ui->nameLineEdit->text().length() == 0)
    {
        QMessageBox msg;
        msg.setText("Please enter a name for this neural network.");
        msg.exec();
        return;
    }
    else if(selectedPlume.sPlumeName == "")
    {
        QMessageBox msg;
        msg.setText("Please select the data plume for neural network state.");
        msg.exec();
        return;
    }
    else
    {
        // save the network
        SaveNeuralNetwork();
        // return
        accept();
    }
}

void CreateNeuralNetworkDialog::on_cancelButton_clicked()
{
    reject();
}

void CreateNeuralNetworkDialog::on_selectPlumePushButton_clicked()
{
    // show the select plume dialog for my plumes only
    SelectPlumeDialog *pd = new SelectPlumeDialog(this, this, false);
    QObject::connect(pd, SIGNAL(finished (int)), this, SLOT(dialogIsFinished(int)));
    pd->exec();
    // capture the returned plume
}

void CreateNeuralNetworkDialog::dialogIsFinished(int result)
{ //this is a slot
   if(result == QDialog::Accepted)
   {
       // update display of plume id
       ui->plumeLineEdit->setText(QString::fromStdString(selectedPlume.sPlumeName));
   }
   else
   {

   }
}

void CreateNeuralNetworkDialog::SaveNeuralNetwork()
{
    AiLib api;
    CNeuralNetworkHeader hdr = api.CreateNeuralNetwork(
    ui->inputSpinBox->value(),
    ui->hiddenSpinBox->value(),
    ui->outputSpinBox->value(),
    ui->nameLineEdit->text().toStdString(),
    ui->localCheckBox->isChecked(),
    selectedPlume.GetPlumeId());
    uint256 networkId = hdr.GetNeuralNetworkId();

    QMessageBox msg;
    msg.setText("Neural Network Project Created\n" + QString::fromStdString(networkId.ToString()));
    msg.setInformativeText("Please wait for the request to circulate before feeding data.");
    msg.setDetailedText("The communication is an asynchronous process and could take some time to complete.");
    msg.exec();
}
