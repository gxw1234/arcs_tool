#include "smartpowercontroller.h"
#include <QDebug>


SmartPowerController* SmartPowerController::instance = nullptr;


SmartPowerController* SmartPowerController::getInstance()
{
    if (instance == nullptr) {
        instance = new SmartPowerController();
    }
    return instance;
}


SmartPowerController::SmartPowerController()
    : status(NotConnected),
      connected(false),
      outputOn(false),
      lastVoltage(4.5),
      lastCurrent(1.5)
{
    setStatus(NotConnected, "未连接");
}


SmartPowerController::~SmartPowerController()
{

    if (connected) {
        disconnectDevice();
    }
}

// 获取设备数量
int SmartPowerController::getDeviceCount()
{
    try {
        return SmartPower_DeviceNums();
    } catch (...) {
        setStatus(Error, "获取设备数量失败");
        return 0;
    }
}

// 连接设备
bool SmartPowerController::connectDevice(int index)
{
    // 如果已连接，先断开
    if (connected) {
        disconnectDevice();
    }
    
    try {
        bool success = SmartPower_DeviceOpen(index);
        if (success) {
            connected = true;
            setStatus(Connected, "设备已连接");
            return true;
        } else {
            setStatus(Error, "连接设备失败");
            return false;
        }
    } catch (...) {
        setStatus(Error, "连接设备时发生异常");
        return false;
    }
}

// 断开设备连接
bool SmartPowerController::disconnectDevice()
{
    if (!connected) {
        return true; // 已经断开，无需操作
    }
    
    try {
        // 如果输出开启，先关闭输出
        if (outputOn) {
            setOutput(false);
        }
        
        // 关闭设备
        bool success = SmartPower_DeviceClose();
        if (success) {
            connected = false;
            setStatus(NotConnected, "设备已断开连接");
            return true;
        } else {
            setStatus(Error, "断开设备连接失败");
            return false;
        }
    } catch (...) {
        setStatus(Error, "断开设备连接时发生异常");
        connected = false; // 强制置为断开状态
        return false;
    }
}

// 检查是否已连接
bool SmartPowerController::isConnected() const
{
    return connected;
}

// 检查输出是否开启
bool SmartPowerController::isOutputOn() const
{
    return outputOn;
}

// 设置输出状态
bool SmartPowerController::setOutput(bool on, double voltage, double current)
{
    if (!checkConnection()) {
        return false;
    }
    
    try {
        bool success = false;
        
        if (on) {
            // 如果电压电流为0，使用上次的值
            if (voltage <= 0) voltage = lastVoltage;
            if (current <= 0) current = lastCurrent;
            
            // 保存设置值
            lastVoltage = voltage;
            lastCurrent = current;
            
            // 开启输出
            success = SmartPower_OutputOn(voltage, current);
            if (success) {
                outputOn = true;
                setStatus(OutputOn, QString("输出已开启: %1V, %2A").arg(voltage).arg(current));
            } else {
                setStatus(Error, "开启输出失败");
            }
        } else {
            // 关闭输出
            success = SmartPower_OutputOff(1);
            if (success) {
                outputOn = false;
                setStatus(Connected, "输出已关闭");
            } else {
                setStatus(Error, "关闭输出失败");
            }
        }
        
        return success;
    } catch (...) {
        setStatus(Error, on ? "开启输出时发生异常" : "关闭输出时发生异常");
        return false;
    }
}

// 更新输出参数
bool SmartPowerController::updateOutput(double voltage, double current)
{
    if (!checkConnection() || !outputOn) {
        return false;
    }
    
    try {
        // 保存设置值
        lastVoltage = voltage;
        lastCurrent = current;
        
        // 更新输出
        bool success = SmartPower_OutputUpdate(voltage, current);
        if (success) {
            setStatus(OutputOn, QString("输出已更新: %1V, %2A").arg(voltage).arg(current));
            return true;
        } else {
            setStatus(Error, "更新输出参数失败");
            return false;
        }
    } catch (...) {
        setStatus(Error, "更新输出参数时发生异常");
        return false;
    }
}

// 获取设备状态
bool SmartPowerController::getStatus(double values[4])
{
    if (!checkConnection()) {
        return false;
    }
    
    try {
        return SmartPower_GetStatus(values);
    } catch (...) {
        setStatus(Error, "获取设备状态时发生异常");
        return false;
    }
}

// 获取详细状态信息（返回格式化后的值）
bool SmartPowerController::getDetailedStatus(double &voltage, double &current, double &bnc, int readCount)
{
    if (!checkConnection()) {
        return false;
    }
    
    // 防止输入非法值
    if (readCount <= 0) {
        readCount = 1;
    }
    
    try {
        // 累计值
        double totalVoltage = 0.0;
        double totalCurrent = 0.0;
        double totalBnc = 0.0;
        int successCount = 0;
        
        // 多次读取
        for (int i = 0; i < readCount; i++) {
            double values[4] = {0};
            bool success = SmartPower_GetStatus(values);
            
            if (success) {
                totalVoltage += values[0];
                totalCurrent += values[1];
                totalBnc += values[2];
                successCount++;
                
                // 如果读取多次，在每次读取之间添加小延时以增加稳定性
                if (readCount > 1 && i < readCount - 1) {
                    // 使用QThread::msleep需要包含<QThread>
                    // 这里用更简单的方式，阻塞10毫秒
                    for (volatile int j = 0; j < 1000000; j++) {}
                }
            }
        }
        
        // 计算平均值
        if (successCount > 0) {
            voltage = totalVoltage / successCount;
            current = totalCurrent / successCount;
            bnc = totalBnc / successCount;
            return true;
        } else {
            setStatus(Error, "获取设备状态失败");
            return false;
        }
    } catch (...) {
        setStatus(Error, "获取设备状态时发生异常");
        return false;
    }
}

// 获取设备状态
SmartPowerController::DeviceStatus SmartPowerController::getDeviceStatus() const
{
    return status;
}

// 获取状态消息
QString SmartPowerController::getStatusMessage() const
{
    return statusMessage;
}

// 设置状态
void SmartPowerController::setStatus(DeviceStatus newStatus, const QString &message)
{
    status = newStatus;
    statusMessage = message;
    
    if (!message.isEmpty()) {
        qDebug() << "SmartPower状态: " << message;
    }
}

// 检查连接
bool SmartPowerController::checkConnection()
{
    if (!connected) {
        setStatus(Error, "设备未连接");
        return false;
    }
    return true;
}
