#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTableWidgetItem>
#include <QColor>
#include <QTemporaryFile>
#include <QCoreApplication> // <--- 新增头文件，用于获取程序路径
#include <QDir>             // <--- 新增头文件，用于处理路径
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //为按钮设置图标 (可选，但推荐)
        ui->clearButton->setIcon(QIcon(":/icons/clear.png"));
        ui->startButton->setIcon(QIcon(":/icons/start.png"));
        ui->stopButton->setIcon(QIcon(":/icons/stop.png"));
        ui->syncTimeButton->setIcon(QIcon(":/icons/time.png"));
        ui->clearButton->setIconSize(QSize(24, 24));
        ui->startButton->setIconSize(QSize(24, 24));
        ui->stopButton->setIconSize(QSize(24, 24));
        ui->syncTimeButton->setIconSize(QSize(24, 24));

    m_server = new TcpServer(this);
    m_parser = new PacketParser(this);

    // ================== 新增 QProcess 初始化 ==================
    m_syncTimeProcess = new QProcess(this);
    // 连接进程结束的信号，以便我们知道脚本何时执行完毕
    connect(m_syncTimeProcess, &QProcess::finished, this, &MainWindow::onSyncProcessFinished);
    // --- 核心修改：修复拼写错误，并将日志输出到当前活动的日志框 ---
    connect(m_syncTimeProcess, &QProcess::readyReadStandardOutput, this, [=](){
        // 判断当前是哪个Tab页，就将输出追加到哪个日志框
        if (ui->mainTabWidget->currentIndex() == 0) {
            ui->logTextEdit_1->appendPlainText(m_syncTimeProcess->readAllStandardOutput());
        } else {
            ui->logTextEdit_2->appendPlainText(m_syncTimeProcess->readAllStandardOutput());
        }
    });
    connect(m_syncTimeProcess, &QProcess::readyReadStandardError, this, [=](){
        if (ui->mainTabWidget->currentIndex() == 0) {
            ui->logTextEdit_1->appendPlainText(m_syncTimeProcess->readAllStandardError());
        } else {
            ui->logTextEdit_2->appendPlainText(m_syncTimeProcess->readAllStandardError());
        }
    });
    // ========================================================
    connect(ui->syncTimeButton, &QPushButton::clicked, this, &MainWindow::onSyncTimeButtonClicked);//新增同步时间按钮信号槽

    // ---- 新的、最终的核心信号槽连接 ----

    // 1. 服务器收到任何客户端的任何数据后，都统一交给解析器处理
    // (注意：这里我们不再需要连接 clientConnected 信号了)
    connect(m_server, &TcpServer::newDataReady, m_parser, &PacketParser::processData);

    // 2. 解析器成功确认了某个客户端的身份后，通知主窗口进行处理
    connect(m_parser, &PacketParser::identityConfirmed, this, &MainWindow::onIdentityConfirmed);

    // 3. 解析器成功解析出统计数据后，通知主窗口更新UI
    connect(m_parser, &PacketParser::rs485DataReady, this, &MainWindow::onRs485DataReady);
    connect(m_parser, &PacketParser::udpDataReady, this, &MainWindow::onUdpDataReady);

    // 4. 连接UI按钮的点击信号到对应的槽函数
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::onClearButtonClicked);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::onStopButtonClicked);

    // 5. 启动服务器
    m_server->startServer(54321);
    ui->statusbar->showMessage("正在监听端口 54321...");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 新的身份确认槽函数
void MainWindow::onIdentityConfirmed(QTcpSocket *client, const QString &identity)
{
    if (identity == "SENDER") {
        m_senderSocket = client;
        qDebug() << "Sender client identified and assigned:" << client->peerAddress().toString();
        ui->statusbar->showMessage("发送端已连接", 3000);
        // 当此socket断开时，自动将其指针置空
        connect(client, &QTcpSocket::disconnected, this, [=](){
            m_senderSocket = nullptr;
            qDebug() << "Sender client disconnected.";
            ui->statusbar->showMessage("发送端已断开", 3000);
        });

    } else if (identity == "RECEIVER") {
        m_receiverSocket = client;
        qDebug() << "Receiver client identified and assigned:" << client->peerAddress().toString();
        ui->statusbar->showMessage("接收端已连接", 3000);
        connect(client, &QTcpSocket::disconnected, this, [=](){
            m_receiverSocket = nullptr;
            qDebug() << "Receiver client disconnected.";
            ui->statusbar->showMessage("接收端已断开", 3000);
        });
    }
}

void MainWindow::onStartButtonClicked()
{
    if (m_senderSocket && m_receiverSocket)
    {
        char command = 0xE1;
        QByteArray dataToSend;
        dataToSend.append(command);
        m_senderSocket->write(dataToSend);
        m_receiverSocket->write(dataToSend);
        ui->statusbar->showMessage("已发送“开始所有测试”指令", 3000);
    } else {
        ui->statusbar->showMessage("发送失败：需要两台下位机都已连接", 3000);
    }
}

void MainWindow::onStopButtonClicked()
{
    if (m_senderSocket && m_receiverSocket)
    {
        char command = 0xE2;
        QByteArray dataToSend;
        dataToSend.append(command);
        m_senderSocket->write(dataToSend);
        m_receiverSocket->write(dataToSend);
        ui->statusbar->showMessage("已发送“停止所有测试”指令", 3000);
    } else {
        ui->statusbar->showMessage("发送失败：需要两台下位机都已连接", 3000);
    }
}

void MainWindow::onClearButtonClicked()
{
    if (m_senderSocket)
    {
        char command = 0xD1;
        QByteArray dataToSend;
        dataToSend.append(command);
        m_senderSocket->write(dataToSend);
        ui->statusbar->showMessage("已发送“复位统计”指令", 3000);
    } else {
        ui->statusbar->showMessage("发送失败：发送端未连接", 3000);
    }
}

void MainWindow::onRs485DataReady(const Rs485Data &data)
{
    int row = data.portNumber;
    if (row < 0 || row >= ui->rs485TableWidget_1->rowCount()) return;
    QTableWidget *table = ui->rs485TableWidget_1;
    QTableWidgetItem *statusItem = new QTableWidgetItem();
    if (data.sentFrames == 0) {
        statusItem->setText("未启动");
        statusItem->setBackground(QColor("#d3d3d3"));
    } else if (data.errorFrames > 0 || data.droppedFrames > 0) {
        statusItem->setText("异常");
        statusItem->setBackground(QColor("#ffcdd2"));
    } else {
        statusItem->setText("正常");
        statusItem->setBackground(QColor("#c8e6c9"));
    }
    table->setItem(row, 0, statusItem);
    table->setItem(row, 1, new QTableWidgetItem(QString::number(data.sentFrames)));
    table->setItem(row, 2, new QTableWidgetItem(QString::number(data.recvFrames)));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(data.sentBytes)));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(data.recvBytes)));
    table->setItem(row, 5, new QTableWidgetItem(QString::number(data.errorFrames)));
    table->setItem(row, 6, new QTableWidgetItem(QString::number(data.droppedFrames)));
    double errorRate = (data.sentFrames > 0) ? (double)data.errorFrames / data.sentFrames * 100 : 0.0;
    table->setItem(row, 7, new QTableWidgetItem(QString::asprintf("%.4f", errorRate)));
}

void MainWindow::onUdpDataReady(const UdpData &data)
{
    int row = data.portNumber;
    if (row < 0 || row >= ui->udpTableWidget_1->rowCount()) return;
    QTableWidget *table = ui->udpTableWidget_1;
    QTableWidgetItem *statusItem = new QTableWidgetItem();
    if (data.sentPackets == 0) {
        statusItem->setText("未启动");
        statusItem->setBackground(QColor("#d3d3d3"));
    } else if (data.timeoutCount > 0) {
        statusItem->setText("超时");
        statusItem->setBackground(QColor("#fff9c4"));
    } else {
        statusItem->setText("正常");
        statusItem->setBackground(QColor("#c8e6c9"));
    }
    table->setItem(row, 0, statusItem);
    table->setItem(row, 1, new QTableWidgetItem(QString::number(data.sentPackets)));
    table->setItem(row, 2, new QTableWidgetItem(QString::number(data.recvPackets)));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(data.sentBytes)));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(data.recvBytes)));
    table->setItem(row, 5, new QTableWidgetItem(QString::number(data.timeoutCount)));
    double lossRate = (data.sentPackets > 0) ? (double)data.timeoutCount / data.sentPackets * 100 : 0.0;
    table->setItem(row, 6, new QTableWidgetItem(QString::asprintf("%.4f", lossRate)));
}

// ================== “同步时间”按钮的槽函数实现 ==================
void MainWindow::onSyncTimeButtonClicked()
{
    if (m_syncTimeProcess->state() != QProcess::NotRunning) {
        ui->statusbar->showMessage("时间同步正在进行中，请稍候...", 3000);
        return;
    }

    ui->statusbar->showMessage("开始同步两台下位机时间...");
    QPlainTextEdit* currentLog = (ui->mainTabWidget->currentIndex() == 0) ? ui->logTextEdit_1 : ui->logTextEdit_2;
    currentLog->appendPlainText("--- [INFO] Starting time synchronization by executing remote set_time.sh script ---");

    // --- 1. 获取时间参数字符串 ---
    // 格式: "yyyy MM dd HH mm ss"
    QString timeArgs = QDateTime::currentDateTime().toString("yyyy MM dd HH mm ss");

    // --- 2. 构建新的、极致简单的SSH命令，只调用脚本并传递参数 ---
    QStringList commands;
    QString sshOptions = "ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null";

    QString remoteCommandTemplate = "%1 root@%2 /mnt/EMMC/emc_test01/set_time.sh %3";

    // 命令1：为发送端设置时间
    QString cmd1 = remoteCommandTemplate.arg(sshOptions).arg("192.168.3.10").arg(timeArgs);
    commands << cmd1;

    // 命令2：为接收端设置时间
    QString cmd2 = remoteCommandTemplate.arg(sshOptions).arg("192.168.3.20").arg(timeArgs);
    commands << cmd2;

    // --- 3. 执行命令 (后续逻辑不变) ---
    QString commandString = commands.join(" && ");
#ifdef Q_OS_WIN
    m_syncTimeProcess->start("cmd.exe", QStringList() << "/C" << commandString);
#else
    m_syncTimeProcess->start("sh", QStringList() << "-c" << commandString);
#endif
}

// === 当时间同步脚本执行完毕后，这个槽函数会被自动调用 ===
void MainWindow::onSyncProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QPlainTextEdit* currentLog = (ui->mainTabWidget->currentIndex() == 0) ? ui->logTextEdit_1 : ui->logTextEdit_2;

    // --- 核心修改：同时检查进程退出状态和退出码 ---
    if (exitStatus == QProcess::NormalExit && exitCode == 0)
    {
        // 只有当进程正常退出，并且返回码为0时，才算成功
        ui->statusbar->showMessage("时间同步成功！", 5000);
        currentLog->appendPlainText("--- [SUCCESS] Time synchronization finished. ---");
    }
    else
    {
        // 其他所有情况都视为失败
        ui->statusbar->showMessage("时间同步失败，请查看日志！", 5000);
        currentLog->appendPlainText(QString("--- [ERROR] Time synchronization failed. Exit code: %1, Exit status: %2 ---").arg(exitCode).arg(exitStatus));
    }
}
