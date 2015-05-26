#include "previewdatadialog.h"
#include "ui_previewdatadialog.h"
#include "guiutil.h"

#include <QDataWidgetMapper>
#include <QMessageBox>

PreviewDataDialog::PreviewDataDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreviewDataDialog)
{
    ui->setupUi(this);

}

PreviewDataDialog::~PreviewDataDialog()
{
    delete ui;
}

void PreviewDataDialog::accept()
{
    QDialog::accept();
}
