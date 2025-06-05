#include "adbcontroller.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QElapsedTimer>
#include <QTime>
#include <QThread>
#include <QCoreApplication>

ADBController::ADBController(QObject *parent) : QObject(parent)
{
    // 查找ADB可执行文件路径
    findADBPath();
}

ADBController::~ADBController()
{
}

bool ADBController::findADBPath()
{
    // 首先在指定的adb__目录中查找
    QString customPath = "./_adb_/adb";
    if (QFileInfo::exists(customPath + ".exe")) {
        m_adbPath = customPath + ".exe";
        return true;
    } else if (QFileInfo::exists(customPath)) {
        m_adbPath = customPath;
        return true;
    }
    
    // 如果找不到，记录错误并返回false
    qDebug() << "找不到ADB可执行文件: " << customPath;
    return false;
}

QStringList ADBController::getDevices()
{
    QStringList deviceList;
    bool success = false;
    QString output = executeADBCommand("devices", &success);
    
    if (!success) {
        return deviceList;
    }
    
    // 解析输出内容，获取设备列表
    QStringList lines = output.split("\n", QString::SkipEmptyParts);
    // 移除第一行（标题行）
    if (lines.size() > 0) {
        lines.removeFirst(); // 移除 "List of devices attached"
    }
    
    // 遍历剩余行，提取设备ID
    for (const QString &line : lines) {
        QString trimmedLine = line.trimmed();
        if (!trimmedLine.isEmpty()) {
            // 格式: <device-id>\t<state>
            QStringList parts = trimmedLine.split("\t", QString::SkipEmptyParts);
            if (parts.size() >= 1) {
                deviceList.append(parts[0]);
            }
        }
    }
    
    return deviceList;
}

int ADBController::getDeviceCount()
{
    return getDevices().size();
}

QString ADBController::getDeviceInfo(const QString &serial)
{
    bool success = false;
    QString output = executeADBCommand("-s " + serial + " shell getprop", &success);
    
    if (!success) {
        return "无法获取设备信息";
    }
    
    // 可以根据需要解析输出中的特定属性
    // 这里只是返回原始输出
    return output;
}

QString ADBController::executeADBCommand(const QString &command, bool *success)
{
    if (m_adbPath.isEmpty() && !findADBPath()) {
        if (success) *success = false;
        return "找不到ADB可执行文件";
    }
    
    QProcess process;
    QString program = m_adbPath;
    QStringList arguments = command.split(" ", QString::SkipEmptyParts);
    
    qDebug() << "执行ADB命令: " << program << arguments;
    process.start(program, arguments);
    
    if (!process.waitForStarted(5000)) {
        if (success) *success = false;
        return "命令启动失败: " + command;
    }
    
    if (!process.waitForFinished(30000)) {
        process.kill();
        if (success) *success = false;
        return "命令执行超时: " + command;
    }
    
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString errorOutput = QString::fromLocal8Bit(process.readAllStandardError());
    
    // 合并标准输出和标准错误，以确保获取全部输出
    QString combinedOutput = output;
    if (!errorOutput.isEmpty()) {
        if (!combinedOutput.isEmpty()) combinedOutput += "\n";
        combinedOutput += errorOutput;
    }
    
    if (process.exitCode() != 0) {
        if (success) *success = false;
        return "命令执行失败: " + combinedOutput;
    }
    
    if (success) *success = true;
    return combinedOutput;
}

QString ADBController::pushFile(const QString &localFile, const QString &remotePath, bool *success)
{
    if (m_adbPath.isEmpty() && !findADBPath()) {
        if (success) *success = false;
        return "找不到ADB可执行文件";
    }
    
    QProcess process;
    QStringList arguments;
    arguments << "push" << localFile << remotePath;
    
    qDebug() << "执行ADB push: " << m_adbPath << arguments;
    process.start(m_adbPath, arguments);
    
    if (!process.waitForStarted(5000)) {
        if (success) *success = false;
        return "命令启动失败: push";
    }
    
    if (!process.waitForFinished(30000)) {
        process.kill();
        if (success) *success = false;
        return "命令执行超时: push";
    }
    
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString errorOutput = QString::fromLocal8Bit(process.readAllStandardError());
    
    // 合并标准输出和标准错误，以确保获取全部输出
    QString combinedOutput = output;
    if (!errorOutput.isEmpty()) {
        if (!combinedOutput.isEmpty()) combinedOutput += "\n";
        combinedOutput += errorOutput;
    }
    
    if (process.exitCode() != 0) {
        if (success) *success = false;
        return "命令执行失败: " + combinedOutput;
    }
    
    if (success) *success = true;
    return combinedOutput;
}

QString ADBController::pullFile(const QString &remotePath, const QString &localFile, bool *success)
{
    if (m_adbPath.isEmpty() && !findADBPath()) {
        if (success) *success = false;
        return "找不到ADB可执行文件";
    }
    
    QProcess process;
    QStringList arguments;
    arguments << "pull" << remotePath << localFile;
    
    qDebug() << "执行ADB pull: " << m_adbPath << arguments;
    process.start(m_adbPath, arguments);
    
    if (!process.waitForStarted(5000)) {
        if (success) *success = false;
        return "命令启动失败: pull";
    }
    
    if (!process.waitForFinished(30000)) {
        process.kill();
        if (success) *success = false;
        return "命令执行超时: pull";
    }
    
    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());
    QString errorOutput = QString::fromLocal8Bit(process.readAllStandardError());
    
    // 合并标准输出和标准错误，以确保获取全部输出
    QString combinedOutput = output;
    if (!errorOutput.isEmpty()) {
        if (!combinedOutput.isEmpty()) combinedOutput += "\n";
        combinedOutput += errorOutput;
    }
    
    if (process.exitCode() != 0) {
        if (success) *success = false;
        return "命令执行失败: " + combinedOutput;
    }
    
    if (success) *success = true;
    return combinedOutput;
}

QString ADBController::executeShellCommand(const QString &command, bool *success)
{
    if (m_adbPath.isEmpty() && !findADBPath()) {
        if (success) *success = false;
        return "找不到ADB可执行文件";
    }
    
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels); // 合并标准输出和标准错误
    QStringList arguments;
    arguments << "shell" << command;
    
    qDebug() << "执行ADB shell命令: " << m_adbPath << arguments;
    process.start(m_adbPath, arguments);
    
    if (!process.waitForStarted(5000)) {
        if (success) *success = false;
        return "命令启动失败";
    }
    
    QString output;
    bool foundReturn = false;
    int maxWaitTime = 10; // 最长等待时间（秒）
    QTime startTime = QTime::currentTime();
    while (process.state() != QProcess::NotRunning) {
        // 检查是否超时
        if (startTime.secsTo(QTime::currentTime()) > maxWaitTime) {
            qDebug() << "命令执行超时，发送Ctrl+C";
            
            // 发送Ctrl+C
            QByteArray ctrlC;
            ctrlC.append('\x03');
            process.write(ctrlC);
            
            // 等待一下确保命令已经中断
            process.waitForFinished(1000);
            
            // 如果仍未结束，强制终止
            if (process.state() != QProcess::NotRunning) {
                process.terminate();
                process.waitForFinished(1000);
                
                if (process.state() != QProcess::NotRunning) {
                    process.kill();
                }
            }
            output += "\n[命令执行超时，已强制终止]";
            break;
        }
        if (process.waitForReadyRead(100)) {
            QByteArray newData = process.readAll();
            QString newOutput = QString::fromUtf8(newData);
            output += newOutput;
            if (newOutput.contains("Return")) {
                qDebug() << "发现Return关键字，等待200ms后终止";
                QThread::msleep(200);
                QByteArray ctrlC;
                ctrlC.append('\x03');
                process.write(ctrlC);
                
                foundReturn = true;
                break;
            }
        }
    }
    
    // 确保读取所有剩余输出
    QByteArray remainingData = process.readAll();
    if (!remainingData.isEmpty()) {
        output += QString::fromUtf8(remainingData);
    }
    
    // 最终确保进程已经结束
    if (process.state() != QProcess::NotRunning) {
        process.terminate();
        process.waitForFinished(1000);
        
        if (process.state() != QProcess::NotRunning) {
            process.kill();
        }
    }
    
    if (foundReturn) {
        output += "\n[检测到Return关键字，命令已被终止]";
        if (success) *success = true;
    } else {
        if (success) *success = false;
    }
    
    return output;
}
