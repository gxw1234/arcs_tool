#ifndef BLU_SERIAL_H
#define BLU_SERIAL_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QString>

// BLU串口通信类
class BLUSerial : public QObject
{
    Q_OBJECT

public:
    explicit BLUSerial(QObject *parent = nullptr);
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

private:
    QSerialPort *m_serialPort;
    bool m_connected;
};

#endif // BLU_SERIAL_H
