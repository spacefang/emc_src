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
    // 新的信号会同时传递是哪个客户端(socket)发送了什么数据(data)
    void newDataReady(QTcpSocket *client, const QByteArray &data);

private slots:
    // 新增一个私有槽，用于处理socket断开连接的事件
    void onSocketDisconnected();

private:
    qintptr m_socketDescriptor; // 套接字描述符，用于标识客户端连接
    QTcpSocket *m_socket = nullptr; // 指向TCP socket的指针
};

#endif // CLIENTHANDLER_H
