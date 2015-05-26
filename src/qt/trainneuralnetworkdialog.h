#ifndef TRAINNEURALNETWORKDIALOG_H
#define TRAINNEURALNETWORKDIALOG_H

#include <QDialog>

namespace Ui {
class TrainNeuralNetworkDialog;
}

/** Preferences dialog. */
class TrainNeuralNetworkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TrainNeuralNetworkDialog(QWidget *parent = 0);
    ~TrainNeuralNetworkDialog();
    void PopulatePlumes();

protected:

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_tableWidget_itemSelectionChanged();


signals:

private:
    Ui::TrainNeuralNetworkDialog *ui;

};

#endif // TRAINNEURALNETWORKDIALOG_H
