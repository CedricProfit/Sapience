#include "selectplumedialog.h"
#include "ui_selectplumedialog.h"

#include "util.h"
#include "createneuralnetworkdialog.h"
#include "plume/plumecore.h"
#include "plume/plumeapi.h"
#include "plume/dataplume.h"

#include <QTableWidgetItem>
#include <QDebug>

SelectPlumeDialog::SelectPlumeDialog(QWidget *parent, CreateNeuralNetworkDialog *nnDialog, bool fIncludePublicPlumes) :
    QDialog(parent),
    ui(new Ui::SelectPlumeDialog)
{
    ui->setupUi(this);
    ui->okButton->setEnabled(false);
    this->nn = nnDialog;

    // populate table
    PopulatePlumes();

}

SelectPlumeDialog::~SelectPlumeDialog()
{
    delete ui;
}

void SelectPlumeDialog::PopulatePlumes()
{
    CPlumeApi api;
    std::vector<CPlumeHeader> plumes = api.ListMyDataPlumes();
    BOOST_FOREACH(CPlumeHeader h, plumes)
    {
        ui->plumesTableWidget->insertRow(0);
        QTableWidgetItem *nameItem = new QTableWidgetItem(QString::fromStdString(h.sPlumeName));
        QTableWidgetItem *createdItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat(h.nCreatedTime)));
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::fromStdString(h.GetPlumeId().ToString()));

        ui->plumesTableWidget->setItem(0, 0, nameItem);
        ui->plumesTableWidget->setItem(0, 1, createdItem);
        ui->plumesTableWidget->setItem(0, 2, idItem);
    }
}

void SelectPlumeDialog::on_plumesTableWidget_itemSelectionChanged()
{
    // if any plume is selected, enable Details
    if(ui->plumesTableWidget->selectedItems().count() > 0)
    {
        QItemSelectionModel* selectionModel = ui->plumesTableWidget->selectionModel();
        QModelIndexList selected = selectionModel->selectedRows();
        if(selected.count() == 0)
            return;

        QModelIndex index = selected.at(0);
        int r = index.row();
        uint256 hash = uint256(ui->plumesTableWidget->item(r, 2)->text().toStdString());
        CPlumeApi api;
        CPlumeHeader hdr = api.GetPlume(hash);
        nn->selectedPlume = hdr;
        qDebug() << nn->selectedPlume.GetPlumeId().ToString().c_str();
        ui->okButton->setEnabled(true);
    }
}

void SelectPlumeDialog::on_okButton_clicked()
{
    accept();
}

void SelectPlumeDialog::on_cancelButton_clicked()
{
    reject();
}

