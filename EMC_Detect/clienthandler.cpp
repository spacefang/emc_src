#include "clienthandler.h"
#include <QDebug>

ClientHandler::ClientHandler(qintptr socketDescriptor, QObject *parent)
    : QThread(parent), m_socketDescriptor(socketDescriptor)
{
}

void ClientHandler::run()
{
    // 在新线程中创建socket对象，避免跨线程问题
    m_socket = new QTcpSocket();

    // 将socket与传入的描述符关联起来
    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "Failed to set socket descriptor:" << m_socket->errorString();
        delete m_socket;
        return;
    }

    qDebug() << "Client handler thread started for" << m_socket->peerAddress().toString();

    // 使用事件循环，等待数据到来
    // waitForReadyRead会阻塞线程，直到有数据可读或超时
    while (m_socket->waitForConnected())
    {
        if (m_socket->waitForReadyRead(-1)) // -1表示无限期等待
        {
            // 读取所有可用的数据
            QByteArray data = m_socket->readAll();
            if (!data.isEmpty())
            {
                // 发射信号，将读取到的数据传递出去
                emit newDataReady(data);
            }
        }
    }

    // 当连接断开时，循环会退出
    qDebug() << "Client disconnected.";
    m_socket->disconnectFromHost();
    delete m_socket;
}
