#ifndef __CRC_H__
#define __CRC_H__

/*! \file  crc.h
    \brief ������ ����������� ����
*/


#include<QtCore>
//=====================================================================
//! ������ ����������� ����
//=====================================================================
class CRC
{
public:
//! ������ CRC16
/**
    \param   ba ������ ������
    \return     �������� ����������� ����� CRC16
*/
  static quint16 CRC16( const QByteArray &ba );

//! ������ CRC16
/**
    \param   ptr ��������� �� ������ ������
    \param   len ����� ������� ������
    \return      �������� ����������� ����� CRC16
*/
  static quint16 CRC16( const char* ptr, int len );

//! �������� CRC16 � ������� ������
/**
   ��������� � ����� ������� ������ ����������� ����� CRC16.
   ��� ���� ������ ������� ������������� �� 2 �����.
   
    \param   ba ������ ������ (�� ������)
*/
  static void appendCRC16( QByteArray &ba );
private:
  static const unsigned char auchCRCHi[256];
  static const unsigned char auchCRCLo[256];
};

#endif
