#include "mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QTableWidgetItem>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), test_thread(nullptr)
{
    
    setWindowTitle("test_tool");
    resize(600, 500);
    powerController = SmartPowerController::getInstance();
    setupUi_();
    // initSmartPowerDevice();
    // QTimer *timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    // timer->start(1000);

    // statusTimer = new QTimer(this);
    // connect(statusTimer, &QTimer::timeout, this, &MainWindow::updateDeviceStatus);
    // // 初始更新时间
    // updateTime();

}

MainWindow::~MainWindow()
{
   
}

void MainWindow::setupUi_()
{

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);




    
    QStringList list_test = {"烧录","检测adb设备", "wifi扫描", "adb_push_pull", "自播自录MIC检测", "EMMC检测", "背光检测", "OCR检测", "效准触摸","未连接wifi功耗","扫描功耗","息屏的功耗","连接wifi功耗"};
    
    table_widget = new QTableWidget(list_test.size(), 6, this);
    table_widget->setHorizontalHeaderLabels({"测试项目", "测试模式", "测试内容", "进度", "结果", "备注"});

    resize(1800,900);
    // 设置表格的列宽
    table_widget->setColumnWidth(0, 300);   // ID 列宽
    table_widget->setColumnWidth(1, 100);  // 文本列宽
    table_widget->setColumnWidth(2, 650);  // 状态列宽
    table_widget->setColumnWidth(3, 150);  // 备注列宽
    table_widget->setColumnWidth(4, 150);  // 备注列宽
    table_widget->setColumnWidth(5, 350);  // 备注列宽

    QString text_1 = "自动";
    QString text_2 = "手动";
    
    for (int row_ = 0; row_ < list_test.size(); row_++) {
        // 设置 ID 列的文本
        QTableWidgetItem *id_item = new QTableWidgetItem(QString(" %1").arg(list_test[row_]));
        table_widget->setItem(row_, 0, id_item);
        
        QLineEdit *comment_edit = new QLineEdit("");
        table_widget->setCellWidget(row_, 3, comment_edit);

        QLineEdit *comment_edit_notes = new QLineEdit("");
        table_widget->setCellWidget(row_, 5, comment_edit_notes);
        
        QComboBox *combo_box = new QComboBox();
        combo_box->addItems({"正常", "异常"});
        combo_box->setCurrentIndex(1);  // 默认选择异常
        // combo_box->currentIndexChanged.connect(lambda index, row=row_: self.combo_changed(index, row));
        table_widget->setCellWidget(row_, 4, combo_box);

        QLineEdit *text_edit = nullptr;
        QLineEdit *comment_edit_2 = nullptr;

        if (row_ == 0 || row_ == 1 || row_ == 2 || row_ == 3 || row_ == 4 || row_ == 5) {
            text_edit = new QLineEdit(QString(" %1").arg(text_1));
            text_edit->setStyleSheet("background-color: lightgreen;");
            comment_edit_2 = new QLineEdit("");
            comment_edit = new QLineEdit("");
            table_widget->setCellWidget(row_, 3, comment_edit);
        } else if (row_ == 6) {
            text_edit = new QLineEdit(QString(" %1").arg(text_2));
            text_edit->setStyleSheet("background-color: lightblue;");
            comment_edit_2 = new QLineEdit("手动调节背光检测背光变化");
            QComboBox *combo_box_5 = new QComboBox();
            combo_box_5->addItems({"空","0", "20", "50", "80", "100"});
            connect(combo_box_5, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::combo_changed_5);
            table_widget->setCellWidget(row_, 3, combo_box_5);
        } else if (row_ == 7) {
            comment_edit_2 = new QLineEdit("请扫描，观察灯光，OCR，TTS 翻译");
            text_edit = new QLineEdit(QString(" %1").arg(text_2));
            text_edit->setStyleSheet("background-color: lightblue;");
        } else if (row_ == 8) {
            comment_edit_2 = new QLineEdit("进入效准，请效准");
            text_edit = new QLineEdit(QString(" %1").arg(text_2));
            text_edit->setStyleSheet("background-color: lightblue;");
            
            QComboBox *combo_box_8 = new QComboBox();
            combo_box_8->addItems({"空","进入", "退出"});
            connect(combo_box_8, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::combo_changed_8);
            table_widget->setCellWidget(row_, 3, combo_box_8);
        } else if (row_ == 9) {
            comment_edit_2 = new QLineEdit("查看静态功耗 标准在208MA左右 正负5MA,比208小也算正常");
            text_edit = new QLineEdit(QString(" %1").arg(text_2));
            text_edit->setStyleSheet("background-color: lightblue;");
            QPushButton *combo_box_6 = new QPushButton();
            combo_box_6->setText("");
            table_widget->setCellWidget(row_, 3, combo_box_6);
        } else if (row_ == 10) {
            comment_edit_2 = new QLineEdit("扫描时的功耗");
            text_edit = new QLineEdit(QString(" %1").arg(text_2));
            text_edit->setStyleSheet("background-color: lightblue;");
            QPushButton *combo_box_6 = new QPushButton();
            combo_box_6->setText("");
            table_widget->setCellWidget(row_, 3, combo_box_6);
        } else if (row_ == 12) {
            comment_edit_2 = new QLineEdit("查看连接wifi功耗 功耗105 正负10 ，有心跳包");
            text_edit = new QLineEdit(QString(" %1").arg(text_2));
            text_edit->setStyleSheet("background-color: lightblue;");
            QPushButton *combo_box_11 = new QPushButton();
            combo_box_11->setText("连接wifi");
            connect(combo_box_11, &QPushButton::clicked, this, &MainWindow::start_test_content_11);
            table_widget->setCellWidget(row_, 3, combo_box_11);
        } else if (row_ == 11) {
            comment_edit_2 = new QLineEdit("查看息屏功耗 80 正负10");
            text_edit = new QLineEdit(QString(" %1").arg(text_2));
            text_edit->setStyleSheet("background-color: lightblue;");
            QPushButton *combo_box_12 = new QPushButton();
            combo_box_12->setText("息屏");
            connect(combo_box_12, &QPushButton::clicked, this, &MainWindow::start_test_content_12);
            table_widget->setCellWidget(row_, 3, combo_box_12);
        }

        table_widget->setCellWidget(row_, 1, text_edit);
        table_widget->setCellWidget(row_, 2, comment_edit_2);
    }
    
    mainLayout->addWidget(table_widget);

    show_log = new QTextEdit(this);
    mainLayout->addWidget(show_log);

    start_test = new QPushButton("开始测试", this);
    connect(start_test, &QPushButton::clicked, this, &MainWindow::start_test_content);
    mainLayout->addWidget(start_test);
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
    voltageEdit = new QLineEdit("4.5", this);  // 使用默认值4.5V
    voltageEdit->setValidator(new QDoubleValidator(0, 30, 2, this));
    controlLayout->addWidget(voltageEdit, 0, 1);
    
    controlLayout->addWidget(new QLabel("电流 (A):"), 1, 0);
    currentEdit = new QLineEdit("1.5", this);  // 使用默认值1.5A
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
    // 获取应用程序路径（仅用于显示）
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
    
    // 获取可用设备数量
    int deviceCount = powerController->getDeviceCount();
    deviceCountLabel->setText(QString("可用设备：%1").arg(deviceCount));

    if (deviceCount == 0) {
        statusLabel->setText("未检测到SmartPower设备");
        statusLabel->setStyleSheet("color: red;");
        connectButton->setEnabled(false);
    } else {
        statusLabel->setText("已检测到SmartPower设备，请点击连接");
        statusLabel->setStyleSheet("color: green;");
    }
}



void MainWindow::updateTime()
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    timeLabel->setText("当前时间: " + currentTime);
}

void MainWindow::onConnectButtonClicked()
{
    if (!powerController->isConnected()) {
        // 连接设备
        if (powerController->connectDevice(0)) {
            // 更新UI状态
            statusLabel->setText("设备已连接");
            statusLabel->setStyleSheet("color: green; font-weight: bold;");
            connectButton->setText("断开连接");
            controlGroupBox->setEnabled(true);
            statusGroupBox->setEnabled(true);
            
            // 启动状态更新定时器
            statusTimer->start(500);  // 每500毫秒更新一次状态
        } else {
            QMessageBox::critical(this, "错误", "无法连接到SmartPower设备");
        }
    } else {
        // 断开连接
        statusTimer->stop();
        powerController->disconnectDevice();
        
        // 手动更新UI状态
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
    if (powerController->isConnected()) {
        // 获取用户输入的电压和电流值
        bool okV = false, okC = false;
        double voltage = voltageEdit->text().toDouble(&okV);
        double current = currentEdit->text().toDouble(&okC);
        
        if (!okV || !okC) {
            QMessageBox::warning(this, "警告", "请输入有效的电压和电流值");
            return;
        }
        
        // 开启输出
        if (powerController->setOutput(true, voltage, current)) {
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
    if (powerController->isConnected()) {
        // 关闭输出
        if (powerController->setOutput(false)) {
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
    if (powerController->isConnected()) {
        // 获取设备状态，读取10次并计算平均值
        double voltage = 0.0, current = 0.0, bnc = 0.0;
        if (powerController->getDetailedStatus(voltage, current, bnc, 10)) {
            // 直接更新UI
            voltageLabel->setText(QString("%1 V").arg(voltage, 0, 'f', 3));
            currentLabel->setText(QString("%1 A").arg(current, 0, 'f', 3));
            bncLabel->setText(QString("%1").arg(bnc, 0, 'f', 3));
        }
    }
}

// 处理背光检测下拉框变化的槽函数
void MainWindow::combo_changed_5(int index)
{
    // 这里可以根据背光值实现相应的功能
    // 例如设置设备背光或记录测试状态
    qDebug() << "背光值选择: " << index;
    // 如果有具体的背光控制API，可以在这里调用
}

// 处理效准触摸下拉框变化的槽函数
void MainWindow::combo_changed_8(int index)
{
    // 根据选择进入或退出效准模式
    qDebug() << "效准操作: " << index;
    if (index == 1) { // 进入
        // 实现进入效准模式的代码
        qDebug() << "进入效准模式";
    } else if (index == 2) { // 退出
        // 实现退出效准模式的代码
        qDebug() << "退出效准模式";
    }
}

// 连接wifi按钮点击处理函数
void MainWindow::start_test_content_11()
{
    // 实现连接wifi功能
    qDebug() << "开始连接wifi测试";
    // 在这里添加连接wifi的具体实现
    // 可以通过adb命令或其他方式连接设备wifi
}

// 息屏按钮点击处理函数
void MainWindow::start_test_content_12()
{
    // 实现息屏功能
    qDebug() << "执行息屏操作";
    // 在这里添加息屏的具体实现
    // 可以通过adb命令或其他方式控制设备息屏
}

// 开始测试按钮点击处理函数
void MainWindow::start_test_content()
{
    qDebug() << "开始测试";
    
    // 如果有正在运行的测试线程，先停止它
    if (test_thread && test_thread->isRunning()) {
        test_thread->requestStop();
        test_thread->wait();
        delete test_thread;
        test_thread = nullptr;
    }
    
    // 创建新的测试线程
    test_thread = new TestThread(table_widget, this);
    
    // 连接信号到槽
    connect(test_thread, &TestThread::updateLog, this, [this](const QString &message) {
        // 更新日志输出到QTextEdit
        show_log->append(message);
    });
    
    connect(test_thread, &TestThread::finished, this, [this]() {
        // 线程结束后的清理工作
        if (test_thread) {
            test_thread->deleteLater();
            test_thread = nullptr;
        }
        show_log->append("测试线程已完成");
    });
    
    // 启动线程
    test_thread->start();
    
    // 更新UI状态
    start_test->setEnabled(false);
    show_log->append("测试已开始，请等待...");
}