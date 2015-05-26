#ifndef PREVIEWDATADIALOG_H
#define PREVIEWDATADIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QDataWidgetMapper;
QT_END_NAMESPACE

namespace Ui {
    class PreviewDataDialog;
}

/** Dialog for creating a data plume.
 */
class PreviewDataDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreviewDataDialog(QWidget *parent = 0);
    ~PreviewDataDialog();

public slots:
    void accept();

private:
    Ui::PreviewDataDialog *ui;
};

#endif // PREVIEWDATADIALOG_H
