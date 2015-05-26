#include "aicorepage.h"
#include "ui_aicorepage.h"

#include "guiutil.h"
#include "guiconstants.h"
#include "ui_interface.h"

#include "createneuralnetworkdialog.h"
#include "trainneuralnetworkdialog.h"


AiCorePage::AiCorePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AiCorePage)
{
    ui->setupUi(this);
    subscribeToCoreSignals();
    ui->trainNeuralNetworkButton->setEnabled(false);
}

AiCorePage::~AiCorePage()
{
    unsubscribeFromCoreSignals();
    delete ui;
}


void AiCorePage::on_createNeuralNetworkButton_clicked()
{

    CreateNeuralNetworkDialog *cnnd = new CreateNeuralNetworkDialog(this);
    cnnd->exec();
}

void AiCorePage::on_trainNeuralNetworkButton_clicked()
{

    TrainNeuralNetworkDialog *cnnd = new TrainNeuralNetworkDialog(this);
    cnnd->exec();
}

void AiCorePage::on_jobsTableWidget_itemSelectionChanged()
{
    // if any job is selected, enable Train
    if(ui->jobsTableWidget->selectedItems().count() > 0)
    {
        ui->trainNeuralNetworkButton->setEnabled(true);
    }
    else
    {
        ui->trainNeuralNetworkButton->setEnabled(false);
    }


}

void AiCorePage::updateNeuralNetwork(QString name, QString created, QString expiration, QString networkId)
{
    LOCK(cs_networkstats);
    bool bFound = false;
    int networkRow = 0;
    for(int i=0; i < ui->jobsTableWidget->rowCount(); i++)
    {
        if(ui->jobsTableWidget->item(i, 3)->text() == networkId)
        {
            bFound = true;
            networkRow = i;
            break;
        }
    }

    if(networkRow == 0 && !bFound)
        ui->jobsTableWidget->insertRow(0);

    QTableWidgetItem *nameItem = new QTableWidgetItem(name);
    QTableWidgetItem *expireItem = new QTableWidgetItem(expiration);
    QTableWidgetItem *createdItem = new QTableWidgetItem(created);
    QTableWidgetItem *idItem = new QTableWidgetItem(networkId);

    ui->jobsTableWidget->setItem(networkRow, 0, nameItem);
    ui->jobsTableWidget->setItem(networkRow, 1, createdItem);
    ui->jobsTableWidget->setItem(networkRow, 2, expireItem);
    ui->jobsTableWidget->setItem(networkRow, 3, idItem);
}


static void NotifyNeuralNetworkUpdated(AiCorePage *page, CNeuralNetworkHeader header)
{

    //QString owner = QString::fromStdString(header.nOriginatorPeerId.ToString());
    QString networkName = QString::fromStdString(header.sNetworkName);
    QString created = QString::fromStdString(DateTimeStrFormat(header.nCreatedTime));
    QString expiration = QString::fromStdString(DateTimeStrFormat(header.nExpirationDate));
    QString networkId = QString::fromStdString(header.GetNeuralNetworkId().ToString());

    QMetaObject::invokeMethod(page, "updateNeuralNetwork", Qt::QueuedConnection,
                              Q_ARG(QString, networkName),
                              Q_ARG(QString, created),
                              Q_ARG(QString, expiration),
                              Q_ARG(QString, networkId)
                              );
}



void AiCorePage::subscribeToCoreSignals()
{
    // Connect signals to core
    uiInterface.NotifyNeuralNetworkUpdated.connect(boost::bind(&NotifyNeuralNetworkUpdated, this, _1));
}

void AiCorePage::unsubscribeFromCoreSignals()
{
    // Disconnect signals from core
    uiInterface.NotifyNeuralNetworkUpdated.disconnect(boost::bind(&NotifyNeuralNetworkUpdated, this, _1));
}
