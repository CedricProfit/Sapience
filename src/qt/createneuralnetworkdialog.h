#ifndef CREATENEURALNETWORKDIALOG_H
#define CREATENEURALNETWORKDIALOG_H

#include <QDialog>
#include "util.h"
#include "plume/dataplume.h"

namespace Ui {
class CreateNeuralNetworkDialog;
}

class CreateNeuralNetworkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateNeuralNetworkDialog(QWidget *parent = 0);
    ~CreateNeuralNetworkDialog();
    CPlumeHeader selectedPlume;
    void SaveNeuralNetwork();

protected:


private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_selectPlumePushButton_clicked();
    void dialogIsFinished(int result);

signals:

private:
    Ui::CreateNeuralNetworkDialog *ui;

};

#endif // CREATENEURALNETWORKDIALOG_H
