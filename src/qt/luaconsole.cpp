#include "luaconsole.h"
#include "ui_luaconsole.h"

#include "guiutil.h"
#include "guiconstants.h"
#include "lua.hpp"
#include "QtLua/String"
#include "QtLua/Value"
#include "QtLua/ValueBase"

#include <iostream>
#include <cstdio>
#include <string>

#include <QTextBlock>
#include <QtGui/QClipboard>
#include <QTime>
#include <QMessageBox>
#include <QThread>
#include <QFileDialog>
#include <QSettings>
#include <QDebug>

#include "plume/plumeapi.h"


using namespace std;

LuaConsole *LuaConsole::m_instance;
static QTime performanceTimer;
CPlumeApi api;


// Qt 4.6 lacks QTextCursor.positionInBlock()
int positionInBlock(const QTextCursor& cursor) {
    return cursor.position() - cursor.block().position();
}

static const struct luaL_Reg printlib [] = {
  {"print", LuaConsole::lua_cmd_print},
  {NULL, NULL} /* end of array */
};

QString prompt = "XAI>>> ";
int promptLength = 7;

LuaConsole::LuaConsole(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LuaConsole),
    prompt("XAI>>> "),
  promptLength(prompt.length()),
  multilineCommand(""),
  commandRing(50), // remember latest 50 commands
  bLuaReadline(false)
{
    ui->setupUi(this);
    setupMenus();

    LuaConsole::m_instance = this;


        L = luaL_newstate();
        //L = lua_open();
        luaL_openlibs( L );

        lua_getglobal(L, "_G");

        luaL_setfuncs(L, printlib, 0);
          lua_pop(L, 1);

        luaInterpreter = new LuaInterpreter(L);

        connect(luaInterpreter, SIGNAL(commandComplete()), this, SLOT(onCommandComplete()));
        connect(luaInterpreter, SIGNAL(incompleteCommand(QString)), this, SLOT(onIncompleteCommand(QString)));
        connect( this, SIGNAL(commandIssued(QString)),
                luaInterpreter, SLOT(interpretLine(QString)) );
        connect( luaInterpreter, SIGNAL(startReadline()),
                this, SLOT(onLuaReadline()) );
        connect( this, SIGNAL(luaReadlineEntered(QString)),
                luaInterpreter, SLOT(finishReadline(QString)) );
        connect(luaInterpreter, SIGNAL(outputSent(QString)), this, SLOT(onOutput(QString)));

#ifndef QT_NO_CLIPBOARD
    connect( QApplication::clipboard(), SIGNAL(dataChanged()),
            this, SLOT(onClipboardDataChanged()) );
#endif
    connect( ui->plainTextEdit, SIGNAL(selectionChanged()),
            this, SLOT(onSelectionChanged()) );

    ui->plainTextEdit->installEventFilter(this);
    ui->plainTextEdit->viewport()->installEventFilter(this); // to get mousePressed
        connect( this, SIGNAL(returnPressed()), this, SLOT(onReturnPressed()) );

        // Don't let cursor leave the editing region.
        connect( ui->plainTextEdit, SIGNAL(cursorPositionChanged ()),
                this, SLOT(onCursorPositionChanged()) );

        ui->plainTextEdit->setWordWrapMode(QTextOption::WrapAnywhere);

    ui->plainTextEdit->appendPlainText(
    tr("     _______         ___         _______    ___     ________    __    __      ______   ________\n") +
    tr("    /       |       /   \\        |   _  \\   |  |    |   ____|  |  \\ |  |     /      |  |   ____|    \n") +
    tr("   |   (----`      /  ^  \\       |  |_)  |  |  |    |  |__     |   \\|  |    |  ,----'  |  |__      \n") +
    tr("    \\   \\         /  /_\\  \\      |   ___/   |  |    |   __|    |  . `  |    |  |       |   __|     \n") +
    tr(".----)   |    __ /  _____  \\   __|  |     __|  |  __|  |____ __|  |\\   |  __|  `----.__|  |____ __ \n") +
    tr("|_______/    (__)__/     \\__\\ (__) _|    (__)__| (__)_______(__)__| \\__| (__)\\______(__)_______(__)\n") +
tr("                                                                                                   \n") +
    tr("\nSapient Artificial Primary Intelligence Extensible Neural Cognitive Engine\n\n") );

    //std::string ver = "Python ";
    //ver += Py_GetVersion();
    //ver += " on ";
    //ver += Py_GetPlatform();
    //ui->plainTextEdit->appendPlainText(ver.c_str());

    //ui->plainTextEdit->appendPlainText(
    //    "Type \"help\", \"copyright\", \"credits\" or "
    //    "\"license\" for more information.");

    ui->plainTextEdit->appendPlainText("\n");

    placeNewPrompt(true);

        // Make cursor about the size of a letter
        int cursorSize = QFontMetrics(ui->plainTextEdit->font()).width("m");
        if (cursorSize > 0)
                ui->plainTextEdit->setCursorWidth(cursorSize);
        else
                ui->plainTextEdit->setCursorWidth(1);

}

void LuaConsole::about()
{
}

void LuaConsole::setPrompt(const QString& newPrompt)
{
    prompt = newPrompt;
    promptLength = prompt.length();
}

void LuaConsole::onOutput(QString msg)
{
    ui->plainTextEdit->moveCursor(QTextCursor::End);
    ui->plainTextEdit->insertPlainText( msg );
    QCoreApplication::processEvents(); // flush text
}

void LuaConsole::onClipboardDataChanged()
{
    // cerr << "Data changed" << endl;
    // emit pasteAvailable(plainTextEdit->canPaste()); // slow
}

void LuaConsole::setupMenus()
{
}

void LuaConsole::runScript() {
    QString fileName =  QFileDialog::getOpenFileName( this,
                tr("Choose Lua script file to run"),
                QDir::currentPath(),
                tr("Lua scripts (*.lua);;AllFiles (*.*)"));
    if ( ! fileName.isNull() ) {
        // Move past prompt
        ui->plainTextEdit->appendPlainText("");
        luaInterpreter->runScriptFile(fileName);
        addRecent(fileName);
    }
}

void LuaConsole::addRecent(const QString& fileName)
{
    // qDebug() << "addRecent() " << fileName;
    QSettings settings;
    QStringList files = settings.value("recentScriptList").toStringList();
    // Perhaps this is already the most recent script
    if ( (files.size() > 0) && (files[0] == fileName) )
        return;
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > recentScripts.size())
        files.removeLast();
    settings.setValue("recentScriptList", files);

    int numRecentFiles = qMin(files.size(), (int)recentScripts.size());

    for (int i = 0; i < numRecentFiles; ++i) {
        recentScripts[i]->setText(files[i]);
        recentScripts[i]->setData(files[i]);
        recentScripts[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < recentScripts.size(); ++j)
        recentScripts[j]->setVisible(false);

    // TODO: menuRun_recent->setVisible(numRecentFiles > 0);
}

void LuaConsole::updateRecent()
{
    QSettings settings;
    QStringList files = settings.value("recentScriptList").toStringList();
    int numRecentFiles = qMin(files.size(), (int)recentScripts.size());

    for (int i = 0; i < numRecentFiles; ++i) {
        recentScripts[i]->setText(files[i]);
        recentScripts[i]->setData(files[i]);
        recentScripts[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < recentScripts.size(); ++j)
        recentScripts[j]->setVisible(false);

    //TODO: menuRun_recent->setEnabled(numRecentFiles > 0);
}

void LuaConsole::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (! action) return;
    QString fileName(action->data().toString());
    if (fileName.isNull()) return;
    // Move past prompt
    ui->plainTextEdit->appendPlainText("");
    luaInterpreter->runScriptFile(fileName);
    addRecent(fileName);
}

void LuaConsole::zoomIn() {
    QFont newFont(ui->plainTextEdit->font());
    int oldSize = newFont.pointSize();
    int newSize = int(1.03 * oldSize + 0.5) + 1;
    newFont.setPointSize(newSize);
    ui->plainTextEdit->setFont(newFont);
    int cursorSize = QFontMetrics(ui->plainTextEdit->font()).width("m");
    if (cursorSize > 0)
            ui->plainTextEdit->setCursorWidth(cursorSize);
    else
            ui->plainTextEdit->setCursorWidth(1);
}

void LuaConsole::zoomOut() {
    QFont newFont(ui->plainTextEdit->font());
    int oldSize = newFont.pointSize();
    int newSize = int(oldSize / 1.03 + 0.5) - 1;
    if (newSize < 1) newSize = 1;
    newFont.setPointSize(newSize);
    ui->plainTextEdit->setFont(newFont);
    int cursorSize = QFontMetrics(ui->plainTextEdit->font()).width("m");
    if (cursorSize > 0)
            ui->plainTextEdit->setCursorWidth(cursorSize);
    else
            ui->plainTextEdit->setCursorWidth(1);
}

bool LuaConsole::eventFilter(QObject * watched, QEvent * event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);


            if (keyEvent->modifiers() & Qt::AltModifier) {}
            else if (keyEvent->modifiers() & Qt::MetaModifier) {}
            else
            { // non-Ctrl keystrokes
            switch(keyEvent->key())
            {
            // Use up and down arrows for history
            case Qt::Key_Up:
                showPreviousCommand();
                return true;
            case Qt::Key_Down:
                showNextCommand();
                return true;
            // Prevent left arrow from leaving editing area
            case Qt::Key_Left:
            case Qt::Key_Backspace:
                                // Qt 4.6 lacks QTextCursor.positionInBlock
                if((ui->plainTextEdit->textCursor().positionInBlock() == promptLength)
                        && cursorIsInEditingRegion(ui->plainTextEdit->textCursor()) )
                {
                        return true; // no moving left into prompt with arrow key
                }
                break;
            // Trigger command execution with <Return>
            case Qt::Key_Return:
            case Qt::Key_Enter:
                emit returnPressed();
                return true; // Consume event.  We will take care of inserting the newline.
            }
            // If this is a printing character, make sure the editing console is activated
            if (keyEvent->text().length() > 0)
            {
                if ( ! cursorIsInEditingRegion(ui->plainTextEdit->textCursor()) )
                    ui->plainTextEdit->setTextCursor(latestGoodCursorPosition);
            }
            }
    }

    return QWidget::eventFilter(watched, event);
}

void LuaConsole::showPreviousCommand() {
    QString provisionalCommand = getCurrentCommand();
    QString previousCommand =
            commandRing.getPreviousCommand(provisionalCommand);
    if (previousCommand != provisionalCommand)
        replaceCurrentCommand(previousCommand);
}
void LuaConsole::showNextCommand() {
    QString provisionalCommand = getCurrentCommand();
    QString nextCommand =
            commandRing.getNextCommand(provisionalCommand);
    if (nextCommand != provisionalCommand)
        replaceCurrentCommand(nextCommand);
}

void LuaConsole::executeCommand(const QString& command)
{
    ui->plainTextEdit->moveCursor(QTextCursor::End);
    currentCommandStartPosition =
            ui->plainTextEdit->textCursor().position();
    ui->plainTextEdit->insertPlainText(command);
    onReturnPressed();
}

void LuaConsole::onReturnPressed()
{
    // Clear undo/redo buffer, we don't want prompts and output in there.
    ui->plainTextEdit->setUndoRedoEnabled(false);
    // Scroll down after command, if and only if bottom is visible now.
    bool endIsVisible = ui->plainTextEdit->document()->lastBlock().isVisible();

    QString command = getCurrentCommand();

    // No command line history for readline.  I guess.
    // (this could be changed)
    if (!bLuaReadline)
    {
        commandRing.addHistory(command);
        if (multilineCommand.length() > 0) {
           // multi-line command can only be ended with a blank line.
           if (command.length() == 0)
               command = multilineCommand; // execute it now
           else {
               multilineCommand = multilineCommand + command + "\n";
               command = ""; // skip execution until next time
           }
        }
    }

    // Add carriage return, so output will appear on subsequent line.
    // (It would be too late if we waited for plainTextEdit
    //  to process the <Return>)
    ui->plainTextEdit->moveCursor(QTextCursor::End);
    ui->plainTextEdit->appendPlainText("");  // We consumed the key event, so we have to add the newline.

    if (bLuaReadline) // fetching user input to lua script
    {
        bLuaReadline = false;
        setPrompt("XAI>>> "); // restore normal prompt
        emit luaReadlineEntered(command + "\n");
    }
    else { // regular lua command entry
        if (command.length() > 0)
            emit commandIssued(command);
        else
            placeNewPrompt(endIsVisible);
    }
}

void LuaConsole::onCommandComplete()
{
    multilineCommand = "";
    bool endIsVisible = ui->plainTextEdit->document()->lastBlock().isVisible();
    setPrompt("XAI>>> ");
    placeNewPrompt(endIsVisible);
}

void LuaConsole::onIncompleteCommand(QString partialCmd)
{
    multilineCommand = partialCmd + "\n";
    bool endIsVisible = ui->plainTextEdit->document()->lastBlock().isVisible();
    setPrompt("... ");
    placeNewPrompt(endIsVisible);
}

void LuaConsole::onLuaReadline()
{
    bLuaReadline = true;
    multilineCommand = "";
    setPrompt("");
    placeNewPrompt(true);
    // zero-length prompt requires explicit shift to read-write mode
    ui->plainTextEdit->setReadOnly(false);
}

void LuaConsole::onCopyAvailable(bool bCopyAvailable)
{
    if (! bCopyAvailable)
        emit cutAvailable(false);
    else if (cursorIsInEditingRegion(ui->plainTextEdit->textCursor()))
        emit cutAvailable(true);
    else
        emit cutAvailable(false);
}

bool LuaConsole::cursorIsInEditingRegion(const QTextCursor& cursor)
{
    // Want to be to the right of the prompt...
    if (positionInBlock(cursor) < promptLength)
        return false;
    // ... and in the final line.
    if (cursor.blockNumber() != ui->plainTextEdit->blockCount() - 1)
        return false;
    if (cursor.anchor() != cursor.position()) {
        // Anchor might be outside of editing region
        QTextCursor anchorCursor(cursor);
        anchorCursor.setPosition(cursor.anchor());
        if (positionInBlock(anchorCursor) < promptLength)
            return false;
        if (anchorCursor.blockNumber() != ui->plainTextEdit->blockCount() - 1)
            return false;
    }
    return true;
}

void LuaConsole::onSelectionChanged()
{
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    bool bReadOnly = ! cursorIsInEditingRegion(cursor);
    if (bReadOnly != ui->plainTextEdit->isReadOnly())
        ui->plainTextEdit->setReadOnly(bReadOnly);
}

void LuaConsole::onCursorPositionChanged()
{
    // performanceTimer.start();
    // cerr << "Cursor moved" << endl;
    // Don't allow editing outside the editing area.
    QTextCursor currentCursor = ui->plainTextEdit->textCursor();
    bool bReadOnly;

    if (cursorIsInEditingRegion(currentCursor)) {
        // This is a good spot.  Within the editing area
        latestGoodCursorPosition = currentCursor;
        bReadOnly = false;
    }
    else {
        bReadOnly = true;
    }
    if (bReadOnly != ui->plainTextEdit->isReadOnly())
        ui->plainTextEdit->setReadOnly(bReadOnly);

    // cerr << "cursor position elapsed time1 = " << performanceTimer.elapsed() << " ms" << endl;
    if(bReadOnly) {
        emit pasteAvailable(false);
        emit cutAvailable(false);
    }
    else {
        // Performance problem with canPaste() method.
        // plainTextEdit->canPaste(); // slow ~120 ms
        // emit pasteAvailable(plainTextEdit->canPaste()); // slow
        // emit pasteAvailable(!QApplication::clipboard()->text().isEmpty());
        // QApplication::clipboard()->text().isEmpty(); // slow ~ 120 ms
        emit pasteAvailable(true); // whatever...
    }
    // cerr << "cursor position elapsed time2 = " << performanceTimer.elapsed() << " ms" << endl;
}

void LuaConsole::placeNewPrompt(bool bMakeVisible)
{
    ui->plainTextEdit->setUndoRedoEnabled(false); // clear undo/redo buffer
    ui->plainTextEdit->moveCursor(QTextCursor::End);
    ui->plainTextEdit->insertPlainText(prompt);
    ui->plainTextEdit->moveCursor(QTextCursor::End);
    if (bMakeVisible) {
        ui->plainTextEdit->ensureCursorVisible();
        // cerr << "make visible" << endl;
    }
    latestGoodCursorPosition = ui->plainTextEdit->textCursor();
    currentCommandStartPosition = latestGoodCursorPosition.position();
    // Start undo/redo, just for user typing, not for computer output
    ui->plainTextEdit->setUndoRedoEnabled(true);
}

QString LuaConsole::getCurrentCommand()
{
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    cursor.setPosition(currentCommandStartPosition,
            QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString command = cursor.selectedText();
    cursor.clearSelection();
    return command;
}

//Replace current command with a new one
void LuaConsole::replaceCurrentCommand(const QString& newCommand)
{
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    cursor.setPosition(currentCommandStartPosition,
            QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End,
            QTextCursor::KeepAnchor);
    cursor.insertText(newCommand);
}

LuaConsole::~LuaConsole()
{
    lua_close(L);
    delete ui;
}




LuaOutputRedirector::LuaOutputRedirector(LuaInterpreter *p_interpreter)
    : interpreter(p_interpreter)
{}

void LuaOutputRedirector::write( std::string const& str )
{
    interpreter->onOutput(QString(str.c_str()));
}

void LuaOutputRedirector::write_wide( std::wstring const& ws )
{
    std::string temp;
    std::copy(ws.begin(), ws.end(), std::back_inserter(temp));
    QString s(temp.c_str());
    interpreter->onOutput(s);
}

LuaInputRedirector::LuaInputRedirector(LuaInterpreter *p_interpreter)
    : interpreter(p_interpreter)
{}

// TODO - this is a hack that does not actually get user input
std::string LuaInputRedirector::readline()
{
    return interpreter->readline();
    // cerr << "readline" << endl;
    // return "\n"; // TODO this is a hack to avoid hanging during input.
}

std::string LuaInterpreter::readline()
{
    emit startReadline();
    readlineLoop.exec(); // block until readline
    return readlineString.toStdString();
}

int LuaInterpreter::insertOutputFromLua( lua_State *L )
{
    // Retreive the current interpreter for current lua state.
        LuaInterpreter *interpreter;

        lua_pushlightuserdata( L, (void *)&LuaRegistryGUID );
        lua_gettable( L, LUA_REGISTRYINDEX );
        interpreter = static_cast<LuaInterpreter *>(lua_touserdata( L, -1 ));

        if( interpreter )
            interpreter->onOutput(QString::fromStdString(lua_tostring( L, -2 )));

        lua_settop( L, 0 );
        return 0;
}

LuaInterpreter::LuaInterpreter(lua_State *L)
    : mL(L), QObject(NULL),
      stdinRedirector(this),
      stdoutRedirector(this),
      stderrRedirector(this)
{
    lua_pushlightuserdata( mL, (void *)&LuaRegistryGUID );
    lua_pushlightuserdata( mL, this );
    lua_settable( mL, LUA_REGISTRYINDEX );

    lua_register( mL, "interpreterOutput", &LuaInterpreter::insertOutputFromLua );

    //apply_embedded_dynamic_python_hack();
    try {
        //PyImport_AppendInittab( "plumeapi", &initplumeapi );
        //Py_Initialize();

        // Using python Tkinter GUI requires that sys.argv be populated
        //const char *argv[1] = {"python"};
        //PySys_SetArgv(1, const_cast<char**>(argv));

        //main_module = bp::object((
        //  bp::handle<>(bp::borrowed(PyImport_AddModule("__main__")))));
        //main_namespace = main_module.attr("__dict__");

        // Connect python stdout/stderr to output to GUI
        // Adapted from
        //   http://onegazhang.spaces.live.com/blog/cns!D5E642BC862BA286!727.entry
        //main_namespace["PythonOutputRedirector"] =
        //    bp::class_<PythonOutputRedirector>(
        //            "PythonOutputRedirector", bp::init<>())
        //        .def("write", &PythonOutputRedirector::write)
        //        .def("write", &PythonOutputRedirector::write_wide)
        //        ;
        //main_namespace["PythonInputRedirector"] =
         //   bp::class_<PythonInputRedirector>(
         //           "PythonInputRedirector", bp::init<>())
         //       .def("readline", &PythonInputRedirector::readline)
        //        ;

        //bp::object cpp_module( (bp::handle<>(PyImport_ImportModule("plumeapi"))) );
       // main_namespace["plumeapi"] = cpp_module;
        //bp::scope(cpp_module).attr("plume") = bp::ptr(&api);

        //bp::import("sys").attr("stdin") = stdinRedirector;
        //bp::import("sys").attr("stdout") = stdoutRedirector;
        //bp::import("sys").attr("stderr") = stderrRedirector;
    }
    catch(std::exception)
    {
    //catch( bp::error_already_set ) {
        //PyErr_Print();
    }
}

LuaInterpreter::~LuaInterpreter() {
    // TODO - using Py_Finalize() may be unsafe with boost::python
    // http://www.boost.org/libs/python/todo.html#pyfinalize-safety
    // http://lists.boost.org/Archives/boost/2006/07/107149.php
    //Py_Finalize();
    //clean_up_embedded_dynamic_python_hack();

    lua_register( mL, "interpreterOutput", NULL );
        lua_pushlightuserdata( mL, (void *)&LuaRegistryGUID );
        lua_pushnil( mL );
        lua_settable( mL, LUA_REGISTRYINDEX );
}

void LuaInterpreter::onOutput(QString msg) {
    emit outputSent(msg);
}

void LuaInterpreter::runScriptFile(QString fileName)
{
    QByteArray fname = fileName.toLocal8Bit();
    FILE *fp = fopen((const char*)fname, "r");
    if (fp) {
        // Set argv to add file directory to os.path, just like regular python does
        char *argv[1];
        argv[0] = const_cast<char*>((const char*)(fname));
        //PySys_SetArgv(1, argv); // python < 2.6.6 does not have PySys_SetArgvEx()

        //PyRun_SimpleFileEx(fp, (const char*)fname, 1); // 1 means close it for me

        // Revert path to what it was before PySys_SetArgv
        //PyRun_SimpleString("import sys; sys.path.pop(0)\n");
    }
    emit commandComplete();
}

void LuaInterpreter::finishReadline(QString line)
{
    // Don't run command if python stdin is waiting for readline input
    readlineString = line;
    readlineLoop.exit();
    return;
}

void LuaInterpreter::interpretLine(QString line)
{
    std::string command0 = line.toStdString();

    // Skip empty lines
    if (command0.length() == 0) {
        emit commandComplete();
        return; // empty command
    }
    size_t firstNonSpacePos = command0.find_first_not_of(" \t\r\n");
    if (firstNonSpacePos == std::string::npos) {
        emit commandComplete();
        return; // all blanks command
    }
    if (command0[firstNonSpacePos] == '#') {
        emit commandComplete();
        return; // comment line
    }
    // Append newline for best parsing of nascent multiline commands.
    std::string command = command0 + "\n";

    try {
        if( luaL_loadstring( mL, command.c_str() ) )
            {
                std::string error( lua_tostring( mL, -1 ) );
                lua_pop( mL, 1 );

                // If the error is not a syntax error cuased by not enough of the
                // statement been yet entered...
                //if( error.substr( error.length()-6, 5 ) != "<eof>" )
                if(error.find("<eof>") == string::npos)
                {
                    //mOutput += error;
                    //mOutput += "\n";
                    //mOutput += LI_PROMPT;
                    //mCurrentStatement.clear();
                    //mState = LI_ERROR;
                    onOutput(QString::fromStdString(error) + "\n");
                    emit commandComplete();
                }
                // Otherwise...
                else
                {
                    // Secondary prompt
                   // mPrompt = LI_PROMPT2;

                   // mState = LI_NEED_MORE_INPUT;
                    emit incompleteCommand(line);
                }
                return;
        }
        else
        {
            if( lua_pcall( mL, 0, LUA_MULTRET, 0 ) )
                    {
                        // The error message (if any) will be added to the output as part
                        // of the stack reporting.
                        lua_gc( mL, LUA_GCCOLLECT, 0 );     // Do a full garbage collection on errors.
                        //mState = LI_ERROR;
                    }
        }

        // First compile the expression without running it.
        /*bp::object compiledCode(bp::handle<>(Py_CompileString(
                command.c_str(),
                "<stdin>",
                Py_single_input)));
        if (! compiledCode.ptr()) {
            // command failed
            emit commandComplete();
            return;
        }

        bp::object result(bp::handle<>( PyEval_EvalCode(
                (PyCodeObject*) compiledCode.ptr(),
                main_namespace.ptr(),
                main_namespace.ptr())));*/
    }
    catch(std::exception e)
    {
        onOutput(e.what());
    }
    /*catch( bp::error_already_set )
    {
        // Distinguish incomplete input from invalid input
        char *msg = NULL;
        PyObject *exc, *val, *obj, *trb;
        if (PyErr_ExceptionMatches(PyExc_SyntaxError))
        {
            PyErr_Fetch (&exc, &val, &trb);        /* clears exception! */
     //       if (PyArg_ParseTuple (val, "sO", &msg, &obj) &&
      //              !strcmp (msg, "unexpected EOF while parsing")) /* E_EOF */
     /*       {
                Py_XDECREF (exc);
                Py_XDECREF (val);
                Py_XDECREF (trb);
                emit incompleteCommand(line);
                return;
            }
            PyErr_Restore (exc, val, trb);
        }

        PyErr_Print();
    }*/

    // Report stack contents
        if ( lua_gettop(mL) > 0)
        {
          lua_getglobal(mL, "print");
          lua_insert(mL, 1);
          lua_pcall(mL, lua_gettop(mL)-1, 0, 0);
        }


        //mPrompt = LI_PROMPT;

        // Clear stack
        lua_settop( mL, 0 );

    emit commandComplete();
    return;
}

QtLua::String to_string_p(lua_State *st, int index, bool quote_string)
{
  switch (lua_type(st, index))
    {
    case LUA_TNONE:
      return "(none)";

    case LUA_TNIL:
      return "(nil)";

    case LUA_TBOOLEAN: {
      QtLua::String res(lua_toboolean(st, index) ? "true" : "false");
      return res;
    }

    case LUA_TNUMBER: {
      QtLua::String res;
      res.setNum(lua_tonumber(st, index));
      return res;
    }

    case LUA_TSTRING:
      if (quote_string)
    return QtLua::String("\"") + lua_tostring(st, index) + "\"";
      else
    return QtLua::String(lua_tostring(st, index));

    case LUA_TUSERDATA: {
      try {
          /*
    QtLua::UserData::ptr ud = QtLua::UserData::get_ud(st, index);
    QtLua::String res(ud.valid() ? ud->get_value_str() : ud->UserData::get_value_str());
    return res;*/
          //TODO
      } catch (const QtLua::String &e) {
    // goto default
      }
    }

    default: {
      QtLua::String res;
      res.setNum((qulonglong)lua_topointer(st, index), 16);
      return QtLua::String("(%:%)").arg(lua_typename(NULL, lua_type(st, index))).arg(res);
    }

    }
}

int LuaConsole::lua_cmd_print(lua_State *st)
{

  try {
    for (int i = 1; i <= lua_gettop(st); i++)
      {
    QtLua::String s = to_string_p(st, i, true);



            LuaConsole::m_instance->onOutput(s + "\n");

      }

  } catch (QtLua::String &e) {

    luaL_error(st, "%s", e.constData());
  }


  return 0;
}
