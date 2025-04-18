#include "blu_protocol.h"
#include <QDebug>

BLUProtocol::BLUProtocol(QObject *parent) : QObject(parent)
{
    m_mode = SOURCE_MODE;
    m_currentVdd = 4000; // 默认4V
    
    // 初始化掩码
    m_measADC = generateMask(14, 0);      // 14位，从位0开始
    m_measRange = generateMask(3, 14);    // 3位，从位14开始
    m_measLogic = generateMask(8, 24);    // 8位，从位24开始
    
    // 设置默认修正因子
    for (int i = 0; i < 6; i++) {
        // 默认O因子为零偏移
        m_modifierO[i] = 0.0;
        
        // 设置默认R因子，与官方实现类似
        switch(i) {
            case 0: m_modifierR[i] = 1031.64; break;
            case 1: m_modifierR[i] = 101.65; break;
            case 2: m_modifierR[i] = 10.15; break;
            case 3: m_modifierR[i] = 0.94; break;
            case 4: m_modifierR[i] = 0.113; break;
            case 5: m_modifierR[i] = 0.013; break;
            default: m_modifierR[i] = 1.0; break;
        }
    }
    
    // 设置范围0-2的默认偏移值
    m_modifierO[0] = 8000.0;
    m_modifierO[1] = 8000.0;
    m_modifierO[2] = 8000.0;
}

BLUProtocol::~BLUProtocol()
{
    // 无需额外清理
}

void BLUProtocol::setMode(BLUMode mode)
{
    m_mode = mode;
}

BLUMode BLUProtocol::mode() const
{
    return m_mode;
}

int BLUProtocol::currentVdd() const
{
    return m_currentVdd;
}

void BLUProtocol::setCurrentVdd(int voltage_mV)
{
    m_currentVdd = voltage_mV;
}

QByteArray BLUProtocol::convertSourceVoltage(int voltage_mV)
{
    // 限制电压在有效范围内
    if (voltage_mV < m_vddLow) {
        voltage_mV = m_vddLow;
    }
    if (voltage_mV > m_vddHigh) {
        voltage_mV = m_vddHigh;
    }
    
    // 设置基准电压差异
    int offset = 500;
    int diff_to_baseline = voltage_mV - m_vddLow + offset;
    
    // 分配到两个字节
    int ratio = diff_to_baseline / 256;
    int remainder = diff_to_baseline % 256;
    
    // 构建命令字节
    QByteArray result;
    result.append(static_cast<char>(ratio));      // 第一个字节
    result.append(static_cast<char>(remainder));  // 第二个字节
    
    return result;
}

// 生成掩码
quint32 BLUProtocol::generateMask(int bits, int offset)
{
    quint32 mask = 0;
    for (int i = 0; i < bits; i++) {
        mask |= (1 << (i + offset));
    }
    return mask;
}

// 根据掩码提取值
quint32 BLUProtocol::getMaskedValue(quint32 value, quint32 mask)
{
    quint32 result = value & mask;
    
    // 找到最低位
    int shift = 0;
    while ((mask & (1 << shift)) == 0 && shift < 32) {
        shift++;
    }
    
    return result >> shift;
}

bool BLUProtocol::parseMetadata(const QByteArray &data)
{
    if (data.isEmpty()) {
        qDebug() << "元数据为空，无法解析";
        return false;
    }
    
    qDebug() << "开始解析元数据...";
    
    // 按行切分元数据
    QList<QByteArray> lines = data.split('\n');
    
    // 解析元数据
    for (const QByteArray &line : lines) {
        // 跳过空行
        if (line.trimmed().isEmpty()) {
            continue;
        }
        
        // 分割"key: value"格式
        QList<QByteArray> parts = line.split(':');
        if (parts.size() != 2) {
            continue;
        }
        
        QString key = parts[0].trimmed();
        QString value = parts[1].trimmed();
        
        qDebug() << "解析元数据行:" << key << "=" << value;
        
        // 处理O因子
        if (key.startsWith("O") && key.length() == 2) {
            int index = key.at(1).digitValue();
            if (index >= 0 && index <= 5) {
                bool ok;
                double dValue = value.toDouble(&ok);
                if (ok) {
                    m_modifierO[index] = dValue;
                    qDebug() << "设置O因子" << index << "=" << dValue;
                }
            }
        }
        // 处理R因子
        else if (key.startsWith("R") && key.length() == 2) {
            int index = key.at(1).digitValue();
            if (index >= 0 && index <= 5) {
                bool ok;
                double dValue = value.toDouble(&ok);
                if (ok && dValue != 0.0) {  // 确保R因子不为0
                    m_modifierR[index] = dValue;
                    qDebug() << "设置R因子" << index << "=" << dValue;
                }
            }
        }
    }
    
    // 打印当前修正因子
    qDebug() << "当前修正因子:";
    for (int i = 0; i < 6; i++) {
        qDebug() << "O" << i << "=" << m_modifierO[i] << ", R" << i << "=" << m_modifierR[i];
    }
    
    return true;
}

QVector<double> BLUProtocol::processSamples(const QByteArray &data, QByteArray &remainder)
{
    QVector<double> samples;
    
    // 将之前的remainder与新数据合并
    QByteArray processData = remainder + data;
    
    // 每个样本占用4个字节
    const int sample_size = 4;
    
    // 处理完整样本
    int complete_samples = processData.size() / sample_size;
    
    // 限制处理样本数量，与Python版本保持一致
    complete_samples = qMin(complete_samples, 3584);
    
    qDebug() << "处理样本: 总字节数=" << processData.size() 
             << ", 完整样本数=" << complete_samples;
    
    // 处理每个样本
    for (int i = 0; i < complete_samples; i++) {
        int offset = i * sample_size;
        
        // 按小端序读取4字节ADC值
        quint32 adc_val = 0;
        for (int j = 0; j < sample_size; j++) {
            adc_val |= ((quint8)processData.at(offset + j)) << (8 * j);
        }
        
        // 处理ADC值并添加到样本集
        double current_uA = processADCValue(adc_val);
        samples.append(current_uA);
    }
    
    // 更新remainder为剩余不完整的字节
    remainder = processData.mid(complete_samples * sample_size);
    
    return samples;
}

double BLUProtocol::processADCValue(quint32 adc_val)
{
    // 使用正确的掩码函数提取测量范围
    quint32 meas_range = getMaskedValue(adc_val, m_measRange);
    
    // 使用正确的掩码函数提取ADC结果
    quint32 adc_result = getMaskedValue(adc_val, m_measADC) * 4;
    
    // 确保测量范围不超过5
    meas_range = qMin(meas_range, 5u);
    
    // 获取对应范围的修正因子
    double o_offset = m_modifierO[meas_range];
    double r_factor = m_modifierR[meas_range];
    
    // 使用修正因子计算电流值
    double result_without_gain = (adc_result - o_offset) * (m_adcMult / r_factor);
    
    // 转换为微安单位
    double current_uA = result_without_gain * 1000000.0;
    
    // 打印调试信息 (仅前几个样本)
    static int processed = 0;
    if (processed < 5) {
        qDebug() << "样本" << processed 
                 << ": ADC=" << adc_val 
                 << ", 范围=" << meas_range 
                 << ", ADC结果=" << adc_result
                 << ", O偏移=" << o_offset
                 << ", R因子=" << r_factor
                 << ", 电流=" << current_uA << "uA";
        processed++;
    }
    
    return current_uA;
}
