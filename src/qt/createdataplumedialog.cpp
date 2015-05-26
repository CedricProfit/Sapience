#include "createdataplumedialog.h"
#include "ui_createdataplumedialog.h"
#include "guiutil.h"
#include "ui_interface.h"
#include "base58.h"
#include "plume/plumepeer.h"
#include "plume/plumecore.h"
#include "plume/dataplume.h"
#include "plume/plumeheader.h"
#include "plume/plumeapi.h"
#include <QDataWidgetMapper>
#include <QMessageBox>

CreateDataPlumeDialog::CreateDataPlumeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateDataPlumeDialog)
{
    ui->setupUi(this);
    subscribeToCoreSignals();

    ui->acceptProposalsButton->setEnabled(true);

    ui->createPlumeButton->setEnabled(true);

}

CreateDataPlumeDialog::~CreateDataPlumeDialog()
{
    unsubscribeFromCoreSignals();
    delete ui;
}

void CreateDataPlumeDialog::accept()
{
    QDialog::accept();
}

void CreateDataPlumeDialog::on_createPlumeButton_clicked()
{
    CPlumeApi api;
    CPlumeHeader hdr = api.CreateDataPlume(
                ui->nameLineEdit->text().toStdString(),
                ui->attrOneLineEdit->text().toStdString(),
                ui->attrTwoLineEdit->text().toStdString(),
                ui->attrThreeLineEdit->text().toStdString(),
                ui->slavesSpinBox->value(),
                ui->plumeTypeCheckBox->isChecked());
    double kbh = ui->kbhSpinBox->value();
    uint256 requestId = api.CreateDataReservationRequest(hdr, kbh);

    QMessageBox msg;
    msg.setText("Data Reservation Created\n" + QString::fromStdString(requestId.ToString()));
    msg.setInformativeText("Please wait for the request to circulat and proposals to return from neural nodes.");
    msg.setDetailedText("The communication is an asynchronous process and could take some time to complete.");
    msg.exec();
}

void CreateDataPlumeDialog::on_cancelButton_clicked()
{
    QDialog::accept();
}

void CreateDataPlumeDialog::on_acceptProposalsButton_clicked()
{
    if(ui->proposalsTableWidget->selectedItems().count() == 0)
    {
        QMessageBox msg;
        msg.setText("Please select one or more proposals to accept.");
        msg.exec();
        return;
    }

    // get the proposal id's
    std::vector<uint256> proposalIds;
    QItemSelectionModel* selectionModel = ui->proposalsTableWidget->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows();
    for(int i= 0; i< selected.count();i++)
    {
        QModelIndex index = selected.at(i);
        int r = index.row();
        uint256 hash = uint256(ui->proposalsTableWidget->item(r, 4)->text().toStdString());
        proposalIds.push_back(hash);
    }

    bool hasErrors = false;
    CPlumeApi api;
    BOOST_FOREACH(uint256 p, proposalIds)
    {
        std::string err;
        bool res = api.AcceptProposal(p, err);
        if(!res)
        {
            hasErrors = true;
            QMessageBox msg;
            msg.setText(QString::fromStdString(err));
            msg.exec();
        }
    }

    if(!hasErrors)
        QDialog::accept();
}

void CreateDataPlumeDialog::on_slavesSpinBox_valueChanged(int value)
{
    // if 0 slaves selected, enable create plume button
    if(value == 0)
        ui->createPlumeButton->setEnabled(true);

    // if >0 slaves selected, but 0 plume peers are connected, disable create plume button


}

void CreateDataPlumeDialog::updateConnectedPeers(int peerCount)
{
    ui->peersLabel->setText(QString::number(peerCount) + " Plume Peers connected");
}

//peer id, rate, cost, payment address, proposal hash
void CreateDataPlumeDialog::addReceivedProposal(QString peerId, QString rate, QString cost, QString paymentAddress, QString proposalHash)
{
    ui->proposalsTableWidget->insertRow(0);

    QTableWidgetItem *peerIdItem = new QTableWidgetItem(peerId);
    QTableWidgetItem *rateItem = new QTableWidgetItem(rate);
    QTableWidgetItem *costItem = new QTableWidgetItem(cost);
    QTableWidgetItem *paymentAddressItem = new QTableWidgetItem(paymentAddress);
    QTableWidgetItem *proposalHashItem = new QTableWidgetItem(proposalHash);

    ui->proposalsTableWidget->setItem(0, 0, peerIdItem);
    ui->proposalsTableWidget->setItem(0, 1, rateItem);
    ui->proposalsTableWidget->setItem(0, 2, costItem);
    ui->proposalsTableWidget->setItem(0, 3, paymentAddressItem);
    ui->proposalsTableWidget->setItem(0, 4, proposalHashItem);

}

static void NotifyConnectedPeerCountChanged(CreateDataPlumeDialog *page, int peerCount)
{
    QMetaObject::invokeMethod(page, "updateConnectedPeers", Qt::QueuedConnection,
                              Q_ARG(int, peerCount)
                              );
}

static void NotifyProposalReceived(CreateDataPlumeDialog *page, CDataReservationProposal proposal)
{
    QString peerId = QString::fromStdString(proposal.nRequestHash.ToString());
    QString propHash = QString::fromStdString(proposal.GetHash().ToString());
    QString rate = QString::number(proposal.nKilobyteHourRate, 'g', 8);

    // get reservation
    double kbh = 0.0;
    if(mapReservationsWaiting.find(proposal.nRequestHash) != mapReservationsWaiting.end())
    {
        CDataReservationRequest req = mapReservationsWaiting[proposal.nRequestHash];
        kbh = req.nKilobyteHours;
    }

    double totalCost = kbh * proposal.nKilobyteHourRate;
    QString cost = QString::number(totalCost, 'g', 8);
    QString pmtAddy = QString::fromStdString(CBitcoinAddress(proposal.paymentPubKey.GetID()).ToString());

    QMetaObject::invokeMethod(page, "addReceivedProposal", Qt::QueuedConnection,
                              Q_ARG(QString, peerId),
                              Q_ARG(QString, rate),
                              Q_ARG(QString, cost),
                              Q_ARG(QString, pmtAddy),
                              Q_ARG(QString, propHash)
                              );
}

void CreateDataPlumeDialog::subscribeToCoreSignals()
{
    // Connect signals to core
    uiInterface.NotifyPlumeConnectedPeerCountChanged.connect(boost::bind(&NotifyConnectedPeerCountChanged, this, _1));
    uiInterface.NotifyPlumeProposalReceived.connect(boost::bind(&NotifyProposalReceived, this, _1));
}

void CreateDataPlumeDialog::unsubscribeFromCoreSignals()
{
    // Disconnect signals from core
    uiInterface.NotifyPlumeConnectedPeerCountChanged.disconnect(boost::bind(&NotifyConnectedPeerCountChanged, this, _1));
    uiInterface.NotifyPlumeProposalReceived.disconnect(boost::bind(&NotifyProposalReceived, this, _1));
}
