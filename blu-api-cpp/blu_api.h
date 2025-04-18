//CSerialPort Version: https://github.com/itas109/CSerialPort - V4.2.1.221107

#pragma once

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#define imsleep(microsecond) Sleep(microsecond) // ms
#else
#include <unistd.h>
#define imsleep(microsecond) usleep(1000 * microsecond) // ms
#endif

#include <vector>
#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"
#include <string>
#include <unordered_map>
#include <cmath>
#include <stdexcept>
#include <tuple>

using namespace itas109;

namespace BLU
{
	class MyListener : public has_slots<>
	{
	public:
		MyListener(CSerialPort* sp);
		bool _parse_metadata(const std::string& metadata);
		std::vector<double> get_samples(char* buf, int lenbuf);
		void Set_avg_Current(double ac);
		double Get_avg_Current();
		void onReadEvent();
		double _handle_raw_data(int adc_value);
		int _get_masked_value(int value, std::unordered_map<std::string, int>& meas);
		std::unordered_map<std::string, int> _generate_mask(int bits, int pos);
		double get_adc_result(int current_range, int adc_value);
	private:
		CSerialPort* p_sp;
		double avg_Current = 0;
		double adc_mult = 0;
		double R_FWx[6];
		double O_FWx[6];
	public:
		std::unordered_map<std::string, int> MEAS_ADC;
		std::unordered_map<std::string, int> MEAS_RANGE;
		std::unordered_map<std::string, int> MEAS_LOGIC;
		unsigned char current_cmd;
		double rolling_avg = 0;
		double rolling_avg5 = 0;
		double spike_filter_alpha = 0;
		double spike_filter_alpha5 = 0;
		long long prev_range = 0;
		long long consecutive_range_samples = 0;
		long long after_spike = 0;
		long long spike_filter_samples = 0;
	};
	class BLU_Command final
	{

	public:
		static constexpr unsigned char NO_OP = 0x00;
		static constexpr unsigned char TRIGGER_SET = 0x01;
		static constexpr unsigned char AVERAGE_START = 0x06;
		static constexpr unsigned char AVERAGE_STOP = 0x07;
		static constexpr unsigned char DEVICE_RUNNING_SET = 0x0c;
		static constexpr unsigned char REGULATOR_SET = 0x0d;
		static constexpr unsigned char GET_META_DATA = 0x19;
	};
	/****************class BLU_API/****************/
	class BLU_API
	{

	public:
		class BLU_Modes
		{

		public:
			std::string SOURCE_MODE = "SOURCE_MODE";
		};

	public:
		double avg_Current = 0;
		unsigned char current_cmd = BLU_Command::NO_OP;
		CSerialPort* ser;
		MyListener* plistener;
	private:
		int vdd_low = 0;
		int vdd_high = 0;
		int current_vdd = 0;
		std::string mode;

	public:
		virtual ~BLU_API()
		{
			delete plistener;
			delete ser;
		}

		BLU_API(std::string portName);

	public:
		virtual void _write_serial(std::tuple<unsigned char, unsigned char, unsigned char> &cmd_tuple);
		virtual std::tuple<unsigned char, unsigned char> _convert_source_voltage(int mV);
		virtual bool get_modifiers();
		virtual void set_source_voltage(int mV);
		virtual void toggle_DUT_power(const std::string &state);
		virtual void stop_measuring();
		virtual void start_measuring();
		double Get_avg_Current();
	};
}
