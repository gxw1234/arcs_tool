#include "testthread.h"
#include <QDebug>
#include <QThread>
#include <QTableWidget>
#include <QString>
#include <QLineEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QFile>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QApplication>
#include <QMessageBox>
#include <QProgressBar>
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

    // QThread::msleep(4000);
    emit updateLog("自动化测试开始");
    
    for (int row = 0; row < table_widget->rowCount(); ++row) {
        // 获取当前行的ID和测试名称
        QTableWidgetItem* idItem = table_widget->item(row, 0);
        QString rowId = idItem ? idItem->text() : "";
        QString testName = table_widget->item(row, 1)->text().trimmed();
        emit updateLog(QString("开始测试行 %1 (ID: %2): %3").arg(row).arg(rowId).arg(testName));

        // 分别设置  结果  内容 进度
        QComboBox *resultCombo = qobject_cast<QComboBox*>(table_widget->cellWidget(row, 5));
        QLineEdit *contentEdit = qobject_cast<QLineEdit*>(table_widget->cellWidget(row, 3));
        QProgressBar *progressBar = qobject_cast<QProgressBar*>(table_widget->cellWidget(row, 4));
        if (rowId == "A1") {
            // QString output;
            // progressBar->setValue(20);
            // if (runCskBurn(m_burnCOM, 3000000, "0x0", "./fw/ap.bin", output)) {
            //     contentEdit->setText("AP固件烧录成功");
            //     resultCombo->setCurrentIndex(0);

            // } else {
            //     contentEdit->setText("AP固件烧录失败");
            //     resultCombo->setCurrentIndex(1);

            // }
            // emit updateLog("AP固件烧录日志: " + output);

            // progressBar->setValue(100);
            
        } 
        else if (rowId == "A2") {
            // QString output;

            // if (runCskBurn(m_burnCOM, 3000000, "0xE00000", "./fw/cp.bin", output)) {
            //     contentEdit->setText("CP固件烧录成功");
            //     resultCombo->setCurrentIndex(0);
            //     emit updateLog("烧录成功: " + output);
            // } else {
            //     contentEdit->setText("CP固件烧录失败");
            //     resultCombo->setCurrentIndex(1);
            //     emit updateLog("烧录失败: " + output);
            // }
            // emit updateLog("CP固件烧录日志: " + output);

        }
        else if (rowId == "A3") {
            // QString output;

            // if (runCskBurn(m_burnCOM, 3000000, "0x200000", "./fw/respak.bin", output)) {
            //     contentEdit->setText("REPAK固件烧录成功");
            //     resultCombo->setCurrentIndex(0);    
            //     emit updateLog("烧录成功: " + output);
            // } else {
            //     contentEdit->setText("REPAK固件烧录失败");
            //     resultCombo->setCurrentIndex(1);
            //     emit updateLog("烧录失败: " + output);
            // }
            // emit updateLog("REPAK固件烧录日志: " + output);

        }
        else if (rowId == "B1") {
            emit updateProgress(row, 20);
            emit updateLog("正在检测ADB设备...");
            contentEdit->setText("正在获取设备信息...");

            // 使用成员变量m_adbController而不是创建新实例
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
            QString remotePullFile = remoteDir + "/100000";
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
            
   
            // 验证拉取的文件
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
        QThread::msleep(500);
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
    
    // 等待进程完成，最多等待30秒
    if (!process.waitForStarted(5000)) {
        output = "启动cskburn进程失败";
        return false;
    }
    
    // 等待进程完成，可能需要更长的超时时间
    if (!process.waitForFinished(80000)) {
        output = "执行cskburn命令超时";
        process.kill();
        return false;
    }
    
    // 获取命令的标准输出和错误输出
    QString stdOut = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString stdErr = QString::fromLocal8Bit(process.readAllStandardError());
    
    output = stdOut;
    if (!stdErr.isEmpty()) {
        output += "\n" + stdErr;
    }
    
    // 检查进程的返回码
    bool success = (process.exitCode() == 0);
    
    return success;
}

