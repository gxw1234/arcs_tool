"""
BLU2 设备简单 Demo (直接使用分离的串口和协议模块)
- 连接设备 (COM8)
- 设置电压为 4V
- 测量电流并显示
"""
import time

from src.blu2_api.blu2_serial import SerialCommunicator
from src.blu2_api.blu2_protocol import BLU2_Protocol, BLU2_Command, BLU2_Modes
def main():
    # 连接参数设置
    serial_port = "COM8"
    source_voltage_mV = 4000  # 4V = 4000mV
    print(f"正在连接 BLU2 设备 (端口: {serial_port})...")
    try:
        # 分别初始化串口和协议模块
        comm = SerialCommunicator(serial_port)
        protocol = BLU2_Protocol()
        # 获取设备修正因子
        comm.write_serial((BLU2_Command.GET_META_DATA, ))
        metadata = comm.read_until_marker()
        ret = protocol._parse_metadata(metadata)
        
        if not ret:
            raise Exception(f"获取修正因子失败，请检查设备连接")
        print(f"成功连接 BLU2 设备")
        # 切换到源表模式
        protocol.mode = BLU2_Modes.SOURCE_MODE
        comm.write_serial((BLU2_Command.SET_POWER_MODE, BLU2_Command.AVG_NUM_SET))  # 17,2
        print("已切换到源表模式")
        # 设置源电压为 4V
        b_1, b_2 = protocol._convert_source_voltage(source_voltage_mV)
        comm.write_serial((BLU2_Command.REGULATOR_SET, b_1, b_2))
        protocol.current_vdd = source_voltage_mV
        print(f"已设置源电压为 {source_voltage_mV/1000}V")
        
        # 打开 DUT 电源
        comm.write_serial((BLU2_Command.DEVICE_RUNNING_SET, BLU2_Command.TRIGGER_SET))  # 12,1
        print("已打开 DUT 电源")
        
        print("等待系统稳定 (5秒)...")
        time.sleep(5)
        
        print("开始测量...")
        # 开始连续测量
        comm.write_serial((BLU2_Command.AVERAGE_START, ))
        time.sleep(5)
        # 获取数据
        data = comm.read_data()
        
        if data != b'':
            # 处理样本数据
            samples, raw_digital_output, comm.remainder = protocol.process_samples(data, comm.remainder)
            
            # 显示样本数量和平均值
            if samples and len(samples) > 0:
                avg_current_uA = sum(samples) / len(samples)
                avg_current_mA = avg_current_uA / 1000
                avg_power_mW = (source_voltage_mV / 1000) * avg_current_mA
                
                print(f"获取样本数量: {len(samples)}")
                print(f"平均电流: {avg_current_mA:.3f} mA")
                print(f"平均功率: {avg_power_mW:.3f} mW")
            else:
                print("未获取到样本数据")
        else:
            print("未获取到数据")
        
        # 停止测量
        comm.write_serial((BLU2_Command.AVERAGE_STOP, ))
        print("停止测量")
        
        # 关闭 DUT 电源
        comm.write_serial((BLU2_Command.DEVICE_RUNNING_SET, BLU2_Command.NO_OP))  # 12,0
        print("关闭 DUT 电源")
        
    except Exception as e:
        print(f"错误: {e}")
    
    print("完成")

if __name__ == "__main__":
    main()
