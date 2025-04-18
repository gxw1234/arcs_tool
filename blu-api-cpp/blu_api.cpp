//CSerialPort Version: https://github.com/itas109/CSerialPort - V4.2.1.221107

#ifdef _WIN32
#include <windows.h>
#define imsleep(microsecond) Sleep(microsecond) // ms
#else
#include <unistd.h>
#define imsleep(microsecond) usleep(1000 * microsecond) // ms
#endif

#include <regex>
#include "blu_api.h"

using namespace itas109;

#define SPIKE_FILTER

std::vector<std::string> stringSplit(const std::string& str, char delim) {
	std::string s;
	s.append(1, delim);
	std::regex reg(s);
	std::vector<std::string> elems(std::sregex_token_iterator(str.begin(), str.end(), reg, -1),
		std::sregex_token_iterator());
	return elems;
}

template<typename T>
T SumVector(std::vector<T>& vec)
{
	T res = 0;
	for (size_t i = 0; i < vec.size(); i++)
	{
		res += vec[i];
	}
	return res;
};

namespace BLU
{
	MyListener::MyListener(CSerialPort* sp)
		//: p_sp(sp)
	{
		p_sp = sp;

		this->MEAS_ADC = this->_generate_mask(14, 0);
		this->MEAS_RANGE = this->_generate_mask(3, 14);
		this->MEAS_LOGIC = this->_generate_mask(8, 24);
		this->rolling_avg = 0xFFFFFFFF; // null;
		this->rolling_avg5 = 0xFFFFFFFF; // null;
		this->spike_filter_alpha = 0.18; // 0.18;
		this->spike_filter_alpha5 = 0.06; // 0.06;
		this->prev_range = 0xFFFFFFFF; // null;
		this->consecutive_range_samples = 0;
		this->after_spike = 0;
		this->spike_filter_samples = 3;
		this->adc_mult = 1.8 / 163840;

		this->current_cmd = 0;

		this->R_FWx[0] = 0;
		this->R_FWx[1] = 0;
		this->R_FWx[2] = 0;
		this->R_FWx[3] = 0;

		this->O_FWx[0] = 0;
		this->O_FWx[1] = 0;
		this->O_FWx[2] = 0;
		this->O_FWx[3] = 0;
	};

	bool MyListener::_parse_metadata(const std::string& metadata)
	{
		char delim = 10;// "\n";
		int i,j;
		int pos, pos2;
		char str_find_r[3] = "R";
		char str_find_o[3] = "O";
		std::string str_r;
		std::string str_o;
		std::vector<std::string> data_split;

		try
		{
			data_split = stringSplit(metadata, delim);
			for (j=0;j<6;j++)
			{
				for (i = 0; i < 47; i++)
				{
					str_find_r[1] = 0x30 + j;
					pos = data_split[i].find(str_find_r);
					if (0xffffffff != pos)
					{
						str_r = data_split[i].substr(4, data_split[i].size());
						this->R_FWx[j] = atof(str_r.c_str());//
					}
					str_find_o[1] = 0x30 + j;
					pos2 = data_split[i].find(str_find_o);
					if (0xffffffff != pos2)
					{
						str_o = data_split[i].substr(4, data_split[i].size());
						this->O_FWx[j] = atof(str_o.c_str());
					}
				}
			}
			return true;
		}
		catch (const std::exception& e)
		{
			std::cerr << "_parse_metadata exception caught: " << e.what() << '\n';
			return false;
		}
	}

	int MyListener::_get_masked_value(int value, std::unordered_map<std::string, int>& meas)
	{
		int masked_value = (value & meas["mask"]) >> static_cast<int>(meas["pos"]);
		if (meas["pos"] == 24)
		{
			if (masked_value == 255)
			{
				masked_value = -1;
			}
		}
		return masked_value;
	}
	double MyListener::get_adc_result(int current_range, int adc_value)
	{
		std::string str_current_range = std::to_string(current_range);
		double result_without_gain = (adc_value - this->O_FWx[current_range]) * (this->adc_mult / this->R_FWx[current_range]);
		double adc = result_without_gain;
		auto prev_rolling_avg = this->rolling_avg;
		auto prev_rolling_avg5 = this->rolling_avg5;
		// spike filtering / rolling average
		//if (this.rolling_avg == null)//0xFFFFFFFF
#if defined(SPIKE_FILTER)
		if (this->rolling_avg == 0xFFFFFFFF)
		{
			this->rolling_avg = adc;
		}
		else
		{
			this->rolling_avg = this->spike_filter_alpha * adc + (1 - this->spike_filter_alpha) * this->rolling_avg;
		}
		if (this->rolling_avg5 == 0xFFFFFFFF)
		{
			this->rolling_avg5 = adc;
		}
		else
		{
			this->rolling_avg5 = this->spike_filter_alpha5 * adc + (1 - this->spike_filter_alpha5) * this->rolling_avg5;
		}
		if (this->prev_range == 0xFFFFFFFF)
		{
			this->prev_range = current_range;
		}
		if (this->prev_range != current_range || this->after_spike > 0)
		{
			if (this->prev_range != current_range)
			{
				this->consecutive_range_samples = 0;
				this->after_spike = this->spike_filter_samples;
			}
			else
			{
				this->consecutive_range_samples += 1;
			}
			if (str_current_range == "5")
			{
				if (this->consecutive_range_samples < 2)
				{
					this->rolling_avg = prev_rolling_avg;
					this->rolling_avg5 = prev_rolling_avg5;
				}
				adc = this->rolling_avg5;
			}
			else
			{
				adc = this->rolling_avg;
			}
			this->after_spike -= 1;
		}
		this->prev_range = current_range;
#endif
		return adc;
	}
	double MyListener::_handle_raw_data(int adc_value)
	{
		double analog_value = 0;
		try
		{
			int current_measurement_range = min(this->_get_masked_value(adc_value, this->MEAS_RANGE), 5);
			int adc_result = this->_get_masked_value(adc_value, this->MEAS_ADC) * 4;
			int bits = this->_get_masked_value(adc_value, this->MEAS_LOGIC);

			analog_value = this->get_adc_result(current_measurement_range, adc_result) * std::pow(10, 6);
			return analog_value;
		}
		catch (const std::exception& e)
		{
			std::cerr << "_handle_raw_data exception caught: " << e.what() << '\n';
			//std::cout << "Measurement outside of range!" << std::endl;
			//return null;
			return 0xFFFFFFFF;
		}
	}

	std::vector<double> MyListener::get_samples(char* buf, int lenbuf)
	{
		int sample_size = 4;
		int offset = 0;
		char next_val[5];
		int adc_val;
		std::vector<double> samples;
		double measurement;

		while (offset <= lenbuf - sample_size)
		{
			memset(next_val,0x00,sizeof(next_val));
			memcpy(next_val, buf + offset,4);
			offset += sample_size;
			memcpy(&adc_val, next_val, 4);
			measurement = this->_handle_raw_data(adc_val);
			if (measurement != 0xFFFFFFFF)
			{
				samples.push_back(measurement);
			}
		}
		return samples;
	}

	std::unordered_map<std::string, int> MyListener::_generate_mask(int bits, int pos)
	{
		int mask = (static_cast<int>(std::pow(2, bits)) - 1) << pos;
		std::unordered_map<std::string, int> rst;
		rst.emplace("mask", mask);
		rst.emplace("pos", pos);
		return rst;
	}

	void MyListener::Set_avg_Current(double ac)
	{
		avg_Current = ac;
	}

	double MyListener::Get_avg_Current()
	{
		return avg_Current;
	}
	void MyListener::onReadEvent()
	{
		char data[2048] = { 0 };//4096
		std::string str;
		std::vector<double> samples;
		int len_samples;
		double sum_samples;
		double avg_samples;
		std::string str_avg_samples;
		std::string str_out;

		memset(data,0x00,sizeof(data));
		int recLen = p_sp->readAllData(data);

		if (recLen > 0)
		{
			if (this->current_cmd == BLU_Command::GET_META_DATA)
			{
				str = data;
				_parse_metadata(str);
			}
			else if (this->current_cmd == BLU_Command::AVERAGE_START)
			{
				samples = this->get_samples(data, recLen);
				len_samples = samples.size();
				sum_samples = SumVector(samples);
				avg_samples = sum_samples / len_samples;
				this->Set_avg_Current(avg_samples);
				str_avg_samples = std::to_string(avg_samples);
				//str_out = "Average of samples is: " + str_avg_samples + "uA";
				//std::cout << str_out << std::endl;
				//_average_samples(get_samples(array, bytesToRead), 512);
				//imsleep(100);
			}
		}
	};

	void BLU_API::_write_serial(std::tuple<unsigned char, unsigned char, unsigned char>& cmd_tuple)
	{
		int count = 1;

		std::vector<unsigned char> send(3);
		send[0] = std::get<0>(cmd_tuple);
		send[1] = std::get<1>(cmd_tuple);
		send[2] = std::get<2>(cmd_tuple);
		std::string str(send.begin(), send.end());
		
		if (0xFF == send[1])
		{
			count = 1;
		}
		else if (0xFF == send[2])
		{
			count = 2;
		}
		else
		{
			count = 3;
		}

		try
		{
			//this->ser->DiscardOutBuffer(); 
			this->ser->writeData(str.c_str(), count);
		}
		catch (const std::exception& e)
		{
			std::cerr << "An error occured when writing to serial port: " << e.what() << '\n';
		}
	}


	bool BLU_API::get_modifiers()
	{
		std::tuple<unsigned char, unsigned char, unsigned char> cmd_tuplex;
		bool ret = false;
		unsigned char param = 0xFF;
		current_cmd = BLU_Command::GET_META_DATA;
		this->plistener->current_cmd = current_cmd;
		cmd_tuplex = std::make_tuple(current_cmd, param, param);
		this->_write_serial(cmd_tuplex);
		imsleep(100);
		return ret;
	}
	std::tuple<unsigned char, unsigned char> BLU_API::_convert_source_voltage(int mV)
	{
		unsigned char set_b_1;
		unsigned char set_b_2;

		if (mV < this->vdd_low)
		{
			mV = this->vdd_low;
		}

		if (mV > this->vdd_high)
		{
			mV = this->vdd_high;
		}

		set_b_1 = static_cast<unsigned char>((mV & 0xFF00) >> 8);
		set_b_2 = static_cast<unsigned char>((mV & 0x00FF));

		return std::make_tuple(set_b_1, set_b_2);
	}
	void BLU_API::set_source_voltage(int mV)
	{
		std::tuple<unsigned char, unsigned char, unsigned char> cmd_tuplex;
		std::tuple<unsigned char, unsigned char> _tup_1 = this->_convert_source_voltage(mV);
		unsigned char b_1 = std::get<0>(_tup_1);
		unsigned char b_2 = std::get<1>(_tup_1);
		current_cmd = BLU_Command::REGULATOR_SET;
		this->plistener->current_cmd = current_cmd;
		cmd_tuplex = std::make_tuple(current_cmd, b_1, b_2);
		this->_write_serial(cmd_tuplex);
		this->current_vdd = mV;
		imsleep(10);
	}
	void BLU_API::toggle_DUT_power(const std::string& state)
	{
		unsigned char param = 0xFF;
		std::tuple<unsigned char, unsigned char, unsigned char> cmd_tuplex;
		if (state == "ON")
		{
			current_cmd = BLU_Command::DEVICE_RUNNING_SET;
			this->plistener->current_cmd = current_cmd;
			cmd_tuplex = std::make_tuple(current_cmd, BLU_Command::TRIGGER_SET, param);
			this->_write_serial(cmd_tuplex);
		}
		if (state == "OFF")
		{
			current_cmd = BLU_Command::DEVICE_RUNNING_SET;
			this->plistener->current_cmd = current_cmd;
			cmd_tuplex = std::make_tuple(current_cmd, BLU_Command::NO_OP, param);
			this->_write_serial(cmd_tuplex);
		}
		imsleep(10);
	}

	void BLU_API::start_measuring()
	{
		unsigned char param = 0xFF;
		std::tuple<unsigned char, unsigned char, unsigned char> cmd_tuplex;
		current_cmd = BLU_Command::AVERAGE_START;
		this->plistener->current_cmd = current_cmd;
		cmd_tuplex = std::make_tuple(current_cmd, param, param);
		this->_write_serial(cmd_tuplex);
	}
	void BLU_API::stop_measuring()
	{
		unsigned char param = 0xFF;
		std::tuple<unsigned char, unsigned char, unsigned char> cmd_tuplex;
		current_cmd = BLU_Command::AVERAGE_STOP;
		this->plistener->current_cmd = current_cmd;
		cmd_tuplex = std::make_tuple(current_cmd, param, param);
		this->_write_serial(cmd_tuplex);
	}
	double BLU_API::Get_avg_Current()
	{
		avg_Current = plistener->Get_avg_Current();
		return avg_Current;
	}
	BLU_API::BLU_API(std::string portName)
	{
		ser = new CSerialPort();
		plistener = new MyListener(ser);
		
		ser->init(portName,				// windows:COM1 Linux:/dev/ttyS0
			itas109::BaudRate9600,		// baudrate
			itas109::ParityNone,		// parity
			itas109::DataBits8,			// data bit
			itas109::StopOne,			// stop bit
			itas109::FlowNone,			// flow
			2048						// read buffer size: 2048
		);
		
		ser->setReadIntervalTimeout(0); // read interval timeout 1ms
		ser->open();
		// connect for read
		ser->readReady.connect(plistener, &MyListener::onReadEvent);
		this->vdd_low = 500;
		this->vdd_high = 5000;
		this->current_vdd = 0;
	};
}
