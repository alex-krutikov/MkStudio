#include <QtCore>

#include "mbcommon.h"

//===================================================================
//
//===================================================================
const char * const MBDataType_id[]    = {
  "unknown",
  "bits",
  "bytes",
  "words",
  "dwords",
  "floats",
  "discrete_inputs",
  "coils",
  "input_registers",
  "holding_registers",
  "dwords_iregs_hl", // рег.вв. дв.слова  ст./мл.
  "dwords_iregs_lh", // рег.вв. дв.слова  мл./ст.
  "floats_iregs_hl", // рег.вв. вещ.числа ст./мл.
  "floats_iregs_lh", // рег.вв. вещ.числа мл./ст.
  "dwords_hregs_hl", // рег.хр. дв.слова  ст./мл.
  "dwords_hregs_lh", // рег.хр. дв.слова  мл./ст.
  "floats_hregs_hl", // рег.хр. вещ.числа ст./мл.
  "floats_hregs_lh", // рег.хр. вещ.числа мл./ст.
  };

const char * const MBDataType_names[] = {
  "?",
  "биты",
  "байты",
  "слова",
  "дв.слова",
  "вещ.числа",
  "дискр. входы",
  "рег. флагов",
  "рег. ввода",
  "рег. хранения",
  "рег.вв. дв.слова  ст./мл.",
  "рег.вв. дв.слова  мл./ст.",
  "рег.вв. вещ.числа ст./мл.",
  "рег.вв. вещ.числа мл./ст.",
  "рег.хр. дв.слова  ст./мл.",
  "рег.хр. дв.слова  мл./ст.",
  "рег.хр. вещ.числа ст./мл.",
  "рег.хр. вещ.числа мл./ст.",
  };

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
    case MBDataType::DwordsInputRegHiLo :
    case MBDataType::FloatsInputRegHiLo :
    case MBDataType::DwordsHoldingRegHiLo :
    case MBDataType::FloatsHoldingRegHiLo :
    case MBDataType::DwordsInputRegLoHi :
    case MBDataType::FloatsInputRegLoHi :
    case MBDataType::DwordsHoldingRegLoHi :
    case MBDataType::FloatsHoldingRegLoHi :
      return true;
  }
  return false;
}

bool MBDataType::isAnalogRegister() const
{
  switch( state )
  { case MBDataType::InputRegisters :
    case MBDataType::HoldingRegisters :
    case MBDataType::DwordsInputRegHiLo :
    case MBDataType::FloatsInputRegHiLo :
    case MBDataType::DwordsHoldingRegHiLo :
    case MBDataType::FloatsHoldingRegHiLo :
    case MBDataType::DwordsInputRegLoHi :
    case MBDataType::FloatsInputRegLoHi :
    case MBDataType::DwordsHoldingRegLoHi :
    case MBDataType::FloatsHoldingRegLoHi :
      return true;
  }
  return false;
}

bool MBDataType::isDiscreteRegister() const
{
  switch( state )
  { case MBDataType::DiscreteInputs :
    case MBDataType::Coils :
      return true;
  }
  return false;
}

bool MBDataType::isExtendedRegister() const
{
  switch( state )
  { case MBDataType::DwordsInputRegHiLo :
    case MBDataType::FloatsInputRegHiLo :
    case MBDataType::DwordsHoldingRegHiLo :
    case MBDataType::FloatsHoldingRegHiLo :
    case MBDataType::DwordsInputRegLoHi :
    case MBDataType::FloatsInputRegLoHi :
    case MBDataType::DwordsHoldingRegLoHi :
    case MBDataType::FloatsHoldingRegLoHi :
      return true;
  }
  return false;
}
