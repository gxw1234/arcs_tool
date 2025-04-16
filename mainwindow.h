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
#include "lib/smartpower.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateTime();
    void onConnectButtonClicked();
    void onOutputOnButtonClicked();
    void onOutputOffButtonClicked();
    void updateDeviceStatus();
    
private:
    void setupUi();
    void initSmartPowerDevice();
    void disconnectDevice();
    
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
    
    // SmartPower 设备状态
    bool deviceConnected;
    int deviceCount;
    QTimer *statusTimer;
    
    // 保存上次的设置值
    double lastVoltage;
    double lastCurrent;
};

#endif // MAINWINDOW_H
