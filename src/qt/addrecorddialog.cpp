#include "addrecorddialog.h"
#include "ui_addrecorddialog.h"
#include "guiutil.h"

#include <QDataWidgetMapper>
#include <QMessageBox>

AddRecordDialog::AddRecordDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddRecordDialog)
{
    ui->setupUi(this);

}

AddRecordDialog::~AddRecordDialog()
{
    delete ui;
}

void AddRecordDialog::accept()
{
    QDialog::accept();
}
