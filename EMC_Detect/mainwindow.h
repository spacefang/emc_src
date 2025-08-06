#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess> // <--- 新增 QProcess 头文件
#include <QTemporaryFile>
#include "tcpserver.h"
#include "packetparser.h"
#include <QByteArray>

// 在此处包含QTcpSocket的完整定义
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // --- 这是最终的、正确的槽函数声明列表 ---
    void onRs485DataReady(const Rs485Data &data); // 修正了此处的拼写错误
    void onUdpDataReady(const UdpData &data);
    void onClearButtonClicked();
    void onStartButtonClicked();
    void onStopButtonClicked();

    // 用于处理身份确认的新槽函数
    void onIdentityConfirmed(QTcpSocket *client, const QString &identity);

    // ================== 新增槽函数同步设备时间声明 ==================
    void onSyncTimeButtonClicked();
    void onSyncProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    // ===============================================


private:
    Ui::MainWindow *ui;
    TcpServer *m_server;
    PacketParser *m_parser;

    // 用于管理两台下位机的连接
    QTcpSocket *m_senderSocket = nullptr;   // 指向发送端 (.10) 的连接
    QTcpSocket *m_receiverSocket = nullptr; // 指向接收端 (.20) 的连接

    QProcess *m_syncTimeProcess; // 用于执行外部同步设备时间脚本的进程对象
    // ================== 新增命令队列成员 ==================
    QStringList m_commandQueue;      // 用于存放要执行的命令队列
    int m_currentCommandIndex = 0; // 用于追踪当前执行到哪条命令
    QByteArray m_timeData; // 用于存储要发送的时间数据
    // ====================================================
};
#endif // MAINWINDOW_H
