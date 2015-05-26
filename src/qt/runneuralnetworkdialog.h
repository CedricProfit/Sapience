#ifndef RUNNEURALNETWORKDIALOG_H
#define RUNNEURALNETWORKDIALOG_H

#include <QDialog>

namespace Ui {
class RunNeuralNetworkDialog;
}

/** Preferences dialog. */
class RunNeuralNetworkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RunNeuralNetworkDialog(QWidget *parent = 0);
    ~RunNeuralNetworkDialog();
    void PopulatePlumes();

protected:

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_tableWidget_itemSelectionChanged();


signals:

private:
    Ui::RunNeuralNetworkDialog *ui;

};

#endif // RUNNEURALNETWORKDIALOG_H
