#include "blu_serial.h"
#include <QDebug>

BLUSerial::BLUSerial(QObject *parent) : QObject(parent)
{
    m_serialPort = new QSerialPort(this);
    m_connected = false;
    remainder.clear();
}

BLUSerial::~BLUSerial()
{
    if (m_connected) {
        closePort();
    }
    
    // m_serialPort会由Qt的父子对象系统自动删除
}

bool BLUSerial::connectToPort(const QString &portName, int baudRate)
{
    // 如果已连接，先关闭
    if (m_connected) {
        closePort();
    }
    
    // 设置串口参数
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    // 打开串口
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_connected = true;
        qDebug() << "成功连接到串口" << portName << "波特率:" << baudRate;
        return true;
    } else {
        qDebug() << "连接串口失败:" << m_serialPort->errorString();
        return false;
    }
}

void BLUSerial::closePort()
{
    if (m_connected) {
        m_serialPort->close();
        m_connected = false;
        qDebug() << "关闭串口连接";
    }
}

bool BLUSerial::writeSerial(const QByteArray &data)
{
    if (!m_connected) {
        qDebug() << "串口未连接，无法写入数据";
        return false;
    }
    
    qint64 bytesWritten = m_serialPort->write(data);
    if (bytesWritten != data.size()) {
        qDebug() << "写入数据失败:" << m_serialPort->errorString();
        return false;
    }
    
    if (!m_serialPort->waitForBytesWritten(1000)) {
        qDebug() << "写入超时:" << m_serialPort->errorString();
        return false;
    }
    
    return true;
}

QByteArray BLUSerial::readData(int maxSize)
{
    if (!m_connected) {
        qDebug() << "串口未连接，无法读取数据";
        return QByteArray();
    }
    
    // 等待数据可用
    if (!m_serialPort->waitForReadyRead(100)) {
        // 超时不一定是错误，可能只是没有数据
        if (m_serialPort->error() != QSerialPort::NoError && 
            m_serialPort->error() != QSerialPort::TimeoutError) {
            qDebug() << "读取等待错误:" << m_serialPort->errorString();
        }
    }
    
    // 读取所有可用数据
    QByteArray data;
    if (maxSize <= 0) {
        // 读取所有可用数据
        data = m_serialPort->readAll();
    } else {
        // 限制读取最大字节数
        data = m_serialPort->read(maxSize);
    }
    
    return data;
}

bool BLUSerial::isConnected() const
{
    return m_connected;
}
