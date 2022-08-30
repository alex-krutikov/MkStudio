#include "mbcommon.h"

//===================================================================
//
//===================================================================
const char *const MBDataType_id[] = {
    "unknown",
    "bits",
    "bytes",
    "words",
    "dwords",
    "floats",
    "doubles",
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
    "bits_arduino",
    "bytes_arduino",
    "words_arduino",
    "dwords_arduino",
    "floats_arduino",
};

const char *const MBDataType_names[] = {
    "?",
    "биты",
    "байты",
    "слова",
    "дв.слова",
    "вещ.числа",
    "дв.вещ.числа",
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
    "Arduino биты",
    "Arduino байты",
    "Arduino слова",
    "Arduino дв.слова",
    "Arduino вещ.числа",
};

//==============================================================================
// массив ==> строка шестнадцатеричных чисел
//==============================================================================
QString QByteArray2QString(const QByteArray &ba, int mode)
{
    int i, len = ba.size();
    const char *p = ba.data();
    QString s;
    QString str;
    if (mode == 0) { str = QString("(size=%1) ").arg(ba.size(), 4); }

    for (i = 0; i < len; i++)
    {
        s = QString::asprintf(" %2.2X", *(unsigned char *)(p + i));
        str = str + s;
    }
    return str;
}

bool MBDataType::isRegister() const
{
    switch (state)
    {
    case MBDataType::DiscreteInputs:
    case MBDataType::Coils:
    case MBDataType::InputRegisters:
    case MBDataType::HoldingRegisters:
    case MBDataType::DwordsInputRegHiLo:
    case MBDataType::FloatsInputRegHiLo:
    case MBDataType::DwordsHoldingRegHiLo:
    case MBDataType::FloatsHoldingRegHiLo:
    case MBDataType::DwordsInputRegLoHi:
    case MBDataType::FloatsInputRegLoHi:
    case MBDataType::DwordsHoldingRegLoHi:
    case MBDataType::FloatsHoldingRegLoHi:
        return true;
    }
    return false;
}

bool MBDataType::isAnalogRegister() const
{
    switch (state)
    {
    case MBDataType::InputRegisters:
    case MBDataType::HoldingRegisters:
    case MBDataType::DwordsInputRegHiLo:
    case MBDataType::FloatsInputRegHiLo:
    case MBDataType::DwordsHoldingRegHiLo:
    case MBDataType::FloatsHoldingRegHiLo:
    case MBDataType::DwordsInputRegLoHi:
    case MBDataType::FloatsInputRegLoHi:
    case MBDataType::DwordsHoldingRegLoHi:
    case MBDataType::FloatsHoldingRegLoHi:
        return true;
    }
    return false;
}

bool MBDataType::isDiscreteRegister() const
{
    switch (state)
    {
    case MBDataType::DiscreteInputs:
    case MBDataType::Coils:
        return true;
    }
    return false;
}

bool MBDataType::isExtendedRegister() const
{
    switch (state)
    {
    case MBDataType::DwordsInputRegHiLo:
    case MBDataType::FloatsInputRegHiLo:
    case MBDataType::DwordsHoldingRegHiLo:
    case MBDataType::FloatsHoldingRegHiLo:
    case MBDataType::DwordsInputRegLoHi:
    case MBDataType::FloatsInputRegLoHi:
    case MBDataType::DwordsHoldingRegLoHi:
    case MBDataType::FloatsHoldingRegLoHi:
        return true;
    }
    return false;
}

bool MBDataType::isArduino() const
{
    switch (state)
    {
    case MBDataType::BitsArduino:
    case MBDataType::BytesArduino:
    case MBDataType::WordsArduino:
    case MBDataType::DwordsArduino:
    case MBDataType::FloatsArduino:
        return true;
    }
    return false;
}

QByteArray encodeArduinoTransport(const QByteArray &ba)
{
    QByteArray ret;

    ret.append(0x80);

    for (int i = 0; i < ba.length();)
    {
        char extraByte = 0;
        for (int j = 0; (j < 7) && ((i + j) < ba.length()); ++j)
        {
            if (ba.at(i + j) & 0x80) extraByte |= (1 << (6 - j));
        }
        ret.append(extraByte);
        for (int j = 0; (j < 7) && ((i + j) < ba.length()); ++j)
        {
            ret.append(ba.at(i + j) & 0x7F);
        }
        i += 7;
    }

    ret.append(0x81);

    return ret;
}

QByteArray decodeArduinoTransport(const QByteArray &ba, bool &ok)
{
    QByteArray ret;
    ok = false;

    if (ba.length() < 2) return QByteArray();
    if (ba.at(0) != (char)0x80) return QByteArray();
    if (ba.at(ba.length() - 1) != (char)0x81) return QByteArray();

    for (int i = 1; i < (ba.length() - 1);)
    {
        char extraByte = ba.at(i);
        i++;
        extraByte <<= 1;
        for (int j = 0; (j < 7) && ((i + j) < (ba.length() - 1)); j++)
        {
            ret.append(ba.at(i + j) | (extraByte & (0x80)));
            extraByte <<= 1;
        }
        i += 7;
    }

    ok = true;
    return ret;
}
