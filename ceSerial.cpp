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
#include <fstream>
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
using namespace std;
namespace ce {

void ceSerial::Delay(unsigned long ms){
    usleep(ms*1000);
}

ceSerial::ceSerial()
{
    fd_ = -1;
    port_ = "/dev/ttyUSB0";
    SetBaudRate(9600);
    SetDataSize(8);
    SetParity('N');
    SetStopBits(1);


}

ceSerial::ceSerial(std::string Device, long BaudRate,long DataSize,char ParityType,float NStopBits)
{

    fd_ = -1;
    port_ = Device;
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
    port_ = Device;
}

std::string ceSerial::GetPort() {
    return port_;
}

void ceSerial::SetDataSize(long nbits) {
    if ((nbits < 5) || (nbits > 8)) nbits = 8;
    dsize_=nbits;
}

long ceSerial::GetDataSize() {
    return dsize_;
}

void ceSerial::SetParity(char p) {
    if ((p != 'N') && (p != 'E') && (p != 'O'))
    {
        p = 'N';
    }
    parity_ = p;
}

char ceSerial::GetParity() {
    return parity_;
}

void ceSerial::SetStopBits(float nbits) {
    if (nbits >= 2) stopbits_ = 2;
    else stopbits_ = 1;
}

float ceSerial::GetStopBits() {
    return stopbits_;
}

long ceSerial::Open(void) {

    struct termios settings;
    memset(&settings, 0, sizeof(settings));
    settings.c_iflag = 0;
    settings.c_oflag = 0;

    settings.c_cflag = CREAD | CLOCAL;
    if(dsize_==5)  settings.c_cflag |= CS5;
    else if (dsize_ == 6)  settings.c_cflag |= CS6;
    else if (dsize_ == 7)  settings.c_cflag |= CS7;
    else settings.c_cflag |= CS8;

    if(stopbits_==2) settings.c_cflag |= CSTOPB;

    if(parity_!='N') settings.c_cflag |= PARENB;

    if (parity_ == 'O') settings.c_cflag |= PARODD;

    settings.c_lflag = 0;
    settings.c_cc[VMIN] = 1;
    settings.c_cc[VTIME] = 0;
    fd_ = open(port_.c_str(), O_RDWR | O_NONBLOCK);
    if (fd_ == -1) {
        return -1;
    }
    cfsetospeed(&settings, baud_);
    cfsetispeed(&settings, baud_);

    tcsetattr(fd_, TCSANOW, &settings);

    int flags = fcntl(fd_, F_GETFL);
    fcntl(fd_, F_SETFL, flags| O_RDWR | O_NONBLOCK);
    tcflush(fd_,TCIOFLUSH);
    return 0;
}

void ceSerial::Close() {
    if(IsOpened()) close(fd_);
    fd_=-1;
}

bool ceSerial::IsOpened()
{
    if(fd_== (-1)) return false;
    else return true;
}

void ceSerial::SetBaudRate(long baudrate) {
    if (baudrate < 50) baud_ = B0;
    else if (baudrate < 75) baud_ = B50;
    else if (baudrate < 110) baud_ = B75;
    else if (baudrate < 134) baud_ = B110;
    else if (baudrate < 150) baud_ = B134;
    else if (baudrate < 200) baud_ = B150;
    else if (baudrate < 300) baud_ = B200;
    else if (baudrate < 600) baud_ = B300;
    else if (baudrate < 1200) baud_ = B600;
    else if (baudrate < 2400) baud_ = B1200;
    else if (baudrate < 4800) baud_ = B2400;
    else if (baudrate < 9600) baud_ = B4800;
    else if (baudrate < 19200) baud_ = B9600;
    else if (baudrate < 38400) baud_ = B19200;
    else if (baudrate < 57600) baud_ = B38400;
    else if (baudrate < 115200) baud_ = B57600;
    else if (baudrate < 230400) baud_ = B115200;
    else baud_ = B230400;
}

long ceSerial::GetBaudRate() {
    long baudrate=9600;
    if (baud_ < B50) baudrate = 0;
    else if (baud_ < B75) baudrate = 50;
    else if (baud_ < B110) baudrate = 75;
    else if (baud_ < B134) baudrate = 110;
    else if (baud_ < B150) baudrate = 134;
    else if (baud_ < B200) baudrate = 150;
    else if (baud_ < B300) baudrate = 200;
    else if (baud_ < B600) baudrate = 300;
    else if (baud_ < B1200) baudrate = 600;
    else if (baud_ < B2400) baudrate = 1200;
    else if (baud_ < B4800) baudrate = 2400;
    else if (baud_ < B9600) baudrate = 4800;
    else if (baud_ < B19200) baudrate =9600;
    else if (baud_ < B38400) baudrate = 19200;
    else if (baud_ < B57600) baudrate = 38400;
    else if (baud_ < B115200) baudrate = 57600;
    else if (baud_ < B230400) baudrate = 115200;
    else baudrate = 230400;
    return baudrate;
}

ce::UartResponse ceSerial:: Read_com(unsigned int timeout){
    bool startPackFlag_=false;
    bool flag_ = 0;
    uint8_t buffer = 0;
    ce::UartResponse pack_;
    uint8_t currentByte_=0;
    timeout = timeout *1000;


    if (!IsOpened()) {std::cout<< "Проверьте соединение со стендом"<<std::endl;
        return {0,0,0,{0,0,0,0,0,0,0,0,0,0}};	}
    struct pollfd fds;
    fds.fd=fd_;
    fds.events = POLLIN;

    while (startPackFlag_ != true) {
        if (poll(&fds, 1, timeout) > 0){
            try {
                read(fd_, &buffer, 1);
            }  catch (...) {
                cerr<<"Проблема с чтением"<< endl;
                return {0,0,0,{0,0,0,0,0,0,0,0,0,0}};
            }
            currentByte_ = buffer;
            if (currentByte_ == 255){ flag_ = 1;}
            else {if (flag_ == 1){
                    if (currentByte_ != 254) {flag_ = 0;}
                    else{startPackFlag_ = true;
                            break;}
                    }else {flag_ = 0;}
                }
        }else {std::cout<< "!!!ВЫШЕЛ ТАЙМАУТ!!!"<<std::endl;
            break;}//конец таймаута
    }
    array <uint16_t,11>  params  = {0,0,0,0,0,0,0,0,0,0,0};
    if (startPackFlag_ == true){
        int count = 10;
        int paramCnt =1;
        unsigned short p=0;
        while (p != 65535) {
            if (poll(&fds, 1, timeout) > 0){
                try {
                    read(fd_, &buffer, 1);
                }  catch (...) {
                    cerr<<"Проблема с чтением"<< endl;
                    return {0,0,0,{0,0,0,0,0,0,0,0,0,0}};
                }
                currentByte_ = buffer;
                if (count>9){ pack_.status_=currentByte_;
                             // p=(p<<8)+currentByte_;
                }
                else if (count>8){ pack_.nameCommand_=currentByte_;
                                   /*p=(p<<8) + currentByte_;
                                   if(p==65535){
                                        return {3,0,0,{0,0,0,0,0,0,0,0,0,0}};}*/}
                else if (count>7){ pack_.crc_=currentByte_;
                                   /*p=(p<<8) + currentByte_;
                                   if(p==65535){
                                        return {3,0,0,{0,0,0,0,0,0,0,0,0,0}};}*/}
                else{
                    p = p + currentByte_;
                    if (count>6){ p = p<<8; }
                    else{
                        paramCnt++;
                        if (paramCnt> (int)params.size()+1){return {0,0,0,{0,0,0,0,0,0,0,0,0,0}};}
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
            }else{std::cout<< "!!!ВЫШЕЛ ТАЙМАУТ!!! \n" << "Параметры не считаны"<<std::endl;
                tcflush(fd_,TCIOFLUSH);
                return {3,0,0,{0,0,0,0,0,0,0,0,0,0}};}
        }
    }else {cout<< " Ответа нет или он некорректный"<<endl;
        tcflush(fd_,TCIOFLUSH);
        return {3,0,0,{0,0,0,0,0,0,0,0,0,0}};}
    for(int i=0; i<(int)params.size()-1; i++){pack_.parameters_[i] = params[i+1];};

    return pack_ ;
}
char ceSerial::ReadChar(bool& success)
{
    success=false;
    if (!IsOpened()) {return 0;	}
    success=read(fd_, &rxchar_, 1)==1;
    return rxchar_;
}
char ceSerial::ReadChar()
{
    if (!IsOpened()) {return 0;	}
    read(fd_, &rxchar_, 1);
    return rxchar_;
}
bool ceSerial::Write(uint8_t data)
{
     if (!IsOpened()) {return false;  }
     tcflush(fd_,TCIFLUSH);
     int dataSize = 1;
     return (write(fd_, &data, dataSize)==dataSize);
}

bool ceSerial::Write(uint16_t data)
{
     if (!IsOpened()) {return false;  }
     tcflush(fd_,TCIFLUSH);
     int dataSize = 2;
     return (write(fd_, &data, dataSize)==dataSize);
}
bool ceSerial::Write(char * data)
{
    if (!IsOpened()) {return false;	}
    tcflush(fd_,TCIFLUSH);
    long n = strlen(data);
    if (n < 0) n = 0;
    else if(n > 1024) n = 1024;
    return (write(fd_, data, n)==n);
}

bool ceSerial::Write(char *data,long n)
{
    if (!IsOpened()) {return false;	}
    tcflush(fd_,TCIFLUSH);
    if (n < 0) n = 0;
    else if(n > 1024) n = 1024;
    return (write(fd_, data, n)==n);
}

bool ceSerial::WriteChar(char ch)
{
    char s[2];
    s[0]=ch;
    s[1]=0;//null terminated
    return Write(s);
}

bool ceSerial::SetRTS(bool value) {
    long RTS_flag = TIOCM_RTS;
    bool success=true;
    if (value) {//Set RTS pin
        if (ioctl(fd_, TIOCMBIS, &RTS_flag) == -1) success=false;
    }
    else {//Clear RTS pin
        if (ioctl(fd_, TIOCMBIC, &RTS_flag) == -1) success=false;
    }
    return success;
}

bool ceSerial::SetDTR(bool value) {
    long DTR_flag = TIOCM_DTR;
    bool success=true;
    if (value) {//Set DTR pin
        if (ioctl(fd_, TIOCMBIS, &DTR_flag) == -1) success=false;
    }
    else {//Clear DTR pin
        if (ioctl(fd_, TIOCMBIC, &DTR_flag) == -1) success=false;
    }
    return success;
}

bool ceSerial::GetCTS(bool& success) {
    success=true;
    long status;
    if(ioctl(fd_, TIOCMGET, &status)== -1) success=false;
    return ((status & TIOCM_CTS) != 0);
}

bool ceSerial::GetDSR(bool& success) {
    success=true;
    long status;
    if(ioctl(fd_, TIOCMGET, &status)== -1) success=false;
    return ((status & TIOCM_DSR) != 0);
}

bool ceSerial::GetRI(bool& success) {
    success=true;
    long status;
    if(ioctl(fd_, TIOCMGET, &status)== -1) success=false;
    return ((status & TIOCM_RI) != 0);
}

bool ceSerial::GetCD(bool& success) {
    success=true;
    long status;
    if(ioctl(fd_, TIOCMGET, &status)== -1) success=false;
    return ((status & TIOCM_CD) != 0);
}

} // namespace ce

