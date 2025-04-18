#include "mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QDebug>
#include <QKeyEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QTableWidgetItem>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), statusTimer(nullptr), powerController(nullptr), test_thread(nullptr)
{
    // 安装事件过滤器，捕获整个应用程序的回车键
    qApp->installEventFilter(this);
    
    setWindowTitle("test_tool");
    resize(600, 500);
    powerController = SmartPowerController::getInstance();
    setupUi_();
    initSmartPowerDevice();
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
    setWindowTitle("test_tool 1.1");
    // 创建中心部件和主布局
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 从JSON文件加载表格配置
    QFile configFile("d:/py/qtqbj/table_config.json");
    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning("无法打开表格配置文件!");
        show_log->append("错误: 无法打开配置文件 d:/py/qtqbj/table_config.json");
        return;
    }

    QByteArray configData = configFile.readAll();
    configFile.close();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(configData, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning("JSON解析错误: %s", qPrintable(jsonError.errorString()));
        show_log->append(QString("JSON解析错误: %1").arg(jsonError.errorString()));
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray columnsArray = jsonObj["columns"].toArray();
    QJsonArray columnWidthsArray = jsonObj["columnWidths"].toArray();
    QStringList headerLabels;
    for (const QJsonValue &val : columnsArray) {
        headerLabels << val.toString();
    }
    QJsonArray rowsArray = jsonObj["rows"].toArray();

    table_widget = new QTableWidget(rowsArray.size(), headerLabels.size(), this);
    table_widget->setHorizontalHeaderLabels(headerLabels);
    resize(1800, 900);

    for (int i = 0; i < qMin(columnWidthsArray.size(), headerLabels.size()); i++) {
        table_widget->setColumnWidth(i, columnWidthsArray[i].toInt());
    }


    for (int row = 0; row < rowsArray.size(); row++) {
        QJsonObject rowObj = rowsArray[row].toObject();
        

        QTableWidgetItem *nameItem = new QTableWidgetItem(QString(" %1").arg(rowObj["name"].toString()));
        table_widget->setItem(row, 0, nameItem);
        
        // 测试模式列
        QString modeText = rowObj["mode"].toString();
        QLineEdit *modeEdit = new QLineEdit(QString(" %1").arg(modeText));
        
        // 根据模式设置样式：自动为绿色，手动为蓝色
        if (modeText == "自动") {
            modeEdit->setStyleSheet("background-color: lightgreen;");
        } else if (modeText == "手动") {
            modeEdit->setStyleSheet("background-color: lightblue;");
        }
        
        table_widget->setCellWidget(row, 1, modeEdit);
        
        // 测试内容列
        QLineEdit *contentEdit = new QLineEdit(rowObj["content"].toString());
        table_widget->setCellWidget(row, 2, contentEdit);
        
        // 进度列 - 使用进度条
        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
        progressBar->setTextVisible(true); // 显示进度百分比
        progressBar->setAlignment(Qt::AlignCenter);
        progressBar->setFormat("%p%");
        // 设置样式表，使用灰色背景而不是默认的绿色
        progressBar->setStyleSheet("QProgressBar {background-color: #e0e0e0; border: 1px solid #c0c0c0; border-radius: 2px; padding: 1px;} "
                                  "QProgressBar::chunk {background-color: #c0c0c0; width: 1px;}");
        table_widget->setCellWidget(row, 3, progressBar);
        
        // 结果列
        QJsonObject resultObj = rowObj["result"].toObject();
        if (resultObj["type"].toString() == "combo") {
            QComboBox *resultCombo = new QComboBox();
            QJsonArray itemsArray = resultObj["items"].toArray();
            QStringList items;
            for (const QJsonValue &item : itemsArray) {
                items << item.toString();
            }
            resultCombo->addItems(items);
            if (resultObj.contains("default")) {
                resultCombo->setCurrentIndex(resultObj["default"].toInt());
            }
            table_widget->setCellWidget(row, 4, resultCombo);
        }
        
        // 备注列
        QLineEdit *noteEdit = new QLineEdit(rowObj["note"].toString());
        table_widget->setCellWidget(row, 5, noteEdit);
    }
    
    mainLayout->addWidget(table_widget);
    show_log = new QTextEdit(this);
    mainLayout->addWidget(show_log);
    start_test = new QPushButton("开始测试", this);
    start_test->setDefault(true); // 设置为默认按钮，支持回车键触发
    start_test->setAutoDefault(true); // 当使用键盘导航时自动成为默认按钮
    connect(start_test, &QPushButton::clicked, this, &MainWindow::start_test_content);
    mainLayout->addWidget(start_test);
}


void MainWindow::setupUi()
{
    // 创建中心部件和主布局
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    timeLabel = new QLabel(this);
    timeLabel->setAlignment(Qt::AlignRight);
    timeLabel->setStyleSheet("font-size: 14px; color: #666;");
    mainLayout->addWidget(timeLabel);
    
    statusLabel = new QLabel("就绪中...", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size: 14px; color: #666; margin-bottom: 10px;");
    mainLayout->addWidget(statusLabel);
    
    deviceGroupBox = new QGroupBox("设备连接", this);
    QHBoxLayout *deviceLayout = new QHBoxLayout(deviceGroupBox);
    
    deviceCountLabel = new QLabel("可用设备：0", this);
    connectButton = new QPushButton("连接设备", this);
    connectButton->setStyleSheet("padding: 5px 15px;");
    
    deviceLayout->addWidget(deviceCountLabel);
    deviceLayout->addWidget(connectButton);
    
    mainLayout->addWidget(deviceGroupBox);
    
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
    int deviceCount = powerController->getDeviceCount();
    show_log->append(QString("可用设备：%1").arg(deviceCount));
    if (!powerController->isConnected()) {
        // 连接设备
        if (powerController->connectDevice(0)) {
            show_log->append("设备已连接");
        } else {
            show_log->append("无法连接设备");
        }
    }
    if (powerController->isConnected()) {
        double voltage = 4.5;
        double current = 1.5;
        // 开启输出
        if (powerController->setOutput(true, voltage, current)) {
            // 更新UI状态
            show_log->append(QString("开启输出: %1V, %2A").arg(voltage).arg(current));

        } else {
            QMessageBox::critical(this, "错误", "无法开启输出");
        }
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
    } else if (index == 0) { // 退出
        // 实现退出效准模式的代码
    }
}

// 事件过滤器实现，捕获回车键事件
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    // 捕获回车键事件 - 只处理KeyPress事件，避免重复触发
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        
        // 如果按下回车键或回车键
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            // 且开始测试按钮存在且可用且没有测试线程运行
            if (start_test && start_test->isEnabled() && (!test_thread || !test_thread->isRunning())) {
                // 立即聚焦并点击开始测试按钮
                start_test->setFocus();
                start_test->click();
                qDebug() << "回车键触发了开始测试";
                return true; // 阻止事件继续传递
            }
        }
    } 
    // ShortcutOverride事件也需要捕获，但不重复触发点击
    else if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && 
            start_test && start_test->isEnabled()) {
            // 只标记事件已处理，但不触发点击
            event->accept();
            return true;
        }
    }
    
    // 其他事件默认处理
    return QMainWindow::eventFilter(watched, event);
}

// 连接wifi按钮点击处理函数
void MainWindow::start_test_content_11()
{
    // 实现连接wifi功能
    qDebug() << "开始连接wifi测试";
    // 在这里添加连接wifi的具体实现
    // 可以通过adb命令或其他方式连接设备wifi
}


void MainWindow::start_test_content()
{

}

// 表格复位函数
void MainWindow::resetTable()
{
    // 从配置文件重新加载数据
    QFile configFile("d:/py/qtqbj/table_config.json");
    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning("无法打开表格配置文件!");
        show_log->append("错误: 无法打开配置文件 d:/py/qtqbj/table_config.json");
        return;
    }

    QByteArray configData = configFile.readAll();
    configFile.close();

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(configData, &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qWarning("JSON解析错误: %s", qPrintable(jsonError.errorString()));
        show_log->append(QString("JSON解析错误: %1").arg(jsonError.errorString()));
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    QJsonArray rowsArray = jsonObj["rows"].toArray();
    
    // 清空日志
    show_log->clear();
    show_log->append("表格已复位到初始状态");
    
    // 更新表格内容
    for (int row = 0; row < qMin(rowsArray.size(), table_widget->rowCount()); row++) {
        QJsonObject rowObj = rowsArray[row].toObject();
        
        // 测试模式列
        QString modeText = rowObj["mode"].toString();
        QLineEdit *modeEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 1));
        if (modeEdit) {
            modeEdit->setText(QString(" %1").arg(modeText));
        }
        
        // 测试内容列
        QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 2));
        if (contentEdit) {
            contentEdit->setText(rowObj["content"].toString());
        }
        
        // 进度列 - 重置进度条为0
        QProgressBar *progressBar = qobject_cast<QProgressBar*>(table_widget->cellWidget(row, 3));
        if (progressBar) {
            progressBar->setValue(0);
        }
        
        // 结果列
        QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 4));
        if (resultCombo) {
            QJsonObject resultObj = rowObj["result"].toObject();
            if (resultObj.contains("default")) {
                resultCombo->setCurrentIndex(resultObj["default"].toInt());
            }
        }
        
        // 备注列
        QLineEdit *noteEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 5));
        if (noteEdit) {
            noteEdit->setText(rowObj["note"].toString());
        }
        
        // 移除所有行的高亮样式
        for (int col = 0; col < table_widget->columnCount(); col++) {
            QTableWidgetItem *item = table_widget->item(row, col);
            if (item) {
                item->setBackground(Qt::transparent);
            }
        }
    }
}

void MainWindow::start_test_content_12()
{
    qDebug() << "开始测试";
    
    resetTable();
    
 
    if (test_thread && test_thread->isRunning()) {
        test_thread->requestStop();
        test_thread->wait();
        delete test_thread;
        test_thread = nullptr;
    }
    
    test_thread = new TestThread(table_widget, this);
    
    connect(test_thread, &TestThread::updateLog, this, [this](const QString &message) {
        show_log->append(message);
    });
    
    connect(test_thread, &TestThread::updateProgress, this, [this](int row, int value) {
        QProgressBar *progressBar = qobject_cast<QProgressBar*>(table_widget->cellWidget(row, 3));
        if (progressBar) {
            progressBar->setValue(value);
        }
    });
    
    connect(test_thread, &TestThread::updateResult, this, [this](int row, const QString &result) {
        QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 4));
        if (resultCombo) {

            int index = resultCombo->findText(result);
            if (index >= 0) {
                resultCombo->setCurrentIndex(index);
            } else {

                for (int i = 0; i < resultCombo->count(); ++i) {
                    if (resultCombo->itemText(i).contains(result)) {
                        resultCombo->setCurrentIndex(i);
                        break;
                    }
                }
            }
        }
    });
    
    connect(test_thread, &TestThread::finished, this, [this]() {
        start_test->setEnabled(true);
        
        if (test_thread) {
            test_thread->deleteLater();
            test_thread = nullptr;
        }
        show_log->append("测试线程已完成");
    });
    

    start_test->setEnabled(false);
    test_thread->start();
    show_log->append("测试已开始，请等待...");
}