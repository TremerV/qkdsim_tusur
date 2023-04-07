#include <debuglogger.h>
#include <ctime>
#include <cstdarg>
#include <iomanip>

std::ofstream* DebugLogger::logfile_;
std::string DebugLogger::indent_;

void DebugLogger::StartLogging(const char *filename)
{
     logfile_ = new std::ofstream(filename, std::ios::app);
}

void DebugLogger::EndLogging()
{
     logfile_->close();
     delete logfile_;
}

DebugLogger::DebugLogger(const char *funcName, std::size_t count, ...)
{
     indent_.append("--");
     funcName_ = funcName;

     std::time_t timestamp = std::time(nullptr);
     std::strftime(timeString_, 64, "%a %H:%M:%S", std::localtime(&timestamp));

     *logfile_ << timeString_ << "   " << indent_ << "> " << funcName_ << "(";

     std::va_list args;
     va_start(args, count);
     for(std::size_t i = 0; i < count; ++i)
     {
          *logfile_ << std::defaultfloat << va_arg(args, double);
          if(i != (count-1))
          {
               *logfile_  << ",";
          }
     }
     *logfile_ << ")" << std::endl;

     va_end(args);
}

DebugLogger::~DebugLogger()
{
     indent_.resize(indent_.length() - 2);
}

void DebugLogger::Return()
{
     std::time_t timestamp = std::time(nullptr);
     std::strftime(timeString_, 64, "%a %H:%M:%S", std::localtime(&timestamp));

     *logfile_ << timeString_ << "   " <<  "<" << indent_ << " " << funcName_ << std::endl;
}

int &DebugLogger::Return(int &response)
{
     std::time_t timestamp = std::time(nullptr);
     std::strftime(timeString_, 64, "%a %H:%M:%S", std::localtime(&timestamp));

     *logfile_ << timeString_ << "   " <<  "<" << indent_ << " " << funcName_;
     *logfile_ << ": [int: " << response << "]" << std::endl;

     return response;
}

api::AdcResponse &DebugLogger::Return(api::AdcResponse &response)
{
     std::time_t timestamp = std::time(nullptr);
     std::strftime(timeString_, 64, "%a %H:%M:%S", std::localtime(&timestamp));

     *logfile_ << timeString_ << "   " <<  "<" << indent_ << " " << funcName_;
     *logfile_ << ": [uint: " << response.adcResponse_ << "; ecode: " << response.errorCode_ << "]" << std::endl;

     return response;
}

api::AngleResponse &DebugLogger::Return(api::AngleResponse &response)
{
     std::time_t timestamp = std::time(nullptr);
     std::strftime(timeString_, 64, "%a %H:%M:%S", std::localtime(&timestamp));

     *logfile_ << timeString_ << "   " <<  "<" << indent_ << " " << funcName_;
     *logfile_ << ": [angle: " << response.angle_ << "; ecode: " << response.errorCode_ << "]" << std::endl;

     return response;
}

api::WAnglesResponse &DebugLogger::Return(api::WAnglesResponse &response)
{
     std::time_t timestamp = std::time(nullptr);
     std::strftime(timeString_, 64, "%a %H:%M:%S", std::localtime(&timestamp));

     *logfile_ << timeString_ << "   " <<  "<" << indent_ << " " << funcName_;
     *logfile_ << ": [angles: "
               << response.angles_.aHalf_  << ","
               << response.angles_.aQuart_ << ","
               << response.angles_.bHalf_  << ","
               << response.angles_.bQuart_ << "; "
               << "ecode: " << response.errorCode_ << "]" << std::endl;

     return response;
}

api::SLevelsResponse &DebugLogger::Return(api::SLevelsResponse &response)
{
     std::time_t timestamp = std::time(nullptr);
     std::strftime(timeString_, 64, "%a %H:%M:%S", std::localtime(&timestamp));

     *logfile_ << timeString_ << "   " <<  "<" << indent_ << " " << funcName_;
     *logfile_ << ": [detectors: "
               << response.signal_.h_  << ","
               << response.signal_.v_ <<  "; "
               << "ecode: " << response.errorCode_ << "]" << std::endl;

     return response;
}

api::InitResponse &DebugLogger::Return(api::InitResponse &response)
{
     std::time_t timestamp = std::time(nullptr);
     std::strftime(timeString_, 64, "%a %H:%M:%S", std::localtime(&timestamp));

     *logfile_ << timeString_ << "   " <<  "<" << indent_ << " " << funcName_;
     *logfile_ << ": [s_angles: "
               << response.startPlatesAngles_.aHalf_  << ","
               << response.startPlatesAngles_.aQuart_ << ","
               << response.startPlatesAngles_.bHalf_  << ","
               << response.startPlatesAngles_.bQuart_ << "; "
               << "m_signals: "
               << response.maxSignalLevels_.h_  << ","
               << response.maxSignalLevels_.v_ <<  "; "
               << "s_noises: "
               << response.startLightNoises_.h_  << ","
               << response.startLightNoises_.v_ <<  "; "
               << "m_laser: " << response.maxLaserPower_ << "; "
               << "ecode: " << response.errorCode_ << "]" << std::endl;

     return response;
}

api::SendMessageResponse &DebugLogger::Return(api::SendMessageResponse &response)
{
     std::time_t timestamp = std::time(nullptr);
     std::strftime(timeString_, 64, "%a %H:%M:%S", std::localtime(&timestamp));

     *logfile_ << timeString_ << "   " <<  "<" << indent_ << " " << funcName_;
     *logfile_ << ": [angles: "
               << response.newPlatesAngles_.aHalf_  << ","
               << response.newPlatesAngles_.aQuart_ << ","
               << response.newPlatesAngles_.bHalf_  << ","
               << response.newPlatesAngles_.bQuart_ << "; "
               << "signals: "
               << response.currentSignalLevels_.h_  << ","
               << response.currentSignalLevels_.v_ <<  "; "
               << "noises: "
               << response.currentLightNoises_.h_  << ","
               << response.currentLightNoises_.v_ <<  "; "
               << "ecode: " << response.errorCode_ << "]" << std::endl;

     return response;
}

