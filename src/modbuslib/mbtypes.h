#ifndef __MBTYPES__H_
#define __MBTYPES__H_

/*! \file  mbtypes.h
    \brief ���� ������ ������ Modbus RTU
*/

#include <QByteArray>
#include <QString>

#include "mbl_global.h"

//===================================================================
//! ������� ������ \ingroup API_group
//===================================================================
class MBL_EXPORT MMValue
{
public:
  enum Status { NotInit=0,Ok,Bad     };
  enum Type   { Invaid=0,Int,Double  };

  inline MMValue();
  inline MMValue( int i, int mask = -1 );
  inline MMValue( double d);
  inline MMValue( const QString &str);
  inline MMValue( const QVariant &var);
  inline MMValue* operator=( char  i );
  inline MMValue* operator=( short i );
  inline MMValue* operator=( int   i );
  inline MMValue* operator=( double d );
  inline MMValue* operator=( QString str );
  inline bool operator==( const MMValue &s ) const;
  inline bool operator!=( const MMValue &s ) const;
  inline QString toString() const;
  inline int     toInt() const;
  inline unsigned int  toUInt() const;
  inline double  toDouble() const;
  inline enum  Status status() const;
  inline void  setStatus( enum  Status status );
  inline bool  isValid() const;
private:
  union Value
  { int    i;
    double d;
  };
  enum  Status st;
  enum  Type   type;
  union Value  v;
  int          mask;
};

inline MMValue::MMValue()
{ type   = Invaid;
  st     = NotInit;
}

inline MMValue::MMValue( int i, int m )
{ type = Int;
  v.i  = i;
  st   = NotInit;
  mask = m;
}

inline MMValue::MMValue( double d)
{ type = Double;
  v.d  = d;
  st   = NotInit;
}

inline MMValue::MMValue( const QString &str)
{ bool ok;
  v.i = str.toInt( &ok );
  if( ok )
  { type = Int;
  } else
  { v.d = str.toDouble( &ok );
    if( ok )
    { type = Double;
    } else
    { type = Invaid;
    }
  }
  st   = NotInit;
}

inline MMValue::MMValue( const QVariant &var)
{ switch( var.type() )
  { case( QVariant::Int ):
      type = Int;
      v.i  = var.toInt();
      break;
    case( QVariant::Double ):
      type = Double;
      v.d  = var.toDouble();
      break;
    case( QVariant::String ):
      (*this) = MMValue( var.toString() );
      break;
    default:
      type = Invaid;
      break;
  }
  st   = NotInit;
}

inline MMValue* MMValue::operator=( char i )
{ st   = Ok;
  type = Int;
  v.i  = i;
  mask = 0xFF;
  return this;
}

inline MMValue* MMValue::operator=( short i )
{ st   = Ok;
  type = Int;
  v.i  = i;
  mask = 0xFFFF;
  return this;
}

inline MMValue* MMValue::operator=( int i )
{ st   = Ok;
  type = Int;
  v.i  = i;
  mask = 0xFFFFFFFF;
  return this;
}

inline MMValue* MMValue::operator=( double d )
{ st   = Ok;
  type = Double;
  v.d  = d;
  return this;
}

inline MMValue* MMValue::operator=( QString str )
{ Q_UNUSED( str );
  return this;
}

inline bool MMValue::operator==( const MMValue &s ) const
{ if( st      != s.st     ) return false;
  if( type    != s.type   ) return false;
  switch( type )
  { case( Invaid ):  return true;
    case( Int ):     if( v.i == s.v.i ) return true;
    case( Double ):  if( v.d == s.v.d ) return true;
    default: break;
  }
  return false;
}

inline bool MMValue::operator!=( const MMValue &s ) const
{ return !( (*this)==s);
}

inline QString MMValue::toString() const
{ switch( type )
  { case( Int ):    return QString::number( v.i );
    case( Double ): return QString::number( v.d );
    default: break;
  }
  return QString();
}

inline int MMValue::toInt() const
{ switch( type )
  { case( Int    ): return      v.i;
    case( Double ): return (int)v.d;
    default: break;
  }
  return 0;
}

inline unsigned int MMValue::toUInt() const
{ switch( type )
  { case( Int   ):  return (unsigned int)( v.i & mask );
    case( Double ): return (unsigned int)v.d;
    default: break;
  }
  return 0;
}

inline double MMValue::toDouble() const
{ switch( type )
  { case( Int ):    return (double)v.i;
    case( Double ): return         v.d;
    default: break;
  }
  return 0;
}

inline enum  MMValue::Status MMValue::status() const
{ return st;
}

inline void MMValue::setStatus( enum  MMValue::Status status )
{ st = status;
}

inline bool  MMValue::isValid() const
{
  return (st == Ok);
}

typedef QVector<MMValue> MMSlotData;

//===================================================================
//! ��������� � ��������� ������ \ingroup Private_group
//===================================================================
struct MBL_EXPORT MMModule
{
  int n;        //!< ����� ������
  int node;     //!< �����
  int subnode;  //!< ��������
  QString name; //!< ��������
  QString desc; //!< ��������

  MMModule() : n(0), node(0), subnode(0) {}
};

//===================================================================
//! ��������� � ��������� ����� ������ \ingroup Private_group
//===================================================================
struct MBL_EXPORT MMSlot
{
//! ������
  enum Status { NotInit = 0,    //!< ���� ��� �� ���� �� �����������
                Ok,             //!< ��������� ����� ������ ������
                Bad,            //!< ��������� ����� ���������� ��������
                Skiped          //!< �������� �� ������
              };
  int status;             //!< ������
  int n;                  //!< ����� �����.
  MMModule module;        //!< ������, �������� ����������� ����
  QString attributes;     //!< ��������� �����
  int addr;               //!< ��������� �����
  int len;                //!< �����
  mutable bool data_read_flag;    //!< ���� � ������� ������
  MBDataType datatype;    //!< ��� ������
  QString    desc;        //!< ��������
  MMSlotData data;        //!< ������

  inline MMSlot()
  { status = NotInit; n=0; addr=0; len=0; data_read_flag=false;
  }
};

//===================================================================
//! ��������� � ��������� ���������� \ingroup Private_group
//===================================================================
struct MBL_EXPORT MMSlotTransaction
{
  MMSlot     *slot;        //!< ������ �� ����
  int        offset;       //!< �������� �� ������ ����� � ���-�� ��������
  int        length;       //!< ���-�� ��������
  int        exp_length;   //!< ��������� ����� ������ (��� �������)
  bool       execute_flag; //!< ���� ���������� ����������
  bool       errortimeout; //!< ������ ���������� -- �������
  int        errorcode;    //!< ��� ������ ����������
  QByteArray request;      //!< ����� �������
  QByteArray answer;       //!< ����� ������
};

//===================================================================
//! ��������� � ��������� ����� MIKKON
/**
   ���������� � ���������������, ������������ �� ������ 0.

   \ingroup Private_group
*/
//===================================================================
struct MBL_EXPORT MikkonModuleDefinition
{
  int id;                       //!< Module ID
  int majorVersion;             //!< Version Major
  int minorVersion;             //!< Version Minor
  QString releaseDate;          //!< Release Date
  QString releaseMadeBy;        //!< Release Made by XYZ (Initials)
  QString productName;          //!< Product Name
  QString productDescription;   //!< Product Description
  QString companyName;          //!< Company Name
  QString legalCopyright;       //!< Legal Copyright
};

#endif
