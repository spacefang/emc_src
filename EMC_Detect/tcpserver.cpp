#include "tcpserver.h"
//#include "clienthandler.h" // 引入ClientHandler的头文件
#include <QDebug>
#include <QTcpSocket>
#include <QThread>
class ClientHandler : public QThread { /* ... */ };

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

    // 创建一个新的socket对象来处理这个连接
    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketDescriptor);

    // ====================== 核心修改点 ======================
    // 1. 发出新客户端连接的信号，将socket指针传递给MainWindow
    emit clientConnected(client);

    // 2. 将此socket的readyRead信号连接到一个lambda表达式
    //    当有数据可读时，直接读取并发出newDataReady信号
    connect(client, &QTcpSocket::readyRead, this, [=](){
        QByteArray data = client->readAll();
        if (!data.isEmpty()) {
            emit newDataReady(client, data);
        }
    });

    // 3. 将此socket的disconnected信号连接到我们的clientDisconnected信号
    //    同时，确保socket在断开连接后被安全删除
    connect(client, &QTcpSocket::disconnected, this, [=](){
        emit clientDisconnected();
        client->deleteLater();
        qDebug() << "Client socket deleted.";
    });
    // =======================================================
}
