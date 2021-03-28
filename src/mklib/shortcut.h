#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QStringList>
#include <QVector>

class ShortCut
{
public:
    enum
    {
        WriteValue,
        CopyToClipBoard
    };
    ShortCut(int ActionType, int KeyCode, QString KeyName, QString Assigns,
             int Value)
        : type(ActionType)
        , key(KeyCode)
        , value(Value)
        , name(KeyName)
    {
        setAssigns(Assigns);
    }

    ShortCut(int ActionType, int KeyCode, QString KeyName, QString Assigns)
        : type(ActionType)
        , key(KeyCode)
        , value(0)
        , name(KeyName)
    {
        setAssigns(Assigns);
    }

    ShortCut(int ActionType, int KeyCode, QString KeyName, QString Assigns,
             QString Separator)
        : type(ActionType)
        , key(KeyCode)
        , value(0)
        , name(KeyName)
    {
        setAssigns(Assigns);
        setSeparator(Separator);
    }

    int actionType() { return type; }
    int keyCode() { return key; }
    QString keyName() { return name; }
    int variant() { return value; }
    QString separateSymbol() { return separator; }
    QVector<int> assign() { return assigns; }
    void setType(const int &Type) { type = Type; }
    void setKey(const int &Key) { key = Key; }
    void setValue(const int &Value) { value = Value; }
    void setKeyName(const QString &Name) { name = Name; }
    void setSeparator(const QString &sep) { separator = sep; }
    void setAssigns(const QString &Assigns)
    {
        QStringList list = Assigns.split(";");
        if (list.size())
        {
            for (int i = 0; i < list.size(); i++)
            {
                QStringList smiList = list.at(i).split("/");
                if (smiList.size() == 3)
                {
                    assigns.append(smiList.at(0).toInt());
                    assigns.append(smiList.at(1).toInt());
                    assigns.append(smiList.at(2).toInt() - 1);
                }
            }
        }
    }

private:
    int type;
    int key;
    int value;
    QString name;
    QVector<int> assigns;
    QString separator;
};

#endif // SHORTCUT_H
