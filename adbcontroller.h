#ifndef ADBCONTROLLER_H
#define ADBCONTROLLER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>

class ADBController : public QObject
{
    Q_OBJECT

public:
    explicit ADBController(QObject *parent = nullptr);
    ~ADBController();

    // 获取已连接的ADB设备列表
    QStringList getDevices();
    
    // 检查是否有设备连接，返回连接的设备数量
    int getDeviceCount();
    
    // 获取特定设备的信息，如序列号、状态等
    QString getDeviceInfo(const QString &serial);
    
    // 执行通用ADB命令并返回结果
    QString executeADBCommand(const QString &command, bool *success = nullptr);
    
    // 文件传输功能
    QString pushFile(const QString &localFile, const QString &remotePath, bool *success = nullptr);
    QString pullFile(const QString &remotePath, const QString &localFile, bool *success = nullptr);
    
    // 交互式shell命令，自动在检测到指定关键字时终止进程
    QString executeShellCommand(const QString &command, bool *success = nullptr, const QString &terminateKeyword = "Return");

private:
    QString m_adbPath;
    bool findADBPath();
};

#endif // ADBCONTROLLER_H
