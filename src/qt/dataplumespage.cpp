#include "dataplumespage.h"
#include "ui_dataplumespage.h"
#include "createdataplumedialog.h"
#include "plumedetailsdialog.h"
#include "previewdatadialog.h"
#include "addrecorddialog.h"

#include "util.h"
#include "plume/plumepeer.h"
#include "plume/plumecore.h"
#include "ui_interface.h"

#include <map>
#include <QSettings>

using namespace std;

extern int nAnnounceInterval;
int nMessagesBuffer;
int nLogBuffer;

static const int MAX_LOG_ITEMS = 500;
static const int MAX_MESSAGE_ITEMS = 500;

DataPlumesPage::DataPlumesPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataPlumesPage)
{
    ui->setupUi(this);

    subscribeToCoreSignals();

    ui->detailsButton->setEnabled(false);
    ui->previewDataButton->setEnabled(false);
    ui->addRecordButton->setEnabled(false);

    QSettings settings;
    nAnnounceInterval = settings.value("nAnnounceInterval", 10).toInt();
    ui->announceSpinBox->setValue(nAnnounceInterval);
    nMessagesBuffer = settings.value("nMessagesBuffer", 500).toInt();
    ui->messagesBufferSpinBox->setValue(nMessagesBuffer);
    nLogBuffer = settings.value("nLogBuffer", 500).toInt();
    ui->logBufferSpinBox->setValue(nLogBuffer);
    ui->dataPlumesTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->peersTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

DataPlumesPage::~DataPlumesPage()
{
    unsubscribeFromCoreSignals();
    delete ui;
}

void DataPlumesPage::on_announceSpinBox_valueChanged(int value)
{
    QSettings settings;
    settings.setValue("nAnnounceInterval", ui->announceSpinBox->value());
    nAnnounceInterval = ui->announceSpinBox->value();
}

void DataPlumesPage::on_messagesBufferSpinBox_valueChanged(int value)
{
    QSettings settings;
    settings.setValue("nMessagesBuffer", ui->messagesBufferSpinBox->value());
    nMessagesBuffer = ui->messagesBufferSpinBox->value();
}

void DataPlumesPage::on_logBufferSpinBox_valueChanged(int value)
{
    QSettings settings;
    settings.setValue("nLogBuffer", ui->logBufferSpinBox->value());
    nLogBuffer = ui->logBufferSpinBox->value();
}

void DataPlumesPage::updateLog(QString logMessage)
{
    LOCK(cs_logs);
    while(ui->logListWidget->count() >= nLogBuffer)
        ui->logListWidget->takeItem(ui->logListWidget->count() - 1);

    ui->logListWidget->insertItem(0, logMessage);
    ui->logListWidget->scrollToTop();
}

void DataPlumesPage::updateMessage(QString sTime, QString msgType, QString peerAddress, QString msg, uint sizeKb)
{
    LOCK(cs_messages);
    while(ui->messagesTableWidget->rowCount() >= nMessagesBuffer)
        ui->messagesTableWidget->removeRow(ui->messagesTableWidget->rowCount() - 1);

    // Insert a new row at row 0
    ui->messagesTableWidget->insertRow(0);
    QTableWidgetItem *nTimeItem = new QTableWidgetItem(sTime);
    QTableWidgetItem *msgTypeItem = new QTableWidgetItem(msgType);
    QTableWidgetItem *peerAddressItem = new QTableWidgetItem(peerAddress);
    QTableWidgetItem *msgItem = new QTableWidgetItem(msg);
    QTableWidgetItem *sizeKbItem = new QTableWidgetItem(QString::number(sizeKb));

    ui->messagesTableWidget->setItem(0, 0, nTimeItem);
    ui->messagesTableWidget->setItem(0, 1, msgTypeItem);
    ui->messagesTableWidget->setItem(0, 2, peerAddressItem);
    ui->messagesTableWidget->setItem(0, 3, msgItem);
    ui->messagesTableWidget->setItem(0, 4, sizeKbItem);

    ui->messagesTableWidget->scrollToTop();
}

void DataPlumesPage::updatePlumePeer(QString idHash)
{
    LOCK(cs_peers);
    // get the peer object
    uint256 peerid = uint256(idHash.toStdString());
    if(mapPlumePeers.find(peerid) == mapPlumePeers.end())
        return;

    CPlumePeer plumepeer = mapPlumePeers[peerid];
    QString ip = QString::fromStdString(plumepeer.address.ToStringIP());
    QString client = QString::fromStdString(plumepeer.pnode && plumepeer.pnode->fSuccessfullyConnected && !plumepeer.pnode->fDisconnect ? plumepeer.pnode->strSubVer : "Unknown");
    QString connected = plumepeer.pnode ? ((plumepeer.pnode->fSuccessfullyConnected && !plumepeer.pnode->fDisconnect) ? QString::fromStdString(DateTimeStrFormat(plumepeer.pnode->nTimeConnected)) : "Not Connected") : "Not Connected";
    QString up = QString::number(plumepeer.pnode ? plumepeer.pnode->nRecvBytes : 0);
    QString down = QString::number(plumepeer.pnode ? plumepeer.pnode->nSendBytes : 0);
    QString usecount = QString::number(plumepeer.nUseScore);
    QString totalDht = QString::number(plumepeer.peerinfo.nTotalDhtEntries);
    QString pctAvail = QString::number(plumepeer.PercentAvailable(), 'f', 2);
    QString bytesAlloc = QString::number(plumepeer.peerinfo.nAllocatedBytes);
    QString bytesAvail = QString::number(plumepeer.peerinfo.nAvailableBytes);
    QString lastActivity = QString::fromStdString(plumepeer.pnode ? DateTimeStrFormat(plumepeer.pnode->nLastRecv) : "Unknown");

    // ip, client, connected, up, down, use count, total dht, % available, bytes allocated, last activity, id hash
    // if this idhash is already in the table, then update the values, otherwise insert a new row
    bool bFound = false;
    int peerRow = 0;
    for(int i=0; i < ui->peersTableWidget->rowCount(); i++)
    {
        if(ui->peersTableWidget->item(i, 11)->text() == idHash)
        {
            bFound = true;
            peerRow = i;
            break;
        }
    }

    if(peerRow == 0 && !bFound)
        ui->peersTableWidget->insertRow(0);

    QTableWidgetItem *ipItem = new QTableWidgetItem(ip);
    QTableWidgetItem *clientItem = new QTableWidgetItem(client);
    QTableWidgetItem *connectedItem = new QTableWidgetItem(connected);
    QTableWidgetItem *upItem = new QTableWidgetItem(up);
    QTableWidgetItem *downItem = new QTableWidgetItem(down);
    QTableWidgetItem *usecountItem = new QTableWidgetItem(usecount);
    QTableWidgetItem *totalDhtItem = new QTableWidgetItem(totalDht);
    QTableWidgetItem *pctAvailItem = new QTableWidgetItem(pctAvail);
    QTableWidgetItem *bytesAllocItem = new QTableWidgetItem(bytesAlloc);
    QTableWidgetItem *bytesAvailItem = new QTableWidgetItem(bytesAvail);
    QTableWidgetItem *lastActivityItem = new QTableWidgetItem(lastActivity);
    QTableWidgetItem *idHashItem = new QTableWidgetItem(idHash);

    ui->peersTableWidget->setItem(peerRow, 0, ipItem);
    ui->peersTableWidget->setItem(peerRow, 1, clientItem);
    ui->peersTableWidget->setItem(peerRow, 2, connectedItem);
    ui->peersTableWidget->setItem(peerRow, 3, upItem);
    ui->peersTableWidget->setItem(peerRow, 4, downItem);
    ui->peersTableWidget->setItem(peerRow, 5, usecountItem);
    ui->peersTableWidget->setItem(peerRow, 6, totalDhtItem);
    ui->peersTableWidget->setItem(peerRow, 7, pctAvailItem);
    ui->peersTableWidget->setItem(peerRow, 8, bytesAllocItem);
    ui->peersTableWidget->setItem(peerRow, 9, bytesAvailItem);
    ui->peersTableWidget->setItem(peerRow, 10, lastActivityItem);
    ui->peersTableWidget->setItem(peerRow, 11, idHashItem);

}

void DataPlumesPage::updateDataPlume(QString name, QString plumeType, QString expiration, QString lastSeen, QString status, QString dataType, QString numRecords, QString numNeuralNodes, QString numNeuralNodesConnected, QString plumeId)
{
    LOCK(cs_plumestats);
    bool bFound = false;
    int plumeRow = 0;
    for(int i=0; i < ui->dataPlumesTableWidget->rowCount(); i++)
    {
        if(ui->dataPlumesTableWidget->item(i, 8)->text() == plumeId)
        {
            bFound = true;
            plumeRow = i;
            break;
        }
    }

    if(plumeRow == 0 && !bFound)
        ui->dataPlumesTableWidget->insertRow(0);

    QTableWidgetItem *nameItem = new QTableWidgetItem(name);
    QTableWidgetItem *typeItem = new QTableWidgetItem(plumeType);
   // QTableWidgetItem *expireItem = new QTableWidgetItem(expiration);
    QTableWidgetItem *lastSeenItem = new QTableWidgetItem(lastSeen);
    QTableWidgetItem *statusItem = new QTableWidgetItem(status);
    QTableWidgetItem *dataTypeItem = new QTableWidgetItem(dataType);
    QTableWidgetItem *numRecordsItem = new QTableWidgetItem(numRecords);
    QTableWidgetItem *numNeuralNodesItem = new QTableWidgetItem(numNeuralNodes);
    QTableWidgetItem *connectedItem = new QTableWidgetItem(numNeuralNodesConnected);
    QTableWidgetItem *idItem = new QTableWidgetItem(plumeId);

    ui->dataPlumesTableWidget->setItem(plumeRow, 0, nameItem);
    ui->dataPlumesTableWidget->setItem(plumeRow, 1, typeItem);
 //   ui->dataPlumesTableWidget->setItem(plumeRow, 2, expireItem);
    ui->dataPlumesTableWidget->setItem(plumeRow, 2, lastSeenItem);
    ui->dataPlumesTableWidget->setItem(plumeRow, 3, statusItem);
    ui->dataPlumesTableWidget->setItem(plumeRow, 4, dataTypeItem);
    ui->dataPlumesTableWidget->setItem(plumeRow, 5, numRecordsItem);
    ui->dataPlumesTableWidget->setItem(plumeRow, 6, numNeuralNodesItem);
    ui->dataPlumesTableWidget->setItem(plumeRow, 7, connectedItem);
    ui->dataPlumesTableWidget->setItem(plumeRow, 8, idItem);
}

static void NotifyLogMessageAdded(DataPlumesPage *page, std::string logMessage)
{
    QMetaObject::invokeMethod(page, "updateLog", Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromStdString(logMessage)));
}

static void NotifyMessageActivity(DataPlumesPage *page, int64_t nTime, std::string msgType, std::string peerAddress, std::string msg, uint sizeKb)
{
    QMetaObject::invokeMethod(page, "updateMessage", Qt::QueuedConnection,
                              Q_ARG(QString, QString::fromStdString(DateTimeStrFormat(nTime))),
                              Q_ARG(QString, QString::fromStdString(msgType)),
                              Q_ARG(QString, QString::fromStdString(peerAddress)),
                              Q_ARG(QString, QString::fromStdString(msg)),
                              Q_ARG(uint, sizeKb));
}

static void NotifyPlumePeerUpdated(DataPlumesPage *page, CPlumePeer plumepeer)
{

        QString idHash = QString::fromStdString(plumepeer.peerinfo.nPlumePeerId.ToString());

        QMetaObject::invokeMethod(page, "updatePlumePeer", Qt::QueuedConnection,
                              Q_ARG(QString, idHash)
                              );

}

static void NotifyPlumeUpdated(DataPlumesPage *page, CPlumeHeader header)
{
    // plume name, plume type, expiration, last seen, status, data type, # records, # neural nodes, # neural nodes connected, owner

    QString owner = QString::fromStdString(header.nOriginatorPeerId.ToString());
    QString plumeName = QString::fromStdString(header.sPlumeName);
    QString plumeType = "Unknown";
    if(header.bIsPublic)
        plumeType = "Public";
    else if(mapMyDataPlumes.find(header.GetPlumeId()) != mapMyDataPlumes.end())
        plumeType = "Private";
    else if(mapMyServicedPlumes.find(header.GetPlumeId()) != mapMyServicedPlumes.end())
        plumeType = "Customer";

    QString expiration = QString::number(header.nExpirationDate);
    QString lastSeen = QString::fromStdString(DateTimeStrFormat(GetTime()));
    QString status = "Unknown";
    //if(header.nExpirationDate <= GetTime())
    //    status = "Expired";
    //else
        status = "Active";

    QString dataType = "JSON";
    QString numRecords = "0";
    QString numNeuralNodes = QString::number(header.nNeuralNodesRequested);
    QString numNeuralNodesConnected = QString::number(header.lNeuralNodes.size());
    QString plumeId = QString::fromStdString(header.GetPlumeId().ToString());

    QMetaObject::invokeMethod(page, "updateDataPlume", Qt::QueuedConnection,
                              Q_ARG(QString, plumeName),
                              Q_ARG(QString, plumeType),
                              Q_ARG(QString, expiration),
                              Q_ARG(QString, lastSeen),
                              Q_ARG(QString, status),
                              Q_ARG(QString, dataType),
                              Q_ARG(QString, numRecords),
                              Q_ARG(QString, numNeuralNodes),
                              Q_ARG(QString, numNeuralNodesConnected),
                              Q_ARG(QString, plumeId)
                              );
}

void DataPlumesPage::subscribeToCoreSignals()
{
    // Connect signals to core
    uiInterface.NotifyPlumeLogEntry.connect(boost::bind(&NotifyLogMessageAdded, this, _1));
    uiInterface.NotifyPlumeMessage.connect(boost::bind(&NotifyMessageActivity, this, _1, _2, _3, _4, _5));
    uiInterface.NotifyPlumePeer.connect(boost::bind(&NotifyPlumePeerUpdated, this, _1));
    uiInterface.NotifyDataPlumeUpdated.connect(boost::bind(&NotifyPlumeUpdated, this, _1));
}

void DataPlumesPage::unsubscribeFromCoreSignals()
{
    // Disconnect signals from core
    uiInterface.NotifyPlumeLogEntry.disconnect(boost::bind(&NotifyLogMessageAdded, this, _1));
    uiInterface.NotifyPlumeMessage.disconnect(boost::bind(&NotifyMessageActivity, this, _1, _2, _3, _4, _5));
    uiInterface.NotifyPlumePeer.disconnect(boost::bind(&NotifyPlumePeerUpdated, this, _1));
    uiInterface.NotifyDataPlumeUpdated.disconnect(boost::bind(&NotifyPlumeUpdated, this, _1));
}

void DataPlumesPage::on_dataPlumesTableWidget_itemSelectionChanged()
{
    // if any plume is selected, enable Details
    if(ui->dataPlumesTableWidget->selectedItems().count() > 0)
        ui->detailsButton->setEnabled(true);

    // if selected plume is mine or is public, enable Preview Data

    // if selected plume is mine, enable Add Record


}

void DataPlumesPage::on_createPlumeButton_clicked()
{
    CreateDataPlumeDialog *createPlumeDialog = new CreateDataPlumeDialog(this);
    createPlumeDialog->exec();
}

void DataPlumesPage::on_detailsButton_clicked()
{
    PlumeDetailsDialog *plumeDetailsDialog = new PlumeDetailsDialog(this);
    // get selected plume id
    QItemSelectionModel* selectionModel = ui->dataPlumesTableWidget->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows();
    if(selected.count() == 0)
        return;

    QModelIndex index = selected.at(0);
    int r = index.row();
    uint256 hash = uint256(ui->dataPlumesTableWidget->item(r, 8)->text().toStdString());
    plumeDetailsDialog->showPlume(hash);
    plumeDetailsDialog->exec();
}

void DataPlumesPage::on_addRecordButton_clicked()
{
    AddRecordDialog *addRecordDialog = new AddRecordDialog(this);
    addRecordDialog->exec();
}

void DataPlumesPage::on_previewDataButton_clicked()
{
    PreviewDataDialog *previewDataDialog = new PreviewDataDialog(this);
    previewDataDialog->exec();
}
