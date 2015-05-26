#ifndef SELECTPLUMEDIALOG_H
#define SELECTPLUMEDIALOG_H
#include "createneuralnetworkdialog.h"

#include <QDialog>

namespace Ui {
class SelectPlumeDialog;
}

class SelectPlumeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectPlumeDialog(QWidget *parent = 0, CreateNeuralNetworkDialog *nnDialog = 0, bool fIncludePublicPlumes = false);
    ~SelectPlumeDialog();
    void PopulatePlumes();

protected:
    CreateNeuralNetworkDialog *nn;


private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_plumesTableWidget_itemSelectionChanged();

signals:

private:
    Ui::SelectPlumeDialog *ui;

};

#endif // SELECTPLUMEDIALOG_H
