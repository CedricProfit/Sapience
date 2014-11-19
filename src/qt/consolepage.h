#ifndef CONSOLEPAGE_H
#define CONSOLEPAGE_H

#include <QWidget>

namespace Ui {
    class ConsolePage;
}
class ClientModel;
class RPCExecutor;

/** Local Bitcoin RPC console. */
class ConsolePage: public QWidget
{
    Q_OBJECT

public:
    explicit ConsolePage(QWidget *parent = 0);
    ~ConsolePage();

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
    static QString FormatBytes(quint64 bytes);
    Ui::ConsolePage *ui;
    ClientModel *clientModel;
    QStringList history;
    int historyPtr;

    void startExecutor();
};

#endif // CONSOLEPAGE_H
