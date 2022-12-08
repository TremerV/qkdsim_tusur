/// @file
/// @brief Файл со структурами возвращаемых API сообщений.
///
/// @copyright Copyright 2022 InfoTeCS.
#ifndef APITYPES_H
#define APITYPES_H

#include <hwetypes.h>
#include <logictypes.h>

/// @brief Пространство имён для структур-сообщений API
namespace api
{

/// @brief Структура сообщения, формируемого в процессе инициализации стенда.
struct InitResponse
{
     SLevels<hwe::adc_t> startLightNoises_;
     WAngles<hwe::angle_t> startPlatesAngles_;
     hwe::adc_t maxLaserPower_;
     SLevels<hwe::adc_t> maxSignalLevels_;

     hwe::adc_t errorCode_ = 0;
};

/// @brief Структура пакета, возвращаемого при передаче сообщения.
struct SendMessageResponse
{
     WAngles<hwe::angle_t> newPlatesAngles_;
     SLevels<hwe::adc_t> currentLightNoises_;
     SLevels<hwe::adc_t> currentSignalLevels_;

     hwe::adc_t errorCode_ = 0;
};


struct AdcResponse
{
     hwe::adc_t adcResponse_;
     hwe::adc_t errorCode_ = 0;
};

struct AngleResponse
{
     hwe::angle_t angle_;
     hwe::adc_t errorCode_ = 0;
};

struct WAnglesResponse
{
     WAngles<hwe::angle_t> angles_;
     hwe::adc_t errorCode_ = 0;
};

struct SLevelsResponse
{
     SLevels<hwe::adc_t> signal_;
     hwe::adc_t errorCode_ = 0;
};

} //namespace
#endif // APITYPES_H
