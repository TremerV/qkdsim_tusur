// File: ceSerial.cpp
// Description: ceSerial communication class implementation for Windows and Linux
// WebSite: http://cool-emerald.blogspot.sg/2017/05/serial-port-programming-in-c-with.html
// MIT License (https://opensource.org/licenses/MIT)
// Copyright (c) 2018 Yan Naing Aye

// References
// https://en.wikibooks.org/wiki/Serial_Programming/termios
// http://www.silabs.com/documents/public/application-notes/an197.pdf
// https://msdn.microsoft.com/en-us/library/ff802693.aspx
// http://www.cplusplus.com/forum/unices/10491/
#include <iostream>
#include "ceSerial.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>


#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/poll.h>
#include <sys/types.h>

#include <termios.h> /* POSIX terminal control definitions */
#include <linux/serial.h> // for RS-485
#include <sys/ioctl.h>

#include <cstddef>
#include <bitset>
namespace ce {

void ceSerial::Delay(unsigned long ms){
	usleep(ms*1000);
}

ceSerial::ceSerial()
{
	fd = -1;
	port = "/dev/ttyUSB0";
	SetBaudRate(9600);
    SetDataSize(8);
	SetParity('N');
    SetStopBits(1);
}

ceSerial::ceSerial(std::string Device, long BaudRate,long DataSize,char ParityType,float NStopBits)
{

	fd = -1;
	port = Device;
	SetBaudRate(BaudRate);
	SetDataSize(DataSize);
	SetParity(ParityType);
	SetStopBits(NStopBits);
}

ceSerial::~ceSerial()
{
	Close();
}

void ceSerial::SetPort(std::string Device) {
	port = Device;
}

std::string ceSerial::GetPort() {
	return port;
}

void ceSerial::SetDataSize(long nbits) {
	if ((nbits < 5) || (nbits > 8)) nbits = 8;
	dsize=nbits;
}

long ceSerial::GetDataSize() {
	return dsize;
}

void ceSerial::SetParity(char p) {
	if ((p != 'N') && (p != 'E') && (p != 'O')) 
	{
		p = 'N';
	}
	parity = p;
}

char ceSerial::GetParity() {
	return parity;
}

void ceSerial::SetStopBits(float nbits) {
	if (nbits >= 2) stopbits = 2;
    else stopbits = 1;
}

float ceSerial::GetStopBits() {
	return stopbits;
}

long ceSerial::Open(void) {

	struct termios settings;
	memset(&settings, 0, sizeof(settings));
	settings.c_iflag = 0;
	settings.c_oflag = 0;

	settings.c_cflag = CREAD | CLOCAL;//see termios.h for more information
	if(dsize==5)  settings.c_cflag |= CS5;//no change
	else if (dsize == 6)  settings.c_cflag |= CS6;
	else if (dsize == 7)  settings.c_cflag |= CS7;
	else settings.c_cflag |= CS8;

	if(stopbits==2) settings.c_cflag |= CSTOPB;

	if(parity!='N') settings.c_cflag |= PARENB;

	if (parity == 'O') settings.c_cflag |= PARODD;

	settings.c_lflag = 0;
	settings.c_cc[VMIN] = 1;
	settings.c_cc[VTIME] = 0;

    fd = open(port.c_str(), O_RDWR | O_NONBLOCK);
	if (fd == -1) {
		return -1;
	}
	cfsetospeed(&settings, baud);
	cfsetispeed(&settings, baud);

	tcsetattr(fd, TCSANOW, &settings);

	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return 0;
}

void ceSerial::Close() {
	if(IsOpened()) close(fd);
	fd=-1;
}

bool ceSerial::IsOpened()
{
	if(fd== (-1)) return false;
	else return true;
}

void ceSerial::SetBaudRate(long baudrate) {
	if (baudrate < 50) baud = B0;
	else if (baudrate < 75) baud = B50;
	else if (baudrate < 110) baud = B75;
	else if (baudrate < 134) baud = B110;
	else if (baudrate < 150) baud = B134;
	else if (baudrate < 200) baud = B150;
	else if (baudrate < 300) baud = B200;
	else if (baudrate < 600) baud = B300;
	else if (baudrate < 1200) baud = B600;
	else if (baudrate < 2400) baud = B1200;
	else if (baudrate < 4800) baud = B2400;
	else if (baudrate < 9600) baud = B4800;
	else if (baudrate < 19200) baud = B9600;
	else if (baudrate < 38400) baud = B19200;
	else if (baudrate < 57600) baud = B38400;
	else if (baudrate < 115200) baud = B57600;
	else if (baudrate < 230400) baud = B115200;
	else baud = B230400;
}

long ceSerial::GetBaudRate() {
	long baudrate=9600;
	if (baud < B50) baudrate = 0;
	else if (baud < B75) baudrate = 50;
	else if (baud < B110) baudrate = 75;
	else if (baud < B134) baudrate = 110;
	else if (baud < B150) baudrate = 134;
	else if (baud < B200) baudrate = 150;
	else if (baud < B300) baudrate = 200;
	else if (baud < B600) baudrate = 300;
	else if (baud < B1200) baudrate = 600;
	else if (baud < B2400) baudrate = 1200;
	else if (baud < B4800) baudrate = 2400;
	else if (baud < B9600) baudrate = 4800;
	else if (baud < B19200) baudrate =9600;
	else if (baud < B38400) baudrate = 19200;
	else if (baud < B57600) baudrate = 38400;
	else if (baud < B115200) baudrate = 57600;
	else if (baud < B230400) baudrate = 115200;
	else baudrate = 230400;
	return baudrate;
}

ce::Package ceSerial:: Read_com(int timeout){
    bool startPackFlag_=false;
    bool flag_ = 0;

    ce::Package pack;
    uint8_t currentByte=0;


    if (!IsOpened()) {  return {0,0,0,0,0,0,0,0,0,0,0,0,0};	}

    struct pollfd fds;
    fds.fd=fd;
    fds.events = POLLIN;

    while (startPackFlag_ != true) {
        if (poll(&fds, 1, timeout) > 0){
            read(fd, &buffer, 1);
            currentByte = buffer;
            if (currentByte == 255){ flag_ = 1;}
            else {if (flag_ == 1){
                    if (currentByte != 254) {flag_ = 0;}
                    else{startPackFlag_ = true;
                            break;}
                    }else {flag_ = 0;}
                }
        }else {std::cout<< "!!!ВЫШЕЛ ТАЙМАУТ!!!"<<std::endl;
            break;}//конец таймаута
    }
    uint16_t  params [11] = {0};
    if (startPackFlag_ == true){
        int count = 10;
        int paramCnt =1;
        unsigned short p=0;
        while (p != 65535) {
            if (poll(&fds, 1, timeout) > 0){
                read(fd, &buffer, 1);
                currentByte = buffer;
                if (count>9){ pack.status_=currentByte; }
                else if (count>8){ pack.nameCommand_=currentByte; }
                else if (count>7){ pack.crc_=currentByte; }
                else{
                    p = p + currentByte;
                    if (count>6){ p = p<<8; }
                    else{
                        paramCnt++;
                        if (p==65535){
                            break;}
                        else{
                            params[0] = paramCnt-1;
                            params[paramCnt-1]= p;
                            p=0;
                            count=8;
                        }
                    }
                }
                count--;
            }else{std::cout<< "!!!ВЫШЕЛ ТАЙМАУТ!!!"<<std::endl;
                break;}
        }
    }else {return {0,0,0,0,0,0,0,0,0,0,0,0,0};}

    pack.param1 = params[1];
    pack.param2 = params[2];
    pack.param3 = params[3];
    pack.param4 = params[4];
    pack.param5 = params[5];
    pack.param6 = params[6];
    pack.param7 = params[7];
    pack.param8 = params[8];
    pack.param9 = params[9];
    pack.param10 = params[10];
    return pack ;
}
uint16_t ceSerial::ReadChar(bool& success)
{
	success=false;
	if (!IsOpened()) {return 0;	}
    success=read(fd, &rxchar, 1)==1;
	return rxchar;
}
std::string ceSerial::ReadChar()
{
    if (!IsOpened()) {return "0";}
    read(fd, &buf, 8);
    return buf;
}
bool ceSerial::Write(uint16_t *data)
{
    if (!IsOpened()) {return false;	}

    int n = sizeof(*data);
    if (n < 0) n = 0;
    else if(n > 1024) n = 1024;
    return (write(fd, data,n)==n);
}

bool ceSerial::Write(uint8_t *data)
{
    if (!IsOpened()) {return false;	}
    long n = sizeof(*data);
    if (n < 0) n = 0;
    else if(n > 1024) n = 1024;
    return (write(fd, data, n)==n);
}
bool ceSerial::Write(char * data)
{
    if (!IsOpened()) {return false;	}
    long n = strlen(data);
    std:: cout<< data<<std::endl;
    std:: cout<< n<<std::endl;
    if (n < 0) n = 0;
    else if(n > 1024) n = 1024;
    return (write(fd, data, n)==n);
}

bool ceSerial::Write(char *data,long n)
{
	if (!IsOpened()) {return false;	}
	if (n < 0) n = 0;
	else if(n > 1024) n = 1024;
	return (write(fd, data, n)==n);
}

bool ceSerial::WriteChar(char *ch)
{
    if (!IsOpened()) {return false;	}
    long n = strlen(ch);
    if (n < 0) n = 0;
    else if(n > 1024) n = 1024;
    return (write(fd, ch, n)==n);
}

bool ceSerial::SetRTS(bool value) {
	long RTS_flag = TIOCM_RTS;
	bool success=true;
	if (value) {//Set RTS pin
		if (ioctl(fd, TIOCMBIS, &RTS_flag) == -1) success=false;
	}
	else {//Clear RTS pin
		if (ioctl(fd, TIOCMBIC, &RTS_flag) == -1) success=false;
	}
	return success;
}

bool ceSerial::SetDTR(bool value) {
	long DTR_flag = TIOCM_DTR;
	bool success=true;
	if (value) {//Set DTR pin
		if (ioctl(fd, TIOCMBIS, &DTR_flag) == -1) success=false;
	}
	else {//Clear DTR pin
		if (ioctl(fd, TIOCMBIC, &DTR_flag) == -1) success=false;
	}
	return success;
}

bool ceSerial::GetCTS(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_CTS) != 0);
}

bool ceSerial::GetDSR(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_DSR) != 0);
}

bool ceSerial::GetRI(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_RI) != 0);
}

bool ceSerial::GetCD(bool& success) {
	success=true;
	long status;
	if(ioctl(fd, TIOCMGET, &status)== -1) success=false;
	return ((status & TIOCM_CD) != 0);
}

} // namespace ce 

