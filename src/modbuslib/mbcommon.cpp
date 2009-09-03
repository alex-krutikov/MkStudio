#include <QtCore>

#include "mbcommon.h"

//===================================================================
//
//===================================================================
const char * const MBDataType_id[]    = { "unknown", "bits", "bytes", "words", "dwords",   "floats",    "discrete_inputs", "coils",       "input_registers",  "holding_registers" };
const char * const MBDataType_names[] = { "?",       "биты", "байты", "слова", "дв.слова", "вещ.числа", "дискр. входы",    "рег. флагов", "рег. ввода",   "рег. хранения" };

//==============================================================================
// массив ==> строка шестнадцатеричных чисел
//==============================================================================
QString QByteArray2QString( const QByteArray &ba, int mode )
{
	int i, len = ba.size();
	const char* p = ba.data();
	QString s;
	QString str;
	if( mode == 0 )
	{ str = QString("(length=%1) ").arg( ba.size(),4 );
	}

	for( i=0; i<len; i++ )
	{ s.sprintf(" %2.2X", *(unsigned char*)(p+i) );
		str = str + s;
	}
	return str;
}

bool MBDataType::isRegister() const
{
  switch( state )
  { case MBDataType::DiscreteInputs :
    case MBDataType::Coils :
    case MBDataType::InputRegisters :
    case MBDataType::HoldingRegisters :
      return true;
    default:
     return false;
  }
}

bool MBDataType::isAnalogRegister() const
{
  switch( state )
  { case MBDataType::InputRegisters :
    case MBDataType::HoldingRegisters :
      return true;
    default:
     return false;
  }
}
