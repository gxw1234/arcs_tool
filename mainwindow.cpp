#include "mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), deviceConnected(false), deviceCount(0), lastVoltage(4.5), lastCurrent(1.5)
{
    // 设置窗口标题和大小
    setWindowTitle("SmartPower 控制器");
    resize(600, 500);
    
    // 创建用户界面
    setupUi();
    
    // 初始化SmartPower设备
    initSmartPowerDevice();
    
    // 创建定时器，每秒更新时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000);
    
    // 创建状态更新定时器
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &MainWindow::updateDeviceStatus);
    
    // 初始更新时间
    updateTime();
}

MainWindow::~MainWindow()
{
    // 确保关闭设备
    disconnectDevice();
}

void MainWindow::setupUi()
{
    // 创建中心部件和主布局
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建时间标签
    timeLabel = new QLabel(this);
    timeLabel->setAlignment(Qt::AlignRight);
    timeLabel->setStyleSheet("font-size: 14px; color: #666;");
    mainLayout->addWidget(timeLabel);
    
    // 创建状态标签
    statusLabel = new QLabel("就绪中...", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size: 14px; color: #666; margin-bottom: 10px;");
    mainLayout->addWidget(statusLabel);
    
    // 创建设备组
    deviceGroupBox = new QGroupBox("设备连接", this);
    QHBoxLayout *deviceLayout = new QHBoxLayout(deviceGroupBox);
    
    deviceCountLabel = new QLabel("可用设备：0", this);
    connectButton = new QPushButton("连接设备", this);
    connectButton->setStyleSheet("padding: 5px 15px;");
    
    deviceLayout->addWidget(deviceCountLabel);
    deviceLayout->addWidget(connectButton);
    
    mainLayout->addWidget(deviceGroupBox);
    
    // 创建控制组
    controlGroupBox = new QGroupBox("输出控制", this);
    controlGroupBox->setEnabled(false);  // 初始禁用，直到设备连接
    QGridLayout *controlLayout = new QGridLayout(controlGroupBox);
    
    controlLayout->addWidget(new QLabel("电压 (V):"), 0, 0);
    voltageEdit = new QLineEdit(QString::number(lastVoltage), this);
    voltageEdit->setValidator(new QDoubleValidator(0, 30, 2, this));
    controlLayout->addWidget(voltageEdit, 0, 1);
    
    controlLayout->addWidget(new QLabel("电流 (A):"), 1, 0);
    currentEdit = new QLineEdit(QString::number(lastCurrent), this);
    currentEdit->setValidator(new QDoubleValidator(0, 5, 2, this));
    controlLayout->addWidget(currentEdit, 1, 1);
    
    outputOnButton = new QPushButton("开启输出", this);
    outputOnButton->setStyleSheet("padding: 5px 15px; background-color: #4CAF50; color: white;");
    controlLayout->addWidget(outputOnButton, 0, 2);
    
    outputOffButton = new QPushButton("关闭输出", this);
    outputOffButton->setStyleSheet("padding: 5px 15px; background-color: #f44336; color: white;");
    outputOffButton->setEnabled(false);
    controlLayout->addWidget(outputOffButton, 1, 2);
    
    mainLayout->addWidget(controlGroupBox);
    
    // 创建状态组
    statusGroupBox = new QGroupBox("当前状态", this);
    statusGroupBox->setEnabled(false);  // 初始禁用，直到设备连接
    QGridLayout *statusLayout = new QGridLayout(statusGroupBox);
    
    statusLayout->addWidget(new QLabel("实际电压:"), 0, 0);
    voltageLabel = new QLabel("-- V", this);
    voltageLabel->setStyleSheet("font-weight: bold; color: #2196F3;");
    statusLayout->addWidget(voltageLabel, 0, 1);
    
    statusLayout->addWidget(new QLabel("实际电流:"), 1, 0);
    currentLabel = new QLabel("-- A", this);
    currentLabel->setStyleSheet("font-weight: bold; color: #FF9800;");
    statusLayout->addWidget(currentLabel, 1, 1);
    
    statusLayout->addWidget(new QLabel("BNC 状态:"), 2, 0);
    bncLabel = new QLabel("--", this);
    bncLabel->setStyleSheet("font-weight: bold;");
    statusLayout->addWidget(bncLabel, 2, 1);
    
    mainLayout->addWidget(statusGroupBox);
    
    // 设置间距
    mainLayout->addStretch();
    
    // 连接信号和槽
    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(outputOnButton, &QPushButton::clicked, this, &MainWindow::onOutputOnButtonClicked);
    connect(outputOffButton, &QPushButton::clicked, this, &MainWindow::onOutputOffButtonClicked);
}

void MainWindow::initSmartPowerDevice()
{
    // 获取应用程序路径
    QString appDir = QCoreApplication::applicationDirPath();
    statusLabel->setText(QString("应用程序路径: %1").arg(appDir));
    
    // 检查目录下的lib文件夹是否存在
    QDir libDir(appDir + "/lib");
    if (!libDir.exists()) {
        statusLabel->setText("错误: bin目录下没有lib文件夹");
        statusLabel->setStyleSheet("color: red;");
        connectButton->setEnabled(false);
        return;
    }
    
    // 检查DLL文件是否存在
    if (!QFile::exists(appDir + "/lib/SmartPower.dll")) {
        statusLabel->setText("错误: 没有找到lib/SmartPower.dll");
        statusLabel->setStyleSheet("color: red;");
        connectButton->setEnabled(false);
        return;
    }
    
    // 尝试使用SmartPower API
    try {
        // 获取可用设备数量
        deviceCount = SmartPower_DeviceNums();
        deviceCountLabel->setText(QString("可用设备：%1").arg(deviceCount));
    
        if (deviceCount == 0) {
            statusLabel->setText("未检测到SmartPower设备");
            statusLabel->setStyleSheet("color: red;");
            connectButton->setEnabled(false);
        } else {
            statusLabel->setText("已检测到SmartPower设备，请点击连接");
            statusLabel->setStyleSheet("color: green;");
        }
    } catch (...) {
        statusLabel->setText("错误: 调用SmartPower API失败");
        statusLabel->setStyleSheet("color: red;");
        connectButton->setEnabled(false);
    }
}

void MainWindow::disconnectDevice()
{
    if (deviceConnected) {
        // 停止状态更新定时器
        statusTimer->stop();
        
        // 关闭输出
        SmartPower_OutputOff(1);
        
        // 关闭设备
        SmartPower_DeviceClose();
        
        deviceConnected = false;
    }
}

void MainWindow::updateTime()
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    timeLabel->setText("当前时间: " + currentTime);
}

void MainWindow::onConnectButtonClicked()
{
    if (!deviceConnected) {
        // 连接设备
        bool bRet = SmartPower_DeviceOpen(0);
        if (bRet) {
            deviceConnected = true;
            statusLabel->setText("设备已连接");
            statusLabel->setStyleSheet("color: green; font-weight: bold;");
            
            // 更新UI状态
            connectButton->setText("断开连接");
            controlGroupBox->setEnabled(true);
            statusGroupBox->setEnabled(true);
            
            // 启动状态更新定时器
            statusTimer->start(500);  // 每500毫秒更新一次
        } else {
            QMessageBox::critical(this, "错误", "无法连接到SmartPower设备");
        }
    } else {
        // 断开连接
        disconnectDevice();
        
        // 更新UI状态
        statusLabel->setText("设备已断开连接");
        statusLabel->setStyleSheet("color: orange;");
        connectButton->setText("连接设备");
        controlGroupBox->setEnabled(false);
        statusGroupBox->setEnabled(false);
        outputOnButton->setEnabled(true);
        outputOffButton->setEnabled(false);
        
        // 重置状态显示
        voltageLabel->setText("-- V");
        currentLabel->setText("-- A");
        bncLabel->setText("--");
    }
}

void MainWindow::onOutputOnButtonClicked()
{
    if (deviceConnected) {
        // 获取用户输入的电压和电流值
        bool okV = false, okC = false;
        double voltage = voltageEdit->text().toDouble(&okV);
        double current = currentEdit->text().toDouble(&okC);
        
        if (!okV || !okC) {
            QMessageBox::warning(this, "警告", "请输入有效的电压和电流值");
            return;
        }
        
        // 保存设置值
        lastVoltage = voltage;
        lastCurrent = current;
        
        // 开启输出
        bool bRet = SmartPower_OutputOn(voltage, current);
        if (bRet) {
            // 更新UI状态
            statusLabel->setText(QString("开启输出: %1V, %2A").arg(voltage).arg(current));
            outputOnButton->setEnabled(false);
            outputOffButton->setEnabled(true);
        } else {
            QMessageBox::critical(this, "错误", "无法开启输出");
        }
    }
}

void MainWindow::onOutputOffButtonClicked()
{
    if (deviceConnected) {
        // 关闭输出
        bool bRet = SmartPower_OutputOff(1);
        if (bRet) {
            // 更新UI状态
            statusLabel->setText("输出已关闭");
            outputOnButton->setEnabled(true);
            outputOffButton->setEnabled(false);
        } else {
            QMessageBox::critical(this, "错误", "无法关闭输出");
        }
    }
}

void MainWindow::updateDeviceStatus()
{
    if (deviceConnected) {
        // 获取设备状态
        double fArr_ValueRead[4] = {0};
        bool bRet = SmartPower_GetStatus(fArr_ValueRead);
        
        if (bRet) {
            // 更新UI显示
            voltageLabel->setText(QString("%1 V").arg(fArr_ValueRead[0], 0, 'f', 3));
            currentLabel->setText(QString("%1 A").arg(fArr_ValueRead[1], 0, 'f', 3));
            bncLabel->setText(QString("%1").arg(fArr_ValueRead[2], 0, 'f', 3));
        }
    }
}
