#include "tabbedconsolepage.h"
#include "ui_tabbedconsolepage.h"

#include "guiutil.h"
#include "guiconstants.h"
#include "consolepage.h"

#include <QAbstractItemDelegate>
#include <QPainter>

TabbedConsolePage::TabbedConsolePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabbedConsolePage)
{
    ui->setupUi(this);
    ConsolePage* cp = new ConsolePage(this);
    ui->tabWidget->clear();
    ui->tabWidget->addTab(cp, "Console 1");

    QWidget* bp = new QWidget(this);
    ui->tabWidget->addTab(bp, "New...");
}

void TabbedConsolePage::on_tabWidget_currentChanged(int index)
{
    if(ui->tabWidget->count() > 1 && index == ui->tabWidget->count() - 1)
    {
	// insert a new console page
        QString title = "Console " + QString::number(index + 1);
	ui->tabWidget->insertTab(index, new ConsolePage(this), title);
	ui->tabWidget->setCurrentIndex(index);
    }
}

void TabbedConsolePage::on_tabWidget_tabCloseRequested(int index)
{
    if(index > 0 && index < ui->tabWidget->count() - 1)
    {
	ui->tabWidget->removeTab(index);
	ui->tabWidget->setCurrentIndex(index - 1);
    }
}

TabbedConsolePage::~TabbedConsolePage()
{
    delete ui;
}
