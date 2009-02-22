#ifndef __MBCOMMON_H_
#define __MBCOMMON_H_

#include <QtCore>

QString QByteArray2QString( const QByteArray &ba, int mode=0 );

//===================================================================
//! Перечисляемые значения с пердставлением в виде строк.
/**
     Класс для хранения перечисляемых значений с их пердставлением в
     виде строк.
*/
//===================================================================
class AbstractEnumValues
{
protected:
  int state;
public:

  AbstractEnumValues(
       const char * const *id_arg=0,
       const char * const *name_arg=0,
       int len_arg=0, int init_state=0 )
    :  state(init_state), id_str(id_arg), name_str(name_arg), len(len_arg) {}
  QString toTextId() const  { return QString(id_str[state]);   }
  QString toName() const    { return QString(name_str[state]); }
  void fromTextId( QString &textId )
    { state = 0;
      for( int i=0; i<len; i++ )
      { if( textId == id_str[i] )
        { state = i;
          break;
        }
      }
    }
  void step()
    { state++; if(state>=len) state=1;
    }

private:
  const char * const *id_str;
  const char * const *name_str;
  int len;
};

//===================================================================
//
//===================================================================
extern const char * const MBDataType_id[];
extern const char * const MBDataType_names[];
extern const char * const MBOpType_id[];
extern const char * const MBOpType_names[];

//===================================================================
/// Представление modbus типов данных - байты, биты, слова...
//===================================================================
class MBDataType : public AbstractEnumValues
{
public:
  MBDataType( int id = 0 ) : AbstractEnumValues( MBDataType_id, MBDataType_names ,6, id ) {}
  enum Id { Unknown=0, Bits, Bytes, Words, Dwords, Floats };
  inline int id() const { return (MBDataType::Id)state; }
  inline  MBDataType& operator=( const MBDataType::Id &value )
  { state = (int)value; return *this;
  }
};

#endif
