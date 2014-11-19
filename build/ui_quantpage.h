/********************************************************************************
** Form generated from reading UI file 'quantpage.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QUANTPAGE_H
#define UI_QUANTPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_QuantPage
{
public:
    QHBoxLayout *horizontalLayout;
    QHBoxLayout *horizontalLayout_2;
    QVBoxLayout *verticalLayout_2;
    QCustomPlot *depthPlot;
    QCustomPlot *customPlot;
    QCustomPlot *volumePlot;
    QVBoxLayout *verticalLayout;
    QTableWidget *askTableWidget;
    QLabel *lastPriceLabel;
    QLabel *trexLastPriceLabel;
    QTableWidget *bidTableWidget;

    void setupUi(QWidget *QuantPage)
    {
        if (QuantPage->objectName().isEmpty())
            QuantPage->setObjectName(QStringLiteral("QuantPage"));
        QuantPage->resize(573, 458);
        QuantPage->setStyleSheet(QStringLiteral("#QuantPage { background-color: #000000; color: #ffffff; } QToolTip { color: #000000; }"));
        horizontalLayout = new QHBoxLayout(QuantPage);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        depthPlot = new QCustomPlot(QuantPage);
        depthPlot->setObjectName(QStringLiteral("depthPlot"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(depthPlot->sizePolicy().hasHeightForWidth());
        depthPlot->setSizePolicy(sizePolicy);
        depthPlot->setMinimumSize(QSize(300, 0));
        depthPlot->setMaximumSize(QSize(16777215, 150));
        depthPlot->setStyleSheet(QStringLiteral("#depthPlot { background-color: #202020; }"));

        verticalLayout_2->addWidget(depthPlot);

        customPlot = new QCustomPlot(QuantPage);
        customPlot->setObjectName(QStringLiteral("customPlot"));
        sizePolicy.setHeightForWidth(customPlot->sizePolicy().hasHeightForWidth());
        customPlot->setSizePolicy(sizePolicy);
        customPlot->setMinimumSize(QSize(300, 0));
        customPlot->setStyleSheet(QStringLiteral("#customPlot { background-color: #202020; }"));

        verticalLayout_2->addWidget(customPlot);

        volumePlot = new QCustomPlot(QuantPage);
        volumePlot->setObjectName(QStringLiteral("volumePlot"));
        sizePolicy.setHeightForWidth(volumePlot->sizePolicy().hasHeightForWidth());
        volumePlot->setSizePolicy(sizePolicy);
        volumePlot->setMinimumSize(QSize(300, 0));
        volumePlot->setMaximumSize(QSize(16777215, 150));
        volumePlot->setStyleSheet(QStringLiteral("#volumePlot { background-color: #202020; }"));

        verticalLayout_2->addWidget(volumePlot);


        horizontalLayout_2->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        askTableWidget = new QTableWidget(QuantPage);
        if (askTableWidget->columnCount() < 2)
            askTableWidget->setColumnCount(2);
        askTableWidget->setObjectName(QStringLiteral("askTableWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(askTableWidget->sizePolicy().hasHeightForWidth());
        askTableWidget->setSizePolicy(sizePolicy1);
        askTableWidget->setMinimumSize(QSize(200, 0));
        askTableWidget->setMaximumSize(QSize(200, 16777215));
        askTableWidget->setStyleSheet(QStringLiteral("background-color: #000000; color: #cccccc;"));
        askTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        askTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
        askTableWidget->setShowGrid(false);
        askTableWidget->setColumnCount(2);
        askTableWidget->horizontalHeader()->setDefaultSectionSize(90);

        verticalLayout->addWidget(askTableWidget);

        lastPriceLabel = new QLabel(QuantPage);
        lastPriceLabel->setObjectName(QStringLiteral("lastPriceLabel"));
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        font.setWeight(75);
        lastPriceLabel->setFont(font);
        lastPriceLabel->setStyleSheet(QStringLiteral("background-color: #000000; color: #00FF00;"));
        lastPriceLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(lastPriceLabel);

        trexLastPriceLabel = new QLabel(QuantPage);
        trexLastPriceLabel->setObjectName(QStringLiteral("trexLastPriceLabel"));
        trexLastPriceLabel->setFont(font);
        trexLastPriceLabel->setStyleSheet(QStringLiteral("background-color: #000000; color: #00FF00;"));
        trexLastPriceLabel->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(trexLastPriceLabel);

        bidTableWidget = new QTableWidget(QuantPage);
        if (bidTableWidget->columnCount() < 2)
            bidTableWidget->setColumnCount(2);
        bidTableWidget->setObjectName(QStringLiteral("bidTableWidget"));
        sizePolicy1.setHeightForWidth(bidTableWidget->sizePolicy().hasHeightForWidth());
        bidTableWidget->setSizePolicy(sizePolicy1);
        bidTableWidget->setMinimumSize(QSize(200, 0));
        bidTableWidget->setMaximumSize(QSize(200, 16777215));
        bidTableWidget->setStyleSheet(QStringLiteral("background-color: #000000; color: #cccccc;"));
        bidTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        bidTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
        bidTableWidget->setShowGrid(false);
        bidTableWidget->setColumnCount(2);
        bidTableWidget->horizontalHeader()->setDefaultSectionSize(90);

        verticalLayout->addWidget(bidTableWidget);


        horizontalLayout_2->addLayout(verticalLayout);


        horizontalLayout->addLayout(horizontalLayout_2);


        retranslateUi(QuantPage);

        QMetaObject::connectSlotsByName(QuantPage);
    } // setupUi

    void retranslateUi(QWidget *QuantPage)
    {
        QuantPage->setWindowTitle(QApplication::translate("QuantPage", "Form", 0));
        lastPriceLabel->setText(QApplication::translate("QuantPage", "E: 0.00300000", 0));
        trexLastPriceLabel->setText(QApplication::translate("QuantPage", "B: 0.00200000", 0));
    } // retranslateUi

};

namespace Ui {
    class QuantPage: public Ui_QuantPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QUANTPAGE_H
