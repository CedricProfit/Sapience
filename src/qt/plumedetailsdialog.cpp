#include "plumedetailsdialog.h"
#include "ui_plumedetailsdialog.h"
#include "guiutil.h"

#include <QDataWidgetMapper>
#include <QMessageBox>

#include "plume/plumecore.h"
#include "plume/plumeapi.h"
#include "plume/plumeheader.h"

PlumeDetailsDialog::PlumeDetailsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlumeDetailsDialog)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

PlumeDetailsDialog::~PlumeDetailsDialog()
{
    delete ui;
}

void PlumeDetailsDialog::accept()
{
    QDialog::accept();
}

void PlumeDetailsDialog::on_closeButton_clicked()
{
    accept();
}

void PlumeDetailsDialog::showPlume(uint256 plumeId)
{
    CPlumeApi api;
    CPlumeHeader hdr;
    hdr = api.GetPlume(plumeId);
    ui->nameLabel->setText(QString::fromStdString(hdr.sPlumeName));
    ui->idLabel->setText(QString::fromStdString(hdr.GetPlumeId().ToString()));
    ui->attrOneLabel->setText(QString::fromStdString(hdr.sAttrOneName));
    ui->attrTwoLabel->setText(QString::fromStdString(hdr.sAttrTwoName));
    ui->attrThreeLabel->setText(QString::fromStdString(hdr.sAttrThreeName));
    ui->originatorLabel->setText(QString::fromStdString(hdr.nOriginatorPeerId.ToString()));

    // put neural nodes in table
    BOOST_FOREACH(uint256 p, hdr.lNeuralNodes)
    {
        CPlumePeer neuralPeer = api.GetPlumePeer(p);
        QString id = QString::fromStdString(neuralPeer.peerinfo.nPlumePeerId.ToString());
        QString state = "Not Connected";
        if(neuralPeer.pnode->fSuccessfullyConnected)
            state = "Connected";
        QString addr = QString::fromStdString(neuralPeer.pnode->addr.ToStringIPPort());

        ui->tableWidget->insertRow(0);
        QTableWidgetItem *idItem = new QTableWidgetItem(id);
        QTableWidgetItem *addrItem = new QTableWidgetItem(addr);
        QTableWidgetItem *stateItem = new QTableWidgetItem(state);

        ui->tableWidget->setItem(0, 0, idItem);
        ui->tableWidget->setItem(0, 1, addrItem);
        ui->tableWidget->setItem(0, 2, stateItem);
    }
}
