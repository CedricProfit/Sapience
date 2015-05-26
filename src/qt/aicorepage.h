#ifndef AICOREPAGE_H
#define AICOREPAGE_H

#include "sync.h"
#include <QWidget>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
    class AiCorePage;
}

class AiCorePage : public QWidget
{
    Q_OBJECT

public:
    explicit AiCorePage(QWidget *parent = 0);
    ~AiCorePage();


public slots:
    void updateNeuralNetwork(QString name, QString created, QString expiration, QString networkId);

signals:


private:
    Ui::AiCorePage *ui;
    CCriticalSection cs_networkstats;
    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();


private slots:
    void on_trainNeuralNetworkButton_clicked();
    void on_createNeuralNetworkButton_clicked();
    void on_jobsTableWidget_itemSelectionChanged();
};

#endif // AICOREPAGE_H
