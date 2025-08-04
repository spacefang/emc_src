#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "tcpserver.h"
#include "packetparser.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots: //新增 private slots
    void onRs485DataReady(const Rs485Data &data);
    void onUdpDataReady(const UdpData &data);

private:
    Ui::MainWindow *ui;
    TcpServer *m_server; // 添加一个服务器成员变量指针
    PacketParser *m_parser; // 添加一个解析器成员变量指针
};
#endif // MAINWINDOW_H
