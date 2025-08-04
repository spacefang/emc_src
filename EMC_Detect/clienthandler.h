#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QThread>
#include <QTcpSocket>

class ClientHandler : public QThread
{
    Q_OBJECT

public:
    explicit ClientHandler(qintptr socketDescriptor, QObject *parent = nullptr);

    // QThread的核心函数，所有线程相关的代码都在这里执行
    void run() override;

signals:
    // 当从socket中读取到新的数据时，发射此信号
    void newDataReady(const QByteArray &data);

private:
    qintptr m_socketDescriptor; // 套接字描述符，用于标识客户端连接
    QTcpSocket *m_socket = nullptr; // 指向TCP socket的指针
};

#endif // CLIENTHANDLER_H
