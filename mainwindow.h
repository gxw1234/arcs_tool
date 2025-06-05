#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QTimer>
#include <QDateTime>
#include <QThread>
#include <QProgressBar>
#include "smartpowercontroller.h"
#include <QTableWidget>
#include <QTextEdit>
#include "testthread.h"
#include "blu_serial.h"
#include "blu_protocol.h"
#include <QSerialPort>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool eventFilter(QObject *watched, QEvent *event) override;
    bool runMysqlQuery(const QString &sn, const QString &site, const QString &result, 
        const QString &chipid, const QString &log1, const QString &log2, const QString &log3, 
        QString &output);
    bool runIperfCommand(const QString &ipAddress, int port, int interval, int duration, QString &output);
        

private slots:
    void updateTime();
    void onConnectButtonClicked();
    void onOutputOnButtonClicked();
    void onOutputOffButtonClicked();
    void updateDeviceStatus();
    void combo_changed_5(int index);
    void combo_changed_8(int index);
    void start_test_content_11();
    void start_test_content_12();
    void start_test_content();
    void resetTable();
    void onTestBootTime(int row, bool on, int voltage = 3500); 
    void onTestSoftReset(int row, bool on); // 处理软复位结果
    void updatedeviceId(const QString &deviceId); // 处理设备ID
    void showSNInputDialog(); // 显示SN扫描对话框
    void updateTestContent_(int row, const QString &content); // 处理测试内容更新
    void updateipValue_(const QString &ipresult,int row ,int testtime); 
private:
    void setupUi();
    void setupUi_();
    void initSmartPowerDevice();
    QSerialPort *serialPort;
    void saveTestLog(); // 保存测试日志到文件
    void recordSNDeviceID(); // 记录SN和设备ID的对应关系
    
private:

    QWidget *centralWidget;
    QLabel *timeLabel;
    QLabel *statusLabel;

    QGroupBox *deviceGroupBox;
    QPushButton *connectButton;
    QLabel *deviceCountLabel;
    QGroupBox *controlGroupBox;
    QLineEdit *voltageEdit;
    QLineEdit *currentEdit;
    QPushButton *outputOnButton;
    QPushButton *outputOffButton;
    QGroupBox *statusGroupBox;
    QLabel *voltageLabel;
    QLabel *currentLabel;
    QLabel *bncLabel;
    QTimer *statusTimer;
    SmartPowerController *powerController;
    QTableWidget *table_widget;
    QPushButton *start_test;
    QTextEdit *show_log;
    TestThread *test_thread;

    BLUSerial *bluSerial;
    BLUProtocol *bluProtocol;
    QString bluComPort;
    QString Connect_Internet;
    QString Checkpoint;
    QString snopne;
    int bluVoltageValue;

    QString burnComPort;   // 烧录使用的COM口

    void closeTestSession();

    QDateTime powerOnTime;
    QDateTime firstResponseTime;
    bool hasReceivedFirstResponse;

    QString deviceSN;
    QString serialReceivedData;
    QString Title_test = "test_tool 1.4.2";
    QString deviceId_;
    QString ipValue;


    bool allowEnterToStartTest; // 控制回车键是否触发测试
};

#endif // MAINWINDOW_H
