#include "aiconsolepage.h"
#include "ui_aiconsolepage.h"

#include "guiutil.h"
#include "guiconstants.h"
#include "clientmodel.h"

#include <QTime>
#include <QTimer>
#include <QThread>
#include <QTextEdit>
#include <QKeyEvent>
#include <QUrl>
#include <QScrollBar>

#include <openssl/crypto.h>

const int CONSOLE_SCROLLBACK = 50;
const int CONSOLE_HISTORY = 50;

const QSize ICON_SIZE(24, 24);

const struct {
    const char *url;
    const char *source;
} ICON_MAPPING[] = {
    {"cmd-request", ":/icons/tx_input"},
    {"cmd-reply", ":/icons/tx_output"},
    {"cmd-error", ":/icons/tx_output"},
    {"misc", ":/icons/tx_inout"},
    {NULL, NULL}
};



AiConsolePage::AiConsolePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AiConsolePage),
    historyPtr(0)
{
    ui->setupUi(this);

    // Install event filter for up and down arrow
    ui->lineEdit->installEventFilter(this);
    ui->messagesWidget->installEventFilter(this);
    ui->clearButton->setVisible(false);

    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    startExecutor();
    clear();

}

AiConsolePage::~AiConsolePage()
{
    emit stopExecutor();
    delete ui;
}

bool AiConsolePage::eventFilter(QObject* obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress) // Special key handling
    {
        QKeyEvent *keyevt = static_cast<QKeyEvent*>(event);
        int key = keyevt->key();
        Qt::KeyboardModifiers mod = keyevt->modifiers();
        switch(key)
        {
        case Qt::Key_Up: if(obj == ui->lineEdit) { browseHistory(-1); return true; } break;
        case Qt::Key_Down: if(obj == ui->lineEdit) { browseHistory(1); return true; } break;
        case Qt::Key_PageUp: /* pass paging keys to messages widget */
        case Qt::Key_PageDown:
            if(obj == ui->lineEdit)
            {
                QApplication::postEvent(ui->messagesWidget, new QKeyEvent(*keyevt));
                return true;
            }
            break;
        default:
            // Typing in messages widget brings focus to line edit, and redirects key there
            // Exclude most combinations and keys that emit no text, except paste shortcuts
            if(obj == ui->messagesWidget && (
                  (!mod && !keyevt->text().isEmpty() && key != Qt::Key_Tab) ||
                  ((mod & Qt::ControlModifier) && key == Qt::Key_V) ||
                  ((mod & Qt::ShiftModifier) && key == Qt::Key_Insert)))
            {
                ui->lineEdit->setFocus();
                QApplication::postEvent(ui->lineEdit, new QKeyEvent(*keyevt));
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void AiConsolePage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
    }
}

static QString categoryClass(int category)
{
    switch(category)
    {
    case AiConsolePage::CMD_REQUEST:  return "cmd-request"; break;
    case AiConsolePage::CMD_REPLY:    return "cmd-reply"; break;
    case AiConsolePage::CMD_ERROR:    return "cmd-error"; break;
    default:                       return "misc";
    }
}

void AiConsolePage::entryFocus()
{
    ui->lineEdit->setFocus();
}

void AiConsolePage::clear()
{
    ui->messagesWidget->clear();
    ui->lineEdit->clear();
    ui->lineEdit->setFocus();

    // Add smoothly scaled icon images.
    // (when using width/height on an img, Qt uses nearest instead of linear interpolation)
    for(int i=0; ICON_MAPPING[i].url; ++i)
    {
        ui->messagesWidget->document()->addResource(
                    QTextDocument::ImageResource,
                    QUrl(ICON_MAPPING[i].url),
                    QImage(ICON_MAPPING[i].source).scaled(ICON_SIZE, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }

    // Set default style sheet
    ui->messagesWidget->document()->setDefaultStyleSheet(
                "table { }"
                "td.time { color: #eeeeee; padding-top: 3px; } "
                "td.message { font-family: Monospace; font-size: 12px; } "
                "td.cmd-request { color: #00bb00; } "
                "td.cmd-error { color: #ff6600; } "
                "b { color: #cccccc; } "
                );
message(CMD_REPLY, tr("<pre>     _______.        ___         .______     __      _______   .__   __.      ______    _______    <br>") +
        tr("    /       |       /   \\        |   _  \\   |  |    |   ____|  |  \\ |  |     /      |  |   ____|    <br>") +
        tr("   |   (----`      /  ^  \\       |  |_)  |  |  |    |  |__     |   \\|  |    |  ,----'  |  |__      <br>") +
        tr("    \\   \\         /  /_\\  \\      |   ___/   |  |    |   __|    |  . `  |    |  |       |   __|     <br>") +
        tr(".----)   |    __ /  _____  \\   __|  |     __|  |  __|  |____ __|  |\\   |  __|  `----.__|  |____ __ <br>") +
        tr("|_______/    (__)__/     \\__\\ (__) _|    (__)__| (__)_______(__)__| \\__| (__)\\______(__)_______(__)<br>") +
tr("                                                                                                   </pre><br>") +
        tr("<b>Sapient Artificial Primary Intelligence Extensible Neural Cognitive Engine</b><br><br>") +
        tr("Note that this console is a placeholder and will be replaced in the next update.<br><br>") +
        tr("Use up and down arrows to navigate history, and <b>Ctrl-L</b> or clear to clear screen.") + "<br>" +
        tr("Type <b>help</b> for an overview of available commands."), true);

}

void AiConsolePage::message(int category, const QString &message, bool html)
{
    QTime time = QTime::currentTime();
    QString timeString = time.toString();
    QString out;
    out += "<table><tr><td class=\"time\" width=\"65\">" + timeString + "</td>";
    out += "<td class=\"icon\" width=\"32\"><img src=\"" + categoryClass(category) + "\"></td>";
    out += "<td class=\"message " + categoryClass(category) + "\" valign=\"middle\">";
    if(html)
        out += message;
    else
        out += GUIUtil::HtmlEscape(message, true);
    out += "</td></tr></table>";
    ui->messagesWidget->append(out);
}

void AiConsolePage::on_lineEdit_returnPressed()
{
    QString cmd = ui->lineEdit->text();
    ui->lineEdit->clear();

    if(!cmd.isEmpty())
    {
        if(cmd.toStdString() == "clear")
    {
        message(CMD_REQUEST, cmd);
        clear();
        // Remove command, if already in history
            history.removeOne(cmd);
            // Append command to history
            history.append(cmd);
            // Enforce maximum history size
            while(history.size() > CONSOLE_HISTORY)
                history.removeFirst();
            // Set pointer to end of history
            historyPtr = history.size();
            // Scroll console view to end
            scrollToEnd();
    }
    else
    {
            message(CMD_REQUEST, cmd);
            emit cmdRequest(cmd);
            // Remove command, if already in history
            history.removeOne(cmd);
            // Append command to history
            history.append(cmd);
            // Enforce maximum history size
            while(history.size() > CONSOLE_HISTORY)
                history.removeFirst();
            // Set pointer to end of history
            historyPtr = history.size();
            // Scroll console view to end
            scrollToEnd();
    }
    }
}

void AiConsolePage::browseHistory(int offset)
{
    historyPtr += offset;
    if(historyPtr < 0)
        historyPtr = 0;
    if(historyPtr > history.size())
        historyPtr = history.size();
    QString cmd;
    if(historyPtr < history.size())
        cmd = history.at(historyPtr);
    ui->lineEdit->setText(cmd);
}

void AiConsolePage::startExecutor()
{
    QThread* thread = new QThread;
    RPCExecutor *executor = new RPCExecutor();
    executor->moveToThread(thread);

    // Notify executor when thread started (in executor thread)
    connect(thread, SIGNAL(started()), executor, SLOT(start()));
    // Replies from executor object must go to this object
    connect(executor, SIGNAL(reply(int,QString)), this, SLOT(message(int,QString)));
    // Requests from this object must go to executor
    connect(this, SIGNAL(cmdRequest(QString)), executor, SLOT(request(QString)));
    // On stopExecutor signal
    // - queue executor for deletion (in execution thread)
    // - quit the Qt event loop in the execution thread
    connect(this, SIGNAL(stopExecutor()), executor, SLOT(deleteLater()));
    connect(this, SIGNAL(stopExecutor()), thread, SLOT(quit()));
    // Queue the thread for deletion (in this thread) when it is finished
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    // Default implementation of QThread::run() simply spins up an event loop in the thread,
    // which is what we want.
    thread->start();
}

void AiConsolePage::scrollToEnd()
{
    QScrollBar *scrollbar = ui->messagesWidget->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}
