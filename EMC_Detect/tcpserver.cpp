#include "tcpserver.h"
#include "clienthandler.h" // 引入ClientHandler的头文件
#include <QDebug>

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
}

void TcpServer::startServer(quint16 port)
{
    // QHostAddress::Any 表示监听本机上所有IP地址的指定端口
    if (!this->listen(QHostAddress::Any, port))
    {
        qDebug() << "Server could not start! Error: " << this->errorString();
    }
    else
    {
        qDebug() << "Server started! Listening on port" << port << "...";
    }
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "New connection incoming with socket descriptor:" << socketDescriptor;

    // 1. 创建一个新的 ClientHandler 线程来处理这个连接
    ClientHandler *handler = new ClientHandler(socketDescriptor, this);

    // 2. 将工作线程的 finished 信号连接到 deleteLater 槽
    //    这样可以确保线程任务完成后，其占用的内存被安全地释放
    connect(handler, &ClientHandler::finished, handler, &ClientHandler::deleteLater);

    // 3. 将工作线程的 newDataReady 信号，转发到 TcpServer 自身的 newDataReady 信号
    //    这样，主窗口只需要连接 TcpServer 的信号即可，实现了逻辑解耦
    connect(handler, &ClientHandler::newDataReady, this, &TcpServer::newDataReady);

    // 4. 启动线程
    handler->start();
}
