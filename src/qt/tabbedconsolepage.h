#ifndef TABBEDCONSOLEPAGE_H
#define TABBEDCONSOLEPAGE_H

#include <QWidget>

namespace Ui {
    class TabbedConsolePage;
}

class TabbedConsolePage : public QWidget
{
    Q_OBJECT

public:
    explicit TabbedConsolePage(QWidget *parent = 0);
    ~TabbedConsolePage();

public slots:
    void on_tabWidget_currentChanged(int index);
    void on_tabWidget_tabCloseRequested(int index);

private:
    Ui::TabbedConsolePage *ui;

};

#endif // TABBEDCONSOLEPAGE_H
