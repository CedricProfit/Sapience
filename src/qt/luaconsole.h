#ifndef LUACONSOLE_H
#define LUACONSOLE_H

#include <iostream>
#include <cstdio>
#include <string>
#include <lua.hpp>
#include <QWidget>
#include <QTextCursor>
#include <QEventLoop>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
    class LuaConsole;
}

// The address of this int in memory is used as a garenteed unique id
// in the lua registry
static const char LuaRegistryGUID = 0;

class LuaInterpreter;

class LuaOutputRedirector
{
public:
    LuaOutputRedirector(LuaInterpreter *interpreter = NULL);
    void write( std::string const& str );
    void write_wide(std::wstring const& str);

private:
    LuaInterpreter *interpreter;
};

class LuaInputRedirector
{
public:
    LuaInputRedirector(LuaInterpreter *interpreter = NULL);
    std::string readline( );

private:
    LuaInterpreter *interpreter;
};

class LuaInterpreter : public QObject
{
    Q_OBJECT

public:
    friend class LuaOutputRedirector;
    friend class LuaInputRedirector;

    LuaInterpreter(lua_State *L);
    virtual ~LuaInterpreter();
    std::string readline();
    // Callback for lua to provide output.
    static int insertOutputFromLua( lua_State *L );

signals:
    void startReadline();
    void outputSent(QString msg);
    void commandComplete();
    void incompleteCommand(QString partialCmd);

public slots:
    // run lua command
    void interpretLine(QString line);
    // accept input from sys.stdin.readline() command
    void finishReadline(QString line);
    void runScriptFile(QString fileName);

private:
    lua_State *mL;
    void onOutput(QString msg);

    QEventLoop readlineLoop;
    QString readlineString;

    //boost::python::object main_module;
    //boost::python::object main_namespace;
    LuaInputRedirector stdinRedirector;
    LuaOutputRedirector stdoutRedirector;
    LuaOutputRedirector stderrRedirector;
};


class CommandRing
{
public:
    CommandRing(int ringSize)
        : commands(ringSize),
          newestCommandIndex(-1),
          oldestCommandIndex(-1),
          currentCommandIndex(-1),
          storedProvisionalCommand("")
    {}

    bool addHistory(const QString& command) {
        if (command.length() == 0)
            return false; // Don't store empty commands
        if (newestCommandIndex == -1) {
            // This is the first command ever
            newestCommandIndex = 0;
            oldestCommandIndex = 0;
            currentCommandIndex = -1;
            commands[newestCommandIndex] = command;
            return true;
        }
        QString previousCommand = commands[newestCommandIndex];
        if (previousCommand == command) {
            currentCommandIndex = -1;
            return false; // Don't store repeated commands
    }

        increment(newestCommandIndex);
        if (oldestCommandIndex == newestCommandIndex)
            increment(oldestCommandIndex);
        currentCommandIndex = -1; // one past latest
        commands[newestCommandIndex] = command;
        return true;
    }

    QString getNextCommand(const QString& provisionalCommand)
    {
        if (newestCommandIndex == -1) // no commands yet
            return provisionalCommand;
        if (currentCommandIndex == -1) // one past newest
            return provisionalCommand;
        if (currentCommandIndex == newestCommandIndex) {
            currentCommandIndex = -1;
            return storedProvisionalCommand;
        }
        increment(currentCommandIndex);
        return commands[currentCommandIndex];
    }

    QString getPreviousCommand(const QString& provisionalCommand)
    {
        if (newestCommandIndex == -1) // no commands yet
            return provisionalCommand;
        if (currentCommandIndex == -1) { // one past newest
            currentCommandIndex = newestCommandIndex;
            storedProvisionalCommand = provisionalCommand;
            return commands[currentCommandIndex];
        }
        // Sorry, you cannot get the very oldest command.
        if (currentCommandIndex == oldestCommandIndex)
            return provisionalCommand;
        decrement(currentCommandIndex);
        return commands[currentCommandIndex];
    }

private:
    void increment(int& val) { // circular increment
        val = (val+1) % commands.size();
    }

    void decrement(int& val) { // circular decrement
        val = (val+commands.size()-1) % commands.size();
    }

    std::vector<QString> commands;
    int newestCommandIndex;
    int oldestCommandIndex;
    int currentCommandIndex;
    QString storedProvisionalCommand;
};


class LuaConsole : public QWidget
{
    Q_OBJECT

public:
    explicit LuaConsole(QWidget *parent = 0);
    ~LuaConsole();
    void executeCommand(const QString& command);
    bool eventFilter(QObject *watched, QEvent *event);
    LuaInterpreter *luaInterpreter;
    static int lua_cmd_print(lua_State *st);

public slots:
    void runScript();

signals:
    void returnPressed();
    void pasteAvailable(bool);
    void cutAvailable(bool);
    void commandIssued(QString);
    void luaReadlineEntered(QString);

protected:
    lua_State *L;
    static LuaConsole *m_instance;

private:
    Ui::LuaConsole *ui;
    //boost::python::object main_module;
    //boost::python::object main_namespace;
    void setupMenus();
    QString getCurrentCommand();
    void placeNewPrompt(bool bMakeVisible=false);
    void setPrompt(const QString& newPrompt);
    bool cursorIsInEditingRegion(const QTextCursor& cursor);
    void showPreviousCommand();
    void showNextCommand();
    void replaceCurrentCommand(const QString& newCommand);
    void addRecent(const QString& fileName);
    void updateRecent();

    bool bLuaReadline;
    QString prompt;
    int promptLength;
    QTextCursor latestGoodCursorPosition;
    int currentCommandStartPosition;
    QString multilineCommand;
    CommandRing commandRing;

    // static const int maxRecentScripts = 10;
    // QAction* recentScripts[maxRecentScripts];
    //c_array<QAction*, 10> recentScripts;
    std::vector<QAction*> recentScripts;

private slots:
    void onReturnPressed();
    void onCursorPositionChanged();
    void onClipboardDataChanged();
    void onSelectionChanged();
    void onCopyAvailable(bool);
    void onCommandComplete();
    void onIncompleteCommand(QString partialCmd);
    void onLuaReadline();
    void onOutput(QString msg);
    void about();
    void zoomIn();
    void zoomOut();
    void openRecentFile();
};






#endif // LUACONSOLE_H
