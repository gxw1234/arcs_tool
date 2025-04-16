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
#include "smartpowercontroller.h"
#include <QTableWidget>


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
    void combo_changed_5(int index);
    void combo_changed_8(int index);
    void start_test_content_11();
    void start_test_content_12();
    
private:
    void setupUi();
    void setupUi_();
    void initSmartPowerDevice();
    
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



};

#endif // MAINWINDOW_H
