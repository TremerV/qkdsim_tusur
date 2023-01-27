/// @file
/// @brief Заголовочный файл класса AbstractHardwareApi.
///
/// @copyright Copyright 2022 InfoTeCS.
#ifndef ABSTRACTHARDWAREAPI_H
#define ABSTRACTHARDWAREAPI_H

#include <apitypes.h>
#include <hwetypes.h>
#include <logictypes.h>

namespace hwe
{

/// @brief Чистый абстрактрый класс, представляющий интерфейс для использования API стенда.
class AbstractHardwareApi
{
public:
     virtual ~AbstractHardwareApi() {};

     /// @brief Выполняет инициализацию оборудования стенда.
     /// @return Структура, содержащая информацию о начальных уровнях шумов, начальных положениях волновых пластин,максимальном уровне накачки лазера и уровне сигнала при максимальном уровне накачке детекторов.
     virtual api::InitResponse Init() = 0;
     virtual api::InitResponse InitByPD() = 0;
     virtual api::InitResponse InitByButtons(WAngles<hwe::angle_t> angles) = 0;

     virtual api::AdcResponse RunTest(adc_t testId = 0) = 0;

     /// @param [in] angles Углы поворотов волновых пластин для передачи сообщения.
     /// @param [in] power Мощность лазера для передачи сообщения.
     virtual api::SendMessageResponse Sendmessage(WAngles<hwe::angle_t> angles, hwe::adc_t power) = 0;

     /// @param [in] timeout Устанавливаемый таймаут ответа.
     /// @return Установленный таймаут ответа.
     virtual api::AdcResponse SetTimeout(hwe::adc_t timeout) = 0;

     /// @param [in] on Устанавливаемое состояние лазера.
     /// @return Установленное состояние лазера.
     virtual api::AdcResponse SetLaserState(hwe::adc_t on) = 0;

     /// @param [in] power Устанавливаемый уровень накачки лазера
     /// @return Установленный уровень накачки лазера.
     virtual api::AdcResponse SetLaserPower(hwe::adc_t power) = 0;

     /// @param [in] plateNumber Номер волновой пластины.
     /// @param [in] angle Угол поворота волновой пластины в градусах.
     /// @return Установленный угол поворота пластины
     virtual api::AngleResponse SetPlateAngle(hwe::adc_t plateNumber, hwe::angle_t angle) = 0;

     /// @return Текущее состояние лазера.
     virtual api::AdcResponse GetLaserState() = 0;

     /// @return Текущий уровень накачки лазера.
     virtual api::AdcResponse GetLaserPower() = 0;

     /// @return Максимальный уровень накачки лазера, соответствующий линейным характеристикам детекторов.
     virtual api::AdcResponse GetMaxLaserPower() = 0;

     /// @return Углы поворотов волновых пласнин, не преобразующих поляризацию сигнала.
     virtual api::WAnglesResponse GetStartPlatesAngles() = 0;

     /// @return Текущие углы поворотов волновых пластин.
     virtual api::WAnglesResponse GetPlatesAngles() = 0;

     /// @return Начальный уровень засветки детекторов, определенный при инициализации
     virtual api::SLevelsResponse GetStartLightNoises() = 0;

     /// @return Уровень сигнала на детекторах.
     virtual api::SLevelsResponse GetSignalLevels() = 0;

     /// @return Шаг поворота волновой пластинки.
     virtual api::AngleResponse GetRotateStep() = 0;

     /// @return Текущий уровень засветки детекторов.
     virtual api::SLevelsResponse GetLightNoises() = 0;

     /// @return Уровени сигнала на детекторах при накачке лазера, соответствующей максимуму, вычисленному на этапе инициализации.
     virtual api::SLevelsResponse GetMaxSignalLevels() = 0;

     virtual api::AdcResponse GetErrorCode() = 0;

     /// @return Таймаут ответа в секундах.
     virtual api::AdcResponse GetTimeout() = 0;
};

}
#endif // ABSTRACTHARDWAREAPI_H

