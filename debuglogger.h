#ifndef DEBUGLOGGER_H
#define DEBUGLOGGER_H

#include <fstream>
#include <apitypes.h>

class DebugLogger
{
public:
     static void StartLogging(const char *filename);
     static void EndLogging();

     DebugLogger(const char* funcName, std::size_t count = 0, ...);
     ~DebugLogger();

     void Return();
     int &Return(int &response);
     api::AdcResponse &Return(api::AdcResponse& response);
     api::AngleResponse &Return(api::AngleResponse& response);
     api::WAnglesResponse &Return(api::WAnglesResponse& response);
     api::SLevelsResponse  &Return(api::SLevelsResponse&  response);
     api::InitResponse      &Return(api::InitResponse& response);
     api::SendMessageResponse &Return(api::SendMessageResponse& response);

private:
     const char* funcName_;
     char timeString_[64];

     static std::ofstream* logfile_;
     static std::string indent_;

};

#endif // DEBUGLOGGER_H

