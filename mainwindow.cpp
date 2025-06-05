#include "mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QDebug>
#include <QSettings>
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
#include <QInputDialog>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), statusTimer(nullptr), powerController(nullptr), test_thread(nullptr), serialPort(nullptr), allowEnterToStartTest(true)
{
    qApp->installEventFilter(this);
    resize(600, 500);
    powerController = SmartPowerController::getInstance();
    setupUi_();
    QSettings settings("cfg.ini", QSettings::IniFormat);
    burnComPort = settings.value("Serial/BurnCOM").toString();
    bluComPort = settings.value("BLU/DeviceCOM").toString();
    snopne = settings.value("SN/open").toString();
    Connect_Internet=settings.value("IN/Internet").toString();
    Checkpoint=settings.value("CP/InterneCheckpointt").toString();

    bluVoltageValue = 4000;
    bluProtocol = new BLUProtocol(this);
    bluSerial = new BLUSerial(bluProtocol, this);
    if (snopne =="on")
    {
        QTimer::singleShot(500, this, SLOT(showSNInputDialog()));
    }
}

MainWindow::~MainWindow()
{
    if (bluSerial) {
        bluSerial->closePort();
    }
}

void MainWindow::setupUi_()
{
    setWindowTitle(Title_test);
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QFile configFile("./table_config.json");
    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning("无法打开表格配置文件!");
        show_log->append("错误: 无法打开配置文件 ./table_config.json");
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

  
    table_widget->horizontalHeader()->setStretchLastSection(true);

   
    table_widget->hideColumn(0);
    for (int row = 0; row < rowsArray.size(); row++) {
        QJsonObject rowObj = rowsArray[row].toObject();
       
        QTableWidgetItem *idItem = new QTableWidgetItem(rowObj["id"].toString());
        idItem->setTextAlignment(Qt::AlignCenter);
        table_widget->setItem(row, 0, idItem);

        QTableWidgetItem *nameItem = new QTableWidgetItem(QString(" %1").arg(rowObj["name"].toString()));
        table_widget->setItem(row, 1, nameItem);

        QString modeText = rowObj["mode"].toString();
        QLineEdit *modeEdit = new QLineEdit(QString(" %1").arg(modeText));
        if (modeText == "自动") {
            modeEdit->setStyleSheet("background-color: lightgreen;");
        } else if (modeText == "手动") {
            modeEdit->setStyleSheet("background-color: lightblue;");
        }else if (modeText == "提示") {
            modeEdit->setStyleSheet("background-color: pink;");
        }
        
        
        table_widget->setCellWidget(row, 2, modeEdit);

        QLineEdit *contentEdit = new QLineEdit(rowObj["content"].toString());
        table_widget->setCellWidget(row, 3, contentEdit);
        

        QProgressBar *progressBar = new QProgressBar();
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
        progressBar->setTextVisible(true); // 显示进度百分比
        progressBar->setAlignment(Qt::AlignCenter);
        progressBar->setFormat("%p%");

        progressBar->setStyleSheet("QProgressBar {background-color: #e0e0e0; border: 1px solid #c0c0c0; border-radius: 2px; padding: 1px;} "
                                  "QProgressBar::chunk {background-color: #c0c0c0; width: 1px;}");
        table_widget->setCellWidget(row, 4, progressBar);
        

        QComboBox *resultCombo = new QComboBox();
        QStringList items;
        items <<"未测试"<< "正常" << "异常";
        resultCombo->addItems(items);
        resultCombo->setCurrentIndex(0); 
        resultCombo->setItemData(0, QBrush(QColor("blue")), Qt::ForegroundRole);
        resultCombo->setItemData(1, QBrush(QColor("green")), Qt::ForegroundRole);
        resultCombo->setItemData(2, QBrush(QColor("red")), Qt::ForegroundRole);
        resultCombo->setEnabled(false); 
        connect(resultCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [resultCombo](int index) {
            QColor color;
            switch(index) {
                case 0: color = QColor("blue"); break;
                case 1: color = QColor("green"); break;
                case 2: color = QColor("red"); break;
                default: color = QColor("black"); break;
            }
            resultCombo->setStyleSheet(QString("QComboBox { color: %1; }").arg(color.name()));
        });
        
        resultCombo->setStyleSheet("QComboBox { color: blue; }"); 
        
        table_widget->setCellWidget(row, 5, resultCombo);

        QLineEdit *noteEdit = new QLineEdit(rowObj["note"].toString());
        table_widget->setCellWidget(row, 6, noteEdit);
    }
    mainLayout->addWidget(table_widget);
    show_log = new QTextEdit(this);
    show_log->setReadOnly(true);
    show_log->setMaximumHeight(200);
    mainLayout->addWidget(show_log);
    start_test = new QPushButton("开始测试", this);
    start_test->setDefault(true); // 设置为默认按钮，支持回车键触发
    start_test->setAutoDefault(true); // 当使用键盘导航时自动成为默认按钮
    connect(start_test, &QPushButton::clicked, this, &MainWindow::start_test_content);
    mainLayout->addWidget(start_test);
}


void MainWindow::setupUi()
{

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
    
    mainLayout->addStretch();
    

    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(outputOnButton, &QPushButton::clicked, this, &MainWindow::onOutputOnButtonClicked);
    connect(outputOffButton, &QPushButton::clicked, this, &MainWindow::onOutputOffButtonClicked);
}

void MainWindow::initSmartPowerDevice()
{
    int deviceCount = powerController->getDeviceCount();
    show_log->append(QString("可用设备：%1").arg(deviceCount));
    if (!powerController->isConnected()) {

        if (powerController->connectDevice(0)) {
            show_log->append("设备已连接");
        } else {
            show_log->append("无法连接设备");
        }
    }
    if (powerController->isConnected()) {
        double voltage = 4.5;
        double current = 1.5;

        if (powerController->setOutput(true, voltage, current)) {

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

        if (powerController->connectDevice(0)) {

            statusLabel->setText("设备已连接");
            statusLabel->setStyleSheet("color: green; font-weight: bold;");
            connectButton->setText("断开连接");
            controlGroupBox->setEnabled(true);
            statusGroupBox->setEnabled(true);
            
            statusTimer->start(500);  // 每500毫秒更新一次状态
        } else {
            QMessageBox::critical(this, "错误", "无法连接到SmartPower设备");
        }
    } else {

        statusTimer->stop();
        powerController->disconnectDevice();

        statusLabel->setText("设备已断开连接");
        statusLabel->setStyleSheet("color: orange;");
        connectButton->setText("连接设备");
        controlGroupBox->setEnabled(false);
        statusGroupBox->setEnabled(false);
        outputOnButton->setEnabled(true);
        outputOffButton->setEnabled(false);
        voltageLabel->setText("-- V");
        currentLabel->setText("-- A");
        bncLabel->setText("--");
    }
}

void MainWindow::onOutputOnButtonClicked()
{
    if (powerController->isConnected()) {
        bool okV = false, okC = false;
        double voltage = voltageEdit->text().toDouble(&okV);
        double current = currentEdit->text().toDouble(&okC);
        
        if (!okV || !okC) {
            QMessageBox::warning(this, "警告", "请输入有效的电压和电流值");
            return;
        }

        if (powerController->setOutput(true, voltage, current)) {

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

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {

            if (allowEnterToStartTest) {
                if (start_test && start_test->isEnabled() && (!test_thread || !test_thread->isRunning())) {
                    start_test->setFocus();
                    start_test->click();
                    qDebug() << "回车键触发了开始测试";
                    return true; // 阻止事件继续传递
                }
            }
        }
    } 
    else if (event->type() == QEvent::ShortcutOverride) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) && 
            allowEnterToStartTest && start_test && start_test->isEnabled()) {
            event->accept();
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}


void MainWindow::start_test_content_11()
{

    qDebug() << "开始连接wifi测试";

}


void MainWindow::start_test_content()
{

    resetTable();
    

    if (test_thread && test_thread->isRunning()) {
        test_thread->requestStop();
        test_thread->wait();
        delete test_thread;
        test_thread = nullptr;
    }

    start_test->setEnabled(false);

    show_log->append("连接BLU设备...");
    if (!bluSerial->connectToPort(bluComPort, 9600)) {
        show_log->append("无法打开端口：" + bluComPort);
        start_test->setEnabled(true);
        return;
    }
    show_log->append("设备连接成功");

    QByteArray metaData = bluSerial->getMetadata();
    if (metaData.isEmpty()) {
        show_log->append("读取设备元数据失败，使用默认值");
    } else {
        bool ok = bluProtocol->parseMetadata(metaData);
        if (ok) {
            show_log->append("已读取校准因子");
        } else {
            show_log->append("解析校准因子失败，使用默认值");
        }
    }
 
    if (!bluSerial->setMode(BLUMode::SOURCE_MODE)) {
        show_log->append("切换到源表模式失败");
        closeTestSession();
        return;
    }
    show_log->append("已切换到源表模式");
    
    if (!bluSerial->setVoltage(bluVoltageValue)) {
        show_log->append("设置源电压失败");
        closeTestSession();
        return;
    }
    
    show_log->append(QString("已设置源电压为 %1V").arg(bluVoltageValue/1000.0));
    
    if (!bluSerial->toggleDUTPower(true)) {
        show_log->append("打开DUT电源失败");
        closeTestSession();
        return;
    }


  
    show_log->append("已打开DUT电源");
    show_log->append("创建测试线程...");
    test_thread = new TestThread(table_widget, bluSerial, this, burnComPort,Connect_Internet,Checkpoint);



    connect(test_thread, &TestThread::updateSoftReset, this, &MainWindow::onTestSoftReset);
    connect(test_thread, &TestThread::updatedeviceId, this, &MainWindow::updatedeviceId);
    connect(test_thread, &TestThread::updateipValue, this, &MainWindow::updateipValue_);
    connect(test_thread, &TestThread::updateBootTime, this, &MainWindow::onTestBootTime);
    
    // 连接测试线程的信号
    connect(test_thread, &TestThread::updateLog, this, [this](const QString &message) {
        show_log->append(message);
    });
    
    connect(test_thread, &TestThread::updateProgress, this, [this](int row, int value) {
        QProgressBar *progressBar = qobject_cast<QProgressBar*>(table_widget->cellWidget(row, 4));
        if (progressBar) {
            progressBar->setValue(value);
        }
    });
    
    //结果
    connect(test_thread, &TestThread::updateResult, this, [this](int row, const QString &result) {
        QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 5));
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

    connect(test_thread, &TestThread::highlightRow, this, [this](int row) {
        table_widget->selectRow(row);
    });

    // 连接测试内容更新信号
    connect(test_thread, &TestThread::updateTestContent, this, &MainWindow::updateTestContent_);

    // 测试完成后关闭设备
    connect(test_thread, &TestThread::finished, this, [this]() {
        start_test->setEnabled(true);
        
        if (test_thread) {
            test_thread->deleteLater();
            test_thread = nullptr;
        }
        show_log->append("测试线程已完成");
        bluSerial->stopMeasurement();
        closeTestSession();
        recordSNDeviceID();
        saveTestLog();
        if (snopne =="on")
        {
            showSNInputDialog();
           
        }
    });
    
    test_thread->start();
    show_log->append("测试线程已启动，请等待...");
}

// 关闭测试会话
void MainWindow::closeTestSession()
{
    // 关闭DUT电源
    bluSerial->toggleDUTPower(false);
    show_log->append("关闭DUT电源");
    
    // 关闭串口连接
    bluSerial->closePort();
    show_log->append("测试完成");
    
    // 重新启用开始测试按钮
    start_test->setEnabled(true);
}

// 表格复位函数
void MainWindow::resetTable()
{

    QFile configFile("./table_config.json");
    if (!configFile.open(QIODevice::ReadOnly)) {
        qWarning("无法打开表格配置文件!");
        show_log->append("错误: 无法打开配置文件 ./table_config.json");
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
    

    show_log->clear();
    show_log->append("表格已复位到初始状态");
    
    // 更新表格内容
    for (int row = 0; row < qMin(rowsArray.size(), table_widget->rowCount()); row++) {

        if(row != 0){
            QJsonObject rowObj = rowsArray[row].toObject();
            
            // 测试模式列
            QString modeText = rowObj["mode"].toString();
            QLineEdit *modeEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 2));
            if (modeEdit) {
                modeEdit->setText(QString(" %1").arg(modeText));
            }
            
            // 测试内容列
            QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 3));
            if (contentEdit) {
                contentEdit->setText(rowObj["content"].toString());
            }
            
            // 进度列 - 重置进度条为0
            QProgressBar *progressBar = qobject_cast<QProgressBar*>(table_widget->cellWidget(row, 4));
            if (progressBar) {
                progressBar->setValue(0);
            }
            
            // 结果列 - 重置为异常
            QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 5));
            if (resultCombo) {
                resultCombo->setCurrentIndex(0); // 默认固定为异常
            }
            
            // 备注列
            QLineEdit *noteEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 6));
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
    
}

void MainWindow::updatedeviceId(const QString &deviceId)
{
    deviceId_ = deviceId;
    
}

void MainWindow::updateTestContent_(int row, const QString &content)
{
    QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 3));
    if (contentEdit) {
        contentEdit->setText(content);
    }
}

void MainWindow::onTestSoftReset(int row, bool on)
{
    QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 3));
    QProgressBar *progressBar = qobject_cast<QProgressBar*>(table_widget->cellWidget(row, 4));
    QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 5));
    if(on){
        serialReceivedData.clear();
        if (!serialPort) {
            serialPort = new QSerialPort(this);
        }
        if (!serialPort->isOpen()) {
            QString portName = burnComPort; 
            serialPort->setPortName(portName);
            serialPort->setBaudRate(921600);
            if (serialPort->open(QIODevice::ReadWrite)) {
                // 禁用DTR和RTS信号，避免影响设备启动
                serialPort->setDataTerminalReady(false);
                serialPort->setRequestToSend(false);
                serialPort->clear();
                show_log->append("串口已打开: " + portName);
                connect(serialPort, &QSerialPort::readyRead, this, [this]() {
                    QByteArray data = serialPort->readAll();
                    if (!data.isEmpty()) {
                        // 将接收到的所有数据存储起来
                        serialReceivedData.append(QString::fromUtf8(data));
                        show_log->append("收到数据: " + QString::fromUtf8(data));
                    }
                });
            }
        }
        else{
            show_log->append("----串口已打开失败: " + burnComPort);
        }
    }
    else{
        if (serialPort && serialPort->isOpen()) {
            
            disconnect(serialPort, &QSerialPort::readyRead, this, nullptr);
            serialPort->close();
        }
        
        
        if (serialReceivedData.contains("version", Qt::CaseInsensitive)) {
            contentEdit->setText("软复位成功，检测到版本信息");
            resultCombo->setCurrentIndex(1); // 设置为正常
            show_log->append("软复位测试通过：检测到版本信息");
        } else {
            contentEdit->setText("软复位失败，未检测到版本信息");
            resultCombo->setCurrentIndex(2); // 设置为异常
            show_log->append("软复位测试失败：未检测到版本信息");
        }
    }

    
}

void MainWindow::onTestBootTime(int row, bool on, int voltage)
{
    QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 3));
    QProgressBar *progressBar = qobject_cast<QProgressBar*>(table_widget->cellWidget(row, 4));
    QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 5));
    
    if (on) {
        // 上电操作
        if (contentEdit) {
            contentEdit->setText("设备上电中...");
        }
        if (progressBar) {
            progressBar->setValue(30);
        }

        if (!bluSerial->setVoltage(voltage)) {
            show_log->append(QString("设置源电压失败: %1 mV").arg(voltage));
        }
        else{
            show_log->append(QString("已设置源电压为 %1 mV").arg(voltage));
        }
        
        bluSerial->toggleDUTPower(false);
        QThread::msleep(1000);

        if (!serialPort) {
            serialPort = new QSerialPort(this);
        }
        if (!serialPort->isOpen()) {
            QString portName = burnComPort; // 可以从设置中读取或使用固定值
            serialPort->setPortName(portName);
            serialPort->setBaudRate(921600);
            if (serialPort->open(QIODevice::ReadWrite)) {
                // 禁用DTR和RTS信号，避免影响设备启动
                serialPort->setDataTerminalReady(false);
                serialPort->setRequestToSend(false);
                serialPort->clear();
                show_log->append("串口已打开: " + portName);
               
                connect(serialPort, &QSerialPort::readyRead, this, [this]() {
                    QByteArray data = serialPort->readAll();
                    if (!data.isEmpty()) {
                        // 记录首次收到数据的时间
                        if (!hasReceivedFirstResponse) {
                            firstResponseTime = QDateTime::currentDateTime();
                            hasReceivedFirstResponse = true;
                        }
                    }
                });
            } else {
                show_log->append("串口打开失败: " + portName);
            }
        }
        QThread::msleep(1000);
        
        // 上电前记录时间
        powerOnTime = QDateTime::currentDateTime();
        hasReceivedFirstResponse = false;
        
        bluSerial->toggleDUTPower(true);
    } else {
        // 断电操作
        // 关闭串口
        if (serialPort && serialPort->isOpen()) {
            
            disconnect(serialPort, &QSerialPort::readyRead, this, nullptr);
            serialPort->close();
        }
       
        // 计算并显示耗时
        if (hasReceivedFirstResponse) {
            int bootTime = powerOnTime.msecsTo(firstResponseTime);
            QString bootTimeStr = QString("上电耗时: %1 ms").arg(bootTime);
            if (contentEdit) {
                contentEdit->setText(bootTimeStr);
            }
            if (progressBar) {
                progressBar->setValue(100);
            }
            if (resultCombo) {
                resultCombo->setCurrentIndex(1 ); // 设置为正常
            }
            show_log->append(bootTimeStr);
        } else {
            if (contentEdit) {
                contentEdit->setText("未收到响应");
            }
            if (progressBar) {
                progressBar->setValue(0);
            }
            if (resultCombo) {
                resultCombo->setCurrentIndex(2); // 设置为异常
            }
            show_log->append("未收到任何响应，无法计算启动耗时");
        }
    }
}
void MainWindow::start_test_content_12()
{

}
void MainWindow::showSNInputDialog()
{
    allowEnterToStartTest = false;
    bool ok;
    QString text;
    bool hasAbnormalTest = false;
    bool hasNormalTest = false;
    for (int row = 0; row < table_widget->rowCount(); ++row) {
        QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 5));
        if (resultCombo) {
            if (resultCombo->currentIndex() == 2) { // 2表示"异常"
                hasAbnormalTest = true;
                break;
            } else if (resultCombo->currentIndex() == 1) { // 1表示"正常"
                hasNormalTest = true;
            }
        }
    }
    QInputDialog inputDialog(this);
    if (hasAbnormalTest) {
        inputDialog.setWindowTitle(tr("测试异常 - 扫描SN"));
        inputDialog.setLabelText(tr("<span style='font-size:14pt; font-weight:bold; color:red;'>测试异常！请注意检查后再录入SN:</span>"));
        inputDialog.setStyleSheet("QInputDialog { background-color: #ffdddd; }");
    } else if (hasNormalTest) {
        inputDialog.setWindowTitle(tr("测试正常 - 扫描SN"));
        inputDialog.setLabelText(tr("<span style='font-size:14pt; font-weight:bold; color:green;'>测试正常！请扫描设备SN:</span>"));
        inputDialog.setStyleSheet("QInputDialog { background-color: #ddffdd; }");
    } else {
        inputDialog.setWindowTitle(tr("扫描SN"));
        inputDialog.setLabelText(tr("请输入设备SN:"));
    }
    inputDialog.setTextEchoMode(QLineEdit::Normal);
    do {
        ok = inputDialog.exec();
        text = inputDialog.textValue();
        if (!ok) {
            allowEnterToStartTest = true;
            return;
        }
    } while (!text.startsWith("LS") && text != "123");
    deviceSN = text;
    deviceId_ = "";
    this->setWindowTitle( Title_test + QString(" - SN: %1").arg(deviceSN));
    QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(0, 5));
    if (resultCombo) {
        QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(0, 3));
        if (contentEdit) {
            contentEdit->setText(deviceSN);
        }
    }
    QTimer::singleShot(1000, this, [this, hasAbnormalTest, hasNormalTest]() {
        QMessageBox confirmBox;
        confirmBox.setWindowTitle("===确认测试====");
        if (hasAbnormalTest) {
            confirmBox.setText(QString("警告：存在测试异常项！\nSN已输入: %1\n准备开始测试?").arg(deviceSN));
            confirmBox.setStyleSheet("QMessageBox { background-color: #ffdddd; }"
                                   "QLabel { color: red; font-weight: bold; font-size: 14pt; }");
        } else if (hasNormalTest) {
            confirmBox.setText(QString("测试正常完成！\nSN已输入: %1\n准备开始测试?").arg(deviceSN));
            confirmBox.setStyleSheet("QMessageBox { background-color: #ddffdd; }"
                                   "QLabel { color: green; font-weight: bold; font-size: 14pt; }");
        } else {
            confirmBox.setText(QString("SN已输入: %1\n准备开始测试?").arg(deviceSN));
        }
        confirmBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        confirmBox.setDefaultButton(QMessageBox::Ok);
        confirmBox.setButtonText(QMessageBox::Ok, "开始测试");
        confirmBox.setButtonText(QMessageBox::Cancel, "取消测试");
        int ret = confirmBox.exec();
        allowEnterToStartTest = true;
        if (ret == QMessageBox::Ok) {
            QTimer::singleShot(100, this, [this]() {
                start_test_content();
            });
        } else {
            QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(0, 3));
            if (contentEdit) {
                contentEdit->setText("");
            }
            QTimer::singleShot(100, this, [this]() {
                showSNInputDialog();
            });
        }
    });
}

bool MainWindow::runMysqlQuery(const QString &sn, const QString &site, const QString &result, 
    const QString &chipid, const QString &log1, const QString &log2, const QString &log3, 
    QString &output)
{
    QString currentDir = QCoreApplication::applicationDirPath();
    QString mysqlPath = QDir::cleanPath(currentDir + "/test_mysql.exe");
    QString logJson = QString("{\"log1\":\"%1\",\"log2\":\"%2\",\"log3\":\"%3\"}")
                        .arg(log1)
                        .arg(log2)
                        .arg(log3);


    QStringList arguments;
    arguments << "insert" << sn << site << result << chipid << logJson;
    QString command = QString("%1 %2").arg(mysqlPath).arg(arguments.join(" "));
    QProcess process;
    process.start(mysqlPath, arguments);
    if (!process.waitForStarted(5000)) {
    output = "启动test_mysql进程失败";
    return false;
    }
    if (!process.waitForFinished(180000)) {
    output = "执行test_mysql命令超时";
    process.kill();
    return false;
    }
    QString stdOut = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString stdErr = QString::fromLocal8Bit(process.readAllStandardError());
    output = stdOut;
    if (!stdErr.isEmpty()) {
    output += "\n" + stdErr;
    }

    bool success = (process.exitCode() == 0);
    return success;
}





void MainWindow::saveTestLog()
{

    bool hasAbnormalTest = false;
    bool hasNormalTest = false;
    for (int row = 0; row < table_widget->rowCount(); ++row) {
        QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 5));
        if (resultCombo) {
            if (resultCombo->currentIndex() == 2) { // 2表示"异常"
                hasAbnormalTest = true;
                break;
            } else if (resultCombo->currentIndex() == 1) { // 1表示"正常"
                hasNormalTest = true;
            }
        }
    }
    QString logDirPath;
    if (hasAbnormalTest) {
        logDirPath = "./logs/test_log/异常";
    } else if (hasNormalTest) {
        logDirPath = "./logs/test_log/正常";
    } else {
        logDirPath = "./logs/test_log";  // 默认路径，未测试或其他情况
    }
    QDir logsDir(logDirPath);
    if (!logsDir.exists()) {
        logsDir.mkpath(".");
    }
    QString currentTime = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString fileName = QString("%1/%2_%3_%4.txt")
                          .arg(logDirPath)
                          .arg(currentTime)
                          .arg(deviceSN.isEmpty() ? "NoSN" : deviceSN)
                          .arg(deviceId_.isEmpty() ? "NoDeviceID" : deviceId_);
    
    QFile logFile(fileName);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&logFile);
        stream.setCodec("UTF-8");
        stream.setGenerateByteOrderMark(true); 
        stream << "Test Time: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
        stream << "Device SN: " << (deviceSN.isEmpty() ? "Not specified" : deviceSN) << "\n";
        stream << "Device ID: " << (deviceId_.isEmpty() ? "Not specified" : deviceId_) << "\n";
        stream << "Test Result: " << (hasAbnormalTest ? "Abnormal" : (hasNormalTest ? "Normal" : "Not tested")) << "\n";
        stream << "====================================\n\n";
        stream << "Table Content:\n";
        stream << "-----------------------------------\n";
        QStringList headers;
        for (int col = 0; col < table_widget->columnCount(); ++col) {
            if (!table_widget->isColumnHidden(col)) {
                headers << table_widget->horizontalHeaderItem(col)->text();
            }
        }
        stream << headers.join("\t") << "\n";
        for (int row = 0; row < table_widget->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < table_widget->columnCount(); ++col) {
                if (!table_widget->isColumnHidden(col)) {
                    QString cellContent;
                    QTableWidgetItem* item = table_widget->item(row, col);
                    if (item) {
                        cellContent = item->text();
                    } else {
                        QWidget* widget = table_widget->cellWidget(row, col);
                        if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                            cellContent = lineEdit->text();
                        } else if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                            cellContent = comboBox->currentText();
                        } else if (QProgressBar* progressBar = qobject_cast<QProgressBar*>(widget)) {
                            cellContent = QString::number(progressBar->value()) + "%";
                        }
                    }
                    rowData << cellContent;
                }
            }
            stream << rowData.join("\t") << "\n";
        }
        stream << "\n====================================\n\n";
        stream << "Test Log:\n";
        stream << "-----------------------------------\n";
        stream << show_log->toPlainText();
        
        logFile.close();
        
        show_log->append(QString("测试日志已保存至文件: %1").arg(fileName));
    } else {
        show_log->append(QString("无法创建日志文件: %1").arg(fileName));
    }


    if (Connect_Internet== "Y") {
        QString rawLog = show_log->toPlainText();
        QString log1 = rawLog;

        log1.replace("\\", "\\\\"); // 将反斜杠替换为两个反斜杠
        log1.replace('"', '\\"'); // 将双引号转义
        log1.replace('\n', "@"); // 换行符替换为@@@分隔符
        log1.replace('\r', ""); // 移除回车符
        log1.replace('\t', " "); // 制表符替换为空格
        for (int i = 0; i < 32; ++i) {
            log1.replace(QChar(i), " ");
        }

        QString log2;
        QStringList headerList;
        for (int col = 0; col < table_widget->columnCount(); ++col) {
            if (!table_widget->isColumnHidden(col)) {
                headerList << table_widget->horizontalHeaderItem(col)->text();
            }
        }
        log2 = headerList.join("|") + "@";
        
        for (int row = 0; row < table_widget->rowCount(); ++row) {
            QStringList rowDataList;
            for (int col = 0; col < table_widget->columnCount(); ++col) {
                if (!table_widget->isColumnHidden(col)) {
                    QString cellContent;
                    QTableWidgetItem* item = table_widget->item(row, col);
                    if (item) {
                        cellContent = item->text();
                    } else {
                        QWidget* widget = table_widget->cellWidget(row, col);
                        if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                            cellContent = lineEdit->text();
                        } else if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                            cellContent = comboBox->currentText();
                        } else if (QProgressBar* progressBar = qobject_cast<QProgressBar*>(widget)) {
                            cellContent = QString::number(progressBar->value()) + "%";
                        }
                    }
                    rowDataList << cellContent;
                }
            }
            log2 += rowDataList.join("|") + "@";
        }
        QString log3 = "";
        
        QString output;
        if (!runMysqlQuery(deviceSN.isEmpty() ? "NoSN" : deviceSN, "3", "PASS", deviceId_.isEmpty() ? "NoDeviceID" : deviceId_, log1, log2, log3, output)) {
            show_log->append(QString("执行失败： %1").arg(output));
        } else {
            show_log->append(QString("执行成功，输出： %1").arg(output));
        }
    }



}


void MainWindow::recordSNDeviceID()
{
    if (deviceSN.isEmpty() || deviceId_.isEmpty()) {
        return;
    }
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH:mm:ss");
    QDir logsDir("./logs");
    if (!logsDir.exists()) {
        logsDir.mkpath(".");
    }
    QFile recordFile("./logs/Record_sn.txt");
    if (recordFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&recordFile);
        stream.setCodec("UTF-8");
        stream << currentTime << "     SN:" << deviceSN << "     DeviceID:" << deviceId_ << "\n";
        recordFile.close();
    }
}

bool MainWindow::runIperfCommand(const QString &ipAddress, int port, int interval, int duration, QString &output)
{
    QString currentDir = QCoreApplication::applicationDirPath();
    QString iperfPath = QDir::cleanPath(currentDir + "/iperf.exe");
    
    // 构造命令行
    QString command = QString("\"%1\" -c %2 -p %3 -i %4 -t %5")
                        .arg(iperfPath)
                        .arg(ipAddress)
                        .arg(port)
                        .arg(interval)
                        .arg(duration);
    
    QProcess process;
    show_log->append("===CMD=====" + command);
    process.start(command);
    if (!process.waitForStarted(5000)) {
        output = "启动 iperf 进程失败"+ process.errorString();
        return false;
    }
    if (!process.waitForFinished(180000)) {
        output = "执行 iperf 命令超时";
        process.kill();
        return false;
    }
    
    QString stdOut = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString stdErr = QString::fromLocal8Bit(process.readAllStandardError());
    output = stdOut;
    
    if (!stdErr.isEmpty()) {
        output += "\n" + stdErr;
    }
    
    bool success = (process.exitCode() == 0);
    return success;  
}
void MainWindow::updateipValue_(const QString &ipresult,int row ,int testtime)
{
    bool shellSuccess = false;
    QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 3));

    QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 5));

    ipValue = ipresult;
    QString output;
    if (runIperfCommand(ipValue, 5001, 1, testtime, output)) {
        show_log->append("iperf 命令执行成功，输出：" + output);
        QRegularExpression regex(R"((\d+\.\d+)\s*Mbits\/sec)");
        QRegularExpressionMatchIterator i = regex.globalMatch(output);
        QString lastBandwidth; 
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            lastBandwidth = match.captured(1); 
        }
        if (!lastBandwidth.isEmpty()) {
            show_log->append("iperf 命令执行成功，输出：" + lastBandwidth + "Mbits/s");
            contentEdit->setText(" iperf_server ： " + lastBandwidth + "Mbits/s");
            if(lastBandwidth.toFloat()>=10){
                shellSuccess=true;
            }
        }
        else{
            show_log->append("iperf 命令执行失败，错误信息：");
        }
    } else {
        show_log->append("iperf 命令执行失败，错误信息：" + output);
    }
    resultCombo->setCurrentIndex(shellSuccess ? 1 : 2);
}