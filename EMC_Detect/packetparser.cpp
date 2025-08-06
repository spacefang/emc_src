#include "packetparser.h"
#include <QDebug>
#include <QIODevice>
#include <QDataStream>

const quint8 PKT_TYPE_RS485 = 0xA1;
const quint8 PKT_TYPE_UDP   = 0xB1;
const int RS485_PACKET_LEN = 38;
const int UDP_PACKET_LEN   = 34;

PacketParser::PacketParser(QObject *parent) : QObject(parent) {}

// ================== 核心修改点 3 ==================
// 函数签名现在与.h文件中的声明完全匹配
void PacketParser::processData(QTcpSocket *client, const QByteArray &data)
{
    m_buffer.append(data);

    // 优先处理身份识别
    if (m_buffer.startsWith("ID:")) {
        QString id_string = QString::fromUtf8(m_buffer);
        if (id_string.contains("SENDER")) {
            emit identityConfirmed(client, "SENDER");
            m_buffer.clear();
            return;
        } else if (id_string.contains("RECEIVER")) {
            emit identityConfirmed(client, "RECEIVER");
            m_buffer.clear();
            return;
        }
    }

    // 如果不是身份信息，则按原来的逻辑处理
    parseBuffer(client); // 调用现在也匹配了
}

// ================== 核心修改点 4 ==================
// 函数签名现在与.h文件中的声明完全匹配
void PacketParser::parseBuffer(QTcpSocket *client)
{
    // 这个函数内部的解析逻辑和之前一样，是正确的
    while (true)
    {
        if (m_buffer.size() < 2)
            return;

        quint8 packetType = m_buffer[0];
        quint8 packetLen = m_buffer[1];

        if (!((packetType == PKT_TYPE_RS485 && packetLen == RS485_PACKET_LEN) ||
              (packetType == PKT_TYPE_UDP && packetLen == UDP_PACKET_LEN)))
        {
            qDebug() << "Invalid packet header found. Discarding 1 byte.";
            m_buffer.remove(0, 1);
            continue;
        }

        if (m_buffer.size() < packetLen)
            return;

        QByteArray packet = m_buffer.left(packetLen);
        m_buffer.remove(0, packetLen);

        quint16 calculatedCrc = crc16Modbus(packet.left(packetLen - 2));
        quint16 receivedCrc = (quint8)packet[packetLen - 2] | ((quint8)packet[packetLen - 1] << 8);

        if (calculatedCrc != receivedCrc) {
            qDebug() << "CRC check failed! Packet discarded.";
            continue;
        }

        QDataStream stream(&packet, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.skipRawData(2);

        if (packetType == PKT_TYPE_RS485)
        {
            Rs485Data parsedData;
            stream >> parsedData.portNumber;
            stream >> parsedData.sentFrames >> parsedData.recvFrames >> parsedData.errorFrames >> parsedData.droppedFrames;
            stream >> parsedData.sentBytes >> parsedData.recvBytes;
            emit rs485DataReady(parsedData);
        }
        else if (packetType == PKT_TYPE_UDP)
        {
            UdpData parsedData;
            stream >> parsedData.portNumber;
            stream >> parsedData.sentPackets >> parsedData.recvPackets >> parsedData.timeoutCount;
            stream >> parsedData.sentBytes >> parsedData.recvBytes;
            emit udpDataReady(parsedData);
        }
    }
}

quint16 PacketParser::crc16Modbus(const QByteArray &data)
{
    quint16 crc = 0xFFFF;
    for (char byte : data) {
        crc ^= (quint8)byte;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

// ================== 核心修改点 5 ==================
// 删除了文件末尾多余的 '}'
// ===============================================
