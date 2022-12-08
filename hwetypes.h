/// @file
/// @brief Файл с константами, алиасами и структурами для аппаратной части виртуальной версии стенда SimQKD-VA.
///
/// @copyright Copyright 2022 InfoTeCS.
#ifndef HWETYPES_H
#define HWETYPES_H

#include <cstdint>
#include <utility>
#include <complex>

/// @brief Пространство имён для констант, контейнеров и классов элементов виртаульной версии стенда SimQKD-VA.
namespace hwe
{
/// @brief Контейнер для хранения поляризации имитируемого светового сигнала.
/// Представляет из себя нормированный вектор Джонса.
/// Содержит горизонтальную и вертикальную комплексные амплитуды.
using JonesVector = std::pair<std::complex<double>, std::complex<double>>;

/// @brief Фиксированный целочисленный беззнаковый тип.
/// Используется в качестве кодов АЦП детекторов, кодов ЦАП лазера и везде, где нет необходимости в вещественных числах.
using adc_t = uint16_t;

/// @brief Угол или шаг поворота пластинки в градусах.
using angle_t = float;

/// @brief Константа, хранящая максимально возможное значение типа данных adc_t.
const adc_t adc_max = UINT16_MAX;

const double pi = std::acos(-1); ///< Константа со значением pi

} //namespace

#endif // HWETYPES_H



