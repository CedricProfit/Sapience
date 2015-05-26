#ifndef DATAPLUMESPAGE_H
#define DATAPLUMESPAGE_H

#include <QWidget>

#include <map>
#include "plume/plumepeer.h"
#include "plume/plumecore.h"
#include "util.h"

namespace Ui {
    class DataPlumesPage;
}

class DataPlumesPage : public QWidget
{
    Q_OBJECT

public:
    explicit DataPlumesPage(QWidget *parent = 0);
    ~DataPlumesPage();

public slots:
    void updateLog(QString logMessage);
    void updateMessage(QString sTime, QString msgType, QString peerAddress, QString msg, uint sizeKb);
    void updatePlumePeer(QString idHash);
    void updateDataPlume(QString name, QString plumeType, QString expiration, QString lastSeen, QString status, QString dataType, QString numRecords, QString numNeuralNodes, QString numNeuralNodesConnected, QString plumeId);
    
private:
    Ui::DataPlumesPage *ui;
    CCriticalSection cs_plumestats;
    CCriticalSection cs_messages;
    CCriticalSection cs_logs;
    CCriticalSection cs_peers;
    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();

private slots:
    void on_announceSpinBox_valueChanged(int value);
    void on_messagesBufferSpinBox_valueChanged(int value);
    void on_logBufferSpinBox_valueChanged(int value);
    void on_dataPlumesTableWidget_itemSelectionChanged();
    void on_createPlumeButton_clicked();
    void on_detailsButton_clicked();
    void on_addRecordButton_clicked();
    void on_previewDataButton_clicked();
};

#endif // DATAPLUMESPAGE_H
