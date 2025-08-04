#include "packetparser.h"
#include <QDebug>
#include <QIODevice>
#include <QDataStream>

const quint8 PKT_TYPE_RS485 = 0xA1;
const quint8 PKT_TYPE_UDP   = 0xB1;
const int RS485_PACKET_LEN = 38;
const int UDP_PACKET_LEN   = 34;

PacketParser::PacketParser(QObject *parent) : QObject(parent) {}

void PacketParser::processData(const QByteArray &data)
{
    m_buffer.append(data);
    parseBuffer();
}

void PacketParser::parseBuffer()
{
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
        m_buffer.remove(0, packetLen); // 立即移除，防止校验失败时死循环

        quint16 calculatedCrc = crc16Modbus(packet.left(packetLen - 2));
        quint16 receivedCrc = (quint8)packet[packetLen - 2] | ((quint8)packet[packetLen - 1] << 8); // 正确的小端CRC读取

        if (calculatedCrc != receivedCrc) {
            qDebug() << "CRC check failed! Packet discarded.";
            continue;
        }

        QDataStream stream(&packet, QIODevice::ReadOnly);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.skipRawData(2); // 跳过类型和长度

        if (packetType == PKT_TYPE_RS485)
        {
            Rs485Data parsedData;
            stream >> parsedData.portNumber;
            // stream.skipRawData(3); // <<< 已删除此错误行
            stream >> parsedData.sentFrames >> parsedData.recvFrames >> parsedData.errorFrames >> parsedData.droppedFrames;
            stream >> parsedData.sentBytes >> parsedData.recvBytes;
            emit rs485DataReady(parsedData);
        }
        else if (packetType == PKT_TYPE_UDP)
        {
            UdpData parsedData;
            stream >> parsedData.portNumber;
            // stream.skipRawData(3); // <<< 已删除此错误行
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
