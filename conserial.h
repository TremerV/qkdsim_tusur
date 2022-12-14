/// @file
/// @brief Заголовочный файл класса, общающегося с микроконтроллером.
///
/// @copyright Copyright 2022 InfoTeCS.
#ifndef CONSERIAL_H
#define CONSERIAL_H

#include <ceSerial.h>
#include <abstracthardwareapi.h>
#include <map>

namespace hwe
{

class Conserial : public AbstractHardwareApi
{
public:

     Conserial();
     virtual ~Conserial();

     // Эти команды вот в таком виде нужно реализовать в файле .cpp
     virtual api::InitResponse Init();
     virtual api::InitResponse InitByButtons();
     virtual api::AdcResponse RunTest(adc_t testId = 0);
     virtual api::SendMessageResponse Sendmessage(WAngles<angle_t> angles, adc_t power);
     virtual api::AdcResponse SetTimeout(adc_t timeout);
     virtual api::AdcResponse SetLaserState(adc_t on);
     virtual api::AdcResponse SetLaserPower(adc_t power);
     virtual api::AngleResponse SetPlateAngle(adc_t plateNumber, angle_t angle);
     virtual api::AdcResponse GetLaserState();
     virtual api::AdcResponse GetLaserPower();
     virtual api::AdcResponse GetMaxLaserPower();
     virtual api::WAnglesResponse GetStartPlatesAngles();
     virtual api::WAnglesResponse GetPlatesAngles();
     virtual api::SLevelsResponse GetStartLightNoises();
     virtual api::SLevelsResponse GetSignalLevels();
     virtual api::AngleResponse GetRotateStep();
     virtual api::SLevelsResponse GetLightNoises();
     virtual api::SLevelsResponse GetMaxSignalLevels();
     virtual api::AdcResponse GetErrorCode();
     virtual api::AdcResponse GetTimeout();

private:
     ce::ceSerial com_; // Обект класса для соединения с МК

     /// @todo Не нужно ли сделать поля для всех значений, определяемых при инициализации?
     WAngles<angle_t>curAngles_;
     adc_t timeoutTime_ = 2000;
     angle_t rotateStep_ = 0.3;
     adc_t maxLaserPower_ = 100;

     uint16_t SendUart(char commandName, uint16_t parameter = 0);
     void ReadUart(std::string * readBuffer);
     uint16_t ParseData(std::string * readBuffer);
     uint8_t Crc8(uint8_t *buffer, uint8_t size = 2);
     uint16_t CalcSteps(angle_t angle, angle_t rotateStep, int * dir);

     const std::map <std::string, char> dict_ = {
          {"Init", 'A'},
          {"SendMessage", 'B'},
          {"SetLaserState", 'C'},
          {"SetLaserPower", 'D'},
          {"SetPlateAngle", 'E'},
          {"SetTimeout", 'F'},
          {"GetErrorCode", 'G'},
          {"GetLaserState", 'H'},
          {"GetLaserPower", 'I'},
          {"GetTimeout", 'J'},
          {"GetStartPlatesAngles", 'K'},
          {"GetCurPlatesAngles", 'L'},
          {"GetSignalLevel", 'M'},
          {"GetRotateStep", 'N'},
          {"GetMaxLaserPower", 'O'},
          {"GetLightNoises", 'P'},
          {"GetStartLightNoises", 'Q'},
          {"GetMaxSignalLevel", 'R'},
          {"RunSelfTest", 'S'},
          {"InitByButtons", 'T'}
     };
};

} //namespace
#endif // CONSERIAL_H
