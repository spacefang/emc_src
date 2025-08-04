#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTableWidgetItem> // 引入 QTableWidgetItem

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 1. 创建服务器和解析器对象
    m_server = new TcpServer(this);
    m_parser = new PacketParser(this);

    // 2. 核心连接：将服务器收到的原始数据，转发给解析器处理
    connect(m_server, &TcpServer::newDataReady, m_parser, &PacketParser::processData);

    // 3. 将解析器解析成功后的信号，连接到本窗口的槽函数，用于更新UI
    connect(m_parser, &PacketParser::rs485DataReady, this, &MainWindow::onRs485DataReady);
    connect(m_parser, &PacketParser::udpDataReady, this, &MainWindow::onUdpDataReady);

    // 4. 启动服务器
    m_server->startServer(54321);
    ui->statusbar->showMessage("正在监听端口 54321...");
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 槽函数：更新RS485表格
void MainWindow::onRs485DataReady(const Rs485Data &data)
{
    // 根据端口号，确定要更新的行
    int row = data.portNumber;
    if (row < 0 || row >= ui->rs485TableWidget_1->rowCount()) return;

    // 为了方便，我们指向发送端的表格 (rs485TableWidget_1)
    // 您可以根据需要添加逻辑来更新接收端的表格
    QTableWidget *table = ui->rs485TableWidget_1;

    // 更新每个单元格的内容
    table->setItem(row, 1, new QTableWidgetItem(QString::number(data.sentFrames)));
    table->setItem(row, 2, new QTableWidgetItem(QString::number(data.recvFrames)));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(data.sentBytes)));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(data.recvBytes)));
    table->setItem(row, 5, new QTableWidgetItem(QString::number(data.errorFrames)));
    table->setItem(row, 6, new QTableWidgetItem(QString::number(data.droppedFrames)));

    // 计算并更新误码率
    double errorRate = (data.sentFrames > 0) ? (double)data.errorFrames / data.sentFrames * 100 : 0.0;
    table->setItem(row, 7, new QTableWidgetItem(QString::asprintf("%.4f", errorRate)));
}

// 槽函数：更新UDP表格
void MainWindow::onUdpDataReady(const UdpData &data)
{
    int row = data.portNumber;
    if (row < 0 || row >= ui->udpTableWidget_1->rowCount()) return;

    QTableWidget *table = ui->udpTableWidget_1;

    table->setItem(row, 1, new QTableWidgetItem(QString::number(data.sentPackets)));
    table->setItem(row, 2, new QTableWidgetItem(QString::number(data.recvPackets)));
    table->setItem(row, 3, new QTableWidgetItem(QString::number(data.sentBytes)));
    table->setItem(row, 4, new QTableWidgetItem(QString::number(data.recvBytes)));
    table->setItem(row, 5, new QTableWidgetItem(QString::number(data.timeoutCount)));

    // 计算并更新丢包率
    double lossRate = (data.sentPackets > 0) ? (double)data.timeoutCount / data.sentPackets * 100 : 0.0;
    table->setItem(row, 6, new QTableWidgetItem(QString::asprintf("%.4f", lossRate)));
}
