#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include <QObject>
#include <QByteArray>
#include <QTcpSocket>

// --- 数据结构定义 (这部分不变) ---
struct Rs485Data {
    quint8 portNumber;
    quint32 sentFrames;
    quint32 recvFrames;
    quint32 errorFrames;
    quint32 droppedFrames;
    quint64 sentBytes;
    quint64 recvBytes;
};

struct UdpData {
    quint8 portNumber;
    quint32 sentPackets;
    quint32 recvPackets;
    quint32 timeoutCount;
    quint64 sentBytes;
    quint64 recvBytes;
};

// --- 解析器类定义 ---

class PacketParser : public QObject
{
    Q_OBJECT

public:
    explicit PacketParser(QObject *parent = nullptr);

public slots:
    // ================== 核心修改点 1 ==================
    // 修改槽函数声明，让它能接收 client 指针和 data
    void processData(QTcpSocket *client, const QByteArray &data);
    // ===============================================

signals:
    // 当成功确认客户端身份时，发射此信号
    void identityConfirmed(QTcpSocket *client, const QString &identity);
    // 当成功解析出一个RS485报文时，发射此信号
    void rs485DataReady(const Rs485Data &data);
    // 当成功解析出一个UDP报文时，发射此信号
    void udpDataReady(const UdpData &data);

private:
    QByteArray m_buffer;

    quint16 crc16Modbus(const QByteArray &data);

    // ================== 核心修改点 2 ==================
    // 修改函数声明，让它也能接收 client 指针
    void parseBuffer(QTcpSocket *client);
    // ===============================================
};

#endif // PACKETPARSER_H
