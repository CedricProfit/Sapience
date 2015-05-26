#ifndef PLUMEDETAILSDIALOG_H
#define PLUMEDETAILSDIALOG_H

#include "util.h"
#include <QDialog>

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

namespace Ui {
    class PlumeDetailsDialog;
}

/** Dialog for viewing details about a data plume.
 */
class PlumeDetailsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PlumeDetailsDialog(QWidget *parent = 0);
    ~PlumeDetailsDialog();
    void showPlume(uint256 plumeId);

public slots:
    void accept();
    void on_closeButton_clicked();

private:
    Ui::PlumeDetailsDialog *ui;
};

#endif // PLUMEDETAILSDIALOG_H
