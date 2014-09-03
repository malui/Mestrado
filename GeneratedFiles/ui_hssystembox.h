/********************************************************************************
** Form generated from reading UI file 'hssystembox.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HSSYSTEMBOX_H
#define UI_HSSYSTEMBOX_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HsSystemboxClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *HsSystemboxClass)
    {
        if (HsSystemboxClass->objectName().isEmpty())
            HsSystemboxClass->setObjectName(QStringLiteral("HsSystemboxClass"));
        HsSystemboxClass->resize(600, 400);
        menuBar = new QMenuBar(HsSystemboxClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        HsSystemboxClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(HsSystemboxClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        HsSystemboxClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(HsSystemboxClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        HsSystemboxClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(HsSystemboxClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        HsSystemboxClass->setStatusBar(statusBar);

        retranslateUi(HsSystemboxClass);

        QMetaObject::connectSlotsByName(HsSystemboxClass);
    } // setupUi

    void retranslateUi(QMainWindow *HsSystemboxClass)
    {
        HsSystemboxClass->setWindowTitle(QApplication::translate("HsSystemboxClass", "HsSystembox", 0));
    } // retranslateUi

};

namespace Ui {
    class HsSystemboxClass: public Ui_HsSystemboxClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HSSYSTEMBOX_H
