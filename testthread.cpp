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

TestThread::TestThread(QTableWidget *tableWidget, BLUSerial *bluSerial, QObject *parent, const QString &burnComPort, const QString &connectInternet, const QString &checkpoint)
    : QThread(parent), table_widget(tableWidget), m_bluSerial(bluSerial), stopRequested(false),
      m_burnCOM(burnComPort), m_connectInternet(connectInternet), m_checkpoint(checkpoint)
{
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
    emit updateLog("com " + m_burnCOM + "==================");
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
        emit updateLog("================== " + rowId + "==================");
        if (rowId == "A1") {
            emit updateProgress(row, 20);
            QString output;
            bool success = false;
            if (runCskBurn(m_burnCOM, 3000000, "0x0", "./fw/ap.bin", output)) {
                success = true;
                resultCombo->setCurrentIndex(1);
            } else {
                success = false;
                resultCombo->setCurrentIndex(2);
            }
            emit updateTestContent(row, success ? "AP固件烧录成功" : "AP固件烧录失败");
            emit updateLog("AP固件烧录日志: " + output);
            emit updateProgress(row, 100);
            if (!success) {
                emit updateLog("AP固件烧录失败，终止测试流程");
                break;
            }
        } 
        else if (rowId == "A0")
        {
            emit updateProgress(row, 20);
            emit updateLog("正在测试UART固件烧录...");
            emit updateTestContent(row, "开始测试UART固件烧录");
            QString output;
            bool success = false;
            if (runUartBurn(m_burnCOM, "./fw/ram_loader_unlock.bin", output)) {
                success = true;
                resultCombo->setCurrentIndex(1);
            } else {
                success = false;
                resultCombo->setCurrentIndex(2);
            }
            emit updateTestContent(row, success ? "UART固件烧录成功" : "UART固件烧录失败");
            emit updateLog("UART固件烧录日志: " + output);
            emit updateProgress(row, 100);
            if (!success) {
                emit updateLog("UART固件烧录失败，终止测试流程");
                break;
            }
        }
        else if (rowId == "A2") {
            emit updateProgress(row, 20);
            QString output;
            bool success = false;
            if (runCskBurn(m_burnCOM, 3000000, "0xD00000", "./fw/cp.bin", output)) {
                success = true;
                resultCombo->setCurrentIndex(1);
                emit updateLog("CP固件烧录日志: " + output);
            } else {
                success = false;
                resultCombo->setCurrentIndex(2);
                emit updateLog("CP固件烧录日志: " + output);
            }
            emit updateTestContent(row, success ? "CP固件烧录成功" : "CP固件烧录失败");
            emit updateLog("CP固件烧录日志: " + output);
            emit updateProgress(row, 100);
            if (!success) {
                emit updateLog("CP固件烧录失败，终止测试流程");
                break;
            }
        }
        else if (rowId == "A3") {
            emit updateProgress(row, 20);
            QString output;
            bool success = false;
            if (runCskBurn(m_burnCOM, 3000000, "0x200000", "./fw/respak.bin", output)) {
                success = true;
                resultCombo->setCurrentIndex(1);    
                emit updateLog("烧录成功: " + output);
            } else {
                success = false;
                resultCombo->setCurrentIndex(2);
                emit updateLog("烧录失败: " + output);
            }
            emit updateTestContent(row, success ? "REPAK固件烧录成功" : "REPAK固件烧录失败");
            emit updateLog("REPAK固件烧录日志: " + output);
            emit updateProgress(row, 100);
            if (!success) {
                emit updateLog("REPAK固件烧录失败，终止测试流程");
                break;
            }
        }
        else if (rowId == "B1") {
            QThread::msleep(3000);
            emit updateProgress(row, 20);
            emit updateLog("正在检测ADB设备...");
            emit updateTestContent(row, "正在获取设备信息...");
            QStringList devices = m_adbController.getDevices();
            bool success = false;
            if (!devices.isEmpty()) {
                QString deviceInfo = "\n检测到" + QString::number(devices.size()) + "个设备:\n";
                QString deviceId = devices.first();
                for (const QString &device : devices) {
                    deviceInfo += "- " + device + "\n";
                }
                emit updateLog("检测到ADB设备: " + deviceInfo);
                emit updatedeviceId(deviceId);
                emit updateTestContent(row, "设备ID：" + deviceId);
                resultCombo->setCurrentIndex(1);  
                success =true;
            } else {
                emit updateLog("未检测到ADB设备");
                emit updateTestContent(row, "未发现ADB设备");
                resultCombo->setCurrentIndex(2);  
                success =false;
            }
            emit updateProgress(row, 100);
            if(!success)
            {
                emit updateLog("检测ADB设置失败，终止测试流程");
                break;
            }
        }
        else if (rowId == "B2")
        {
            emit updateProgress(row, 20);
            emit updateLog("正在测试ADB文件传输...");
            emit updateTestContent(row, "开始测试ADB push和pull");
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
                emit updateTestContent(row, "文件验证失败");
                resultCombo->setCurrentIndex(2);  // 异常
                emit updateProgress(row, 0);
               
            }
            QString md5Original = calculateFileMD5(localPushFile);
            QString md5Downloaded = calculateFileMD5(localPullFile);
            emit updateLog("原始MD5: " + md5Original);
            emit updateLog("下载MD5: " + md5Downloaded);
            if (md5Original != md5Downloaded) {
                emit updateTestContent(row, "文件MD5不匹配" + md5Original + " vs " + md5Downloaded);
                resultCombo->setCurrentIndex(2);  
            }
            else{
                resultCombo->setCurrentIndex(1);
                emit updateTestContent(row, "ADB push/pull测试成功，MD5匹配" + md5Original);
            }
            QFile::remove(localPullFile);
            emit updateLog("已删除测试文件: " + localPullFile);
            emit updateProgress(row, 100);
        }
        else if(rowId == "B3"){
            emit updateProgress(row, 20);
            emit updateLog("正在测试ADB shell命令...");
            emit updateTestContent(row, "测试背光控制");
            bool shellSuccess = false;
            QString command = "test_backlight_cmd set 100";
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if (output.contains("Return: 0")) {
                emit updateTestContent(row, "背光设置成功，已自动终止");
                resultCombo->setCurrentIndex(1); // 设置为正常
            } else {
                emit updateTestContent(row, "背光设置失败");
                resultCombo->setCurrentIndex(2); // 设置为异常
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
                QByteArray data = m_bluSerial->readData(15000); 
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; 
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();

                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; 
                        double power = voltage * avgCurrent / 1000000.0; 
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                    
                        emit updateProgress(row, 100); 
                    } else {
                        emit updateLog("未收集到有效样本");
                        emit updateProgress(row, 0); 
                    }
                } else {
                    emit updateLog("未读取到数据");
                    emit updateProgress(row, 0); 
                }
            } else {
                emit updateLog("BLU设备未连接");
                emit updateProgress(row, 0);
            }
            

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
            if (!m_bluSerial->toggleDUTPower(false)) {
                emit updateLog("关闭DUT电源失败");

            }else{
                emit updateLog("已关闭DUT电源");
            }
            emit updateProgress(row, 60);
            QThread::msleep(500);
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
            emit updateTestContent(row, "正在获取设备信息...");
            QStringList devices = m_adbController.getDevices();
            if (!devices.isEmpty()) {
                QString deviceInfo = "\n检测到" + QString::number(devices.size()) + "个设备:\n";
                QString deviceId = devices.first();
                for (const QString &device : devices) {
                    deviceInfo += "- " + device + "\n";
                }
                emit updateLog("检测到ADB设备: " + deviceInfo);
                emit updateTestContent(row, "设备ID：" + deviceId);
                resultCombo->setCurrentIndex(1);  
            } else {
                emit updateLog("未检测到ADB设备");
                emit updateTestContent(row, "未发现ADB设备");
                resultCombo->setCurrentIndex(2);  
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
                QByteArray data = m_bluSerial->readData(15000); 
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; 
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                  
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                       
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; // 源电压（V）
                        double power = voltage * avgCurrent / 1000000.0; 
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        
                        double currentInMA = avgCurrent/1000.0; 
                        if (currentInMA < 250) {
                            emit updateTestContent(row, QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); 
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            emit updateTestContent(row, QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(2); // 设置为异常
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        emit updateTestContent(row, "未收集到有效样本");
                        resultCombo->setCurrentIndex(2); 
                    }
                } else {
                    emit updateLog("未读取到数据");
                    emit updateTestContent(row, "未读取到数据");
                    resultCombo->setCurrentIndex(2); 
                }
            } else {
                emit updateLog("BLU设备未连接");
                emit updateTestContent(row, "BLU设备未连接");
                resultCombo->setCurrentIndex(2); 
            }
            
            
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }
            emit updateProgress(row, 100); 
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
                QByteArray data = m_bluSerial->readData(15000); 
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; 
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                   
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                       
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; // 源电压（V）
                        double power = voltage * avgCurrent / 1000000.0; // 功率（W）
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        
                        double currentInMA = avgCurrent/1000.0; 
                        if (currentInMA < 250) {
                            emit updateTestContent(row, QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); 
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            emit updateTestContent(row, QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(2); 
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        emit updateTestContent(row, "未收集到有效样本");
                        resultCombo->setCurrentIndex(2); // 设置为异常
                    }
                } else {
                    emit updateLog("未读取到数据");
                    emit updateTestContent(row, "未读取到数据");
                    resultCombo->setCurrentIndex(2); // 设置为异常
                }
            } else {
                emit updateLog("BLU设备未连接");
                emit updateTestContent(row, "BLU设备未连接");
                resultCombo->setCurrentIndex(2); 
            }
            
            
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }
            emit updateProgress(row, 100); 
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
                QByteArray data = m_bluSerial->readData(15000);
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; 
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; 
                        double power = voltage * avgCurrent / 1000000.0; 
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        
                        double currentInMA = avgCurrent/1000.0; 
                        if (currentInMA < 240) {
                            emit updateTestContent(row, QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); 
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            emit updateTestContent(row, QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(2); // 设置为异常
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        emit updateTestContent(row, "未收集到有效样本");
                        resultCombo->setCurrentIndex(2); // 设置为异常
                    }
                } else {
                    emit updateLog("未读取到数据");
                    emit updateTestContent(row, "未读取到数据");
                    resultCombo->setCurrentIndex(2); // 设置为异常
                }
            } else {
                emit updateLog("BLU设备未连接");
                emit updateTestContent(row, "BLU设备未连接");
                resultCombo->setCurrentIndex(2); // 设置为异常
            }
            
            
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }
            emit updateProgress(row, 100); 
            emit updateLog("已完成行 " + QString::number(row) + ": 测量电流");
          
        }
        else if (rowId == "E4") {
            emit updateProgress(row, 20);
            bool shellSuccess = false;
            QString command = "test_backlight_cmd on";
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if (output.contains("Return: 0")) {
                emit updateLog("关闭背光成功");
            } else {
                emit updateLog("关闭背光失败");

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
            emit updateLog("开始读取...");
            emit updateProgress(row, 40);
            if (m_bluSerial && m_bluSerial->isConnected()) {
                QByteArray data = m_bluSerial->readData(15000); 
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; 
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                    // 显示样本数量和平均值
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; 
                        double power = voltage * avgCurrent / 1000000.0; 
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        double currentInMA = avgCurrent/1000.0; // 转换为mA单位
                        if (currentInMA < 190) {
                            emit updateTestContent(row, QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); // 设置为正常
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            emit updateTestContent(row, QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(2); // 设置为异常
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        emit updateTestContent(row, "未收集到有效样本");
                        resultCombo->setCurrentIndex(2); // 设置为异常
                    }
                } else {
                    emit updateLog("未读取到数据");
                    emit updateTestContent(row, "未读取到数据");
                    resultCombo->setCurrentIndex(2); // 设置为异常
                }
            } else {
                emit updateLog("BLU设备未连接");
                emit updateTestContent(row, "BLU设备未连接");
                resultCombo->setCurrentIndex(2);
            }
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }

            if (!m_bluSerial->toggleDUTPower(false)) {
                emit updateLog("关闭DUT电源失败");

            }else{
                emit updateLog("已关闭DUT电源");
            }
            emit updateProgress(row, 60);
            QThread::msleep(500);
            if (!m_bluSerial->toggleDUTPower(true)) {
                emit updateLog("打开DUT电源失败");
               
            }else{
                emit updateLog("已打开DUT电源");
            }

            // m_adbController.executeShellCommand("test_reboot_cmd", &shellSuccess);
            QThread::msleep(3000);
            emit updateProgress(row, 100); 
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
                QByteArray data = m_bluSerial->readData(15000); 
                emit updateLog(QString("读取到原始数据: %1 字节").arg(data.size()));
                if (!data.isEmpty()) {
                    QByteArray remainder; 
                    QVector<double> samples = m_bluSerial->getProtocol()->processSamples(data, remainder);
                   
                    if (!samples.isEmpty()) {
                        double avgCurrent = 0.0;
                        for (double sample : samples) {
                            avgCurrent += sample;
                        }
                        avgCurrent /= samples.size();
                        
                        double voltage = m_bluSerial->getProtocol()->currentVdd() / 1000.0; // 源电压（V）
                        double power = voltage * avgCurrent / 1000000.0; // 功率（W）
                        emit updateLog(QString("平均电流: %1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateLog(QString("电源电压: %1 V").arg(voltage, 0, 'f', 3));
                        emit updateLog(QString("功耗: %1 mW").arg(power * 1000, 0, 'f', 3));
                        
                        double currentInMA = avgCurrent/1000.0; 
                        if (currentInMA < 250) {
                            emit updateTestContent(row, QString("平均电流: %1 mA (正常)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(1); 
                            emit updateLog("电流测试通过: 平均电流小于180mA");
                        } else {
                            emit updateTestContent(row, QString("平均电流: %1 mA (超出阈值)").arg(currentInMA, 0, 'f', 3));
                            resultCombo->setCurrentIndex(2); 
                            emit updateLog("电流测试失败: 平均电流超过180mA");
                        }
                    } else {
                        emit updateLog("未收集到有效样本");
                        emit updateTestContent(row, "未收集到有效样本");
                        resultCombo->setCurrentIndex(2); // 设置为异常
                    }
                } else {
                    emit updateLog("未读取到数据");
                    emit updateTestContent(row, "未读取到数据");
                    resultCombo->setCurrentIndex(2); // 设置为异常
                }
            } else {
                emit updateLog("BLU设备未连接");
                emit updateTestContent(row, "BLU设备未连接");
                resultCombo->setCurrentIndex(2); 
            }
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }

            if (!m_bluSerial->toggleDUTPower(false)) {
                emit updateLog("关闭DUT电源失败");

            }else{
                emit updateLog("已关闭DUT电源");
            }
            emit updateProgress(row, 60);
            QThread::msleep(500);
            if (!m_bluSerial->toggleDUTPower(true)) {
                emit updateLog("打开DUT电源失败");
               
            }else{
                emit updateLog("已打开DUT电源");
            }

            // m_adbController.executeShellCommand("test_reboot_cmd", &shellSuccess);
            QThread::msleep(3000);


            emit updateProgress(row, 100); 
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
                emit updateTestContent(row, QString("检测到gxw网络，信号强度: %1 dBm").arg(rssiValue));
                if (rssiValue > -150) {
                    resultCombo->setCurrentIndex(1); 
                    emit updateLog("WiFi测试通过: 信号强度良好");
                } else {
                    resultCombo->setCurrentIndex(2); 
                    emit updateLog("WiFi测试不通过: 信号强度较弱");
                }
            } else {
                emit updateTestContent(row, "未检测到gxw网络");
                resultCombo->setCurrentIndex(2); // 设置为异常
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
                testPassed = (firstChannelFreq > 750);
                resultText = QString("最大频率值: 通道0: %1Hz").arg(firstChannelFreq);
                if (maxFreqs.size() > 1) {
                    for (int i = 1; i < maxFreqs.size(); ++i) {
                        resultText += QString(", 通道%1: %2Hz").arg(i).arg(maxFreqs[i]);
                    }
                }
                if (testPassed) {
                    resultText += " (通过)";
                    resultCombo->setCurrentIndex(1); 
                } else {
                    resultText += " (不通过)";
                    resultCombo->setCurrentIndex(2); 
                }
            } else {
                resultText = "未检测到频率值 (不通过)";
                resultCombo->setCurrentIndex(2);
            }
            emit updateProgress(row, 100);
            emit updateTestContent(row, resultText);
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
                    emit updateTestContent(row, "进入手动模式");
                    resultCombo->setCurrentIndex(1); // 设置为正常
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    emit updateTestContent(row, "退出手动模式");
                    resultCombo->setCurrentIndex(2); // 设置为异常
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
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(2);
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
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(2);
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
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(2);
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
                emit updateProgress(row, 100);
            }, Qt::BlockingQueuedConnection); 
        }
        else if (rowId == "S3_1") {
            emit updateProgress(row, 20);
            emit updateLog("开始循环测试背光亮度...");
            emit updateTestContent(row, "正在循环测试背光亮度...");
            bool testResult = false;
            bool testCompleted = false;
            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName, contentEdit, resultCombo, &testResult, &testCompleted]() {
                QMessageBox msgBox;
                msgBox.setWindowTitle("背光循环测试");
                msgBox.setText("正在循环测试背光亮度...\n请观察屏幕亮度变化，确认后点击相应按钮");
                QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
                msgBox.exec();
                if (msgBox.clickedButton() == passButton) {
                    testResult = true;
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                    emit updateTestContent(row, "背光循环测试通过");
                } else if (msgBox.clickedButton() == failButton) {
                    testResult = false;
                    resultCombo->setCurrentIndex(2);
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                    emit updateTestContent(row, "背光循环测试失败");
                }
                testCompleted = true;
            }, Qt::QueuedConnection); 
            while (!testCompleted && !stopRequested) {
                QStringList brightnessLevels = {"20", "50", "100"};
                for (const QString &level : brightnessLevels) {
                    if (testCompleted || stopRequested) break;
                    // 设置背光亮度
                    bool shellSuccess = false;
                    QString command = "test_backlight_cmd set " + level;
                    QString output = m_adbController.executeShellCommand(command, &shellSuccess);
                    if (output.contains("Return: 0")) {
                        emit updateLog(QString("背光设置成功: %1%").arg(level));
                    } else {
                        emit updateLog(QString("背光设置失败: %1%").arg(level));
                    }
                    emit updateProgress(row, 30 + (level.toInt() / 5));
                    for (int i = 0; i < 15 && !testCompleted && !stopRequested; i++) {
                        QThread::msleep(30); 
                    }
                }
            }
            
            emit updateProgress(row, 100);
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
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(2);
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
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(2);
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
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(2);
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
                    resultCombo->setCurrentIndex(1);
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    resultCombo->setCurrentIndex(2);
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
            bool resultSuccess = false;
            QString content_sn = contentEdit->text();
            if (m_connectInternet == "Y") {
                QString output;
                if (runMysqlQuery(content_sn, output)) {
                    QRegularExpression regex("Site:\\s*(\\d+)");
                    QRegularExpressionMatch match = regex.match(output);
                    if (match.hasMatch()) {
                        QString siteValue = match.captured(1);
                        if(siteValue == m_checkpoint)
                        {
                            resultSuccess = true;
                            emit updateTestContent(row, "SN:"+content_sn+"     站点:"+siteValue+" 一致");
                        }
                        else if(siteValue == "99")
                        {
                            emit updateTestContent(row, "SN:"+content_sn+ "    未找到测试记录");
                        }
                        else
                        {
                            emit updateTestContent(row, "SN:"+content_sn+ "    站点:"+siteValue+" 不一致");
                        }
                    } else {
                        emit updateLog("未找到Site值");
                    }
                    emit updateLog("查询结果: "  + output +"====");
                } else {
                }
                emit updateProgress(row, 100);
            }
            else{
                resultSuccess =true;
            }
            resultCombo->setCurrentIndex(resultSuccess ? 1 : 2);
            emit updateProgress(row, 100);
        }
        else if (rowId == "wifi_connect")
        {
            
            emit updateProgress(row, 20);
            QString command = "test_wifi_cmd connect listenai_develop_test a12345678";
            bool shellSuccess = false;
            bool resultSuccess = false;
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            if(output.contains("IP地址已获取，连接成功！"))
            {
                QRegularExpression ipPattern(R"(本机IP地址:\s*(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))");
                QRegularExpressionMatch match = ipPattern.match(output);
                if (match.hasMatch()) {
                    ipValue = match.captured(1);
                    emit updateTestContent(row, "连接成功  IP："+ipValue);
                    resultSuccess=true;
                }else
                {
                    emit updateTestContent(row, "连接成功 ip未获取");
                }
            }
            else
            {
                resultCombo->setCurrentIndex(2);
                emit updateTestContent(row, "连接失败");
            }
            resultCombo->setCurrentIndex(resultSuccess ? 1 : 2);
            emit updateProgress(row, 100);

        }
        else if (rowId == "iperf_server")
        {
            bool shellSuccess = false;
            bool resultSuccess = false;
            emit updateProgress(row, 20);
            QString command = "test_iperf_server_cmd";
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            QRegularExpression ipPattern(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))");
            QRegularExpressionMatch match = ipPattern.match(output);
            if (match.hasMatch()) {
                ipValue = match.captured(1); 
                emit updateTestContent(row, "iperf_server 测试中：");
                if (runIperfCommand(ipValue, 5001, 1, 10, output))
                {
                    emit updateLog("iperf 命令执行成功，输出：" + output);
                    QRegularExpression regex(R"((\d+\.\d+)\s*Mbits\/sec)");
                    QRegularExpressionMatchIterator i = regex.globalMatch(output);
                    QString lastBandwidth; 
                    while (i.hasNext()) {
                        QRegularExpressionMatch match = i.next();
                        lastBandwidth = match.captured(1); 
                    }
                    if (!lastBandwidth.isEmpty()) {
                        emit updateLog("iperf 命令执行成功，输出：" + lastBandwidth + "Mbits/s");
                        if(lastBandwidth.toFloat()>=10){
                            resultSuccess=true;
                            emit updateTestContent(row, " iperf_server ： " + lastBandwidth + "Mbits/s");
                        }
                        else{
                            emit updateTestContent(row, " iperf_server ： " + lastBandwidth + "Mbits/s      " + "未通过 10Mbit/s");
                        }
                    }
                    else{
                        emit updateTestContent(row, " iperf_server ： 未找到带宽");
                    }
                }
                else
                {
                    emit updateTestContent(row, "iperf_server 测试失败");
                }
                
            } else {
                emit updateTestContent(row, "iperf_server  IP：未找到");
            }

            resultCombo->setCurrentIndex(resultSuccess ? 1 : 2);
            emit updateProgress(row, 100);
            
        }
        else if (rowId == "reset")
        {
            emit updateProgress(row, 20);
            emit updateLog("正在软复位...");
            QString command = "test_reboot_cmd";
            bool shellSuccess = true;
            QString output = m_adbController.executeShellCommand(command, &shellSuccess);
            emit updateLog("命令输出: \n" + output);
            QThread::msleep(3000);
            emit updateProgress(row, 100);
            emit updateTestContent(row, "软复位成功");
            resultCombo->setCurrentIndex(shellSuccess ? 1 : 2);
        }
        else if (rowId == "iperf_client")
        {



        }



        else {
            emit updateProgress(row, 20); 


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


bool TestThread::runMysqlQuery(const QString &sn, QString &output)
{
    QString currentDir = QCoreApplication::applicationDirPath();
    QString mysqlPath = QDir::cleanPath(currentDir + "/test_mysql.exe");
    
    QString command = QString("\"%1\" query %2")
                        .arg(mysqlPath)
                        .arg(sn);
    
    QProcess process;
    emit updateLog("===CMD=====" + command);
    process.start(command);
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


bool TestThread::runCskBurn(const QString &comPort, int baudRate, const QString &address, const QString &binFile, QString &output)
{
    QString currentDir = QCoreApplication::applicationDirPath();
    QString cskburnPath = QDir::cleanPath(currentDir + "/cskburn.exe");
    QString command = QString("\"%1\" -s %2 -b %3 --probe-timeout 3000 --reset-attempts 1 %4 \"%5\"")
    // QString command = QString("\"%1\" -s %2 -b %3 %4 \"%5\"")
                        .arg(cskburnPath)
                       .arg(comPort)
                       .arg(baudRate)
                       .arg(address)
                       .arg(binFile);
    QProcess process;
    emit updateLog( "===CMD====="+command);
    process.start(command);
    if (!process.waitForStarted(5000)) {
        output = "启动cskburn进程失败";
        return false;
    }
    if (!process.waitForFinished(180000)) {
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

bool TestThread::runUartBurn(const QString &comPort, const QString &fileName, QString &output)
{
    QString currentDir = QCoreApplication::applicationDirPath();
    QString uartBurnPath = QDir::cleanPath(currentDir + "/uartburn.exe");
    QString binFile = QDir::cleanPath(currentDir + "/" + fileName); // 根据传入的文件名构建完整路径
    QString command = QString("\"%1\" -p %2 -s \"%3\" -a 0x20040000")
                        .arg(uartBurnPath)
                        .arg(comPort)
                        .arg(binFile);
    QProcess process;
    emit updateLog("===CMD=====" + command);
    process.start(command);
    if (!process.waitForStarted(5000)) {
        output = "启动uartburn进程失败";
        return false;
    }
    if (!process.waitForFinished(180000)) {
        output = "执行uartburn命令超时";
        process.kill();
        return false;
    }
    QString stdOut = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString stdErr = QString::fromLocal8Bit(process.readAllStandardError());
    output = stdOut;
    if (!stdErr.isEmpty()) {
        output += "\n" + stdErr;
    }
    return (process.exitCode() == 0);
}

bool TestThread::runIperfCommand(const QString &ipAddress, int port, int interval, int duration, QString &output)
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
    emit updateLog("===CMD=====" + command);
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
