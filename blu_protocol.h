#ifndef BLU_PROTOCOL_H
#define BLU_PROTOCOL_H

#include <QObject>
#include <QByteArray>
#include <QVector>
#include <QMap>
#include "blu_serial.h"

// BLU设备的模式定义
enum BLUMode {
    SOURCE_MODE,    // 源表模式
    MEASURE_MODE    // 测量模式
};

// BLU协议处理类
class BLUProtocol : public QObject
{
    Q_OBJECT

public:
    explicit BLUProtocol(QObject *parent = nullptr);
    ~BLUProtocol();
    
    // 协议命令枚举
    enum Command {
        NO_OP = 0x00,
        TRIGGER_SET = 0x01,
        AVG_NUM_SET = 0x02,
        TRIGGER_WINDOW_SET = 0x03,
        TRIGGER_INTERVAL_SET = 0x04,
        TRIGGER_SINGLE_SET = 0x05,
        AVERAGE_START = 0x06,
        AVERAGE_STOP = 0x07,
        RANGE_SET = 0x08,
        LCD_SET = 0x09,
        TRIGGER_STOP = 0x0A,
        DEVICE_RUNNING_SET = 0x0C,
        REGULATOR_SET = 0x0D,
        SWITCH_POINT_DOWN = 0x0E,
        SWITCH_POINT_UP = 0x0F,
        TRIGGER_EXT_TOGGLE = 0x11,
        SET_POWER_MODE = 0x11,
        RES_USER_SET = 0x12,
        SPIKE_FILTERING_ON = 0x15,
        SPIKE_FILTERING_OFF = 0x16,
        GET_META_DATA = 0x19,
        RESET = 0x20,
        SET_USER_GAINS = 0x25
    };
    
    // 设置工作模式
    void setMode(BLUMode mode);
    BLUMode mode() const;
    
    // 源电压转换
    QByteArray convertSourceVoltage(int voltage_mV);
    
    // 设置/获取当前电压值
    int currentVdd() const;
    void setCurrentVdd(int voltage_mV);
    
    // 解析元数据
    bool parseMetadata(const QByteArray &data);
    
    // 处理样本数据
    QVector<double> processSamples(const QByteArray &data, QByteArray &remainder);
    
private:
    BLUMode m_mode;
    int m_currentVdd;
    
    // 电压范围限制
    const int m_vddLow = 500;   // 最小500mV
    const int m_vddHigh = 5000; // 最大5000mV
    
    // ADC乘数因子
    const double m_adcMult = 1.8 / 163840.0;
    
    // 设备修正因子
    QMap<int, double> m_modifierO; // O因子（偏移）
    QMap<int, double> m_modifierR; // R因子（分流电阻）
    
    // 掩码常量
    quint32 m_measADC;     // ADC值掩码
    quint32 m_measRange;   // 测量范围掩码
    quint32 m_measLogic;   // 逻辑掩码
    
    // 生成掩码
    quint32 generateMask(int bits, int offset);
    
    // 根据掩码提取值
    quint32 getMaskedValue(quint32 value, quint32 mask);
    
    // 处理单个ADC值
    double processADCValue(quint32 adc_val);
};

#endif // BLU_PROTOCOL_H
