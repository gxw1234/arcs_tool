#include "blu_serial.h"
#include "blu_protocol.h" // 包含协议定义
#include <QDebug>
#include <QThread>

BLUSerial::BLUSerial(QObject *parent) : QObject(parent)
{
    m_serialPort = new QSerialPort(this);
    m_protocol = nullptr;
    m_connected = false;
    remainder.clear();
}

BLUSerial::BLUSerial(BLUProtocol *protocol, QObject *parent) : QObject(parent)
{
    m_serialPort = new QSerialPort(this);
    m_protocol = protocol;
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
        
        if (m_serialPort->error() != QSerialPort::NoError && 
            m_serialPort->error() != QSerialPort::TimeoutError) {
            qDebug() << "读取等待错误:" << m_serialPort->errorString();
        }
    }
    

    QByteArray data;
    if (maxSize <= 0) {
        
        data = m_serialPort->readAll();
    } else {
        
        data = m_serialPort->read(maxSize);
    }
    
    return data;
}

bool BLUSerial::isConnected() const
{
    return m_connected;
}

// 通用命令发送函数
bool BLUSerial::sendCommand(const QByteArray &command)
{
    if (!m_connected) {
        qDebug() << "串口未连接，无法发送命令";
        return false;
    }
    return writeSerial(command);
}

// 设置协议对象
void BLUSerial::setProtocol(BLUProtocol *protocol)
{
    m_protocol = protocol;
}

// 设置源电压
bool BLUSerial::setVoltage(int voltage_mV)
{
    if (!m_protocol) {
        qDebug() << "协议对象为空，无法设置电压";
        return false;
    }
    // 设置协议对象中的当前电压值
    m_protocol->setCurrentVdd(voltage_mV);
    // 生成和发送设置电压命令
    QByteArray voltBytes = m_protocol->convertSourceVoltage(voltage_mV);
    QByteArray command;
    command.append(static_cast<char>(BLUProtocol::Command::REGULATOR_SET));
    command.append(voltBytes);
    return sendCommand(command);
}

// 打开/关闭DUT电源
bool BLUSerial::toggleDUTPower(bool turnOn)
{
    if (!m_protocol) {
        qDebug() << "协议对象为空，无法切换电源";
        return false;
    }
    QByteArray command = m_protocol->generateToggleDUTPowerCommand(turnOn);
    return sendCommand(command);
}

// 开始测量
bool BLUSerial::startMeasurement()
{
    if (!m_protocol) {
        qDebug() << "协议对象为空，无法开始测量";
        return false;
    }
    m_serialPort->clear();
    
    QByteArray command = m_protocol->generateStartMeasurementCommand();
    return sendCommand(command);
}

// 停止测量
bool BLUSerial::stopMeasurement()
{
    if (!m_protocol) {
        qDebug() << "协议对象为空，无法停止测量";
        return false;
    }
    
    QByteArray command = m_protocol->generateStopMeasurementCommand();
    return sendCommand(command);
}

// 获取元数据
QByteArray BLUSerial::getMetadata()
{
    if (!m_protocol) {
        qDebug() << "协议对象为空，无法获取元数据";
        return QByteArray();
    }
    
    // 发送获取元数据命令
    QByteArray command = m_protocol->generateGetMetadataCommand();
    if (!sendCommand(command)) {
        return QByteArray();
    }
    
    // 等待一小段时间，然后读取元数据
    QThread::msleep(100);
    return readData(0);
}

// 设置模式
bool BLUSerial::setMode(int mode)
{
    if (!m_protocol) {
        qDebug() << "协议对象为空，无法设置模式";
        return false;
    }
    
    // 设置协议对象中的模式
    m_protocol->setMode(static_cast<BLUMode>(mode));
    
    // 发送设置模式命令
    QByteArray command = m_protocol->generateSetModeCommand(static_cast<BLUMode>(mode));
    return sendCommand(command);
}
