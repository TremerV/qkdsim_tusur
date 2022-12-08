/// @file
/// @brief Файл с часто используемыми контейнерами для значений углов, сигналов с детекторов и команд API.
///
/// @copyright Copyright 2022 InfoTeCS.
#ifndef LOGICTYPES_H
#define LOGICTYPES_H


/// @brief Структура для хранения значений, соответствующих детекторам стенда.
/// @todo !!!!Протестировать!
template <typename T>
struct SLevels
{
     T h_;
     T v_;

     bool oneG(float x) const
     {
          return h_ > x || v_ > x;
     }

     bool oneGE(float x) const
     {
          return h_ >= x || v_ >= x;
     }

     bool oneL(float x) const
     {
          return h_ < x || v_ < x;
     }

     bool oneLE(float x) const
     {
          return h_ <= x || v_ <= x;
     }

     bool bothG(float x) const
     {
          return h_ > x && v_ > x;
     }

     bool bothGE(float x) const
     {
          return h_ >= x&& v_ >= x;
     }

     bool bothL(float x) const
     {
          return h_ < x && v_ < x;
     }

     bool bothLE(float x) const
     {
          return h_ <= x && v_ <= x;
     }

     SLevels<T> operator+(SLevels<T> x)
     {
          SLevels<T> z;
          z.h_ = x.h_ + this->h_;
          z.v_ = x.v_ + this->v_;

          return z;
     }

}; //SLevels

/// @brief Структура для хранения углов поворота волновых пластин.
///
/// Структура сделана шаблонной по следующим причинам:
/// 1. чтобы сделать logictypes независимой от hwetypes
/// 2. чтобы сделать в коде более прозрачным контейнер (пользователь должен будет явно указывать, как хранятся углы)
/// 3.для буддущих расширений
template<typename T>
struct WAngles
{
     T aHalf_;
     T aQuart_;
     T bHalf_;
     T bQuart_;

     WAngles operator+(WAngles& set) const
     {
          WAngles result;
          result.aHalf_  = this->aHalf_  + set.aHalf_;
          result.aQuart_ = this->aQuart_ + set.aQuart_;
          result.bHalf_  = this->bHalf_  + set.bHalf_;
          result.bQuart_ = this->bQuart_ + set.bQuart_;

          return result;
     }

     WAngles operator-(WAngles& set) const
     {
          WAngles result;
          result.aHalf_  = this->aHalf_  - set.aHalf_;
          result.aQuart_ = this->aQuart_ - set.aQuart_;
          result.bHalf_  = this->bHalf_  - set.bHalf_;
          result.bQuart_ = this->bQuart_ - set.bQuart_;

          return result;
     }
}; // WAngles

struct BasisBit
{
     int basis_;
     bool bit_;

     // Определяем операцию сравениня для использования в качестве ключа map
     bool operator<(const BasisBit& second) const
     {
          if(this->basis_ < second.basis_)
               return true;
          else if(this->basis_ == second.basis_)
          {
               if(this->bit_ < second.bit_)
                    return true;
               else
                    return false;
          }
          else
               return false;
     }
};

enum BitStatus
{
     BIT_UNRECOGNIZED, ///< Бит не распознан при приёме
     BIT_NOTSIFTED,    ///< Бит распознан, но базисы не сошлись
     BIT_NOTCLEARED,   ///< Бит распознан, базисы сошлись, но принятый бит отличается от отправленного
     BIT_OK
};

/// Перечисление типов излучателей.
enum Emitter
{
     CLASSIC,  ///< Непрерывный режим работы лазера.
     SP,       ///< Однофотонный режим работы лазера.
     QSP       ///< Квазиоднофотонный режим работы лазера.
};

#endif // LOGICTYPES_H
