#include <QtCore>

#include "mbcommon.h"

//===================================================================
//
//===================================================================
const char * const MBDataType_id[]    = { "unknown", "bits", "bytes", "words", "dwords",   "floats",    };
const char * const MBDataType_names[] = { "?",       "биты", "байты", "слова", "дв.слова", "вещ.числа", };

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
