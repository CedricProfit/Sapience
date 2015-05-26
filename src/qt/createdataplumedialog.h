#ifndef CREATEDATAPLUMEDIALOG_H
#define CREATEDATAPLUMEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

namespace Ui {
    class CreateDataPlumeDialog;
}

/** Dialog for creating a data plume.
 */
class CreateDataPlumeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateDataPlumeDialog(QWidget *parent = 0);
    ~CreateDataPlumeDialog();

public slots:
    void accept();
    void updateConnectedPeers(int peerCount);
    void addReceivedProposal(QString peerId, QString rate, QString cost, QString paymentAddress, QString proposalHash);

private:
    Ui::CreateDataPlumeDialog *ui;
    void subscribeToCoreSignals();
    void unsubscribeFromCoreSignals();

private slots:
    void on_createPlumeButton_clicked();
    void on_cancelButton_clicked();
    void on_acceptProposalsButton_clicked();
    void on_slavesSpinBox_valueChanged(int value);
};

#endif // CREATEDATAPLUMEDIALOG_H
