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
    // 定义一个信号，当收到新的二进制数据时发射出去
    // QByteArray 是Qt中用于处理字节数组的类
    void newDataReady(const QByteArray &data);

protected:
    // 当有新的客户端连接请求时，Qt会自动调用这个虚函数
    // 我们需要重写它，来创建自己的 ClientHandler 线程
    void incomingConnection(qintptr socketDescriptor) override;
};

#endif // TCPSERVER_H
