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

#include <string>
#include <array>

namespace ce {

struct UartResponse{
        uint8_t status_= 0;
        uint8_t nameCommand_ = 0;
        uint8_t crc_= 0;
        uint16_t parameters_ [10] = {0,0,0,0,0,0,0,0,0,0};

    };

class ceSerial {
    uint16_t rxchar_;
    std::string port_;
    long baud_;
    long dsize_;
    char parity_;
    float stopbits_;
    long fd_; //serial_fd


public:
    static void Delay(unsigned long ms);
    ceSerial();
    ceSerial(std::string Device, long BaudRate, long DataSize, char ParityType, float NStopBits);
    ~ceSerial();
    long Open(void);//return 0 if success
    void Close();
    char ReadChar();
    char ReadChar(bool& success);//return read char if success
    ce::UartResponse Read_com(unsigned int timeout);
    bool WriteChar(char ch);    //return success flag
    bool Write(uint16_t data);
    bool Write(uint8_t data);
    bool Write(char * data);//write null terminated string and return success flag
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

