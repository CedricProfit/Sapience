/********************************************************************************
** Form generated from reading UI file 'profitexplorerpage.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROFITEXPLORERPAGE_H
#define UI_PROFITEXPLORERPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_ProfitExplorerPage
{
public:
    QHBoxLayout *horizontalLayout;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *checkBox;
    QCheckBox *networkCheckBox;
    QLabel *label;
    QSpinBox *spinBox;
    QPushButton *recomputeButton;
    QCustomPlot *difficultyPlot;
    QCustomPlot *customPlot;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QLabel *stakingLabel;
    QLabel *label_4;
    QLabel *difficultyLabel;
    QLabel *label_6;
    QLabel *weightLabel;
    QLabel *label_8;
    QLabel *netWeightLabel;
    QLabel *label_10;
    QLabel *timeToStakeLabel;
    QCustomPlot *velocityPlot;

    void setupUi(QWidget *ProfitExplorerPage)
    {
        if (ProfitExplorerPage->objectName().isEmpty())
            ProfitExplorerPage->setObjectName(QStringLiteral("ProfitExplorerPage"));
        ProfitExplorerPage->resize(573, 342);
        ProfitExplorerPage->setStyleSheet(QStringLiteral(""));
        horizontalLayout = new QHBoxLayout(ProfitExplorerPage);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setSizeConstraint(QLayout::SetDefaultConstraint);
        checkBox = new QCheckBox(ProfitExplorerPage);
        checkBox->setObjectName(QStringLiteral("checkBox"));
        checkBox->setStyleSheet(QStringLiteral("#checkBox { color: #4CFF00; }"));
        checkBox->setChecked(true);

        horizontalLayout_2->addWidget(checkBox);

        networkCheckBox = new QCheckBox(ProfitExplorerPage);
        networkCheckBox->setObjectName(QStringLiteral("networkCheckBox"));
        networkCheckBox->setStyleSheet(QStringLiteral("color: #cecece;"));
        networkCheckBox->setChecked(true);

        horizontalLayout_2->addWidget(networkCheckBox);

        label = new QLabel(ProfitExplorerPage);
        label->setObjectName(QStringLiteral("label"));
        label->setStyleSheet(QStringLiteral("#label { color: #ffffff; }"));

        horizontalLayout_2->addWidget(label);

        spinBox = new QSpinBox(ProfitExplorerPage);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setMinimum(10);
        spinBox->setMaximum(100000);
        spinBox->setSingleStep(100);
        spinBox->setValue(1000);

        horizontalLayout_2->addWidget(spinBox);

        recomputeButton = new QPushButton(ProfitExplorerPage);
        recomputeButton->setObjectName(QStringLiteral("recomputeButton"));

        horizontalLayout_2->addWidget(recomputeButton);


        verticalLayout_3->addLayout(horizontalLayout_2);

        difficultyPlot = new QCustomPlot(ProfitExplorerPage);
        difficultyPlot->setObjectName(QStringLiteral("difficultyPlot"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(difficultyPlot->sizePolicy().hasHeightForWidth());
        difficultyPlot->setSizePolicy(sizePolicy);
        difficultyPlot->setMinimumSize(QSize(300, 100));
        difficultyPlot->setMaximumSize(QSize(16777215, 150));
        difficultyPlot->setStyleSheet(QStringLiteral("#difficultyPlot { background-color: #202020; }"));

        verticalLayout_3->addWidget(difficultyPlot);

        customPlot = new QCustomPlot(ProfitExplorerPage);
        customPlot->setObjectName(QStringLiteral("customPlot"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(customPlot->sizePolicy().hasHeightForWidth());
        customPlot->setSizePolicy(sizePolicy1);
        customPlot->setMinimumSize(QSize(300, 0));
        customPlot->setStyleSheet(QStringLiteral("#customPlot { background-color: #202020; }"));

        verticalLayout_3->addWidget(customPlot);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        label_2 = new QLabel(ProfitExplorerPage);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setStyleSheet(QStringLiteral("color: #cccccc; background-color: #000000;"));

        horizontalLayout_3->addWidget(label_2);

        stakingLabel = new QLabel(ProfitExplorerPage);
        stakingLabel->setObjectName(QStringLiteral("stakingLabel"));
        stakingLabel->setStyleSheet(QStringLiteral("color: #4CFF00;background-color:#000000;"));
        stakingLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(stakingLabel);

        label_4 = new QLabel(ProfitExplorerPage);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setStyleSheet(QStringLiteral("color: #cccccc; background-color: #000000;"));

        horizontalLayout_3->addWidget(label_4);

        difficultyLabel = new QLabel(ProfitExplorerPage);
        difficultyLabel->setObjectName(QStringLiteral("difficultyLabel"));
        difficultyLabel->setStyleSheet(QStringLiteral("color: #4CFF00;background-color:#000000;"));
        difficultyLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(difficultyLabel);

        label_6 = new QLabel(ProfitExplorerPage);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setStyleSheet(QStringLiteral("color: #cccccc; background-color: #000000;"));

        horizontalLayout_3->addWidget(label_6);

        weightLabel = new QLabel(ProfitExplorerPage);
        weightLabel->setObjectName(QStringLiteral("weightLabel"));
        weightLabel->setStyleSheet(QStringLiteral("color: #4CFF00;background-color:#000000;"));
        weightLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(weightLabel);

        label_8 = new QLabel(ProfitExplorerPage);
        label_8->setObjectName(QStringLiteral("label_8"));
        label_8->setStyleSheet(QStringLiteral("color: #cccccc; background-color: #000000;"));

        horizontalLayout_3->addWidget(label_8);

        netWeightLabel = new QLabel(ProfitExplorerPage);
        netWeightLabel->setObjectName(QStringLiteral("netWeightLabel"));
        netWeightLabel->setStyleSheet(QStringLiteral("color: #4CFF00;background-color:#000000;"));
        netWeightLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(netWeightLabel);

        label_10 = new QLabel(ProfitExplorerPage);
        label_10->setObjectName(QStringLiteral("label_10"));
        label_10->setStyleSheet(QStringLiteral("color: #cccccc; background-color: #000000;"));

        horizontalLayout_3->addWidget(label_10);

        timeToStakeLabel = new QLabel(ProfitExplorerPage);
        timeToStakeLabel->setObjectName(QStringLiteral("timeToStakeLabel"));
        timeToStakeLabel->setStyleSheet(QStringLiteral("color: #4CFF00;background-color:#000000;"));
        timeToStakeLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_3->addWidget(timeToStakeLabel);


        verticalLayout_3->addLayout(horizontalLayout_3);

        velocityPlot = new QCustomPlot(ProfitExplorerPage);
        velocityPlot->setObjectName(QStringLiteral("velocityPlot"));
        sizePolicy.setHeightForWidth(velocityPlot->sizePolicy().hasHeightForWidth());
        velocityPlot->setSizePolicy(sizePolicy);
        velocityPlot->setMinimumSize(QSize(300, 100));
        velocityPlot->setMaximumSize(QSize(16777215, 150));
        velocityPlot->setStyleSheet(QStringLiteral("#velocityPlot { background-color: #202020; }"));

        verticalLayout_3->addWidget(velocityPlot);


        horizontalLayout->addLayout(verticalLayout_3);

        horizontalLayout->setStretch(0, 1);

        retranslateUi(ProfitExplorerPage);

        QMetaObject::connectSlotsByName(ProfitExplorerPage);
    } // setupUi

    void retranslateUi(QWidget *ProfitExplorerPage)
    {
        ProfitExplorerPage->setWindowTitle(QApplication::translate("ProfitExplorerPage", "Form", 0));
        checkBox->setText(QApplication::translate("ProfitExplorerPage", "Auto Refresh", 0));
        networkCheckBox->setText(QApplication::translate("ProfitExplorerPage", "Show Network", 0));
        label->setText(QApplication::translate("ProfitExplorerPage", "Blocks To Go Back:", 0));
        recomputeButton->setText(QApplication::translate("ProfitExplorerPage", "Recompute", 0));
        label_2->setText(QApplication::translate("ProfitExplorerPage", "<html><head/><body><p>Staking:</p></body></html>", 0));
        stakingLabel->setText(QApplication::translate("ProfitExplorerPage", "Unknown", 0));
        label_4->setText(QApplication::translate("ProfitExplorerPage", "Difficulty:", 0));
        difficultyLabel->setText(QApplication::translate("ProfitExplorerPage", "<html><head/><body><p>0</p></body></html>", 0));
        label_6->setText(QApplication::translate("ProfitExplorerPage", "Weight:", 0));
        weightLabel->setText(QApplication::translate("ProfitExplorerPage", "0", 0));
        label_8->setText(QApplication::translate("ProfitExplorerPage", "Network Weight:", 0));
        netWeightLabel->setText(QApplication::translate("ProfitExplorerPage", "0", 0));
        label_10->setText(QApplication::translate("ProfitExplorerPage", "Time to stake:", 0));
        timeToStakeLabel->setText(QApplication::translate("ProfitExplorerPage", "0", 0));
    } // retranslateUi

};

namespace Ui {
    class ProfitExplorerPage: public Ui_ProfitExplorerPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROFITEXPLORERPAGE_H
