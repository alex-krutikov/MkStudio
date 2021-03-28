#ifndef __MODEL_H__
#define __MODEL_H__

#include <QAbstractTableModel>
#include <QFont>
#include <QItemDelegate>
#include <QList>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QVector>

enum PossibleRegType
{
    None,
    Analog,
    Digital
};

class RegType
{
private:
    PossibleRegType body;

public:
    RegType();
    RegType(QString str);
    RegType(PossibleRegType);
    PossibleRegType getBody();
    PossibleRegType toggle();
    QString toString();
};

struct Item
{
    int addr;
    QString field;
    int size;
    int count;
    QString desc;
    bool align_flag;
    bool error_flag;
    bool cut_flag;
    bool reg_num_is_big_flag; // stange name, but should do it for now
    int rnum;                 // register number
    RegType rtype;            // register type
    Item()
    {
        addr = -1;
        size = 0;
        count = 0;
        cut_flag = align_flag = error_flag = false;
        rnum = -1;
        rtype = RegType(None);
    }
};

typedef QVector<Item> Items;

extern Items items;

class Model : public QAbstractTableModel
{
public:
    Model(QObject *parent = 0);
    void reload_model();
    void recalc_items();
    void undo();
    void redo();
    void create_undo_point();
    void clear_undo_list();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole);

    bool insertRows(int row, int count,
                    const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count,
                    const QModelIndex &parent = QModelIndex());

private:
    QFont itemfont;
    QFont itemfont_bold;
    QFont itemfont_italic;
    QList<Items> undo_buffer;
    int undo_buffer_pointer;
};

class ItemDelegate : public QItemDelegate
{
public:
    ItemDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void drawFocus(QPainter *painter, const QStyleOptionViewItem &option,
                   const QRect &rect) const;
};

#endif
