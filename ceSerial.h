// File: ceSerial.h
// Description: ceSerial communication class for Windows and Linux
// WebSite: http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html
// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2018 Yan Naing Aye

// References
// https://en.wikibooks.org/wiki/Serial_Programming/termios
// http://www.silabs.com/documents/public/application-notes/an197.pdf
// https://msdn.microsoft.com/en-us/library/ff802693.aspx
// http://www.cplusplus.com/forum/unices/10491/

#ifndef CESERIAL_H
#define CESERIAL_H
#include <string>

namespace ce {

struct Package{
        uint8_t status_= 0;
        uint8_t nameCommand_ = 0;
        uint8_t crc_= 0;
        uint16_t param1= 0;
        uint16_t param2= 0;
        uint16_t param3= 0;
        uint16_t param4= 0;
        uint16_t param5= 0;
        uint16_t param6= 0;
        uint16_t param7= 0;
        uint16_t param8= 0;
        uint16_t param9= 0;
        uint16_t param10= 0;
    };

class ceSerial {
    uint16_t rxchar;
    uint8_t buffer;
    std::string buf;
	std::string port;
	long baud;
	long dsize;
	char parity;
	float stopbits;
    long fd;//serial_fd


public:
	static void Delay(unsigned long ms);
	ceSerial();
	ceSerial(std::string Device, long BaudRate, long DataSize, char ParityType, float NStopBits);
	~ceSerial();
	long Open(void);//return 0 if success
	void Close();
    std::string ReadChar();
    ce::Package Read_com(int timeout);
    uint16_t ReadChar(bool& success);//return read char if success
    bool WriteChar(char *ch);////return success flag
    bool Write(uint16_t *data);//write null terminated string and return success flag
    bool Write(uint8_t *data);
    bool Write(char * data);
	bool Write(char *data,long n);
	bool SetRTS(bool value);//return success flag
	bool SetDTR(bool value);//return success flag
	bool GetCTS(bool& success);
	bool GetDSR(bool& success);
	bool GetRI(bool& success);
	bool GetCD(bool& success);
	bool IsOpened();
	void SetPort(std::string Port);
	std::string GetPort();
	void SetBaudRate(long baudrate);
	long GetBaudRate();
	void SetDataSize(long nbits);
	long GetDataSize();
	void SetParity(char p);
	char GetParity();
	void SetStopBits(float nbits);
	float GetStopBits();


};



} // namespace ce 
#endif // CESERIAL_H

