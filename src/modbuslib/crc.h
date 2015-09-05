#ifndef __CRC_H__
#define __CRC_H__

/*! \file  crc.h
    \brief Расчет контрольных сумм
*/

#include<QtCore>

#include "mbl_global.h"

//=====================================================================
//! Расчет контрольных сумм \ingroup API_group
//=====================================================================
class MBL_EXPORT CRC
{
public:
//! Расчет CRC16
/**
    \param   ba Массив данных
    \return     Значение контрольной суммы CRC16
*/
  static quint16 CRC16( const QByteArray &ba );

//! Расчет CRC16
/**
    \param   ptr Указатель на массив данных
    \param   len Длина массива данных
    \return      Значение контрольной суммы CRC16
*/
  static quint16 CRC16( const char* ptr, int len );

//! Добавить CRC16 к массиву данных
/**
   Добавляет в конец массива данных контрольную сумму CRC16.
   При этом размер массива увеличивается на 2 байта.

    \param   ba Массив данных (по ссылке)
*/
  static void appendCRC16( QByteArray &ba );
private:
  static const unsigned char auchCRCHi[256];
  static const unsigned char auchCRCLo[256];
};

#endif
