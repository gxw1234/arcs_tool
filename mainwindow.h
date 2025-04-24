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
    
    // 事件过滤器，用于捕获回车键
    bool eventFilter(QObject *watched, QEvent *event) override;

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
    void showSNInputDialog(); // 显示SN扫描对话框

private:
    void setupUi();
    void setupUi_();
    void initSmartPowerDevice();
    QSerialPort *serialPort;
    
private:


    // UI 组件
    QWidget *centralWidget;
    QLabel *timeLabel;
    QLabel *statusLabel;
    
    // SmartPower 控制组件
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
    
    // BLU设备相关
    BLUSerial *bluSerial;
    BLUProtocol *bluProtocol;
    QString bluComPort;
    QString snopne;
    QString sntest;
    int bluVoltageValue;
    
    // 配置文件读取的串口信息
    QString burnComPort;   // 烧录使用的COM口
    
    // 关闭测试会话
    void closeTestSession();
    
    // 上电时间记录
    QDateTime powerOnTime;
    QDateTime firstResponseTime;
    bool hasReceivedFirstResponse;
    
    // 设备SN信息
    QString deviceSN;
    
    // 用于软复位测试的数据存储
    QString serialReceivedData;
    QString Title_test = "test_tool 1.1";
};

#endif // MAINWINDOW_H
