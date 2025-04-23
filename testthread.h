#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <QThread>
#include <QTableWidget>
#include <QString>
#include "blu_serial.h"
#include "blu_protocol.h"
#include "adbcontroller.h"

class TestThread : public QThread
{
    Q_OBJECT

public:
    TestThread(QTableWidget *tableWidget, BLUSerial *bluSerial, QObject *parent = nullptr, const QString &burnComPort = "COM4");
    ~TestThread();

protected:
    void run() override;

signals:
    void updateLog(const QString &message);  // 自定义信号，用于更新日志
    void updateProgress(int row, int value);  // 自定义信号，用于更新进度条
    void updateResult(int row, const QString &result);  // 自定义信号，用于更新结果列
    void updateBootTime(int row, bool on, int voltage = 3500);  // 自定义信号，用于更新上电时间，添加row参数和电压参数，默认3500mV


private:
    QTableWidget *table_widget;
    BLUSerial *m_bluSerial; // BLU设备串口对象
    ADBController m_adbController; // ADB控制器全局实例
    bool stopRequested;
    
    // 烧录使用的COM口
    QString m_burnCOM;
    
    // 计算文件的MD5值
    QString calculateFileMD5(const QString &filePath);

    float m_currentSampleAvg;
    int m_sampleCount;

    void runTarget(int targetNum);

    // 执行cskburn.exe命令烧录固件
    bool runCskBurn(const QString &comPort, int baudRate, const QString &address, const QString &binFile, QString &output);

public slots:
    void requestStop();  // 请求停止线程
};

#endif // TESTTHREAD_H
