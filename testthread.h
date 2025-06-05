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
    TestThread(QTableWidget *tableWidget, BLUSerial *bluSerial, QObject *parent = nullptr, const QString &burnComPort = "COM4", const QString &connectInternet = "on", const QString &checkpoint = "on");
    ~TestThread();

protected:
    void run() override;

signals:
    void updateLog(const QString &message);  
    void updateProgress(int row, int value); 
    void updateResult(int row, const QString &result);  
    void updateBootTime(int row, bool on, int voltage = 3500);  // 自定义信号，用于更新上电时间，添加row参数和电压参数，默认3500mV
    void highlightRow(int row);  
    void updateSoftReset(int row, bool on);  
    void updatedeviceId(const QString &result);  
    void updateTestContent(int row, const QString &content);  
    void updateipValue(const QString &ipresult, int row ,int testtime);  

private:
    QTableWidget *table_widget;
    BLUSerial *m_bluSerial; // BLU设备串口对象
    ADBController m_adbController; // ADB控制器全局实例
    bool stopRequested;
    QString m_connectInternet;
    QString m_checkpoint;
    QString m_burnCOM;
    QString ipValue;
 
    QString calculateFileMD5(const QString &filePath);

    float m_currentSampleAvg;
    int m_sampleCount;

    void runTarget(int targetNum);

    bool runCskBurn(const QString &comPort, int baudRate, const QString &address, const QString &binFile, QString &output);

    bool runMysqlQuery(const QString &sn, QString &output);


    bool runUartBurn(const QString &comPort, const QString &fileName, QString &output);
    bool runIperfCommand(const QString &ipAddress, int port, int interval, int duration, QString &output);


public slots:
    void requestStop();  
};

#endif // TESTTHREAD_H
