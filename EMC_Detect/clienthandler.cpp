#include "clienthandler.h"
#include <QDebug>

ClientHandler::ClientHandler(qintptr socketDescriptor, QObject *parent)
    : QThread(parent), m_socketDescriptor(socketDescriptor)
{
}

void ClientHandler::run()
{
    m_socket = new QTcpSocket();
    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        qDebug() << "Failed to set socket descriptor:" << m_socket->errorString();
        delete m_socket;
        return;
    }

    // ====================== 核心修改点 ======================
    // 将socket的disconnected信号连接到我们自己的槽函数
    // 这样当连接断开时，我们的代码能立刻知道
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onSocketDisconnected);
    // =========================================================


    qDebug() << "Client handler thread started for" << m_socket->peerAddress().toString();

    // 使用事件循环来保持线程活跃，等待信号触发
    // exec()会启动一个事件循环，直到我们调用quit()
    exec();

    // 当事件循环结束后，清理资源
    qDebug() << "Client handler thread finished for" << m_socket->peerAddress().toString();
    m_socket->deleteLater(); // 使用deleteLater确保安全删除
}


// --- 新增的槽函数实现 ---
void ClientHandler::onSocketDisconnected()
{
    qDebug() << "Socket disconnected signal received in thread.";
    // 退出事件循环，这将导致run()函数结束，最终线程被安全销毁
    quit();
}
