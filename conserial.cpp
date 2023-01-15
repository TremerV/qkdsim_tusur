/// @file
/// @brief Файл реализации класса, общающегося с микроконтроллером.
///
/// @copyright Copyright 2022 InfoTeCS.

#include <conserial.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include <cstddef>
#include <bitset>
using namespace std;
namespace hwe
{

Conserial::Conserial()
{
    // Заменяем инициализацию объекта вида "com_("/dev/ttyUSB0", 115200, 8, 'N', 1)",
    // то есть вызов конструктора с параметрами, на задание параметров через методы.
    // Конструктор с параметрами работает так же, но здесь хотя бы примерно понятно, что это за магические числа.
    com_.SetPort(std::string("/dev/ttyUSB0"));
    com_.SetBaudRate(115200);
    com_.SetDataSize(8);
    com_.SetParity('N');
    com_.SetStopBits(1);
}

Conserial::~Conserial()
{ }
api:: InitResponse Conserial:: Init()
{
    api::InitResponse response; // Структура для формирования ответа
    std::fstream ini_("./Angles.ini");
    if (!ini_.is_open()) { response = InitByPD();  }
    else {
        int n =6;
        char temp_ [n];

        ini_.getline(temp_, n);//считываем первую строку где лежит угл для 1 пластины. Вид : 003.4
        temp_[3]='.';//Меняем разделитель на точку (На случай если запятая)
        string angle1_ = temp_;
        ini_.getline(temp_, n);
        temp_[3]='.';
        string angle2_ = temp_;
        ini_.getline(temp_, n);
        temp_[3]='.';
        string angle3_ = temp_;
        ini_.getline(temp_, n);
        temp_[3]='.';
        string angle4_ = temp_;

        if (angle1_.length() > 4 && angle2_.length() > 4 && angle3_.length() > 4 && angle4_.length() > 4){ //Если в файл записаны углы
            WAngles<angle_t> anglesIni_;
            anglesIni_.aHalf_= stof (angle1_) ;
            anglesIni_.aQuart_= stof (angle2_);
            anglesIni_.bHalf_= stof (angle3_);
            anglesIni_.bQuart_= stof (angle4_);
            response =  InitByButtons(anglesIni_);} //Инициализация по концевикам
        else {response = InitByPD();}//Инициализация по фотодиодам
    }
    return response;
}

api::InitResponse Conserial::InitByPD()
{
    api::InitResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // После установки соединения...
    SendUart(dict_.find("Init")->second); // Посылаем запрос МК
    // Читаем ответ
    ce::Package pack;
    ReadUart(&pack);

    // Заполняем поля структуры
    response.startPlatesAngles_.aHalf_  = ((float) pack.param1) * rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.startPlatesAngles_.aQuart_ = ((float) pack.param2) * rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.startPlatesAngles_.bHalf_  = ((float) pack.param3) * rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.startPlatesAngles_.bQuart_ = ((float) pack.param4) * rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.startLightNoises_.h_ = pack.param5; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.startLightNoises_.v_ = pack.param6; //<- начальная засветка детектора, принимающего вертикальную поляризацию

    response.maxSignalLevels_.h_ = pack.param7; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.maxSignalLevels_.v_ = pack.param8; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.maxLaserPower_ = pack.param9;

    response.errorCode_ = 0; // Команда отработала корректно

    curAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    com_.Close(); // Закрываем соединение
    return response; // Возвращаем сформированный ответ
}

api::InitResponse Conserial::InitByButtons(WAngles<angle_t> angles)
{
    api::InitResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }
    int steps1_ = round(fmod(angles.aHalf_,360)/rotateStep_);
    int steps2_ = round(fmod(angles.aQuart_,360)/rotateStep_);
    int steps3_ = round(fmod(angles.bHalf_,360)/rotateStep_);
    int steps4_ = round(fmod(angles.bQuart_,360)/rotateStep_);

    // После установки соединения...
    SendUart(dict_.find("InitByButtons")->second, steps1_, steps2_, steps3_, steps4_); // Посылаем запрос МК

    // Читаем ответ
    ce::Package pack;
    ReadUart(&pack);

    // Заполняем поля структуры
    response.startPlatesAngles_.aHalf_  = ((float) pack.param1) * rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.startPlatesAngles_.aQuart_ = ((float) pack.param2) * rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.startPlatesAngles_.bHalf_  = ((float) pack.param3) * rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.startPlatesAngles_.bQuart_ = ((float) pack.param4) * rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.startLightNoises_.h_ = pack.param5; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.startLightNoises_.v_ = pack.param6; //<- начальная засветка детектора, принимающего вертикальную поляризацию

    response.maxSignalLevels_.h_ = pack.param7; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.maxSignalLevels_.v_ = pack.param8; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.maxLaserPower_ = pack.param9;
    maxLaserPower_ = response.maxLaserPower_;
    response.errorCode_ = 0; // Команда отработала корректно

    curAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    com_.Close(); // Закрываем соединение
    return response; // Возвращаем сформированный ответ
}

api::AdcResponse Conserial::RunTest(adc_t testId)
{
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;

    }

    // После установки соединения...
    SendUart(dict_.find("RunSelfTest")->second, testId); // Запрос МК

    // Чтение ответа
    ce::Package pack;
    ReadUart(&pack);

    response.adcResponse_ = pack.param1; // Возвращаем целое число
    response.errorCode_ = 0; // Команда отработала корректно

    com_.Close(); // Закрытие соединения
    return response;
}

api::SendMessageResponse Conserial::Sendmessage(WAngles<angle_t> angles, adc_t power)
{

    // Запомнили состояние лазера
    // Повернули пластинки
    // Вычислили шумы
    // Включили и установили лазер на нужный уровень мощности
    // Получили уровни сигналов
    // Выключаем лазер, если нужно

    // ЛИБО
    // Просим всё это сделать МК


    api::SendMessageResponse response; // Структура для формирования ответа


    adc_t steps1 = CalcSteps(angles.aHalf_,rotateStep_);
    adc_t steps2 = CalcSteps(angles.aQuart_,rotateStep_);
    adc_t steps3 = CalcSteps(angles.bHalf_,rotateStep_);
    adc_t steps4 = CalcSteps(angles.bQuart_,rotateStep_);

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // После установки соединения...

    SendUart(dict_.find("SendMessage")->second,  steps1, steps2, steps3, steps4, power);


    // Принимаем ответ
    ce::Package pack;
    ReadUart(&pack);


    // Заполняем поля
    response.newPlatesAngles_.aHalf_  = ((float)pack.param1) * rotateStep_; // <- полуволновая пластина "Алисы"     (1я пластинка)
    response.newPlatesAngles_.aQuart_ = ((float)pack.param2) * rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.newPlatesAngles_.bHalf_  = ((float)pack.param3) * rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.newPlatesAngles_.bQuart_ = ((float)pack.param4) * rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.currentLightNoises_.h_ = pack.param5; // <- засветка детектора, принимающего горизонтальную поляризацию
    response.currentLightNoises_.v_ = pack.param6; // <- засветка детектора, принимающего вертикальную поляризацию

    response.currentSignalLevels_.h_ = pack.param7; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.currentSignalLevels_.v_ = pack.param8; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.errorCode_ = 0; // Команда отработала корректно

    curAngles_ = response.newPlatesAngles_; // Запомнили текущие значения углов

    com_.Close(); // Закрытие соединения
    return response;
}

api::AdcResponse Conserial::SetTimeout(adc_t timeout)
{
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды

    if(com_.Open() != 0)
    {
        // Не удалось установить соединение
        response.errorCode_ = 1;
        return response;
    }

    // После установки соединения
    // Устанавливаем таймаут
    SendUart(dict_.find("SetTimeout")->second, timeout);

    ce::Package pack;
    ReadUart(&pack);

    response.adcResponse_ = pack.param1;
    response.errorCode_ = 0; // Команда отработала корректно
    timeoutTime_ = response.adcResponse_ ;
    com_.Close(); // Закрываем соединение
    return response;
}

api::AdcResponse Conserial::SetLaserState(adc_t on)
{
    api::AdcResponse response; // Структура для формирования ответа

    if(on != 1 && on != 0)
    {
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return response;
    }

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // После установки соединения...

    SendUart(dict_.find("SetLaserState")->second, on); // Запрос МК

    // Чтение ответа
    ce::Package pack;
    ReadUart(&pack);

    response.adcResponse_ = pack.param1;
    response.errorCode_ = 0; // Команда отработала корректно

    com_.Close(); // Закрытие соединения
    return response; // Возвращаем значение, соответствующее установленному состоянию
}

api::AdcResponse Conserial::SetLaserPower(adc_t power)
{
    api::AdcResponse response; // Структура для формирования ответа

    if (power > maxLaserPower_)
    {
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return response;
    }

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // После установки соединения...

    SendUart(dict_.find("SetLaserPower")->second, power); // Запрос МК

    // Чтение ответа
    ce::Package pack;
    ReadUart(&pack);

    response.adcResponse_ = pack.param1;
    response.errorCode_ = 0; // Команда отработала корректно

    com_.Close(); // Закрытие соединения
    return response; // Возвращаем значение, соответствующее установленному уровню
}

api::AngleResponse Conserial::SetPlateAngle(adc_t plateNumber, angle_t angle)
{
    api::AngleResponse response; // Структура для формирования ответа

    if(plateNumber < 1 || plateNumber > 4)
    {
        response.errorCode_ = 2; // // Принят некорректный входной параметр
        return response;
    }


    // Рассчитываем шаги...

    adc_t Steps;
    Steps = CalcSteps(angle,rotateStep_); //Подсчёт и округление шагов

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // Запросы к МК
    SendUart(dict_.find("SetPlateAngle")->second, Steps, plateNumber);


    // Чтение ответа
    ce::Package pack;
    ReadUart(&pack);


    // Заполняем поля
    response.angle_ = ((float)pack.param1) * rotateStep_;
    response.errorCode_ = 0; // Команда отработала корректно

    // Запоминаем новый угол на будущее
    switch (plateNumber)
    {
    case 1: curAngles_.aHalf_ = response.angle_; break;
    case 2: curAngles_.aQuart_= response.angle_; break;
    case 3: curAngles_.bHalf_ = response.angle_; break;
    case 4: curAngles_.bQuart_= response.angle_; break;
    }
    com_.Close(); // Закрытие соединения
    return response; // Возвращаем, чего там получилось установить
}

api::AdcResponse Conserial::GetLaserState()
{
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // После установки соединения...

    SendUart(dict_.find("GetLaserState")->second); // Запрос МК

    // Чтение ответа
    ce::Package pack;
    ReadUart(&pack);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.param1;
    response.errorCode_ = 0; // Команда отработала корректно

    com_.Close(); // Закрытие соединения
    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserPower()
{
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // После установки соединения...

    SendUart(dict_.find("GetLaserPower")->second); // Запрос МК

    // Чтение ответа
    ce::Package pack;
    ReadUart(&pack);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.param1;
    response.errorCode_ = 0;

    com_.Close(); // Закрытие соединения
    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetMaxLaserPower()
{
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // После установки соединения...
    SendUart(dict_.find("GetMaxLaserPower")->second);

    ce::Package pack;
    ReadUart(&pack);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.param1;
    response.errorCode_ = 0;

    com_.Close(); // Закрытие соединения
    return response; // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetStartPlatesAngles()
{
    api::WAnglesResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // Получаем начальные углы поворота волновых пластин от МК
    SendUart(dict_.find("GetStartPlatesAngles")->second);

    ce::Package pack;
    ReadUart(&pack);

    // Записываем полученное в структуру
    response.angles_.aHalf_  = ((float)pack.param1) * rotateStep_;//<- полуволновая пластина "Алисы"     (1я пластинка)
    response.angles_.aQuart_ = ((float)pack.param2) * rotateStep_; //<- четвертьволновая пластина "Алисы" (2я пластинка)
    response.angles_.bHalf_  = ((float)pack.param3) * rotateStep_; //<- полуволновая пластина "Боба"      (3я пластинка)
    response.angles_.bQuart_ = ((float)pack.param4) * rotateStep_; //<- четвертьволновая пластина "Боба"  (4я пластинка)

    response.errorCode_ = 0;

    com_.Close(); // Закрытие соединения
    // возвращаем структуру
    return response;
}

api::WAnglesResponse Conserial::GetPlatesAngles()
{
    api::WAnglesResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }


    // Получаем текущие углы поворота волновых пластин от МК
    SendUart(dict_.find("GetCurPlatesAngles")->second);

    ce::Package pack;
    ReadUart(&pack);

    // Записываем полученное в структуру
    response.angles_.aHalf_  = ((float)pack.param1) * rotateStep_;//<- полуволновая пластина "Алисы"     (1я пластинка)
    response.angles_.aQuart_ = ((float)pack.param2) * rotateStep_; //<- четвертьволновая пластина "Алисы" (2я пластинка)
    response.angles_.bHalf_  = ((float)pack.param3) * rotateStep_; //<- полуволновая пластина "Боба"      (3я пластинка)
    response.angles_.bQuart_ = ((float)pack.param4) * rotateStep_; //<- четвертьволновая пластина "Боба"  (4я пластинка)

    response.errorCode_ = 0;

    com_.Close(); // Закрытие соединения
    return response;
}

api::SLevelsResponse Conserial::GetStartLightNoises()
{
    api::SLevelsResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    // получаем от МК начальные уровни засветки
    SendUart(dict_.find("GetStartLightNoises")->second);

    ce::Package pack;
    ReadUart(&pack);

    // Заполняем структуру
    response.signal_.h_ = pack.param1; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.signal_.v_ = pack.param2; // <- начальная засветка детектора, принимающего вертикальную поляризацию

    response.errorCode_ = 0;

    com_.Close(); // Закрытие соединения
    return response;
}

api::SLevelsResponse Conserial::GetSignalLevels()
{
    api::SLevelsResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    SendUart(dict_.find("GetSignalLevel")->second);

    ce::Package pack;
    ReadUart(&pack);

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.param1; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.param2; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.errorCode_ = 0;

    com_.Close(); // Закрытие соединения
    return response;
}

api::AngleResponse Conserial::GetRotateStep()
{
    api::AngleResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    SendUart(dict_.find("GetRotateStep")->second);
    ce::Package pack;
    ReadUart(&pack);

    // Получаем от МК количество шагов для поворота на 360 градусов
    // Считаем сколько градусов в одном шаге
    float steps_ = pack.param1;
    if(steps_!=0){  rotateStep_ = 360.0 / steps_;}
    response.errorCode_ = 0;

    com_.Close(); // Закрытие соединения
    return response;
}

api::SLevelsResponse Conserial::GetLightNoises()
{
    api::SLevelsResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    SendUart(dict_.find("GetLightNoises")->second); // Запрос МУ

    // Чтение ответа
    ce::Package pack;
    ReadUart(&pack);

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.param1; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.param2; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере


    response.errorCode_ = 0;

    com_.Close(); // Закрываем соединение
    return response;
}

api::SLevelsResponse Conserial::GetMaxSignalLevels()
{
    api::SLevelsResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(com_.Open() != 0)
    {
        response.errorCode_ = 1; // Не удалось установить соединение
        return response;
    }

    SendUart(dict_.find("GetMaxSignalLevel")->second); // Запрос МК

    // Чтение ответа
    ce::Package pack;
    ReadUart(&pack);


    response.signal_.h_ = pack.param1; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.param2; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.errorCode_ = 0;

    com_.Close(); // Закрываем соединение
    return response;
}

api::AdcResponse Conserial::GetErrorCode()
{
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды
    if(com_.Open() != 0)
    {
        // Не удалось установить соединение
        response.errorCode_ = 1;
        return response;
    }

    // После установки соединения
    SendUart(dict_.find("GetErrorCode")->second);

    ce::Package pack;
    ReadUart(&pack);

    response.adcResponse_ = pack.param1;
    response.errorCode_ = 0; // Команда отработала корректно

    com_.Close(); // Закрываем соединение
    return response;
}

api::AdcResponse Conserial::GetTimeout()
{       api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды
        if(com_.Open() != 0)
        {
            // Не удалось установить соединение
            response.errorCode_ = 1;
            return response;
        }

        // После установки соединения
        SendUart(dict_.find("GetTimeout")->second);

        ce::Package pack;
        ReadUart(&pack);

        response.adcResponse_ = pack.param1;
        response.errorCode_ = 0; // Команда отработала корректно
        timeoutTime_  = response.adcResponse_;
        com_.Close(); // Закрываем соединение
        return response;
}
std::ostream& operator<< (std::ostream& os, std::byte b) {
    return os << std::bitset<8>(std::to_integer<int>(b));
}
//Функция передачи по uart
uint16_t Conserial:: SendUart (char commandName){
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t crc;

    unsigned int tempData_=0;
    tempData_ = (uint8_t) commandName;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));

    com_.Write(&start1);
    com_.Write(&start2);
    com_.Write(&status);
    com_.Write((uint8_t *)&commandName);
    com_.Write(&crc);
    com_.Write(&end);
    return 1;
}
uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1 ){
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;

    unsigned int tempData_=0;
    tempData_ = (uint8_t) commandName + Parameter1;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));

    com_.Write(&start1);
    com_.Write(&start2);
    com_.Write(&status);
    com_.Write((uint8_t *)&commandName);
    com_.Write(&crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter1);}
    com_.Write(&end);
    return 1;
}

uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1, uint16_t Parameter2){
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;

    unsigned int tempData_=0;
    tempData_ = (uint8_t) commandName + Parameter1 + Parameter2;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));

    com_.Write(&start1);
    com_.Write(&start2);
    com_.Write(&status);
    com_.Write((uint8_t *)&commandName);
    com_.Write(&crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter1);}

    if (Parameter2>=256){
        uint8_t a1 = Parameter2 / 256;
        uint8_t a2 = Parameter2 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter2);}

    com_.Write(&end);
    return 1;
}

uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1, uint16_t Parameter2, uint16_t Parameter3){
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;

    unsigned int tempData_=0;
    tempData_ = (uint8_t) commandName + Parameter1 + Parameter2 + Parameter3;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));
    com_.Write(&start1);
    com_.Write(&start2);
    com_.Write(&status);
    com_.Write((uint8_t *)&commandName);
    com_.Write(&crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter1);}

    if (Parameter2>=256){
        uint8_t a1 = Parameter2 / 256;
        uint8_t a2 = Parameter2 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter2);}

    if (Parameter3>=256){
        uint8_t a1 = Parameter3 / 256;
        uint8_t a2 = Parameter3 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter3);}

    com_.Write(&end);
    return 1;
}

uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1, uint16_t Parameter2, uint16_t Parameter3, uint16_t Parameter4){
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;

    unsigned int tempData_=0;
    tempData_ = (uint8_t) commandName + Parameter1 + Parameter2 + Parameter3 + Parameter4;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));
    com_.Write(&start1);
    com_.Write(&start2);
    com_.Write(&status);
    com_.Write((uint8_t *)&commandName);
    com_.Write(&crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter1);}

    if (Parameter2>=256){
        uint8_t a1 = Parameter2 / 256;
        uint8_t a2 = Parameter2 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter2);}

    if (Parameter3>=256){
        uint8_t a1 = Parameter3 / 256;
        uint8_t a2 = Parameter3 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter3);}

    if (Parameter4>=256){
        uint8_t a1 = Parameter4 / 256;
        uint8_t a2 = Parameter4 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter4);}

    com_.Write(&end);
    return 1;
}

uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1, uint16_t Parameter2, uint16_t Parameter3, uint16_t Parameter4, uint16_t Parameter5){
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;

    unsigned int tempData_=0;

    tempData_ = (uint8_t) commandName + Parameter1 + Parameter2 + Parameter3 + Parameter4 + Parameter5;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));
    com_.Write(&start1);
    com_.Write(&start2);
    com_.Write(&status);
    com_.Write((uint8_t *)&commandName);
    com_.Write(&crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter1);}

    if (Parameter2>=256){
        uint8_t a1 = Parameter2 / 256;
        uint8_t a2 = Parameter2 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter2);}

    if (Parameter3>=256){
        uint8_t a1 = Parameter3 / 256;
        uint8_t a2 = Parameter3 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter3);}

    if (Parameter4>=256){
        uint8_t a1 = Parameter4 / 256;
        uint8_t a2 = Parameter4 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter4);}

    if (Parameter5>=256){
        uint8_t a1 = Parameter5 / 256;
        uint8_t a2 = Parameter5 % 256;
        com_.Write(&a1);
        com_.Write(&a2);
    }else{
        com_.Write(&null);
        com_.Write((uint8_t *) &Parameter5);}

    com_.Write(&end);
    return 1;
}

//Функция чтения по Uart
void Conserial::ReadUart(ce::Package * packege_)
{   ce::Package pack;

    pack = com_.Read_com(timeoutTime_);
    int temp = pack.nameCommand_ + pack.param1 + pack.param2 + pack.param3 + pack.param4 + pack.param5 + pack.param6 + pack.param7 + pack.param8+ pack.param9+ pack.param10;
    uint8_t crc = Crc8((uint8_t *)&temp, sizeof(temp));
//    cout<< std::bitset<8>(crc)<<endl;
//    if (crc == pack.crc_){ * packege_ = pack;}
//    else{   cout<< "WrongCheckSum"<<endl; }
    *packege_ = pack;
 //   cout<<  packege_->nameCommand_<<endl;
//    cout<< std::bitset<8>(packege_->crc_)<<endl;

    cout<< "Имя: " << pack.nameCommand_<<endl;
    cout<< "параметр 1: " <<(uint16_t)pack.param1<<endl;
    cout<< "параметр 2: " <<(uint16_t)pack.param2<<endl;
    cout<< "параметр 3: " <<(uint16_t)pack.param3<<endl;
    cout<< "параметр 4: " <<(uint16_t)pack.param4<<endl;
    cout<< "параметр 5: " <<(uint16_t)pack.param5<<endl;
    cout<< "параметр 6: " <<(uint16_t)pack.param6<<endl;
    cout<< "параметр 7: " <<(uint16_t)pack.param7<<endl;
    cout<< "параметр 8: " <<(uint16_t)pack.param8<<endl;
    cout<< "параметр 9: " << (uint16_t)pack.param9<<endl;
    cout<< "параметр 10: " <<(uint16_t)pack.param10<<endl;

}


// Функция подсчёта контрольной суммы
uint8_t Conserial::Crc8(uint8_t *buffer, uint8_t size) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < size ; i++) {
        uint8_t data = buffer[i];
        for (int j = 8; j > 0; j--) {
            crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
            data >>= 1;
        }
    }

    return crc;
}

uint16_t Conserial::CalcSteps(angle_t angle, angle_t rotateStep){

    angle = fmod(angle , 360.0); // Подсчет кратчайшего угла поворота

    int Steps = round (angle / rotateStep); //Подсчёт и округление шагов
    return Steps;
}

}//namespace

