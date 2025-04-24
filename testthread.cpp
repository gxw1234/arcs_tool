#include "testthread.h"
#include <QDebug>
#include <QThread>
#include <QTableWidget>
#include <QString>
#include <QLineEdit>
#include <QComboBox>

#include <QFile>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>

TestThread::TestThread(QTableWidget *tableWidget, BLUSerial *bluSerial, QObject *parent, const QString &burnComPort)
    : QThread(parent), table_widget(tableWidget), m_bluSerial(bluSerial), stopRequested(false),
      m_burnCOM(burnComPort)
{
    // 输出当前的串口设置
    qDebug() << "在TestThread中使用的烧录COM口:" << m_burnCOM;
}

TestThread::~TestThread()
{
    requestStop();
    wait(); 
}

void TestThread::requestStop()
{
    stopRequested = true;
}

void TestThread::run()
{
    QThread::msleep(3000);
    emit updateLog("自动化测试开始");
    for (int row = 0; row < table_widget->rowCount(); ++row) {
        emit highlightRow(row);
        QTableWidgetItem* idItem = table_widget->item(row, 0);
        QString rowId = idItem ? idItem->text() : "";
        QString testName = table_widget->item(row, 1)->text().trimmed();
        emit updateLog(QString("开始测试行 %1 (ID: %2): %3").arg(row).arg(rowId).arg(testName));
        QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 5));
        QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 3));

        if (rowId == "A1") {
            emit updateProgress(row, 20);
            QString output;
            if (runCskBurn(m_burnCOM, 3000000, "0x0", "./fw/ap.bin", output)) {
                contentEdit->setText("AP固件烧录成功");
                resultCombo->setCurrentIndex(0);

            } else {
                contentEdit->setText("AP固件烧录失败");
                resultCombo->setCurrentIndex(1);

            }
            emit updateLog("AP固件烧录日志: " + output);
            emit updateProgress(row, 100);
            
        } 
        else if (rowId == "A2") {
            emit updateProgress(row, 20);
            QString output;

            if (runCskBurn(m_burnCOM, 3000000, "0xD00000", "./fw/cp.bin", output)) {
                contentEdit->setText("CP固件烧录成功");
                resultCombo->setCurrentIndex(0);
                emit updateLog("烧录成功: " + output);
            } else {
                contentEdit->setText("CP固件烧录失败");
                resultCombo->setCurrentIndex(1);
                emit updateLog("烧录失败: " + output);
            }
            emit updateLog("CP固件烧录日志: " + output);
            emit updateProgress(row, 100);
        }
        else if (rowId == "A3") {
            emit updateProgress(row, 20);
            QString output;
            if (runCskBurn(m_burnCOM, 3000000, "0x200000", "./fw/respak.bin", output)) {
                contentEdit->setText("REPAK固件烧录成功");
                resultCombo->setCurrentIndex(0);    
                emit updateLog("烧录成功: " + output);
            } else {
                contentEdit->setText("REPAK固件烧录失败");
                resultCombo->setCurrentIndex(1);
                emit updateLog("烧录失败: " + output);
            }
            emit updateLog("REPAK固件烧录日志: " + output);
            emit updateProgress(row, 100);

        }
        else if (rowId == "B1") {
            QThread::msleep(3000);
            emit updateProgress(row, 20);
            emit updateLog("正在检测ADB设备...");
            contentEdit->setText("正在获取设备信息...");
            QStringList devices = m_adbController.getDevices();
            if (!devices.isEmpty()) {
                QString deviceInfo = "\n检测到" + QString::number(devices.size()) + "个设备:\n";
                QString deviceId = devices.first();
                for (const QString &device : devices) {
                    deviceInfo += "- " + device + "\n";
                }
                emit updateLog("检测到ADB设备: " + deviceInfo);
                contentEdit->setText("设备ID：" + deviceId);
                resultCombo->setCurrentIndex(0);  
            } else {
                emit updateLog("未检测到ADB设备");
                contentEdit->setText("未发现ADB设备");
                resultCombo->setCurrentIndex(1);  
            }
            emit updateProgress(row, 100);
        }
        else if (rowId == "B2")
        {
            emit updateProgress(row, 20);
            emit updateLog("正在测试ADB文件传输...");
            contentEdit->setText("开始测试ADB push和pull");
            QString localPushFile = "./test_push.bin";
            QString remoteDir = "/RAW/SDRAW/40000000";
            QString remotePullFile = remoteDir + "/1400000";
            QString localPullFile = "./test_pull.bin";
            
            emit updateProgress(row, 40);
            emit updateLog("开始ADB push测试...");
            
            bool pushSuccess = false;
            QString pushResult = m_adbController.pushFile(localPushFile, remoteDir, &pushSuccess);
            emit updateLog("推送结果: " + pushResult);
 
            
            emit updateProgress(row, 70);
            emit updateLog("开始ADB pull测试...");
            
            
            bool pullSuccess = false;
            QString pullResult = m_adbController.pullFile(remotePullFile, localPullFile, &pullSuccess);
            emit updateLog("拉取结果: " + pullResult);
            
            QFile pulledFile(localPullFile);
            if (!pulledFile.exists()) {
                contentEdit->setText("文件验证失败");
                resultCombo->setCurrentIndex(1);  // 异常
                emit updateProgress(row, 0);
               
            }
            QString md5Original = calculateFileMD5(localPushFile);
            QString md5Downloaded = calculateFileMD5(localPullFile);
            emit updateLog("原始MD5: " + md5Original);
            emit updateLog("下载MD5: " + md5Downloaded);
            if (md5Original != md5Downloaded) {
                contentEdit->setText("文件MD5不匹配" + md5Original + " vs " + md5Downloaded);
                resultCombo->setCurrentIndex(1);  
            }
            else{
                resultCombo->setCurrentIndex(0);
                contentEdit->setText("ADB push/pull测试成功，MD5匹配" + md5Original);
            }
            // 测试完成后删除test_pull.bin文件
            QFile::remove(localPullFile);
            emit updateLog("已删除测试文件: " + localPullFile);
            emit updateProgress(row, 100);
        }
        else if(rowId == "B3"){
            emit updateProgress(row, 20);
            emit updateLog("正在测试ADB shell命令...");
            contentEdit->setText("测试背光控制");
            bool shellSuccess = false;
            QString command = "test_backlight_cmd set 100";
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if (output.contains("Return: 0")) {
                contentEdit->setText("背光设置成功，已自动终止");
                resultCombo->setCurrentIndex(0); // 设置为正常
            } else {
                contentEdit->setText("背光设置失败");
                resultCombo->setCurrentIndex(1); // 设置为异常
            }
            emit updateProgress(row, 100);
        }
        else if (rowId == "B4") {
            emit updateLog("正在测量电流...");
            emit updateLog("开始连续测量...");
            if (m_bluSerial && !m_bluSerial->startMeasurement()) {
                emit updateLog("开始测量失败");
                return;
            }
            QThread::msleep(5000);
            if (m_bluSerial && m_bluSerial->isConnected()) {
                QByteArray data = m_bluSerial->readData(15000); // 限制最多15000字节
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; // 创建一个临时变量传递给processSamples
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                    // 显示样本数量和平均值
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                        
                        // 计算功率 P = U * I
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; // 源电压（V）
                        double power = voltage * avgCurrent / 1000000.0; // 功率（W）
                        
                        // 统一使用mA单位显示
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                    
                        emit updateProgress(row, 100); // 设置进度为100%
                    } else {
                        emit updateLog("未收集到有效样本");
                        emit updateProgress(row, 0); // 失败时重置进度
                    }
                } else {
                    emit updateLog("未读取到数据");
                    emit updateProgress(row, 0); // 失败时重置进度
                }
            } else {
                emit updateLog("BLU设备未连接");
                emit updateProgress(row, 0); // 失败时重置进度
            }
            
            // 停止测量
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }
            emit updateLog("已完成行 " + QString::number(row) + ": 测量电流");

        }
        else if (rowId == "C1") {
            emit updateProgress(row, 20); 
            if (!m_bluSerial->setVoltage(3500)) {
                emit updateLog("设置源电压失败");
            }
            else{
                emit updateLog("已设置源电压为3.5V");
            }
            emit updateProgress(row, 40);

            // 关闭DUT电源
            if (!m_bluSerial->toggleDUTPower(false)) {
                emit updateLog("关闭DUT电源失败");

            }else{
                emit updateLog("已关闭DUT电源");
            }
            emit updateProgress(row, 60);
            QThread::msleep(500);

            // 打开DUT电源
            if (!m_bluSerial->toggleDUTPower(true)) {
                emit updateLog("打开DUT电源失败");
               
            }else{
                emit updateLog("已打开DUT电源");
            }
            emit updateProgress(row, 80);
        }
        else if (rowId == "C2"){
            emit updateProgress(row, 20);
            m_bluSerial->toggleDUTPower(false);
            QThread::msleep(100);
            m_bluSerial->toggleDUTPower(true);
            QThread::msleep(3000);
            emit updateProgress(row, 40);
            emit updateLog("正在检测ADB设备...");
            contentEdit->setText("正在获取设备信息...");
            QStringList devices = m_adbController.getDevices();
            if (!devices.isEmpty()) {
                QString deviceInfo = "\n检测到" + QString::number(devices.size()) + "个设备:\n";
                QString deviceId = devices.first();
                for (const QString &device : devices) {
                    deviceInfo += "- " + device + "\n";
                }
                emit updateLog("检测到ADB设备: " + deviceInfo);
                contentEdit->setText("设备ID：" + deviceId);
                resultCombo->setCurrentIndex(0);  
            } else {
                emit updateLog("未检测到ADB设备");
                contentEdit->setText("未发现ADB设备");
                resultCombo->setCurrentIndex(1);  
            }
            emit updateProgress(row, 100);
        }
        else if (rowId == "D1") {
            emit updateBootTime(row, true, 3500); 
            QThread::msleep(4000);
            emit updateBootTime(row, false);
        }
        else if (rowId == "D2") {
            emit updateBootTime(row, true, 4000); 
            QThread::msleep(4000);
            emit updateBootTime(row, false);
        }
        else if (rowId == "D3") {
            emit updateBootTime(row, true, 4500); 
            QThread::msleep(4000);
            emit updateBootTime(row, false);
        }
        else if (rowId == "D4") {
            emit updateBootTime(row, true, 5000); 
            QThread::msleep(4000);
            emit updateBootTime(row, false);
        }
        else if (rowId == "E1") {
            emit updateProgress(row, 20);
            if (!m_bluSerial->setVoltage(3500)) {
                emit updateLog("设置源电压失败");
            }
            else{
                emit updateLog("已设置源电压为3.5V");
            }
            QThread::msleep(2000);
            emit updateLog("开始连续测量...");
            if (m_bluSerial && !m_bluSerial->startMeasurement()) {
                emit updateLog("开始测量失败");
                return;
            }
            QThread::msleep(2000);
            emit updateProgress(row, 40);
            if (m_bluSerial && m_bluSerial->isConnected()) {
                QByteArray data = m_bluSerial->readData(0); // 限制最多15000字节
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; // 创建一个临时变量传递给processSamples
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                    // 显示样本数量和平均值
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                        // 计算功率 P = U * I
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; // 源电压（V）
                        double power = voltage * avgCurrent / 1000000.0; // 功率（W）
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        
                        double currentInMA = avgCurrent/1000.0; // 转换为mA单位
                        if (currentInMA < 180) {
                            contentEdit->setText(QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(0); // 设置为正常
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            contentEdit->setText(QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); // 设置为异常
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        contentEdit->setText("未收集到有效样本");
                        resultCombo->setCurrentIndex(1); // 设置为异常
                    }
                } else {
                    emit updateLog("未读取到数据");
                    contentEdit->setText("未读取到数据");
                    resultCombo->setCurrentIndex(1); // 设置为异常
                }
            } else {
                emit updateLog("BLU设备未连接");
                contentEdit->setText("BLU设备未连接");
                resultCombo->setCurrentIndex(1); // 设置为异常
            }
            
            
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }
            emit updateProgress(row, 100); // 设置进度为100%
            emit updateLog("已完成行 " + QString::number(row) + ": 测量电流");
          
        }
        else if (rowId == "E2") {
            emit updateProgress(row, 20);
            if (!m_bluSerial->setVoltage(4000)) {
                emit updateLog("设置源电压失败");
            }
            else{
                emit updateLog("已设置源电压为4V");
            }
            QThread::msleep(2000);
            emit updateLog("开始连续测量...");
            if (m_bluSerial && !m_bluSerial->startMeasurement()) {
                emit updateLog("开始测量失败");
                return;
            }
            QThread::msleep(2000);
            emit updateProgress(row, 40);
            if (m_bluSerial && m_bluSerial->isConnected()) {
                QByteArray data = m_bluSerial->readData(0); // 限制最多15000字节
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; // 创建一个临时变量传递给processSamples
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                    // 显示样本数量和平均值
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                        // 计算功率 P = U * I
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; // 源电压（V）
                        double power = voltage * avgCurrent / 1000000.0; // 功率（W）
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        
                        double currentInMA = avgCurrent/1000.0; // 转换为mA单位
                        if (currentInMA < 180) {
                            contentEdit->setText(QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(0); // 设置为正常
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            contentEdit->setText(QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); // 设置为异常
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        contentEdit->setText("未收集到有效样本");
                        resultCombo->setCurrentIndex(1); // 设置为异常
                    }
                } else {
                    emit updateLog("未读取到数据");
                    contentEdit->setText("未读取到数据");
                    resultCombo->setCurrentIndex(1); // 设置为异常
                }
            } else {
                emit updateLog("BLU设备未连接");
                contentEdit->setText("BLU设备未连接");
                resultCombo->setCurrentIndex(1); // 设置为异常
            }
            
            
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }
            emit updateProgress(row, 100); // 设置进度为100%
            emit updateLog("已完成行 " + QString::number(row) + ": 测量电流");
          
        }
        else if (rowId == "E3") {
            emit updateProgress(row, 20);
            if (!m_bluSerial->setVoltage(5000)) {
                emit updateLog("设置源电压失败");
            }
            else{
                emit updateLog("已设置源电压为5V");
            }
            QThread::msleep(2000);
            emit updateLog("开始连续测量...");
            if (m_bluSerial && !m_bluSerial->startMeasurement()) {
                emit updateLog("开始测量失败");
                return;
            }
            QThread::msleep(2000);
            emit updateProgress(row, 40);
            if (m_bluSerial && m_bluSerial->isConnected()) {
                QByteArray data = m_bluSerial->readData(0); // 限制最多15000字节
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; // 创建一个临时变量传递给processSamples
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                    // 显示样本数量和平均值
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                        // 计算功率 P = U * I
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; // 源电压（V）
                        double power = voltage * avgCurrent / 1000000.0; // 功率（W）
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        
                        double currentInMA = avgCurrent/1000.0; // 转换为mA单位
                        if (currentInMA < 180) {
                            contentEdit->setText(QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(0); // 设置为正常
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            contentEdit->setText(QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); // 设置为异常
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        contentEdit->setText("未收集到有效样本");
                        resultCombo->setCurrentIndex(1); // 设置为异常
                    }
                } else {
                    emit updateLog("未读取到数据");
                    contentEdit->setText("未读取到数据");
                    resultCombo->setCurrentIndex(1); // 设置为异常
                }
            } else {
                emit updateLog("BLU设备未连接");
                contentEdit->setText("BLU设备未连接");
                resultCombo->setCurrentIndex(1); // 设置为异常
            }
            
            
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }
            emit updateProgress(row, 100); // 设置进度为100%
            emit updateLog("已完成行 " + QString::number(row) + ": 测量电流");
          
        }
        else if (rowId == "E4") {
            emit updateProgress(row, 20);

            bool shellSuccess = false;
            QString command = "test_backlight_cmd on";
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if (output.contains("Return: 0")) {
                emit updateLog("背光设置成功");
            } else {
                emit updateLog("背光设置失败");

            }
            QThread::msleep(100);
            output = m_adbController.executeShellCommand("test_backlight_cmd set 0", &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if (output.contains("Return: 0")) {
                emit updateLog("背光设置成功 0");
            } else {
                emit updateLog("背光设置失败 0");
            }
            QThread::msleep(2000);
            emit updateLog("开始连续测量...");
            if (m_bluSerial && !m_bluSerial->startMeasurement()) {
                emit updateLog("开始测量失败");
                return;
            }
            QThread::msleep(2000);
            emit updateProgress(row, 40);
            if (m_bluSerial && m_bluSerial->isConnected()) {
                QByteArray data = m_bluSerial->readData(0); // 限制最多15000字节
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; // 创建一个临时变量传递给processSamples
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                    // 显示样本数量和平均值
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                        // 计算功率 P = U * I
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; // 源电压（V）
                        double power = voltage * avgCurrent / 1000000.0; // 功率（W）
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        
                        double currentInMA = avgCurrent/1000.0; // 转换为mA单位
                        if (currentInMA < 180) {
                            contentEdit->setText(QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(0); // 设置为正常
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            contentEdit->setText(QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); // 设置为异常
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        contentEdit->setText("未收集到有效样本");
                        resultCombo->setCurrentIndex(1); // 设置为异常
                    }
                } else {
                    emit updateLog("未读取到数据");
                    contentEdit->setText("未读取到数据");
                    resultCombo->setCurrentIndex(1); // 设置为异常
                }
            } else {
                emit updateLog("BLU设备未连接");
                contentEdit->setText("BLU设备未连接");
                resultCombo->setCurrentIndex(1); // 设置为异常
            }
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }

            m_adbController.executeShellCommand("test_reboot_cmd", &shellSuccess);
            QThread::msleep(2000);

            emit updateProgress(row, 100); // 设置进度为100%
            emit updateLog("已完成行 " + QString::number(row) + ": 测量电流");

            
          
        }
        else if (rowId == "E5") {
            emit updateProgress(row, 20);
            bool shellSuccess = false;
            QString command = "test_enter_age";

            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if (output.contains("Return: 0")) {
                emit updateLog("满载设置成功");
            } else {
                emit updateLog("满载设置失败");

            }
            QThread::msleep(2000);
            emit updateLog("开始连续测量...");
            if (m_bluSerial && !m_bluSerial->startMeasurement()) {
                emit updateLog("开始测量失败");
                return;
            }

            QThread::msleep(2000);
            emit updateProgress(row, 40);
            if (m_bluSerial && m_bluSerial->isConnected()) {
                QByteArray data = m_bluSerial->readData(0); // 限制最多15000字节
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; // 创建一个临时变量传递给processSamples
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                    // 显示样本数量和平均值
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                        // 计算功率 P = U * I
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; // 源电压（V）
                        double power = voltage * avgCurrent / 1000000.0; // 功率（W）
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        
                        double currentInMA = avgCurrent/1000.0; // 转换为mA单位
                        if (currentInMA < 180) {
                            contentEdit->setText(QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(0); // 设置为正常
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            contentEdit->setText(QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); // 设置为异常
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        contentEdit->setText("未收集到有效样本");
                        resultCombo->setCurrentIndex(1); // 设置为异常
                    }
                } else {
                    emit updateLog("未读取到数据");
                    contentEdit->setText("未读取到数据");
                    resultCombo->setCurrentIndex(1); // 设置为异常
                }
            } else {
                emit updateLog("BLU设备未连接");
                contentEdit->setText("BLU设备未连接");
                resultCombo->setCurrentIndex(1); // 设置为异常
            }
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }

            m_adbController.executeShellCommand("test_reboot_cmd", &shellSuccess);
            QThread::msleep(2000);


            emit updateProgress(row, 100); // 设置进度为100%
            emit updateLog("已完成行 " + QString::number(row) + ": 测量电流");
        }
        else if (rowId == "F1")
        {
            emit updateProgress(row, 20);
            emit updateLog("正在软复位...");
            emit updateSoftReset(row, true);
            QThread::msleep(100);
            QString command = "test_reboot_cmd";
            bool shellSuccess = false;
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);

            QThread::msleep(1000);
            emit updateSoftReset(row, false);
            emit updateProgress(row, 100);
        }
        else if (rowId == "F2")
        {
            emit updateProgress(row, 20);
            emit updateLog("正在软复位...");
            emit updateSoftReset(row, true);
            QThread::msleep(100);
            QString command = "test_wdt_enable 1000";
            bool shellSuccess = false;
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);

            QThread::msleep(2000);
            emit updateSoftReset(row, false);
            emit updateProgress(row, 100);
        }
        else if (rowId == "G1")
        {

            QThread::msleep(1000);
            emit updateProgress(row, 20);
            QString command = "test_wifi_cmd scan";
            bool shellSuccess = false;
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            
            // 解析WiFi扫描结果，查找"gxw"网络
            bool foundGxwNetwork = false;
            int rssiValue = 0;
            
            QRegExp regex("index\\[\\d+\\]: channel \\d+, bssid [0-9A-F:]+, rssi\\s+(-?\\d+), auth .+ SSID (\\S+.*)");
            int pos = 0;
            
            while ((pos = regex.indexIn(output, pos)) != -1) {
                QString rssi = regex.cap(1);
                QString ssid = regex.cap(2);
                
                pos += regex.matchedLength();
                
                emit updateLog(QString("解析到SSID: %1, RSSI: %2").arg(ssid).arg(rssi));
                
                if (ssid.contains("gxw")) {
                    foundGxwNetwork = true;
                    rssiValue = rssi.toInt();
                    emit updateLog(QString("找到gxw网络，RSSI值: %1").arg(rssiValue));
                    break;
                }
            }
            
            if (foundGxwNetwork) {
                contentEdit->setText(QString("检测到gxw网络，信号强度: %1 dBm").arg(rssiValue));
                if (rssiValue > -150) {
                    resultCombo->setCurrentIndex(0); 
                    emit updateLog("WiFi测试通过: 信号强度良好");
                } else {
                    resultCombo->setCurrentIndex(1); 
                    emit updateLog("WiFi测试不通过: 信号强度较弱");
                }
            } else {
                contentEdit->setText("未检测到gxw网络");
                resultCombo->setCurrentIndex(1); // 设置为异常
                emit updateLog("WiFi测试不通过: 未检测到gxw网络");
            }
            emit updateProgress(row, 100);
        }
        else if (rowId == "G2")
        {
            emit updateProgress(row, 20);
            emit updateLog("正在测试ADB文件传输...");
            QString localPushFile = "./audio_1khz_3s.mp3";
            QString remoteDir = "/SD:/adb/audio_1khz_3s.mp3";
            emit updateProgress(row, 40);
            emit updateLog("开始ADB push测试...");
            bool pushSuccess = false;
            QString pushResult = m_adbController.pushFile(localPushFile, remoteDir, &pushSuccess);
            emit updateLog("推送结果: " + pushResult);
            QThread::msleep(100);
            QString command = "test_seeding_cmd";
            bool shellSuccess = false;
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            QStringList maxFreqs;
            QRegExp maxFreqRegex("最大频率\\(m_nMaxFreq\\): (\\d+) Hz");
            int pos = 0;
            while ((pos = maxFreqRegex.indexIn(output, pos)) != -1) {
                maxFreqs << maxFreqRegex.cap(1); 
                pos += maxFreqRegex.matchedLength(); 
            }
            QString resultText = "测试完成";
            bool testPassed = false;
            if (maxFreqs.size() > 0) {
                int firstChannelFreq = maxFreqs[0].toInt();
                testPassed = (firstChannelFreq > 980);
                resultText = QString("最大频率值: 通道0: %1Hz").arg(firstChannelFreq);
                if (maxFreqs.size() > 1) {
                    for (int i = 1; i < maxFreqs.size(); ++i) {
                        resultText += QString(", 通道%1: %2Hz").arg(i).arg(maxFreqs[i]);
                    }
                }
                if (testPassed) {
                    resultText += " (通过)";
                    resultCombo->setCurrentIndex(0); 
                } else {
                    resultText += " (不通过)";
                    resultCombo->setCurrentIndex(1); 
                }
            } else {
                resultText = "未检测到频率值 (不通过)";
                resultCombo->setCurrentIndex(1);
            }
            emit updateProgress(row, 100);
            contentEdit->setText(resultText);
        }
        else if (rowId == "S0") {
            emit updateProgress(row, 20);
            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName, contentEdit, resultCombo]() {
                QString message = "进入手动模式，按回车键进入手动模式";
                QMessageBox msgBox;
                msgBox.setWindowTitle("测试确认");
                msgBox.setText(message);
                QPushButton *passButton = msgBox.addButton("进入手动模式(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("退出手动模式(Fail)", QMessageBox::RejectRole);
                msgBox.exec();
                if (msgBox.clickedButton() == passButton) {
                    contentEdit->setText("进入手动模式");
                    resultCombo->setCurrentIndex(0); // 设置为正常
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    contentEdit->setText("退出手动模式");
                    resultCombo->setCurrentIndex(1); // 设置为异常
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
            }, Qt::BlockingQueuedConnection); 

            emit updateProgress(row, 100);
        }
            
        else if (rowId == "S1") {
            emit updateProgress(row, 20);
            bool shellSuccess = false;
            QString command = "test_backlight_cmd set 20";
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if (output.contains("Return: 0")) {

                emit updateLog("背光设置成功: " );
            } else {
                emit updateLog("背光设置失败: " );
            }
            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName, contentEdit, resultCombo]() {
                QString message = "请确认测试项: " + testName  + "背光是否有变化";
                QMessageBox msgBox;
                msgBox.setWindowTitle("测试确认");
                msgBox.setText(message);
              
                QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
                msgBox.exec(); 
                if (msgBox.clickedButton() == passButton) {
                    resultCombo->setCurrentIndex(0);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
                emit updateProgress(row, 100);
            }, Qt::BlockingQueuedConnection); 
        }
        else if (rowId == "S2") {
            emit updateProgress(row, 20);
            bool shellSuccess = false;
            QString command = "test_backlight_cmd set 50";
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if (output.contains("Return: 0")) {
                emit updateLog("背光设置成功: " );
            } else {
                emit updateLog("背光设置失败: " );
            }
            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName, contentEdit, resultCombo]() {
                QString message = "请确认测试项: " + testName  + "背光是否有变化";
                QMessageBox msgBox;
                msgBox.setWindowTitle("测试确认");
                msgBox.setText(message);
              
                QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
                msgBox.exec(); 
                if (msgBox.clickedButton() == passButton) {
                    resultCombo->setCurrentIndex(0);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
                emit updateProgress(row, 100);
            }, Qt::BlockingQueuedConnection); 
            
        }
        else if (rowId == "S3") {
            emit updateProgress(row, 20);
            bool shellSuccess = false;
            QString command = "test_backlight_cmd set 100";
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if (output.contains("Return: 0")) {
                emit updateLog("背光设置成功: " );
            } else {
                emit updateLog("背光设置失败: " );
            }
            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName, contentEdit, resultCombo]() {
                QString message = "请确认测试项: " + testName  + "背光是否有变化";
                QMessageBox msgBox;
                msgBox.setWindowTitle("测试确认");
                msgBox.setText(message);
              
                QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
                msgBox.exec(); 
                if (msgBox.clickedButton() == passButton) {
                    resultCombo->setCurrentIndex(0);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
                emit updateProgress(row, 100);
            }, Qt::BlockingQueuedConnection); 
            
        }
        else if (rowId == "S4") {
            bool shellSuccess = false;
            QString output = m_adbController.executeShellCommand("test_ocr_down", &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            QThread::msleep(2000);
            QString output2 = m_adbController.executeShellCommand("test_ocr_up", &shellSuccess);
            emit updateLog("命令输出: \n" + output2);

            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName, contentEdit, resultCombo]() {
                QString message = "请确认测试项: " + testName  + "ocr是否有变化,播音是否正常";
                QMessageBox msgBox;
                msgBox.setWindowTitle("测试确认");
                msgBox.setText(message);
              
                QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
                msgBox.exec(); 
                if (msgBox.clickedButton() == passButton) {
                    resultCombo->setCurrentIndex(0);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
                emit updateProgress(row, 100);
            }, Qt::BlockingQueuedConnection); 


        }
        else if (rowId == "S4_1") {
            if (!m_bluSerial->setVoltage(3500)) {
                emit updateLog("设置源电压失败");
            }
            else{
                emit updateLog("已设置源电压为3.5V");
            }
            QThread::msleep(1000);


            bool shellSuccess = false;
            QString output = m_adbController.executeShellCommand("test_ocr_down", &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            QThread::msleep(2000);
            QString output2 = m_adbController.executeShellCommand("test_ocr_up", &shellSuccess);
            emit updateLog("命令输出: \n" + output2);

            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName, contentEdit, resultCombo]() {
                QString message = "请确认测试项: " + testName  + "ocr是否有变化,播音是否正常";
                QMessageBox msgBox;
                msgBox.setWindowTitle("测试确认");
                msgBox.setText(message);
              
                QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
                msgBox.exec(); 
                if (msgBox.clickedButton() == passButton) {
                    resultCombo->setCurrentIndex(0);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
                emit updateProgress(row, 100);
            }, Qt::BlockingQueuedConnection); 


        }
        else if (rowId == "S4_2") {
            if (!m_bluSerial->setVoltage(5000)) {
                emit updateLog("设置源电压失败");
            }
            else{
                emit updateLog("已设置源电压为3.5V");
            }
            QThread::msleep(1000);
            bool shellSuccess = false;
            QString output = m_adbController.executeShellCommand("test_ocr_down", &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            QThread::msleep(2000);
            QString output2 = m_adbController.executeShellCommand("test_ocr_up", &shellSuccess);
            emit updateLog("命令输出: \n" + output2);
            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName, contentEdit, resultCombo]() {
                QString message = "请确认测试项: " + testName  + "ocr是否有变化,播音是否正常";
                QMessageBox msgBox;
                msgBox.setWindowTitle("测试确认");
                msgBox.setText(message);
                QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
                msgBox.exec(); 
                if (msgBox.clickedButton() == passButton) {
                    resultCombo->setCurrentIndex(0);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
                emit updateProgress(row, 100);
            }, Qt::BlockingQueuedConnection); 


        }



        else if (rowId == "S5") {

            bool shellSuccess = false;
            QString output = m_adbController.executeShellCommand("test_ocr_caliaration_cmd enter", &shellSuccess);
            emit updateLog("命令输出: \n" + output);

            m_adbController.executeShellCommand("test_ocr_down", &shellSuccess);



            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName, contentEdit, resultCombo]() {
                QString message = "请确认测试项: " + testName  + "调节是否有变化";
                QMessageBox msgBox;
                msgBox.setWindowTitle("测试确认");
                msgBox.setText(message);
              
                QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
                msgBox.exec(); 
                if (msgBox.clickedButton() == passButton) {
                    resultCombo->setCurrentIndex(0);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
                emit updateProgress(row, 100);
            }, Qt::BlockingQueuedConnection); 

            m_adbController.executeShellCommand("test_ocr_up", &shellSuccess);


            QString output2 = m_adbController.executeShellCommand("test_ocr_caliaration_cmd close", &shellSuccess);
            emit updateLog("命令输出: \n" + output2);
        }
        else if (rowId == "scan_sn")
        {
            resultCombo->setCurrentIndex(0);
            emit updateProgress(row, 100);

        }
        else {
            emit updateProgress(row, 20); // 先将进度设为20%
            // // 需要在主线程中显示对话框
            // QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName]() {
            //     QString message = "请确认测试项: " + testName;
            //     QMessageBox msgBox;
            //     msgBox.setWindowTitle("测试确认");
            //     msgBox.setText(message);
            //     // 添加Pass和Fail两个按钮
            //     QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
            //     QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
            //     msgBox.exec(); // 显示对话框并等待用户响应
            //     // 根据用户选择更新进度、结果和日志
            //     if (msgBox.clickedButton() == passButton) {
            //         // 用户点击Pass，设置进度为100%
            //         emit updateProgress(row, 100);
            //         emit updateResult(row, "正常"); // 更新结果列为“正常”
            //         emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
            //     } else if (msgBox.clickedButton() == failButton) {
            //         // 用户点击Fail，设置进度为0或显示失败状态
            //         emit updateProgress(row, 0); // 将进度设置为0
            //         emit updateResult(row, "异常"); // 更新结果列为“异常”
            //         emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
            //     }
            // }, Qt::BlockingQueuedConnection); // 使用阻塞方式确保对话框处理完成后再继续
        }
        QThread::msleep(100);
    }
    emit updateLog("所有测试已完成");
}

QString TestThread::calculateFileMD5(const QString &filePath)

{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    if (hash.addData(&file)) {
        return hash.result().toHex();
    }
    
    return QString();
}

bool TestThread::runCskBurn(const QString &comPort, int baudRate, const QString &address, const QString &binFile, QString &output)
{
    QString currentDir = QCoreApplication::applicationDirPath();
    QString cskburnPath = QDir::cleanPath(currentDir + "/cskburn.exe");
    QString command = QString("\"%1\" -s %2 -b %3 %4 \"%5\"")
                       .arg(cskburnPath)
                       .arg(comPort)
                       .arg(baudRate)
                       .arg(address)
                       .arg(binFile);
    QProcess process;
    process.start(command);
    
   
    if (!process.waitForStarted(5000)) {
        output = "启动cskburn进程失败";
        return false;
    }
    if (!process.waitForFinished(80000)) {
        output = "执行cskburn命令超时";
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
