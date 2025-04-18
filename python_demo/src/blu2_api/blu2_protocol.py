"""
BLU2 设备的协议模块
处理与 BLU2 设备的所有协议操作和业务逻辑
"""
# -*- coding: utf-8 -*-
import time


class BLU2_Command():
    """串行命令操作码"""
    NO_OP = 0x00
    TRIGGER_SET = 0x01
    AVG_NUM_SET = 0x02  # no-firmware
    TRIGGER_WINDOW_SET = 0x03
    TRIGGER_INTERVAL_SET = 0x04
    TRIGGER_SINGLE_SET = 0x05
    AVERAGE_START = 0x06
    AVERAGE_STOP = 0x07
    RANGE_SET = 0x08
    LCD_SET = 0x09
    TRIGGER_STOP = 0x0a
    DEVICE_RUNNING_SET = 0x0c
    REGULATOR_SET = 0x0d
    SWITCH_POINT_DOWN = 0x0e
    SWITCH_POINT_UP = 0x0f
    TRIGGER_EXT_TOGGLE = 0x11
    SET_POWER_MODE = 0x11
    RES_USER_SET = 0x12
    SPIKE_FILTERING_ON = 0x15
    SPIKE_FILTERING_OFF = 0x16
    GET_META_DATA = 0x19
    RESET = 0x20
    SET_USER_GAINS = 0x25


class BLU2_Modes():
    """BLU2 测量模式"""
    AMPERE_MODE = "AMPERE_MODE"
    SOURCE_MODE = "SOURCE_MODE"


class BLU2_Protocol:
    """
    处理 BLU2 设备的协议逻辑和数据处理
    """
    def __init__(self):
        self.modifiers = {
            "Calibrated": None,
            "R": {"0": 1031.64, "1": 101.65, "2": 10.15, "3": 0.94, "4": 0.113, "5": 0.013},
            "GS": {"0": 1, "1": 1, "2": 1, "3": 1, "4": 1, "5": 1},
            "GI": {"0": 1, "1": 1, "2": 1, "3": 1, "4": 1, "5": 1},
            "O": {"0": 0, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0},
            "S": {"0": 0, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0},
            "I": {"0": 0, "1": 0, "2": 0, "3": 0, "4": 0, "5": 0},
            "UG": {"0": 1, "1": 1, "2": 1, "3": 1, "4": 1, "5": 1},
            "HW": None,
            "IA": None
        }

        self.vdd_low = 500
        self.vdd_high = 5000

        self.current_vdd = None
        self.mode = None

        self.adc_mult = 1.8 / 163840

        self.MEAS_ADC = self._generate_mask(14, 0)
        self.MEAS_RANGE = self._generate_mask(3, 14)
        self.MEAS_LOGIC = self._generate_mask(8, 24)

        self.rolling_avg = None
        self.rolling_avg4 = None
        self.prev_range = None
        self.consecutive_range_samples = 0

        self.spike_filter_alpha = 0.18
        self.spike_filter_alpha5 = 0.06
        self.spike_filter_samples = 3
        self.after_spike = 0

    def _twos_comp(self, val):
        """计算 int32 值的 2 的补码"""
        if (val & (1 << (32 - 1))) != 0:
            val = val - (1 << 32)  # 计算负值
        return val

    def _generate_mask(self, bits, pos):
        """生成掩码"""
        pos = pos
        mask = ((2**bits-1) << pos)
        mask = self._twos_comp(mask)
        return {"mask": mask, "pos": pos}

    def _get_masked_value(self, value, meas):
        """获取掩码值"""
        masked_value = (value & meas["mask"]) >> meas["pos"]
        return masked_value

    def _parse_metadata(self, metadata):
        """解析元数据并将其存储到修正因子中"""
        try:
            data_split = [row.split(": ") for row in metadata.split("\n")]

            for key in self.modifiers.keys():
                for data_pair in data_split:
                    if key == data_pair[0]:
                        self.modifiers[key] = data_pair[1]
                    for ind in range(0, 6):
                        if key+str(ind) == data_pair[0]:
                            if "R" in data_pair[0]:
                                # 一些 BLU2 的校准值有问题 - 这不能修复它
                                if float(data_pair[1]) != 0:
                                    self.modifiers[key][str(ind)] = float(
                                        data_pair[1])
                            else:
                                self.modifiers[key][str(ind)] = float(
                                    data_pair[1])
            return True
        except Exception as e:
            # 如果触发异常，则串口可能不正确
            return None

    def _convert_source_voltage(self, mV):
        """转换输入电压为设备命令"""
        # 最小可能的 mV 是 500
        if mV < self.vdd_low:
            mV = self.vdd_low

        # 最大可能的 mV 是 5000
        if mV > self.vdd_high:
            mV = self.vdd_high

        offset = 500  # 32
        # 获取与基准的差异(基准是 800mV，但初始偏移量是 32)
        diff_to_baseline = mV - self.vdd_low + offset
        base_b_1 = 0  # 3
        base_b_2 = 0  # 实际上是 32 - 用上面的偏移量补偿

        # 获取我们必须增加命令第一个字节的次数
        ratio = int(diff_to_baseline / 256)
        remainder = diff_to_baseline % 256  # 获取字节 2 的余数

        set_b_1 = base_b_1 + ratio
        set_b_2 = base_b_2 + remainder

        return set_b_1, set_b_2

    def _handle_raw_data(self, adc_value):
        """将原始值转换为模拟值"""
        try:
            current_measurement_range = min(self._get_masked_value(
                adc_value, self.MEAS_RANGE), 5)  # 6 是参数的数量
            adc_result = self._get_masked_value(adc_value, self.MEAS_ADC) * 4
            bits = self._get_masked_value(adc_value, self.MEAS_LOGIC)
            analog_value = self.get_adc_result(
                current_measurement_range, adc_result) * 10**6
            return analog_value, bits
        except Exception as e:
            print("测量超出范围！")
            return None, None

    def get_adc_result(self, current_range, adc_value):
        """获取 ADC 转换结果"""
        current_range = str(current_range)
        result_without_gain = (adc_value - self.modifiers["O"][current_range]) * (
            self.adc_mult / self.modifiers["R"][current_range])
        adc = result_without_gain

        prev_rolling_avg = self.rolling_avg
        prev_rolling_avg4 = self.rolling_avg4

        # 尖峰过滤/滚动平均
        if self.rolling_avg is None:
            self.rolling_avg = adc
        else:
            self.rolling_avg = self.spike_filter_alpha * adc + (1 - self.spike_filter_alpha) * self.rolling_avg
        
        if self.rolling_avg4 is None:
            self.rolling_avg4 = adc
        else:
            self.rolling_avg4 = self.spike_filter_alpha5 * adc + (1 - self.spike_filter_alpha5) * self.rolling_avg4

        if self.prev_range is None:
            self.prev_range = current_range

        if self.prev_range != current_range or self.after_spike > 0:
            if self.prev_range != current_range:
                self.consecutive_range_samples = 0
                self.after_spike = self.spike_filter_samples
            else:
                self.consecutive_range_samples += 1

            if current_range == "5":
                if self.consecutive_range_samples < 2:
                    self.rolling_avg = prev_rolling_avg
                    self.rolling_avg4 = prev_rolling_avg4
                adc = self.rolling_avg4
            else:
                adc = self.rolling_avg
            
            self.after_spike -= 1

        self.prev_range = current_range
        return adc

    def digital_channels(self, bits):
        """
        将原始数字数据转换为数字通道。

        返回一个具有 8 行（每个通道一行）的 2d 矩阵。每行包含所选通道的高电平和低电平值。
        """
        # 准备具有 8 行（每个通道一行）的 2d 矩阵
        digital_channels = [[], [], [], [], [], [], [], []]
        for sample in bits:
            digital_channels[0].append((sample & 1) >> 0)
            digital_channels[1].append((sample & 2) >> 1)
            digital_channels[2].append((sample & 4) >> 2)
            digital_channels[3].append((sample & 8) >> 3)
            digital_channels[4].append((sample & 16) >> 4)
            digital_channels[5].append((sample & 32) >> 5)
            digital_channels[6].append((sample & 64) >> 6)
            digital_channels[7].append((sample & 128) >> 7)
        return digital_channels

    def process_samples(self, buf, remainder):
        """
        返回在一个采样周期内读取的样本列表。
        样本值的数量取决于串行读取之间的延迟。
        样本的操作留给用户。
        有关更多信息，请参见示例。
        """
        sample_size = 4  # 一个模拟值的大小为 4 字节
        offset = remainder["len"]
        samples = []
        raw_digital_output = []

        if len(buf) + offset < sample_size:
            remainder["sequence"] += buf
            remainder["len"] += len(buf)
            return samples, raw_digital_output, remainder

        first_reading = (
            remainder["sequence"] + buf[0:sample_size-offset])[:4]
        adc_val = int.from_bytes(first_reading, byteorder="little", signed=False)
        measurement, bits = self._handle_raw_data(adc_val)
        if measurement is not None:
            samples.append(measurement)
        if bits is not None:
            raw_digital_output.append(bits)

        offset = sample_size - offset

        while offset <= len(buf) - sample_size:
            next_val = buf[offset:offset + sample_size]
            offset += sample_size
            adc_val = int.from_bytes(next_val, byteorder="little", signed=False)
            measurement, bits = self._handle_raw_data(adc_val)
            if measurement is not None:
                samples.append(measurement)
            if bits is not None:
                raw_digital_output.append(bits)

        remainder["sequence"] = buf[offset:len(buf)]
        remainder["len"] = len(buf)-offset

        # 返回样本列表和原始数字输出
        return samples, raw_digital_output, remainder
