#ifndef OPTIONSPAGE_H
#define OPTIONSPAGE_H

#include <QWidget>
#include "main.h"

namespace Ui {
    class OptionsPage;
}

class OptionsPage : public QWidget
{
    Q_OBJECT

public:
    explicit OptionsPage(QWidget *parent = 0);
    ~OptionsPage();

public slots:

private:
    Ui::OptionsPage *ui;
    
private slots:
    void enableStakingCheckBox_stateChanged(int state);
    void voiceEnabledCheckBox_stateChanged(int state);
    void dataPlumesEnabledCheckBox_stateChanged(int state);
};

#endif // OPTIONSPAGE_H
