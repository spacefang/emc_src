/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.5.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_1;
    QTabWidget *mainTabWidget;
    QWidget *senderTab;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *rs485GroupBox_1;
    QVBoxLayout *verticalLayout_3;
    QTableWidget *rs485TableWidget_1;
    QGroupBox *udpGroupBox_1;
    QVBoxLayout *verticalLayout_4;
    QTableWidget *udpTableWidget_1;
    QGroupBox *logGroupBox_1;
    QVBoxLayout *verticalLayout_5;
    QPlainTextEdit *logTextEdit_1;
    QWidget *receiverTab;
    QVBoxLayout *verticalLayout_6;
    QGroupBox *rs485GroupBox_2;
    QVBoxLayout *verticalLayout_7;
    QTableWidget *rs485TableWidget_2;
    QGroupBox *udpGroupBox_2;
    QVBoxLayout *verticalLayout_8;
    QTableWidget *udpTableWidget_2;
    QGroupBox *logGroupBox_2;
    QVBoxLayout *verticalLayout_9;
    QPlainTextEdit *logTextEdit_2;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1400, 800);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_1 = new QVBoxLayout(centralwidget);
        verticalLayout_1->setObjectName("verticalLayout_1");
        mainTabWidget = new QTabWidget(centralwidget);
        mainTabWidget->setObjectName("mainTabWidget");
        senderTab = new QWidget();
        senderTab->setObjectName("senderTab");
        verticalLayout_2 = new QVBoxLayout(senderTab);
        verticalLayout_2->setObjectName("verticalLayout_2");
        rs485GroupBox_1 = new QGroupBox(senderTab);
        rs485GroupBox_1->setObjectName("rs485GroupBox_1");
        verticalLayout_3 = new QVBoxLayout(rs485GroupBox_1);
        verticalLayout_3->setObjectName("verticalLayout_3");
        rs485TableWidget_1 = new QTableWidget(rs485GroupBox_1);
        if (rs485TableWidget_1->columnCount() < 8)
            rs485TableWidget_1->setColumnCount(8);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        rs485TableWidget_1->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        rs485TableWidget_1->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        rs485TableWidget_1->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        rs485TableWidget_1->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        rs485TableWidget_1->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        rs485TableWidget_1->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        rs485TableWidget_1->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        rs485TableWidget_1->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        if (rs485TableWidget_1->rowCount() < 8)
            rs485TableWidget_1->setRowCount(8);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        rs485TableWidget_1->setVerticalHeaderItem(0, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        rs485TableWidget_1->setVerticalHeaderItem(1, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        rs485TableWidget_1->setVerticalHeaderItem(2, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        rs485TableWidget_1->setVerticalHeaderItem(3, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        rs485TableWidget_1->setVerticalHeaderItem(4, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        rs485TableWidget_1->setVerticalHeaderItem(5, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        rs485TableWidget_1->setVerticalHeaderItem(6, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        rs485TableWidget_1->setVerticalHeaderItem(7, __qtablewidgetitem15);
        rs485TableWidget_1->setObjectName("rs485TableWidget_1");
        rs485TableWidget_1->setEditTriggers(QAbstractItemView::NoEditTriggers);
        rs485TableWidget_1->setAlternatingRowColors(true);
        rs485TableWidget_1->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_3->addWidget(rs485TableWidget_1);


        verticalLayout_2->addWidget(rs485GroupBox_1);

        udpGroupBox_1 = new QGroupBox(senderTab);
        udpGroupBox_1->setObjectName("udpGroupBox_1");
        verticalLayout_4 = new QVBoxLayout(udpGroupBox_1);
        verticalLayout_4->setObjectName("verticalLayout_4");
        udpTableWidget_1 = new QTableWidget(udpGroupBox_1);
        if (udpTableWidget_1->columnCount() < 7)
            udpTableWidget_1->setColumnCount(7);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        udpTableWidget_1->setHorizontalHeaderItem(0, __qtablewidgetitem16);
        QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
        udpTableWidget_1->setHorizontalHeaderItem(1, __qtablewidgetitem17);
        QTableWidgetItem *__qtablewidgetitem18 = new QTableWidgetItem();
        udpTableWidget_1->setHorizontalHeaderItem(2, __qtablewidgetitem18);
        QTableWidgetItem *__qtablewidgetitem19 = new QTableWidgetItem();
        udpTableWidget_1->setHorizontalHeaderItem(3, __qtablewidgetitem19);
        QTableWidgetItem *__qtablewidgetitem20 = new QTableWidgetItem();
        udpTableWidget_1->setHorizontalHeaderItem(4, __qtablewidgetitem20);
        QTableWidgetItem *__qtablewidgetitem21 = new QTableWidgetItem();
        udpTableWidget_1->setHorizontalHeaderItem(5, __qtablewidgetitem21);
        QTableWidgetItem *__qtablewidgetitem22 = new QTableWidgetItem();
        udpTableWidget_1->setHorizontalHeaderItem(6, __qtablewidgetitem22);
        if (udpTableWidget_1->rowCount() < 8)
            udpTableWidget_1->setRowCount(8);
        QTableWidgetItem *__qtablewidgetitem23 = new QTableWidgetItem();
        udpTableWidget_1->setVerticalHeaderItem(0, __qtablewidgetitem23);
        QTableWidgetItem *__qtablewidgetitem24 = new QTableWidgetItem();
        udpTableWidget_1->setVerticalHeaderItem(1, __qtablewidgetitem24);
        QTableWidgetItem *__qtablewidgetitem25 = new QTableWidgetItem();
        udpTableWidget_1->setVerticalHeaderItem(2, __qtablewidgetitem25);
        QTableWidgetItem *__qtablewidgetitem26 = new QTableWidgetItem();
        udpTableWidget_1->setVerticalHeaderItem(3, __qtablewidgetitem26);
        QTableWidgetItem *__qtablewidgetitem27 = new QTableWidgetItem();
        udpTableWidget_1->setVerticalHeaderItem(4, __qtablewidgetitem27);
        QTableWidgetItem *__qtablewidgetitem28 = new QTableWidgetItem();
        udpTableWidget_1->setVerticalHeaderItem(5, __qtablewidgetitem28);
        QTableWidgetItem *__qtablewidgetitem29 = new QTableWidgetItem();
        udpTableWidget_1->setVerticalHeaderItem(6, __qtablewidgetitem29);
        QTableWidgetItem *__qtablewidgetitem30 = new QTableWidgetItem();
        udpTableWidget_1->setVerticalHeaderItem(7, __qtablewidgetitem30);
        udpTableWidget_1->setObjectName("udpTableWidget_1");
        udpTableWidget_1->setEditTriggers(QAbstractItemView::NoEditTriggers);
        udpTableWidget_1->setAlternatingRowColors(true);
        udpTableWidget_1->setRowCount(8);
        udpTableWidget_1->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_4->addWidget(udpTableWidget_1);


        verticalLayout_2->addWidget(udpGroupBox_1);

        logGroupBox_1 = new QGroupBox(senderTab);
        logGroupBox_1->setObjectName("logGroupBox_1");
        verticalLayout_5 = new QVBoxLayout(logGroupBox_1);
        verticalLayout_5->setObjectName("verticalLayout_5");
        logTextEdit_1 = new QPlainTextEdit(logGroupBox_1);
        logTextEdit_1->setObjectName("logTextEdit_1");
        logTextEdit_1->setReadOnly(true);

        verticalLayout_5->addWidget(logTextEdit_1);


        verticalLayout_2->addWidget(logGroupBox_1);

        mainTabWidget->addTab(senderTab, QString());
        receiverTab = new QWidget();
        receiverTab->setObjectName("receiverTab");
        verticalLayout_6 = new QVBoxLayout(receiverTab);
        verticalLayout_6->setObjectName("verticalLayout_6");
        rs485GroupBox_2 = new QGroupBox(receiverTab);
        rs485GroupBox_2->setObjectName("rs485GroupBox_2");
        verticalLayout_7 = new QVBoxLayout(rs485GroupBox_2);
        verticalLayout_7->setObjectName("verticalLayout_7");
        rs485TableWidget_2 = new QTableWidget(rs485GroupBox_2);
        if (rs485TableWidget_2->columnCount() < 6)
            rs485TableWidget_2->setColumnCount(6);
        QTableWidgetItem *__qtablewidgetitem31 = new QTableWidgetItem();
        rs485TableWidget_2->setHorizontalHeaderItem(0, __qtablewidgetitem31);
        QTableWidgetItem *__qtablewidgetitem32 = new QTableWidgetItem();
        rs485TableWidget_2->setHorizontalHeaderItem(1, __qtablewidgetitem32);
        QTableWidgetItem *__qtablewidgetitem33 = new QTableWidgetItem();
        rs485TableWidget_2->setHorizontalHeaderItem(2, __qtablewidgetitem33);
        QTableWidgetItem *__qtablewidgetitem34 = new QTableWidgetItem();
        rs485TableWidget_2->setHorizontalHeaderItem(3, __qtablewidgetitem34);
        QTableWidgetItem *__qtablewidgetitem35 = new QTableWidgetItem();
        rs485TableWidget_2->setHorizontalHeaderItem(4, __qtablewidgetitem35);
        QTableWidgetItem *__qtablewidgetitem36 = new QTableWidgetItem();
        rs485TableWidget_2->setHorizontalHeaderItem(5, __qtablewidgetitem36);
        if (rs485TableWidget_2->rowCount() < 8)
            rs485TableWidget_2->setRowCount(8);
        QTableWidgetItem *__qtablewidgetitem37 = new QTableWidgetItem();
        rs485TableWidget_2->setVerticalHeaderItem(0, __qtablewidgetitem37);
        QTableWidgetItem *__qtablewidgetitem38 = new QTableWidgetItem();
        rs485TableWidget_2->setVerticalHeaderItem(1, __qtablewidgetitem38);
        QTableWidgetItem *__qtablewidgetitem39 = new QTableWidgetItem();
        rs485TableWidget_2->setVerticalHeaderItem(2, __qtablewidgetitem39);
        QTableWidgetItem *__qtablewidgetitem40 = new QTableWidgetItem();
        rs485TableWidget_2->setVerticalHeaderItem(3, __qtablewidgetitem40);
        QTableWidgetItem *__qtablewidgetitem41 = new QTableWidgetItem();
        rs485TableWidget_2->setVerticalHeaderItem(4, __qtablewidgetitem41);
        QTableWidgetItem *__qtablewidgetitem42 = new QTableWidgetItem();
        rs485TableWidget_2->setVerticalHeaderItem(5, __qtablewidgetitem42);
        QTableWidgetItem *__qtablewidgetitem43 = new QTableWidgetItem();
        rs485TableWidget_2->setVerticalHeaderItem(6, __qtablewidgetitem43);
        QTableWidgetItem *__qtablewidgetitem44 = new QTableWidgetItem();
        rs485TableWidget_2->setVerticalHeaderItem(7, __qtablewidgetitem44);
        rs485TableWidget_2->setObjectName("rs485TableWidget_2");
        rs485TableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
        rs485TableWidget_2->setAlternatingRowColors(true);
        rs485TableWidget_2->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_7->addWidget(rs485TableWidget_2);


        verticalLayout_6->addWidget(rs485GroupBox_2);

        udpGroupBox_2 = new QGroupBox(receiverTab);
        udpGroupBox_2->setObjectName("udpGroupBox_2");
        verticalLayout_8 = new QVBoxLayout(udpGroupBox_2);
        verticalLayout_8->setObjectName("verticalLayout_8");
        udpTableWidget_2 = new QTableWidget(udpGroupBox_2);
        if (udpTableWidget_2->columnCount() < 5)
            udpTableWidget_2->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem45 = new QTableWidgetItem();
        udpTableWidget_2->setHorizontalHeaderItem(0, __qtablewidgetitem45);
        QTableWidgetItem *__qtablewidgetitem46 = new QTableWidgetItem();
        udpTableWidget_2->setHorizontalHeaderItem(1, __qtablewidgetitem46);
        QTableWidgetItem *__qtablewidgetitem47 = new QTableWidgetItem();
        udpTableWidget_2->setHorizontalHeaderItem(2, __qtablewidgetitem47);
        QTableWidgetItem *__qtablewidgetitem48 = new QTableWidgetItem();
        udpTableWidget_2->setHorizontalHeaderItem(3, __qtablewidgetitem48);
        QTableWidgetItem *__qtablewidgetitem49 = new QTableWidgetItem();
        udpTableWidget_2->setHorizontalHeaderItem(4, __qtablewidgetitem49);
        if (udpTableWidget_2->rowCount() < 4)
            udpTableWidget_2->setRowCount(4);
        QTableWidgetItem *__qtablewidgetitem50 = new QTableWidgetItem();
        udpTableWidget_2->setVerticalHeaderItem(0, __qtablewidgetitem50);
        QTableWidgetItem *__qtablewidgetitem51 = new QTableWidgetItem();
        udpTableWidget_2->setVerticalHeaderItem(1, __qtablewidgetitem51);
        QTableWidgetItem *__qtablewidgetitem52 = new QTableWidgetItem();
        udpTableWidget_2->setVerticalHeaderItem(2, __qtablewidgetitem52);
        QTableWidgetItem *__qtablewidgetitem53 = new QTableWidgetItem();
        udpTableWidget_2->setVerticalHeaderItem(3, __qtablewidgetitem53);
        udpTableWidget_2->setObjectName("udpTableWidget_2");
        udpTableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
        udpTableWidget_2->setAlternatingRowColors(true);
        udpTableWidget_2->horizontalHeader()->setStretchLastSection(true);

        verticalLayout_8->addWidget(udpTableWidget_2);


        verticalLayout_6->addWidget(udpGroupBox_2);

        logGroupBox_2 = new QGroupBox(receiverTab);
        logGroupBox_2->setObjectName("logGroupBox_2");
        verticalLayout_9 = new QVBoxLayout(logGroupBox_2);
        verticalLayout_9->setObjectName("verticalLayout_9");
        logTextEdit_2 = new QPlainTextEdit(logGroupBox_2);
        logTextEdit_2->setObjectName("logTextEdit_2");
        logTextEdit_2->setReadOnly(true);

        verticalLayout_9->addWidget(logTextEdit_2);


        verticalLayout_6->addWidget(logGroupBox_2);

        mainTabWidget->addTab(receiverTab, QString());

        verticalLayout_1->addWidget(mainTabWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1400, 18));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        mainTabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "EMC\346\265\213\346\216\247\344\270\200\344\275\223\345\214\226\345\256\236\346\227\266\347\233\221\346\216\247\347\263\273\347\273\237 v1.0", nullptr));
        rs485GroupBox_1->setTitle(QCoreApplication::translate("MainWindow", "RS485\344\270\262\345\217\243\347\212\266\346\200\201", nullptr));
        QTableWidgetItem *___qtablewidgetitem = rs485TableWidget_1->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "\347\212\266\346\200\201", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = rs485TableWidget_1->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "\345\267\262\345\217\221\351\200\201\345\270\247\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = rs485TableWidget_1->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("MainWindow", "\345\267\262\346\216\245\346\224\266\345\270\247\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = rs485TableWidget_1->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201\345\255\227\350\212\202 (Bytes)", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = rs485TableWidget_1->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("MainWindow", "\346\216\245\346\224\266\345\255\227\350\212\202 (Bytes)", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = rs485TableWidget_1->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("MainWindow", "\351\224\231\350\257\257\345\270\247\346\225\260 (CRC)", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = rs485TableWidget_1->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("MainWindow", "\344\270\242\345\270\247\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = rs485TableWidget_1->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("MainWindow", "\350\257\257\347\240\201\347\216\207 (%)", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = rs485TableWidget_1->verticalHeaderItem(0);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("MainWindow", "COM0", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = rs485TableWidget_1->verticalHeaderItem(1);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("MainWindow", "COM1", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = rs485TableWidget_1->verticalHeaderItem(2);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("MainWindow", "COM2", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = rs485TableWidget_1->verticalHeaderItem(3);
        ___qtablewidgetitem11->setText(QCoreApplication::translate("MainWindow", "COM3", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = rs485TableWidget_1->verticalHeaderItem(4);
        ___qtablewidgetitem12->setText(QCoreApplication::translate("MainWindow", "COM4", nullptr));
        QTableWidgetItem *___qtablewidgetitem13 = rs485TableWidget_1->verticalHeaderItem(5);
        ___qtablewidgetitem13->setText(QCoreApplication::translate("MainWindow", "COM5", nullptr));
        QTableWidgetItem *___qtablewidgetitem14 = rs485TableWidget_1->verticalHeaderItem(6);
        ___qtablewidgetitem14->setText(QCoreApplication::translate("MainWindow", "COM6", nullptr));
        QTableWidgetItem *___qtablewidgetitem15 = rs485TableWidget_1->verticalHeaderItem(7);
        ___qtablewidgetitem15->setText(QCoreApplication::translate("MainWindow", "COM7", nullptr));
        udpGroupBox_1->setTitle(QCoreApplication::translate("MainWindow", "UDP\347\275\221\345\217\243\347\212\266\346\200\201", nullptr));
        QTableWidgetItem *___qtablewidgetitem16 = udpTableWidget_1->horizontalHeaderItem(0);
        ___qtablewidgetitem16->setText(QCoreApplication::translate("MainWindow", "\347\212\266\346\200\201", nullptr));
        QTableWidgetItem *___qtablewidgetitem17 = udpTableWidget_1->horizontalHeaderItem(1);
        ___qtablewidgetitem17->setText(QCoreApplication::translate("MainWindow", "\345\267\262\345\217\221\351\200\201\345\214\205\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem18 = udpTableWidget_1->horizontalHeaderItem(2);
        ___qtablewidgetitem18->setText(QCoreApplication::translate("MainWindow", "\345\267\262\346\216\245\346\224\266\345\214\205\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem19 = udpTableWidget_1->horizontalHeaderItem(3);
        ___qtablewidgetitem19->setText(QCoreApplication::translate("MainWindow", "\345\217\221\351\200\201\345\255\227\350\212\202 (Bytes)", nullptr));
        QTableWidgetItem *___qtablewidgetitem20 = udpTableWidget_1->horizontalHeaderItem(4);
        ___qtablewidgetitem20->setText(QCoreApplication::translate("MainWindow", "\346\216\245\346\224\266\345\255\227\350\212\202 (Bytes)", nullptr));
        QTableWidgetItem *___qtablewidgetitem21 = udpTableWidget_1->horizontalHeaderItem(5);
        ___qtablewidgetitem21->setText(QCoreApplication::translate("MainWindow", "\350\266\205\346\227\266\346\254\241\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem22 = udpTableWidget_1->horizontalHeaderItem(6);
        ___qtablewidgetitem22->setText(QCoreApplication::translate("MainWindow", "\344\270\242\345\214\205\347\216\207 (%)", nullptr));
        QTableWidgetItem *___qtablewidgetitem23 = udpTableWidget_1->verticalHeaderItem(0);
        ___qtablewidgetitem23->setText(QCoreApplication::translate("MainWindow", "\347\213\254\347\253\213\347\275\221\345\217\2431", nullptr));
        QTableWidgetItem *___qtablewidgetitem24 = udpTableWidget_1->verticalHeaderItem(1);
        ___qtablewidgetitem24->setText(QCoreApplication::translate("MainWindow", "\347\213\254\347\253\213\347\275\221\345\217\2432", nullptr));
        QTableWidgetItem *___qtablewidgetitem25 = udpTableWidget_1->verticalHeaderItem(2);
        ___qtablewidgetitem25->setText(QCoreApplication::translate("MainWindow", "\347\213\254\347\253\213\347\275\221\345\217\2433", nullptr));
        QTableWidgetItem *___qtablewidgetitem26 = udpTableWidget_1->verticalHeaderItem(3);
        ___qtablewidgetitem26->setText(QCoreApplication::translate("MainWindow", "\344\272\244\346\215\242\346\234\272\347\275\221\345\217\2431", nullptr));
        QTableWidgetItem *___qtablewidgetitem27 = udpTableWidget_1->verticalHeaderItem(4);
        ___qtablewidgetitem27->setText(QCoreApplication::translate("MainWindow", "\344\272\244\346\215\242\346\234\272\347\275\221\345\217\2432", nullptr));
        QTableWidgetItem *___qtablewidgetitem28 = udpTableWidget_1->verticalHeaderItem(5);
        ___qtablewidgetitem28->setText(QCoreApplication::translate("MainWindow", "\344\272\244\346\215\242\346\234\272\347\275\221\345\217\2433", nullptr));
        QTableWidgetItem *___qtablewidgetitem29 = udpTableWidget_1->verticalHeaderItem(6);
        ___qtablewidgetitem29->setText(QCoreApplication::translate("MainWindow", "\344\272\244\346\215\242\346\234\272\347\275\221\345\217\2434", nullptr));
        QTableWidgetItem *___qtablewidgetitem30 = udpTableWidget_1->verticalHeaderItem(7);
        ___qtablewidgetitem30->setText(QCoreApplication::translate("MainWindow", "\344\272\244\346\215\242\346\234\272\347\275\221\345\217\2435", nullptr));
        logGroupBox_1->setTitle(QCoreApplication::translate("MainWindow", "\345\256\236\346\227\266\346\227\245\345\277\227\344\270\216\345\216\237\345\247\213\346\212\245\346\226\207", nullptr));
        mainTabWidget->setTabText(mainTabWidget->indexOf(senderTab), QCoreApplication::translate("MainWindow", "\344\270\213\344\275\215\346\234\272 (\345\217\221\351\200\201\347\253\257 - 192.168.3.10)", nullptr));
        rs485GroupBox_2->setTitle(QCoreApplication::translate("MainWindow", "RS485\344\270\262\345\217\243\347\212\266\346\200\201", nullptr));
        QTableWidgetItem *___qtablewidgetitem31 = rs485TableWidget_2->horizontalHeaderItem(0);
        ___qtablewidgetitem31->setText(QCoreApplication::translate("MainWindow", "\347\212\266\346\200\201", nullptr));
        QTableWidgetItem *___qtablewidgetitem32 = rs485TableWidget_2->horizontalHeaderItem(1);
        ___qtablewidgetitem32->setText(QCoreApplication::translate("MainWindow", "\345\267\262\346\216\245\346\224\266\345\270\247\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem33 = rs485TableWidget_2->horizontalHeaderItem(2);
        ___qtablewidgetitem33->setText(QCoreApplication::translate("MainWindow", "\345\267\262\345\233\236\345\217\221\345\270\247\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem34 = rs485TableWidget_2->horizontalHeaderItem(3);
        ___qtablewidgetitem34->setText(QCoreApplication::translate("MainWindow", "\346\216\245\346\224\266\345\255\227\350\212\202 (Bytes)", nullptr));
        QTableWidgetItem *___qtablewidgetitem35 = rs485TableWidget_2->horizontalHeaderItem(4);
        ___qtablewidgetitem35->setText(QCoreApplication::translate("MainWindow", "\345\233\236\345\217\221\345\255\227\350\212\202 (Bytes)", nullptr));
        QTableWidgetItem *___qtablewidgetitem36 = rs485TableWidget_2->horizontalHeaderItem(5);
        ___qtablewidgetitem36->setText(QCoreApplication::translate("MainWindow", "\351\224\231\350\257\257\345\270\247\346\225\260 (CRC)", nullptr));
        QTableWidgetItem *___qtablewidgetitem37 = rs485TableWidget_2->verticalHeaderItem(0);
        ___qtablewidgetitem37->setText(QCoreApplication::translate("MainWindow", "COM0", nullptr));
        QTableWidgetItem *___qtablewidgetitem38 = rs485TableWidget_2->verticalHeaderItem(1);
        ___qtablewidgetitem38->setText(QCoreApplication::translate("MainWindow", "COM1", nullptr));
        QTableWidgetItem *___qtablewidgetitem39 = rs485TableWidget_2->verticalHeaderItem(2);
        ___qtablewidgetitem39->setText(QCoreApplication::translate("MainWindow", "COM2", nullptr));
        QTableWidgetItem *___qtablewidgetitem40 = rs485TableWidget_2->verticalHeaderItem(3);
        ___qtablewidgetitem40->setText(QCoreApplication::translate("MainWindow", "COM3", nullptr));
        QTableWidgetItem *___qtablewidgetitem41 = rs485TableWidget_2->verticalHeaderItem(4);
        ___qtablewidgetitem41->setText(QCoreApplication::translate("MainWindow", "COM4", nullptr));
        QTableWidgetItem *___qtablewidgetitem42 = rs485TableWidget_2->verticalHeaderItem(5);
        ___qtablewidgetitem42->setText(QCoreApplication::translate("MainWindow", "COM5", nullptr));
        QTableWidgetItem *___qtablewidgetitem43 = rs485TableWidget_2->verticalHeaderItem(6);
        ___qtablewidgetitem43->setText(QCoreApplication::translate("MainWindow", "COM6", nullptr));
        QTableWidgetItem *___qtablewidgetitem44 = rs485TableWidget_2->verticalHeaderItem(7);
        ___qtablewidgetitem44->setText(QCoreApplication::translate("MainWindow", "COM7", nullptr));
        udpGroupBox_2->setTitle(QCoreApplication::translate("MainWindow", "UDP\347\275\221\345\217\243\347\212\266\346\200\201", nullptr));
        QTableWidgetItem *___qtablewidgetitem45 = udpTableWidget_2->horizontalHeaderItem(0);
        ___qtablewidgetitem45->setText(QCoreApplication::translate("MainWindow", "\347\212\266\346\200\201", nullptr));
        QTableWidgetItem *___qtablewidgetitem46 = udpTableWidget_2->horizontalHeaderItem(1);
        ___qtablewidgetitem46->setText(QCoreApplication::translate("MainWindow", "\345\267\262\346\216\245\346\224\266\345\214\205\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem47 = udpTableWidget_2->horizontalHeaderItem(2);
        ___qtablewidgetitem47->setText(QCoreApplication::translate("MainWindow", "\345\267\262\345\233\236\345\217\221\345\214\205\346\225\260", nullptr));
        QTableWidgetItem *___qtablewidgetitem48 = udpTableWidget_2->horizontalHeaderItem(3);
        ___qtablewidgetitem48->setText(QCoreApplication::translate("MainWindow", "\346\216\245\346\224\266\345\255\227\350\212\202 (Bytes)", nullptr));
        QTableWidgetItem *___qtablewidgetitem49 = udpTableWidget_2->horizontalHeaderItem(4);
        ___qtablewidgetitem49->setText(QCoreApplication::translate("MainWindow", "\345\233\236\345\217\221\345\255\227\350\212\202 (Bytes)", nullptr));
        QTableWidgetItem *___qtablewidgetitem50 = udpTableWidget_2->verticalHeaderItem(0);
        ___qtablewidgetitem50->setText(QCoreApplication::translate("MainWindow", "\347\213\254\347\253\213\347\275\221\345\217\2431", nullptr));
        QTableWidgetItem *___qtablewidgetitem51 = udpTableWidget_2->verticalHeaderItem(1);
        ___qtablewidgetitem51->setText(QCoreApplication::translate("MainWindow", "\347\213\254\347\253\213\347\275\221\345\217\2432", nullptr));
        QTableWidgetItem *___qtablewidgetitem52 = udpTableWidget_2->verticalHeaderItem(2);
        ___qtablewidgetitem52->setText(QCoreApplication::translate("MainWindow", "\347\213\254\347\253\213\347\275\221\345\217\2433", nullptr));
        QTableWidgetItem *___qtablewidgetitem53 = udpTableWidget_2->verticalHeaderItem(3);
        ___qtablewidgetitem53->setText(QCoreApplication::translate("MainWindow", "\344\272\244\346\215\242\346\234\272\347\275\221\345\217\243", nullptr));
        logGroupBox_2->setTitle(QCoreApplication::translate("MainWindow", "\345\256\236\346\227\266\346\227\245\345\277\227\344\270\216\345\216\237\345\247\213\346\212\245\346\226\207", nullptr));
        mainTabWidget->setTabText(mainTabWidget->indexOf(receiverTab), QCoreApplication::translate("MainWindow", "\344\270\213\344\275\215\346\234\272 (\346\216\245\346\224\266\347\253\257 - 192.168.3.20)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
