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
    com_.SetPort(std::string("/dev/ttyStandQKD"));
    com_.SetBaudRate(115200);
    com_.SetDataSize(8);
    com_.SetParity('N');
    com_.SetStopBits(1);
    com_.Open();
    com_.Close();
    com_.Open();
}

Conserial::~Conserial()
{ }

api:: InitResponse Conserial:: Init()
{
     DebugLogger debug(__FUNCTION__);
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
    return debug.Return(response);
}

api::InitResponse Conserial::InitByPD()
{
     DebugLogger debug(__FUNCTION__);
    api::InitResponse response; // Структура для формирования ответа
    response.errorCode_ = 0;
    uint16_t tempTimeOut_ = 900;
    uint16_t tempData = timeoutTime_;
    timeoutTime_ = tempTimeOut_;

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    ce::UartResponse pack;


    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("Init")->second); // Посылаем запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }




    // Заполняем поля структуры
    response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_[0]) * rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_[1]) * rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_[2]) * rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_[3]) * rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.startLightNoises_.h_ = pack.parameters_[4]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.startLightNoises_.v_ = pack.parameters_[5]; //<- начальная засветка детектора, принимающего вертикальную поляризацию

    response.maxSignalLevels_.h_ = pack.parameters_[6]; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.maxSignalLevels_.v_ = pack.parameters_[7]; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.maxLaserPower_ = pack.parameters_[9];

    curAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    timeoutTime_ = tempData;
    return debug.Return(response); // Возвращаем сформированный ответ
}

api::InitResponse Conserial::InitByButtons(WAngles<angle_t> angles)
{
     DebugLogger debug(__FUNCTION__, 4, angles.aHalf_, angles.aQuart_, angles.bHalf_, angles.bQuart_);
    api::InitResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    int steps1_ = round(fmod(angles.aHalf_,360)/rotateStep_);
    int steps2_ = round(fmod(angles.aQuart_,360)/rotateStep_);
    int steps3_ = round(fmod(angles.bHalf_,360)/rotateStep_);
    int steps4_ = round(fmod(angles.bQuart_,360)/rotateStep_);

    ce::UartResponse pack;

    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("InitByButtons")->second, steps1_, steps2_, steps3_, steps4_); // Посылаем запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем поля структуры
    response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_[0]) * rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_[1]) * rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_[2]) * rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_[3]) * rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.startLightNoises_.h_ = pack.parameters_[4]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.startLightNoises_.v_ = pack.parameters_[5]; //<- начальная засветка детектора, принимающего вертикальную поляризацию

    response.maxSignalLevels_.h_ = pack.parameters_[6]; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.maxSignalLevels_.v_ = pack.parameters_[7]; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.maxLaserPower_ = pack.parameters_[8];
    maxLaserPower_ = response.maxLaserPower_;


    curAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    return debug.Return(response); // Возвращаем сформированный ответ
}

api::AdcResponse Conserial::RunTest(adc_t testId)
{
     DebugLogger debug(__FUNCTION__, 1, testId);
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }
    //Заглушка до доработки прошивки на МК
    /*
    // После установки соединения...
    SendUart(dict_.find("RunSelfTest")->second, testId); // Запрос МК
    // Чтение ответа
    ce::UartResponse pack;
    ReadUart(&pack);
    response.adcResponse_ = pack.param1; // Возвращаем целое число
    response.errorCode_ = 0; // Команда отработала корректно
    */
    response = {0,0};
    return debug.Return(response);
}

api::SendMessageResponse Conserial::Sendmessage(WAngles<angle_t> angles, adc_t power)
{
     DebugLogger debug(__FUNCTION__, 5, angles.aHalf_, angles.aQuart_, angles.bHalf_, angles.bQuart_, power);
    api::SendMessageResponse response; // Структура для формирования ответа


    adc_t steps1 = CalcSteps(angles.aHalf_,rotateStep_);
    adc_t steps2 = CalcSteps(angles.aQuart_,rotateStep_);
    adc_t steps3 = CalcSteps(angles.bHalf_,rotateStep_);
    adc_t steps4 = CalcSteps(angles.bQuart_,rotateStep_);

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    ce::UartResponse pack;

    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("SendMessage")->second,  steps1, steps2, steps3, steps4, power);
        ReadUart(&pack);
        if (pack.status_==1 &&
            pack.parameters_[0] == steps1 &&
            pack.parameters_[1] == steps2 &&
            pack.parameters_[2] == steps3 &&
            pack.parameters_[3] == steps4){break;}

        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем поля
    response.newPlatesAngles_.aHalf_  = ((float)pack.parameters_[0]) * rotateStep_; // <- полуволновая пластина "Алисы"     (1я пластинка)
    response.newPlatesAngles_.aQuart_ = ((float)pack.parameters_[1]) * rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.newPlatesAngles_.bHalf_  = ((float)pack.parameters_[2]) * rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.newPlatesAngles_.bQuart_ = ((float)pack.parameters_[3]) * rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.currentLightNoises_.h_ = pack.parameters_[4]; // <- засветка детектора, принимающего горизонтальную поляризацию
    response.currentLightNoises_.v_ = pack.parameters_[5]; // <- засветка детектора, принимающего вертикальную поляризацию

    response.currentSignalLevels_.h_ = pack.parameters_[6]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.currentSignalLevels_.v_ = pack.parameters_[7]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере


    curAngles_ = response.newPlatesAngles_; // Запомнили текущие значения углов

    return debug.Return(response);
}

api::AdcResponse Conserial::SetTimeout(adc_t timeout)
{
     DebugLogger debug(__FUNCTION__, 1, timeout);
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды
    if (timeout <= 0){ return {0,2};}
    else if (timeout >= 900){timeout = 900;}
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    ce::UartResponse pack;

    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("SetTimeout")->second, timeout);
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    response.adcResponse_ = pack.parameters_[0];

    timeoutTime_ = response.adcResponse_ ;

    return debug.Return(response);
}

api::AdcResponse Conserial::SetLaserState(adc_t on)
{
     DebugLogger debug(__FUNCTION__, 1, on);
    api::AdcResponse response; // Структура для формирования ответа

    if(on != 1 && on != 0)
    {
        cout<<"Введено некорректное значение параметра";
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return debug.Return(response);
    }

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("SetLaserState")->second, on); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    response.adcResponse_ = pack.parameters_[0];

    return debug.Return(response); // Возвращаем значение, соответствующее установленному состоянию
}

api::AdcResponse Conserial::SetLaserPower(adc_t power)
{
     DebugLogger debug(__FUNCTION__, 1, power);
    api::AdcResponse response; // Структура для формирования ответа

    if (power > maxLaserPower_)
    {
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return debug.Return(response);
    }

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("SetLaserPower")->second, power); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    response.adcResponse_ = pack.parameters_[0];

    return debug.Return(response); // Возвращаем значение, соответствующее установленному уровню
}

api::AngleResponse Conserial::SetPlateAngle(adc_t plateNumber, angle_t angle)
{
     DebugLogger debug(__FUNCTION__, 2, plateNumber, angle);
    api::AngleResponse response; // Структура для формирования ответа

    if(plateNumber < 1 || plateNumber > 4)
    {
        response.errorCode_ = 2; // // Принят некорректный входной параметр
        return debug.Return(response);
    }


    // Рассчитываем шаги...

    adc_t Steps;
    Steps = CalcSteps(angle,rotateStep_); //Подсчёт и округление шагов

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())

        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("SetPlateAngle")->second, Steps, plateNumber); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем поля
    response.angle_ = ((float)pack.parameters_[0]) * rotateStep_;

    // Запоминаем новый угол на будущее
    switch (plateNumber)
    {
    case 1: curAngles_.aHalf_ = response.angle_; break;
    case 2: curAngles_.aQuart_= response.angle_; break;
    case 3: curAngles_.bHalf_ = response.angle_; break;
    case 4: curAngles_.bQuart_= response.angle_; break;
    }

    return debug.Return(response); // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::SetPlatesAngles(WAngles<angle_t> angles)
{
     DebugLogger debug(__FUNCTION__, 4, angles.aHalf_, angles.aQuart_, angles.bHalf_, angles.bQuart_);
    api::WAnglesResponse response; // Структура для формирования ответа

    adc_t steps1_ = CalcSteps(angles.aHalf_,rotateStep_);
    adc_t steps2_ = CalcSteps(angles.aQuart_,rotateStep_);
    adc_t steps3_ = CalcSteps(angles.bHalf_,rotateStep_);
    adc_t steps4_ = CalcSteps(angles.bQuart_,rotateStep_);

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("SetPlatesAngles")->second, steps1_, steps2_, steps3_, steps4_); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }


    // Заполняем поля
    response.angles_.aHalf_ = ((float)pack.parameters_[0]) * rotateStep_;
    response.angles_.aQuart_ = ((float)pack.parameters_[1]) * rotateStep_;
    response.angles_.bHalf_ = ((float)pack.parameters_[2]) * rotateStep_;
    response.angles_.bQuart_ = ((float)pack.parameters_[3]) * rotateStep_;

    return debug.Return(response); // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::UpdateBaseAngle(WAngles<angle_t> angles)
{
     DebugLogger debug(__FUNCTION__, 4, angles.aHalf_, angles.aQuart_, angles.bHalf_, angles.bQuart_);

    api::WAnglesResponse response; // Структура для формирования ответа

    adc_t steps1_ = CalcSteps(angles.aHalf_,rotateStep_);
    adc_t steps2_ = CalcSteps(angles.aQuart_,rotateStep_);
    adc_t steps3_ = CalcSteps(angles.bHalf_,rotateStep_);
    adc_t steps4_ = CalcSteps(angles.bQuart_,rotateStep_);

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("UpdateBaseAngle")->second, steps1_, steps2_, steps3_, steps4_); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем поля
    response.angles_.aHalf_ = ((float)pack.parameters_[0]) * rotateStep_;
    response.angles_.aQuart_ = ((float)pack.parameters_[1]) * rotateStep_;
    response.angles_.bHalf_ = ((float)pack.parameters_[2]) * rotateStep_;
    response.angles_.bQuart_ = ((float)pack.parameters_[3]) * rotateStep_;

    return debug.Return(response); // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::ReadBaseAngles()
{
     DebugLogger debug(__FUNCTION__);
    api::WAnglesResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("ReadBaseAngles")->second); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }


    // Записываем полученное в структуру
    response.angles_.aHalf_  = ((float)pack.parameters_[0]) * rotateStep_;//<- полуволновая пластина "Алисы"     (1я пластинка)
    response.angles_.aQuart_ = ((float)pack.parameters_[1]) * rotateStep_; //<- четвертьволновая пластина "Алисы" (2я пластинка)
    response.angles_.bHalf_  = ((float)pack.parameters_[2]) * rotateStep_; //<- полуволновая пластина "Боба"      (3я пластинка)
    response.angles_.bQuart_ = ((float)pack.parameters_[3]) * rotateStep_; //<- четвертьволновая пластина "Боба"  (4я пластинка)

    return debug.Return(response);
}

api::AdcResponse Conserial::ReadEEPROM(uint8_t numberUnit_)
{
     DebugLogger debug(__FUNCTION__, 1, numberUnit_);
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("ReadEEPROM")->second, numberUnit_); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];

    return debug.Return(response); // Возвращаем полученное состояние
}

api::AdcResponse Conserial::WriteEEPROM(uint8_t numberUnit_, uint16_t param_)
{
     DebugLogger debug(__FUNCTION__, 2, numberUnit_, param_);
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("WriteEEPROM")->second, numberUnit_, param_); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }
    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];

    return debug.Return(response); // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserState()
{
     DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetLaserState")->second); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];

    return debug.Return(response); // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserPower()
{
     DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetLaserPower")->second);  // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];

    return debug.Return(response); // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetMaxLaserPower()
{
     DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetMaxLaserPower")->second);  // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];

    return debug.Return(response); // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetStartPlatesAngles()
{
     DebugLogger debug(__FUNCTION__);
    api::WAnglesResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Получаем начальные углы поворота волновых пластин от МК

    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetStartPlatesAngles")->second);  // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Записываем полученное в структуру
    response.angles_.aHalf_  = ((float)pack.parameters_[0]) * rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.angles_.aQuart_ = ((float)pack.parameters_[1]) * rotateStep_; //<- четвертьволновая пластина "Алисы" (2я пластинка)
    response.angles_.bHalf_  = ((float)pack.parameters_[2]) * rotateStep_; //<- полуволновая пластина "Боба"      (3я пластинка)
    response.angles_.bQuart_ = ((float)pack.parameters_[3]) * rotateStep_; //<- четвертьволновая пластина "Боба"  (4я пластинка)

    // возвращаем структуру
    return debug.Return(response);
}

api::WAnglesResponse Conserial::GetPlatesAngles()
{
     DebugLogger debug(__FUNCTION__);
    api::WAnglesResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Получаем текущие углы поворота волновых пластин от МК

    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetCurPlatesAngles")->second);     // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Записываем полученное в структуру
    response.angles_.aHalf_  = ((float)pack.parameters_[0]) * rotateStep_;//<- полуволновая пластина "Алисы"     (1я пластинка)
    response.angles_.aQuart_ = ((float)pack.parameters_[1]) * rotateStep_; //<- четвертьволновая пластина "Алисы" (2я пластинка)
    response.angles_.bHalf_  = ((float)pack.parameters_[2]) * rotateStep_; //<- полуволновая пластина "Боба"      (3я пластинка)
    response.angles_.bQuart_ = ((float)pack.parameters_[3]) * rotateStep_; //<- четвертьволновая пластина "Боба"  (4я пластинка)

    return debug.Return(response);
}

api::SLevelsResponse Conserial::GetStartLightNoises()
{
     DebugLogger debug(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }
    // получаем от МК начальные уровни засветки

    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetStartLightNoises")->second);    // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем структуру
    response.signal_.h_ = pack.parameters_[0]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.signal_.v_ = pack.parameters_[1]; // <- начальная засветка детектора, принимающего вертикальную поляризацию

    return debug.Return(response);
}

api::SLevelsResponse Conserial::GetSignalLevels()
{
     DebugLogger debug(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetSignalLevel")->second);    // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.parameters_[0]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.parameters_[1]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    return debug.Return(response);
}

api::AngleResponse Conserial::GetRotateStep()
{
     DebugLogger debug(__FUNCTION__);
    api::AngleResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetRotateStep")->second);    // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Получаем от МК количество шагов для поворота на 360 градусов
    uint16_t steps_ = pack.parameters_[0];
    if(steps_!=0){  rotateStep_ = 360.0 / steps_;} // Считаем сколько градусов в одном шаге
    response.angle_= rotateStep_;

    return debug.Return(response);
}

api::SLevelsResponse Conserial::GetLightNoises()
{
     DebugLogger debug(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetRotateStep")->second);    // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.parameters_[0]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.parameters_[1]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    return debug.Return(response);
}

api::SLevelsResponse Conserial::GetMaxSignalLevels()
{
     DebugLogger debug(__FUNCTION__);
    api::SLevelsResponse response; // Структура для формирования ответа

    // Открываем соединение с МК
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    // Чтение ответа
    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetMaxSignalLevel")->second); // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    response.signal_.h_ = pack.parameters_[0]; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.parameters_[1]; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    return debug.Return(response);
}

api::AdcResponse Conserial::GetErrorCode()
{
     DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }
    /*
    // После установки соединения
    SendUart(dict_.find("GetErrorCode")->second);
    ce::UartResponse pack;
    ReadUart(&pack);
    response.adcResponse_ = pack.param1;
    response.errorCode_ = 0; // Команда отработала корректно
*/
    response = {0,0};
    return debug.Return(response);
}

api::AdcResponse Conserial::GetTimeout()
{
     DebugLogger debug(__FUNCTION__);
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            cout<<"Проверьте соединение со стендом"<<endl;
        response.errorCode_ = 1; // Не удалось установить соединение
        return debug.Return(response);
    }

    ce::UartResponse pack;
    int count = 0;
    while (count<=9) {
        SendUart(dict_.find("GetTimeout")->second);         // Запрос МК
        ReadUart(&pack);
        if (pack.status_==1){break;}
        ++count;
    }
    if (pack.status_ != 1){ response.errorCode_ = pack.status_;
    }
    else {response.errorCode_ = 0; /* Команда отработала корректно*/ }

    response.adcResponse_ = pack.parameters_[0];
    timeoutTime_  = response.adcResponse_;
    return debug.Return(response);
}

//Функция передачи по uart
uint16_t Conserial:: SendUart (char commandName){
     DebugLogger debug(__FUNCTION__, 1, commandName);
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t crc;

    uint16_t tempData_=0;
    tempData_ = (uint8_t) commandName;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));
    com_.Write(start1);
    com_.Write(start2);
    com_.Write(status);
    com_.Write((uint8_t)commandName);
    com_.Write(crc);
    com_.Write(end);
    debug.Return();
    return 1;
}
uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1 ){
     DebugLogger debug(__FUNCTION__, 2, commandName, Parameter1);
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;
    uint16_t tempData_=0;
    tempData_ = (uint8_t) commandName + Parameter1;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));

    com_.Write(start1);
    com_.Write(start2);
    com_.Write(status);
    com_.Write((uint8_t )commandName);
    com_.Write(crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t ) Parameter1);}
    com_.Write(end);
    debug.Return();
    return 1;
}

uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1, uint16_t Parameter2){
     DebugLogger debug(__FUNCTION__, 3, commandName, Parameter1, Parameter2);
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;

    uint16_t tempData_=0;
    tempData_ = (uint8_t) commandName + Parameter1 + Parameter2;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));

    com_.Write(start1);
    com_.Write(start2);
    com_.Write(status);
    com_.Write((uint8_t)commandName);
    com_.Write(crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t) Parameter1);}

    if (Parameter2>=256){
        uint8_t a1 = Parameter2 / 256;
        uint8_t a2 = Parameter2 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t) Parameter2);}

    com_.Write(end);
    debug.Return();
    return 1;
}

uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1, uint16_t Parameter2, uint16_t Parameter3){
     DebugLogger debug(__FUNCTION__, 4, commandName, Parameter1, Parameter2, Parameter3);
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;

    uint16_t tempData_=0;
    tempData_ = (uint8_t) commandName + Parameter1 + Parameter2 + Parameter3;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));
    com_.Write(start1);
    com_.Write(start2);
    com_.Write(status);
    com_.Write((uint8_t )commandName);
    com_.Write(crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t ) Parameter1);}

    if (Parameter2>=256){
        uint8_t a1 = Parameter2 / 256;
        uint8_t a2 = Parameter2 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t ) Parameter2);}

    if (Parameter3>=256){
        uint8_t a1 = Parameter3 / 256;
        uint8_t a2 = Parameter3 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t ) Parameter3);}

    com_.Write(end);
    debug.Return();
    return 1;
}

uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1, uint16_t Parameter2, uint16_t Parameter3, uint16_t Parameter4){
     DebugLogger debug(__FUNCTION__, 5, commandName, Parameter1, Parameter2, Parameter3, Parameter4);
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;

    uint16_t tempData_=0;
    tempData_ = (uint8_t) commandName + Parameter1 + Parameter2 + Parameter3 + Parameter4;
    crc = Crc8((uint8_t *) &tempData_, sizeof(tempData_));
    com_.Write(start1);
    com_.Write(start2);
    com_.Write(status);
    com_.Write((uint8_t )commandName);
    com_.Write(crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t ) Parameter1);}

    if (Parameter2>=256){
        uint8_t a1 = Parameter2 / 256;
        uint8_t a2 = Parameter2 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t) Parameter2);}

    if (Parameter3>=256){
        uint8_t a1 = Parameter3 / 256;
        uint8_t a2 = Parameter3 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t) Parameter3);}

    if (Parameter4>=256){
        uint8_t a1 = Parameter4 / 256;
        uint8_t a2 = Parameter4 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t) Parameter4);}

    com_.Write(end);
    debug.Return();
    return 1;
}

uint16_t Conserial:: SendUart (char commandName, uint16_t Parameter1, uint16_t Parameter2, uint16_t Parameter3, uint16_t Parameter4, uint16_t Parameter5){
     DebugLogger debug(__FUNCTION__, 6, commandName, Parameter1, Parameter2, Parameter3, Parameter4, Parameter5);
    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint8_t status = 0;
    uint16_t end = 65535;
    uint8_t null = 0;
    uint8_t crc;

    uint16_t tempData_=0;

    tempData_ = (uint8_t) commandName + Parameter1 + Parameter2 + Parameter3 + Parameter4 + Parameter5;
    crc = Crc8((uint8_t *)&tempData_, sizeof(tempData_));
    com_.Write(start1);
    com_.Write(start2);
    com_.Write(status);
    com_.Write((uint8_t )commandName);
    com_.Write(crc);

    if (Parameter1>=256){
        uint8_t a1 = Parameter1 / 256;
        uint8_t a2 = Parameter1 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t ) Parameter1);}

    if (Parameter2>=256){
        uint8_t a1 = Parameter2 / 256;
        uint8_t a2 = Parameter2 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t ) Parameter2);}

    if (Parameter3>=256){
        uint8_t a1 = Parameter3 / 256;
        uint8_t a2 = Parameter3 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t ) Parameter3);}

    if (Parameter4>=256){
        uint8_t a1 = Parameter4 / 256;
        uint8_t a2 = Parameter4 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t) Parameter4);}

    if (Parameter5>=256){
        uint8_t a1 = Parameter5 / 256;
        uint8_t a2 = Parameter5 % 256;
        com_.Write(a1);
        com_.Write(a2);
    }else{
        com_.Write(null);
        com_.Write((uint8_t) Parameter5);}

    com_.Write(end);
    debug.Return();
    return 1;
}

//Функция чтения по Uart
void Conserial::ReadUart(ce::UartResponse * packege_)
{
     DebugLogger debug(__FUNCTION__);
     ce::UartResponse pack;

    pack = com_.Read_com(timeoutTime_);
    uint16_t temp = pack.nameCommand_ + pack.parameters_[0] + pack.parameters_[1]
            + pack.parameters_[2] + pack.parameters_[3] + pack.parameters_[4]
            + pack.parameters_[5] + pack.parameters_[6] + pack.parameters_[7]
            + pack.parameters_[8]+ pack.parameters_[9];
    uint8_t crc = Crc8((uint8_t*)&temp, sizeof(temp));
    if (crc == pack.crc_){ * packege_ = pack;}
    else{* packege_ = {3,0,0,{0,0,0,0,0,0,0,0,0,0}};
        cout<< "WrongCheckSum"<<endl; }
}

// Функция подсчёта контрольной суммы
uint8_t Conserial::Crc8(uint8_t *pcBlock, uint8_t len)
{
     DebugLogger debug(__FUNCTION__);
    uint8_t crc = 0xFF;

    while (len--)
        crc = Crc8Table[crc ^ *pcBlock++];

    debug.Return();
    return crc;
}

uint16_t Conserial::CalcSteps(angle_t angle, angle_t rotateStep){
    /*!!!!!КОСТЫЛЬ!!!!>*/
     DebugLogger debug(__FUNCTION__, 2, angle, rotateStep);
    if (angle < 0){
        angle = angle + 360;
    }
    /*<!!!!!КОСТЫЛЬ!!!!*/
    angle = fmod(angle , 360.0); // Подсчет кратчайшего угла поворота

    int Steps = round (angle / rotateStep); //Подсчёт и округление шагов
    return debug.Return(Steps);
}

}//namespace

