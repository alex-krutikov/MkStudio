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
  "dwords_iregs_hl", // ���.��. ��.�����  ��./��.
  "dwords_iregs_lh", // ���.��. ��.�����  ��./��.
  "floats_iregs_hl", // ���.��. ���.����� ��./��.
  "floats_iregs_lh", // ���.��. ���.����� ��./��.
  "dwords_hregs_hl", // ���.��. ��.�����  ��./��.
  "dwords_hregs_lh", // ���.��. ��.�����  ��./��.
  "floats_hregs_hl", // ���.��. ���.����� ��./��.
  "floats_hregs_lh", // ���.��. ���.����� ��./��.
  };

const char * const MBDataType_names[] = {
  "?",
  "����",
  "�����",
  "�����",
  "��.�����",
  "���.�����",
  "�����. �����",
  "���. ������",
  "���. �����",
  "���. ��������",
  "���.��. ��.�����  ��./��.",
  "���.��. ��.�����  ��./��.",
  "���.��. ���.����� ��./��.",
  "���.��. ���.����� ��./��.",
  "���.��. ��.�����  ��./��.",
  "���.��. ��.�����  ��./��.",
  "���.��. ���.����� ��./��.",
  "���.��. ���.����� ��./��.",
  };

//==============================================================================
// ������ ==> ������ ����������������� �����
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
