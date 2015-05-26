#ifndef ADDRECORDDIALOG_H
#define ADDRECORDDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

namespace Ui {
    class AddRecordDialog;
}

/** Dialog for manually adding a record to a data plume.
 */
class AddRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddRecordDialog(QWidget *parent = 0);
    ~AddRecordDialog();

public slots:
    void accept();

private:
    Ui::AddRecordDialog *ui;
};

#endif // ADDRECORDDIALOG_H
