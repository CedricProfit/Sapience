#include "trainneuralnetworkdialog.h"
#include "ui_trainneuralnetworkdialog.h"

#include "plume/plumeheader.h"
#include "plume/plumecore.h"
#include "plume/plumeapi.h"
#include "ailib/ailib.h"

#include <QMessageBox>

TrainNeuralNetworkDialog::TrainNeuralNetworkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TrainNeuralNetworkDialog)
{
    ui->setupUi(this);
    ui->okButton->setEnabled(false);
    PopulatePlumes();
}

TrainNeuralNetworkDialog::~TrainNeuralNetworkDialog()
{
    delete ui;
}

void TrainNeuralNetworkDialog::on_okButton_clicked()
{
    if(ui->fieldsPlainTextEdit->toPlainText().length() == 0)
    {
        QMessageBox msg;
        msg.setText("Please enter the field names that map to your input neurons, in order.");
        msg.exec();

    }
    else
    {
        accept();
    }
}

void TrainNeuralNetworkDialog::on_cancelButton_clicked()
{
    reject();
}

void TrainNeuralNetworkDialog::on_tableWidget_itemSelectionChanged()
{
    // if any plume is selected, enable Details
    if(ui->tableWidget->selectedItems().count() > 0)
        ui->okButton->setEnabled(true);
}

void TrainNeuralNetworkDialog::PopulatePlumes()
{
    CPlumeApi api;

    std::vector<CPlumeHeader> pplumes = api.ListPublicDataPlumes();
    BOOST_FOREACH(CPlumeHeader h, pplumes)
    {
        ui->tableWidget->insertRow(0);
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(h.sPlumeName));
        QTableWidgetItem *createdItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat(h.nCreatedTime)));
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::fromStdString(h.GetPlumeId().ToString()));

        ui->tableWidget->setItem(0, 0, nameItem);
        ui->tableWidget->setItem(0, 1, createdItem);
        ui->tableWidget->setItem(0, 2, idItem);
    }

    std::vector<CPlumeHeader> plumes = api.ListMyDataPlumes();
    BOOST_FOREACH(CPlumeHeader h, plumes)
    {
        ui->tableWidget->insertRow(0);
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(h.sPlumeName));
        QTableWidgetItem *createdItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat(h.nCreatedTime)));
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::fromStdString(h.GetPlumeId().ToString()));

        ui->tableWidget->setItem(0, 0, nameItem);
        ui->tableWidget->setItem(0, 1, createdItem);
        ui->tableWidget->setItem(0, 2, idItem);
    }


}
