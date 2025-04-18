"""
BLU2 设备的串口通信模块
处理与 BLU2 设备的所有串口通信操作
"""
# -*- coding: utf-8 -*-
import time
import serial
import struct
import logging

class SerialCommunicator:
    """
    处理与 BLU2 设备的串口通信类
    提供底层的串口读写操作
    """
    def __init__(self, port: str, **kwargs):
        '''
        port - BLU2 连接的端口
        **kwargs - 传递给 pySerial 构造函数的关键字参数
        '''
        self.ser = None
        self.ser = serial.Serial(port, **kwargs)
        self.ser.baudrate = 9600
        
        # ADC 测量缓冲区余数及余数长度
        self.remainder = {"sequence": b'', "len": 0}
        
    def __del__(self):
        """析构函数"""
        try:
            if self.ser:
                self.ser.close()
        except Exception as e:
            logging.error(f"关闭串口时发生错误: {e}")
    
    def _pack_struct(self, cmd_tuple):
        """返回打包的结构体"""
        return struct.pack("B" * len(cmd_tuple), *cmd_tuple)
    
    def write_serial(self, cmd_tuple):
        """向串口写入命令字节"""
        try:
            cmd_packed = self._pack_struct(cmd_tuple)
            self.ser.write(cmd_packed)
        except Exception as e:
            logging.error(f"写入串口时发生错误: {e}")
    
    def read_data(self):
        """返回一个采样周期的读数"""
        sampling_data = self.ser.read(self.ser.in_waiting)
        return sampling_data
    def read_until_marker(self, marker="END", max_attempts=6, delay=0.1):
        """读取直到找到特定标记"""
        for _ in range(0, max_attempts):
            read = self.ser.read(self.ser.in_waiting)
            time.sleep(delay)
            
            if read != b'' and marker in read.decode("utf-8", errors="ignore"):
                return read.decode("utf-8", errors="ignore")
        return None
        
    def digital_to_analog(self, adc_value):
        """将离散值转换为模拟值"""
        return int.from_bytes(adc_value, byteorder="little", signed=False)  # 将读数转换为模拟值
        
    @staticmethod
    def list_devices():
        """列出所有 BLU 设备"""
        import serial.tools.list_ports
        ports = serial.tools.list_ports.comports()
        # devices = [port.device for port in ports if port.product == 'BLU']
        devices = [port.device for port in ports if port.pid == 12298]
        return devices
        
    def flush_buffer(self):
        """清空串口缓冲区"""
        self.ser.read(self.ser.in_waiting)



