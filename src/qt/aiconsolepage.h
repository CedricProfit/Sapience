#ifndef AICONSOLEPAGE_H
#define AICONSOLEPAGE_H

#include "clientmodel.h"
#include "rpcconsole.h"

#include <QWidget>

namespace Ui {
    class AiConsolePage;
}

class AiConsolePage : public QWidget
{
    Q_OBJECT

public:
    explicit AiConsolePage(QWidget *parent = 0);
    ~AiConsolePage();
    void setClientModel(ClientModel *model);
    void entryFocus();

    enum MessageClass {
        MC_ERROR,
        MC_DEBUG,
        CMD_REQUEST,
        CMD_REPLY,
        CMD_ERROR
    };

protected:
    virtual bool eventFilter(QObject* obj, QEvent *event);

private slots:
    void on_lineEdit_returnPressed();

public slots:
    void clear();
    void message(int category, const QString &message, bool html = false);
    /** Go forward or back in history */
    void browseHistory(int offset);
    /** Scroll console view to end */
    void scrollToEnd();
signals:
    // For RPC command executor
    void stopExecutor();
    void cmdRequest(const QString &command);

private:
    Ui::AiConsolePage *ui;
    static QString FormatBytes(quint64 bytes);
    ClientModel *clientModel;
    QStringList history;
    int historyPtr;

    void startExecutor();
};

#endif // AICONSOLEPAGE_H
