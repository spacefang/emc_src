#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include <QObject>
#include <QByteArray>

// --- 数据结构定义 ---
// 结构体与 shared_data.h 和二进制协议完全对应

// 用于存储RS485串口的完整统计数据
struct Rs485Data {
    quint8 portNumber;
    quint32 sentFrames;
    quint32 recvFrames;
    quint32 errorFrames;
    quint32 droppedFrames;
    quint64 sentBytes;
    quint64 recvBytes;
};

// 用于存储UDP网口的完整统计数据
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
    // 这是一个槽函数，用于接收从TcpServer传来的原始二进制数据
    void processData(const QByteArray &data);

signals:
    // 当成功解析出一个RS485报文时，发射此信号
    void rs485DataReady(const Rs485Data &data);

    // 当成功解析出一个UDP报文时，发射此信号
    void udpDataReady(const UdpData &data);

private:
    QByteArray m_buffer; // 内部缓冲区，用于处理粘包、分包问题

    // CRC16-Modbus校验函数
    quint16 crc16Modbus(const QByteArray &data);

    // 尝试从缓冲区中解析报文
    void parseBuffer();
};

#endif // PACKETPARSER_H
