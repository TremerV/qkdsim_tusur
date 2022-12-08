/// @file
/// @brief Заголовочный файл класса, общающегося с микроконтроллером.
///
/// @copyright Copyright 2022 InfoTeCS.
#ifndef HARDWARE_H
#define HARDWARE_H

#include <abstracthardwareapi.h>
#include <ceSerial.h>

namespace hwe
{

/// @brief Реализация интерфейса api в виртуальной версии стенда.
class conserial : public AbstractHardwareApi
{
     

public:

     conserial();
     virtual ~conserial();
    
     
     // Эти команды вот в таком виде нужно реализовать в файле .cpp   
     virtual api::InitResponse Init();
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

     WAngles<angle_t>curAngles_;
     adc_t timeoutTime_ = 2000;
     angle_t rotateStep_ = 0.3;

     uint16_t SendUart(char commandName, uint16_t parameter = 0);
     void ReadUart(std::string * readBuffer);
     uint16_t ParseData(std::string * readBuffer);
     uint8_t Crc8(uint8_t *buffer, uint8_t size = 2);
     uint8_t CheckOpen();
     uint16_t CalcSteps(angle_t angle, angle_t curAngle, angle_t rotateStep, int * dir);
};

} //namespace
#endif // HARDWARE_H
