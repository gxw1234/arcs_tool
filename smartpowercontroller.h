#ifndef SMARTPOWERCONTROLLER_H
#define SMARTPOWERCONTROLLER_H

#include <QString>
#include "lib/smartpower.h"

/**
 * @brief SmartPowerController类用于封装SmartPower设备的所有操作
 * 
 * 该类封装了SmartPower的API调用，提供简单易用的接口
 */
class SmartPowerController
{

public:
    // 设备状态枚举
    enum DeviceStatus {
        NotConnected,    // 未连接
        Connected,       // 已连接
        OutputOn,        // 输出开启
        Error            // 错误状态
    };

    // 获取单例实例
    static SmartPowerController *getInstance();
    
    // 析构函数，确保设备关闭
    ~SmartPowerController();
    
    // 公共方法
    int getDeviceCount();                  // 获取可用设备数量
    bool connectDevice(int index = 0);     // 连接设备，默认连接第一个
    bool disconnectDevice();               // 断开设备连接
    bool isConnected() const;              // 检查是否已连接
    bool isOutputOn() const;               // 检查输出是否开启
    
    // 设备控制方法
    bool setOutput(bool on, double voltage = 0, double current = 0);  // 设置输出状态
    bool updateOutput(double voltage, double current);               // 更新输出参数
    bool getStatus(double values[4]);                                // 获取设备状态
    
    // 获取当前状态
    DeviceStatus getDeviceStatus() const;
    QString getStatusMessage() const;
    
    // 获取详细状态信息
    // readCount: 读取次数，返回多次读取的平均值
    bool getDetailedStatus(double &voltage, double &current, double &bnc, int readCount = 1);

private:
    // 单例模式，禁止外部创建实例
    SmartPowerController();
    static SmartPowerController *instance;
    
    // 私有属性
    DeviceStatus status;
    bool connected;
    bool outputOn;
    QString statusMessage;
    double lastVoltage;
    double lastCurrent;
    
    // 私有方法
    void setStatus(DeviceStatus newStatus, const QString &message = QString());
    bool checkConnection();
};

#endif // SMARTPOWERCONTROLLER_H
