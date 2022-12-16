/// @file
/// @brief Файл реализации класса, общающегося с микроконтроллером.
///
/// @copyright Copyright 2022 InfoTeCS.

#include <conserial.h>
#include <string.h>
#include <iostream>
#include <fstream>
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
     timeoutTime_=56000;
     // Читаем ответ
     std::string BUFread;
     ReadUart(&BUFread);



     // Заполняем поля структуры
     response.startPlatesAngles_.aHalf_  = ParseData(&BUFread); //<- полуволновая пластина "Алисы"     (1я пластинка)
     response.startPlatesAngles_.aQuart_ = ParseData(&BUFread); // <- четвертьволновая пластина "Алисы" (2я пластинка)
     response.startPlatesAngles_.bHalf_  = ParseData(&BUFread); // <- полуволновая пластина "Боба"      (3я пластинка)
     response.startPlatesAngles_.bQuart_ = ParseData(&BUFread); // <- четвертьволновая пластина "Боба"  (4я пластинка)

     response.startLightNoises_.h_ = ParseData(&BUFread); // <- начальная засветка детектора, принимающего горизонтальную поляризацию
     response.startLightNoises_.v_ = ParseData(&BUFread); //<- начальная засветка детектора, принимающего вертикальную поляризацию

     response.maxSignalLevels_.h_ = ParseData(&BUFread); // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
     response.maxSignalLevels_.v_ = ParseData(&BUFread); // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

     response.maxLaserPower_ = ParseData(&BUFread);

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
     SendUart(dict_.find("InitByButtons")->second,steps1_); // Посылаем запрос МК
     SendUart(dict_.find("InitByButtons")->second,steps2_); // Посылаем запрос МК
     SendUart(dict_.find("InitByButtons")->second,steps3_); // Посылаем запрос МК
     SendUart(dict_.find("InitByButtons")->second,steps4_); // Посылаем запрос МК
     timeoutTime_=36000;
     // Читаем ответ
     std::string BUFread;
     ReadUart(&BUFread);



     // Заполняем поля структуры
     response.startPlatesAngles_.aHalf_  = ParseData(&BUFread); //<- полуволновая пластина "Алисы"     (1я пластинка)
     response.startPlatesAngles_.aQuart_ = ParseData(&BUFread); // <- четвертьволновая пластина "Алисы" (2я пластинка)
     response.startPlatesAngles_.bHalf_  = ParseData(&BUFread); // <- полуволновая пластина "Боба"      (3я пластинка)
     response.startPlatesAngles_.bQuart_ = ParseData(&BUFread); // <- четвертьволновая пластина "Боба"  (4я пластинка)

     response.startLightNoises_.h_ = ParseData(&BUFread); // <- начальная засветка детектора, принимающего горизонтальную поляризацию
     response.startLightNoises_.v_ = ParseData(&BUFread); //<- начальная засветка детектора, принимающего вертикальную поляризацию

     response.maxSignalLevels_.h_ = ParseData(&BUFread); // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
     response.maxSignalLevels_.v_ = ParseData(&BUFread); // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

     response.maxLaserPower_ = ParseData(&BUFread);

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
     timeoutTime_=10000;
     SendUart(dict_.find("RunSelfTest")->second, testId); // Запрос МК

     // Чтение ответа
     std::string BUFread;
     ReadUart(&BUFread);



     response.adcResponse_ = ParseData(&BUFread); // Возвращаем целое число
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

     int dir1, dir2, dir3, dir4;

     adc_t steps1 = CalcSteps(angles.aHalf_,rotateStep_, &dir1);
     adc_t steps2 = CalcSteps(angles.aQuart_,rotateStep_, &dir2);
     adc_t steps3 = CalcSteps(angles.bHalf_,rotateStep_, &dir3);
     adc_t steps4 = CalcSteps(angles.bQuart_,rotateStep_, &dir4);

     // Открываем соединение с МК
     if(com_.Open() != 0)
     {
          response.errorCode_ = 1; // Не удалось установить соединение
          return response;
     }

     // После установки соединения...
     timeoutTime_=15000;
     SendUart(dict_.find("SendMessage")->second,  steps1);
     SendUart(dict_.find("SendMessage")->second,  dir1);
     SendUart(dict_.find("SendMessage")->second, steps2);
     SendUart(dict_.find("SendMessage")->second,  dir2);
     SendUart(dict_.find("SendMessage")->second, steps3);
     SendUart(dict_.find("SendMessage")->second,  dir3);
     SendUart(dict_.find("SendMessage")->second, steps4);
     SendUart(dict_.find("SendMessage")->second,  dir4);
     SendUart(dict_.find("SendMessage")->second,  power);

     // Принимаем ответ
     std::string BUFread;
     ReadUart(&BUFread);


     // Заполняем поля
     response.newPlatesAngles_.aHalf_  = ParseData(&BUFread); // <- полуволновая пластина "Алисы"     (1я пластинка)
     response.newPlatesAngles_.aQuart_ = ParseData(&BUFread); // <- четвертьволновая пластина "Алисы" (2я пластинка)
     response.newPlatesAngles_.bHalf_  = ParseData(&BUFread); // <- полуволновая пластина "Боба"      (3я пластинка)
     response.newPlatesAngles_.bQuart_ = ParseData(&BUFread); // <- четвертьволновая пластина "Боба"  (4я пластинка)

     response.currentLightNoises_.h_ = ParseData(&BUFread); // <- засветка детектора, принимающего горизонтальную поляризацию
     response.currentLightNoises_.v_ = ParseData(&BUFread); // <- засветка детектора, принимающего вертикальную поляризацию

     response.currentSignalLevels_.h_ = ParseData(&BUFread); // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
     response.currentSignalLevels_.v_ = ParseData(&BUFread); // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

     response.errorCode_ = 0; // Команда отработала корректно

     curAngles_ = response.newPlatesAngles_; // Запомнили текущие значения углов

     com_.Close(); // Закрытие соединения
     return response;
}

api::AdcResponse Conserial::SetTimeout(adc_t timeout) //!!! Потом будет таймер
{

     // Устанавливаем таймаут
     timeoutTime_ = timeout;
     return {timeoutTime_, 0}; //Возвращаем, что получилось установить
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
     timeoutTime_=3000;
     SendUart(dict_.find("SetLaserState")->second, on); // Запрос МК

     // Чтение ответа
     std::string BUFread;
     ReadUart(&BUFread);

     response.adcResponse_ = ParseData(&BUFread);
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
      timeoutTime_=3000;
      SendUart(dict_.find("SetLaserPower")->second, power); // Запрос МК

      // Чтение ответа
      std::string BUFread;
      ReadUart(&BUFread);

      response.adcResponse_ = ParseData(&BUFread);
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
     int dir;

     adc_t Steps;
     Steps = CalcSteps(angle,rotateStep_, &dir); //Подсчёт и округление шагов

     // Открываем соединение с МК
     if(com_.Open() != 0)
     {
          response.errorCode_ = 1; // Не удалось установить соединение
          return response;
     }
     timeoutTime_=3000;
     // Запросы к МК
     SendUart(dict_.find("SetPlateAngle")->second, Steps);
     SendUart(dict_.find("SetPlateAngle")->second, plateNumber);
     SendUart(dict_.find("SetPlateAngle")->second, dir);

     // Чтение ответа
     std::string BUFread;
     ReadUart(&BUFread);



     // Заполняем поля
     response.angle_ = ParseData(&BUFread) * rotateStep_;
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
     timeoutTime_=2000;
     SendUart(dict_.find("GetLaserState")->second); // Запрос МК

     // Чтение ответа
     std::string BUFread;
     ReadUart(&BUFread);

     // Заполняем поля для ответа
     response.adcResponse_ = ParseData(&BUFread);
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
     timeoutTime_=2000;
     SendUart(dict_.find("GetLaserPower")->second); // Запрос МК

     // Чтение ответа
     std::string BUFread;
     ReadUart(&BUFread);

     // Заполняем поля для ответа
     response.adcResponse_ = ParseData(&BUFread);
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
     timeoutTime_=2000;
     // После установки соединения...
     SendUart(dict_.find("GetMaxLaserPower")->second);

     std::string BUFread;
     ReadUart(&BUFread);

     response.adcResponse_ = ParseData(&BUFread);
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
     timeoutTime_=2000;
     // Получаем начальные углы поворота волновых пластин от МК
     SendUart(dict_.find("GetStartPlatesAngles")->second);

     std::string  BUFread;
     ReadUart(&BUFread);

     // Записываем полученное в структуру
     response.angles_.aHalf_  = ParseData(&BUFread);//<- полуволновая пластина "Алисы"     (1я пластинка)
     response.angles_.aQuart_ = ParseData(&BUFread); //<- четвертьволновая пластина "Алисы" (2я пластинка)
     response.angles_.bHalf_  = ParseData(&BUFread); //<- полуволновая пластина "Боба"      (3я пластинка)
     response.angles_.bQuart_ = ParseData(&BUFread); //<- четвертьволновая пластина "Боба"  (4я пластинка)

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
     timeoutTime_=2000;
     // Получаем текущие углы поворота волновых пластин от МК
     SendUart(dict_.find("GetCurPlatesAngles")->second);

     std::string BUFread;
     ReadUart(&BUFread);

     com_.Close();

     // Записываем полученное в структуру
     response.angles_.aHalf_  = ParseData(&BUFread); // <- полуволновая пластина "Алисы"     (1я пластинка)
     response.angles_.aQuart_ = ParseData(&BUFread); // <- четвертьволновая пластина "Алисы" (2я пластинка)
     response.angles_.bHalf_  = ParseData(&BUFread); // <- полуволновая пластина "Боба"      (3я пластинка)
     response.angles_.bQuart_ = ParseData(&BUFread); // <- четвертьволновая пластина "Боба"  (4я пластинка)

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
     timeoutTime_=2000;
     // получаем от МК начальные уровни засветки
     SendUart(dict_.find("GetStartLightNoises")->second);

     std::string BUFread;
     ReadUart(&BUFread);

     // Заполняем структуру
     response.signal_.h_ = ParseData(&BUFread); // <- начальная засветка детектора, принимающего горизонтальную поляризацию
     response.signal_.v_ = ParseData(&BUFread); // <- начальная засветка детектора, принимающего вертикальную поляризацию

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
     timeoutTime_=2000;
     SendUart(dict_.find("GetSignalLevel")->second);

     std::string BUFread;
     ReadUart(&BUFread);

     // Заполняем структуру для ответа
     response.signal_.h_ = ParseData(&BUFread); // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
     response.signal_.v_ = ParseData(&BUFread); // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

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
     timeoutTime_=2000;
     SendUart(dict_.find("GetRotateStep")->second);
     std::string BUFread;
     ReadUart(&BUFread);

     // Получаем от МК количество шагов для поворота на 360 градусов
     // Считаем сколько градусов в одном шаге
     float steps_ = ParseData(&BUFread);
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

     timeoutTime_=2000;
     SendUart(dict_.find("GetLightNoises")->second); // Запрос МУ

     // Чтение ответа
     std::string BUFread;
     ReadUart(&BUFread);

     response.signal_.h_ = ParseData(&BUFread); // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию
     response.signal_.v_ = ParseData(&BUFread); // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию

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

     timeoutTime_=2000;
     SendUart(dict_.find("GetMaxSignalLevel")->second); // Запрос МК

     // Чтение ответа
     std::string BUFread;
     ReadUart(&BUFread);


     response.signal_.h_ = ParseData(&BUFread); // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
     response.signal_.v_ = ParseData(&BUFread); // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

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
     timeoutTime_=2000;
     // После установки соединения
     SendUart(dict_.find("GetErrorCode")->second);

     std::string BUFread;
     ReadUart(&BUFread);

     response.adcResponse_ = ParseData(&BUFread);
     response.errorCode_ = 0; // Команда отработала корректно

     com_.Close(); // Закрываем соединение
     return response;
}

api::AdcResponse Conserial::GetTimeout()
{
     api::AdcResponse response = {timeoutTime_, 0};
     return response;
}

//Функция передачи по uart
uint16_t Conserial::SendUart (char commandName, uint16_t Parameter)
{

     char start = '/';
     bool status = 0;
     char end = '#';
     std::string param = std::to_string (Parameter);
     std::string pack = "";
     uint16_t temp = Parameter + (int) commandName;

     // Вычисление контрольной суммы
     std::string crc = std::to_string(Crc8((uint8_t *)&temp));


     //Дополнение параметра незначащими нулями
     while ( param.length() < 4)
     {
          param = "0" + param;
     }
     //Дополнение crc незначащими нулями
     while ( crc.length() < 3)
     {
          crc = "0" + crc;
     }

     // Формирование пакета
     pack += start + std::to_string(status) + commandName + param + crc + end;

     char packChar[pack.length()+1];

     // std::cout << pack << std::endl;
     com_.Write(strcpy(packChar, pack.c_str()));
     return 1;
}

//Функция чтения по Uart
void Conserial::ReadUart(std::string * readBuffer)
{
     *readBuffer = "";
     std::string buffer;
     ce::ceSerial::Delay(timeoutTime_); //Задержка для выполнения команды (Ожидание ответа)
     bool successFlag = true;
     while(successFlag)
     {
          buffer += com_.ReadChar(successFlag);
          *readBuffer = buffer;
     }
     std::cout<<*readBuffer<<std::endl;
}


// Парсит данные приходящие ответом от МК
uint16_t Conserial::ParseData(std::string * readBuffer){
     bool condbyte = 0;
     uint16_t key = 0;
     uint16_t value = 0;
     uint16_t checksum = 0;
     std::string buffer = *readBuffer;
     std::string datapack = "";

     if (buffer.length() > 10) //Буфер заполнен
     {

          if(buffer[0] == '/' && buffer[10] == '#')//Полный пакет 11 символов
          {
               datapack = buffer.substr(0,11);
          }
          else
          {
               while(buffer[0] != '/' || buffer[10] != '#' )
               {
                    buffer=buffer.substr(1, buffer.length()); //Удаление лишних символов до начального символа '/'
                    *readBuffer = buffer;
               }
               datapack = buffer.substr(0,11); //Получение первого пакета из буфера
          }


          if(buffer.length() > 1) {
               buffer = buffer.substr(11, buffer.length()); // Обновление буфера
               *readBuffer = buffer;
          }
          else
          {
               buffer = "";
               *readBuffer = buffer;
          }


          if (datapack[0] == '/' && datapack[10] == '#') // Парсер пакета
          {
               condbyte = datapack[1];
               key = (uint16_t) datapack[2];
               value = stoi(datapack.substr(3,4));
               checksum = stoi (datapack.substr(7,3));
          }


          if(condbyte == 1 && datapack.length() > 0) // Существует пакет и бит состояния 1 (команда выполнена)
          {
               int temp = key + value;
               int crc = Crc8((uint8_t *)&temp);
               if (crc != checksum) //Несовпадение контрольных сумм
               {
                    std::cout << "WrongCheckSum" << std::endl; //exception
               }
               return value;
          }
          else
          {
               std::cout << "WrongDataPack" << std::endl; //exception
          }
     }
     else
     {
         buffer = "";
         *readBuffer = buffer;
          std::cout << "NoData" << std::endl;
     }

     return value;
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

uint16_t Conserial::CalcSteps(angle_t angle, angle_t rotateStep, int * dir =0){

     angle = fmod(angle , 360.0); // Подсчет кратчайшего угла поворота
     //Выбор направления
     if ( angle >= 0 ) {
          *dir = 1;
     }
     else {
          *dir = 0;
          angle = -angle;
     }
     int Steps;
     Steps = round (angle / rotateStep); //Подсчёт и округление шагов
     return Steps;
}
}//namespace

