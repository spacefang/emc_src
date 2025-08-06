#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QObject>

// 向前声明 ClientHandler 类，避免头文件循环引用
class ClientHandler;

class TcpServer : public QTcpServer
{
    Q_OBJECT // 启用Qt的元对象系统 (用于信号和槽)

public:
    // 构造函数，parent通常是主窗口或QApplication
    explicit TcpServer(QObject *parent = nullptr);

    // 启动服务器，开始监听
    void startServer(quint16 port);

signals:
    // ====================== 修改和新增这里的信号 ======================
    // 当有新的客户端成功连接时发出
    void clientConnected(QTcpSocket *client);

    // 当有客户端断开连接时发出
    void clientDisconnected();

    // 当收到任何客户端的新数据时发出
    void newDataReady(QTcpSocket *client, const QByteArray &data);
    // =============================================================
protected:
    // 当有新的客户端连接请求时，Qt会自动调用这个虚函数
    // 我们需要重写它，来创建自己的 ClientHandler 线程
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // TCPSERVER_H
