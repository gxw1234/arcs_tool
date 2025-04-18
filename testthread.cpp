#include "testthread.h"
#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include <QProcess>
#include <QDir>
#include <QFileInfo>
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

    QThread::msleep(2000);
    emit updateLog("自动化测试开始");
    for (int row = 0; row < table_widget->rowCount(); ++row) {
        QString testName = table_widget->item(row, 0)->text().trimmed();
        emit updateLog("开始测试行 " + QString::number(row) + ": " + testName);
        
        if (row == 0) {
            // 第一行执行烧录操作
            QString output;
            emit updateProgress(row, 20); // 先将进度设为20%
            
            // 使用配置文件中的COM口进行烧录
            qDebug() << "正在使用" << m_burnCOM << "烧录固件";
            if (runCskBurn(m_burnCOM, 115200, "0x0", "d:/py/qtqbj/bin/test.bin", output)) {
                emit updateProgress(row, 100); // 成功时设置进度为100%
                emit updateResult(row, "正常");
                emit updateLog("烧录成功: " + output);
            } else {
                emit updateProgress(row, 0); // 失败时重置进度
                emit updateResult(row, "异常");
                emit updateLog("烧录失败: " + output);
            }
            emit updateLog("已完成行 " + QString::number(row) + ": " + testName);
        } 
        else if (row == 1) {
            emit updateLog("正在测量电流...");
            emit updateProgress(row, 20); // 先将进度设为20%
           
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
                        
                        // 将电流值作为测试结果
                        emit updateResult(row, QString("%1 mA").arg(avgCurrent/1000.0, 0, 'f', 3));
                        emit updateProgress(row, 100); // 设置进度为100%
                    } else {
                        emit updateLog("未收集到有效样本");
                        emit updateResult(row, "异常");
                        emit updateProgress(row, 0); // 失败时重置进度
                    }
                } else {
                    emit updateLog("未读取到数据");
                    emit updateResult(row, "异常");
                    emit updateProgress(row, 0); // 失败时重置进度
                }
            } else {
                emit updateLog("BLU设备未连接");
                emit updateResult(row, "异常");
                emit updateProgress(row, 0); // 失败时重置进度
            }
            
            // 停止测量
            if (m_bluSerial) {
                m_bluSerial->stopMeasurement();
            }
            
            emit updateLog("已完成行 " + QString::number(row) + ": 测量电流");

        }
        else {
            // 非第一行弹出对话框
            emit updateProgress(row, 20); // 先将进度设为20%
            
            // 需要在主线程中显示对话框
            QMetaObject::invokeMethod(QApplication::instance(), [this, row, testName]() {
                QString message = "请确认测试项: " + testName;
                
                QMessageBox msgBox;
                msgBox.setWindowTitle("测试确认");
                msgBox.setText(message);
                
                // 添加Pass和Fail两个按钮
                QPushButton *passButton = msgBox.addButton("通过(Pass)", QMessageBox::AcceptRole);
                QPushButton *failButton = msgBox.addButton("失败(Fail)", QMessageBox::RejectRole);
                msgBox.exec(); // 显示对话框并等待用户响应
                // 根据用户选择更新进度、结果和日志
                if (msgBox.clickedButton() == passButton) {
                    // 用户点击Pass，设置进度为100%
                    emit updateProgress(row, 100);
                    emit updateResult(row, "正常"); // 更新结果列为“正常”
                    emit updateLog("测试通过: " + testName + " (行" + QString::number(row) + ")");
                } else if (msgBox.clickedButton() == failButton) {
                    // 用户点击Fail，设置进度为0或显示失败状态
                    emit updateProgress(row, 0); // 将进度设置为0
                    emit updateResult(row, "异常"); // 更新结果列为“异常”
                    emit updateLog("测试失败: " + testName + " (行" + QString::number(row) + ")");
                }
            }, Qt::BlockingQueuedConnection); // 使用阻塞方式确保对话框处理完成后再继续
        }
        QThread::msleep(500);
    }
    
    emit updateLog("所有测试已完成");
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
    if (!process.waitForFinished(30000)) {
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

