#ifndef BLU_SERIAL_H
#define BLU_SERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QString>

// 前向声明
class BLUProtocol;

// BLU串口通信类
class BLUSerial : public QObject
{
    Q_OBJECT

public:
    explicit BLUSerial(QObject *parent = nullptr);
    BLUSerial(BLUProtocol *protocol, QObject *parent = nullptr);
    ~BLUSerial();

    // 连接到串口
    bool connectToPort(const QString &portName, int baudRate);
    
    // 关闭串口
    void closePort();
    
    // 写入数据到串口
    bool writeSerial(const QByteArray &data);
    
    // 读取串口数据
    QByteArray readData(int maxSize = 0);
    
    // 获取串口是否已连接
    bool isConnected() const;
    
    // 存储未完整处理的数据
    QByteArray remainder;
    
    // 高级封装函数，简化调用
    // 发送命令并检查结果，通用方法
    bool sendCommand(const QByteArray &command);
    
    // 设置协议对象
    void setProtocol(BLUProtocol *protocol);
    
    // 设置电压值
    bool setVoltage(int voltage_mV);
    
    // 切换DUT电源
    bool toggleDUTPower(bool turnOn);
    
    // 开始测量
    bool startMeasurement();
    
    // 停止测量
    bool stopMeasurement();
    
    // 获取元数据
    QByteArray getMetadata();
    
    // 获取协议对象
    BLUProtocol* getProtocol() const { return m_protocol; }
    
    // 设置设备模式
    bool setMode(int mode);

private:
    QSerialPort *m_serialPort;
    BLUProtocol *m_protocol;
    bool m_connected;
};

#endif // BLU_SERIAL_H
