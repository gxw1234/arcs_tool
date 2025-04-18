//Based on CSerialPort(V4.2.1.221107) serial port library
#ifdef _WIN32
#include <windows.h>
#define imsleep(microsecond) Sleep(microsecond) // ms
#else
#include <unistd.h>
#define imsleep(microsecond) usleep(1000 * microsecond) // ms
#endif

#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"
#include "blu_api.h"
using namespace itas109;
using namespace BLU;

#ifdef _WIN32
#include <tchar.h>
#include <SetupAPI.h>
#pragma comment(lib, "Setupapi.lib")
TCHAR strVidPid[] = _T("VID_15A2&PID_300A");
std::string str_comname;
std::string get_blu_com(void);
#define AUTO_GET_COM //The automatic acquisition of BLU serial port function is only supported on the Windows platform
#endif

#include <conio.h>

int main()
{
	int isPause = 0;
	int Keycode;
	double avg_samples;
	std::string str_avg_samples;
	std::string str_out;
#if defined(AUTO_GET_COM)
    std::string comport = get_blu_com();//Automatically obtain the serial port number of the BLU power analyzer
	if ("" != comport)
	{
		BLU_API blu939(comport);
#else
		BLU_API blu939("COM5");			//Manually specify the BLU power analyzer serial port number
#endif
		blu939.get_modifiers();         //Get the BLU parameters
		blu939.set_source_voltage(500);	//Set the output voltage:500mV
		blu939.toggle_DUT_power("ON");	//Power on
		blu939.start_measuring();		//Start sampling

		for (;;)
		{
			if (0 == isPause)
			{
				imsleep(60);
				avg_samples = blu939.Get_avg_Current();//The average current is obtained
				str_avg_samples = std::to_string(avg_samples);
				str_out = "Average of samples is: " + str_avg_samples + "uA";
				std::cout << str_out << std::endl;
			}
			if (_kbhit())
			{
				Keycode = _getch();					//Gets the key value pressed
				if (Keycode == 27)					//press Escape key
				{
					isPause = 1;
					blu939.stop_measuring();		//Stop sampling
					blu939.toggle_DUT_power("OFF");	//Turn off the power
				}
				else if (Keycode == 13)				//press Enter key
				{
					isPause = 0;
					blu939.toggle_DUT_power("ON");	//Power on
					blu939.start_measuring();		//Start sampling
				}
			}
		}
#if defined(AUTO_GET_COM)
	}
#else
#endif
    return 0;
}
#if defined(AUTO_GET_COM)
std::string TCHAR2STRING(TCHAR* STR)
{
	int iLen = WideCharToMultiByte(CP_ACP, 0,STR, -1, NULL, 0, NULL, NULL);
	char* chRtn = new char[iLen * sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
	std::string str(chRtn);
	return str;
}

std::string get_blu_com(void)
{
	//Get all devices used by the current system
	int nPort = -1;
	int nStart = -1;
	int nEnd = -1;
	int i = 0;
	std::string strName;
	DWORD dwFlag = (DIGCF_ALLCLASSES | DIGCF_PRESENT);
	HDEVINFO hDevInfo = INVALID_HANDLE_VALUE;
	SP_DEVINFO_DATA sDevInfoData;
	TCHAR* pPos;
	TCHAR szDis[MAX_PATH] = { 0x00 }; //Storage device instance ID
	TCHAR szFN[MAX_PATH] = { 0x00 };  //Storage device instance properties
	DWORD nSize = 0;
	size_t len;

	//Prepare to traverse all devices to find USB
	hDevInfo = SetupDiGetClassDevs(NULL, NULL, NULL, dwFlag);
	if (INVALID_HANDLE_VALUE == hDevInfo)
		goto STEP_END;

	//Start traversing all devices
	memset(&sDevInfoData, 0x00, sizeof(SP_DEVICE_INTERFACE_DATA));
	sDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	for (i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &sDevInfoData); i++)
	{
		nSize = 0;
		//Invalid device
		if (!SetupDiGetDeviceInstanceId(hDevInfo, &sDevInfoData, szDis, MAX_PATH, &nSize))
			goto STEP_END;

		//Based on the device information, look for devices with consistent VID PIDs
		pPos = wcsstr(szDis,strVidPid);
		if (NULL != pPos)
		{
			//Find the device properties
			nSize = 0;
			SetupDiGetDeviceRegistryProperty(hDevInfo, &sDevInfoData,
				SPDRP_FRIENDLYNAME,
				0, (PBYTE)szFN,
				sizeof(szFN),
				&nSize);
			pPos = wcsstr(szFN, _T("COM"));
			if (NULL != pPos)
			{
				//Look for serial port information
				len = wcslen(pPos);
				*(pPos + len - 1) = 0x00;//"COM5)"
				str_comname = TCHAR2STRING(pPos);
				return str_comname;
			}
			else
			{
				goto STEP_END;
			}
		}
		else
		{
			continue;
		}
	}
STEP_END:
	//Close the device information set handle
	if (hDevInfo != INVALID_HANDLE_VALUE)
	{
		SetupDiDestroyDeviceInfoList(hDevInfo);
		hDevInfo = INVALID_HANDLE_VALUE;
	}

	return "";
};
#endif