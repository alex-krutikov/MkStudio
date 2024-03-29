#ifndef __MBCOMMON_H_
#define __MBCOMMON_H_

#include "mbl_global.h"

#include <QByteArray>
#include <QString>

QString MBL_EXPORT QByteArray2QString(const QByteArray &ba, int mode = 0);

QByteArray encodeArduinoTransport(const QByteArray &ba);
QByteArray decodeArduinoTransport(const QByteArray &ba, bool &ok);

inline int encodedArduinoTransportLength(int len)
{
    int ret = 0;
    ret += (len / 7) * 8;
    ret += (len % 7) ? ((len % 7) + 1) : 0;
    ret += 2;
    return ret;
}

//===================================================================
//! Перечисления с пердставлением в виде строк
/**
     Класс для хранения перечислений с возможностью их пердставлениея в
     виде строк. Например: биты, байты, слова...

     \ingroup Private_group
*/
//===================================================================
class MBL_EXPORT AbstractEnumValues
{
protected:
    int state;

public:
    AbstractEnumValues(const char *const *id_arg = 0,
                       const char *const *name_arg = 0, int len_arg = 0,
                       int init_state = 0)
        : state(init_state)
        , id_str(id_arg)
        , name_str(name_arg)
        , len(len_arg)
    {
    }
    QString toTextId() const { return QString(id_str[state]); }
    QString toName() const { return QString(name_str[state]); }
    int length() const { return len; }
    void fromTextId(QString &textId)
    {
        state = 0;
        for (int i = 0; i < len; i++)
        {
            if (textId == id_str[i])
            {
                state = i;
                break;
            }
        }
    }
    void step()
    {
        state++;
        if (state >= len) state = 1;
    }

private:
    const char *const *id_str;
    const char *const *name_str;
    int len;
};

//===================================================================
//
//===================================================================
extern MBL_EXPORT const char *const MBDataType_id[];
extern MBL_EXPORT const char *const MBDataType_names[];
extern MBL_EXPORT const char *const MBOpType_id[];
extern MBL_EXPORT const char *const MBOpType_names[];

//===================================================================
//! Типы данных - байты, биты, слова... \ingroup Private_group
//===================================================================
class MBDataType : public AbstractEnumValues
{
public:
    MBDataType(int id = 0)
        : AbstractEnumValues(MBDataType_id, MBDataType_names, 23, id)
    {
    }
    enum Id
    {
        Unknown = 0,
        Bits,
        Bytes,
        Words,
        Dwords,
        Floats,
        Doubles,
        DiscreteInputs,
        Coils,
        InputRegisters,
        HoldingRegisters,
        DwordsInputRegHiLo,
        DwordsInputRegLoHi,
        FloatsInputRegHiLo,
        FloatsInputRegLoHi,
        DwordsHoldingRegHiLo,
        DwordsHoldingRegLoHi,
        FloatsHoldingRegHiLo,
        FloatsHoldingRegLoHi,
        BitsArduino,
        BytesArduino,
        WordsArduino,
        DwordsArduino,
        FloatsArduino,
    };
    inline int id() const { return (MBDataType::Id)state; }
    bool isRegister() const;
    bool isAnalogRegister() const;
    bool isDiscreteRegister() const;
    bool isExtendedRegister() const;
    bool isArduino() const;
    inline MBDataType &operator=(const MBDataType::Id &value)
    {
        state = (int)value;
        return *this;
    }
};

#endif
