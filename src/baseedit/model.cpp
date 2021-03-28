#include "model.h"

#include "mainwindow.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>

Items items;

//##################################################################
//
//##################################################################
Model::Model(QObject *parent)
    : QAbstractTableModel(parent)
{
    undo_buffer_pointer = 0;

    itemfont = QFont("Consolas");
    itemfont.setPixelSize(16);
    itemfont_bold = itemfont;
    itemfont_bold.setBold(true);
    itemfont_italic = itemfont;
    itemfont_italic.setItalic(true);
}

//==================================================================
//
//==================================================================
int Model::rowCount(const QModelIndex &) const
{
    return items.size();
}

//==================================================================
//
//==================================================================
int Model::columnCount(const QModelIndex &) const
{
    return 7;
}

//==================================================================
//
//==================================================================
QVariant Model::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int column = index.column();

    switch (role)
    {
    case (Qt::EditRole):
    case (Qt::DisplayRole):
        if (row <= items.size())
        {
            Item &item = items[row];
            switch (column)
            {
            case (0): // адрес
                if (item.addr >= 0)
                    return QString::number(item.addr, 16).toUpper() + " ";
                break;
            case (1):
                if (item.rnum <= 0) return "";
                return QString::number(item.rnum) + " ";
                break;
            case (2):
                return item.rtype.toString();
                break;
            case (3):
                return item.field;
                break;
            case (4):
                if (item.size > 0) return QString::number(item.size);
                break;
            case (5):
                if (item.count > 0) return QString::number(item.count);
                break;
            case (6):
                return item.desc;
            }
        }
        break;

    case (Qt::FontRole):
        if (items[row].reg_num_is_big_flag && column == 1)
            return itemfont_italic;
        if (items[row].error_flag) return itemfont_bold;
        switch (column)
        {
        case (0):
            return itemfont_bold;
        }
        return itemfont;
        break;

    case (Qt::ForegroundRole):
        if (items[row].reg_num_is_big_flag && column == 1) return QColor("red");
        if (items[row].error_flag)
            return QColor("darkblue");
        else if (column == 0 && items[row].align_flag)
            return QColor("red");
        break;

    case (Qt::BackgroundRole):
        if (items[row].desc.startsWith("---")) return QColor("wheat");
        return QColor(240, 240, 240);
        break;

    case (Qt::TextAlignmentRole):
        switch (column)
        {
        case (0):
            return (int)Qt::AlignVCenter | Qt::AlignRight;
        case (1):
            return (int)Qt::AlignVCenter | Qt::AlignHCenter;
        case (2):
            return (int)Qt::AlignVCenter | Qt::AlignHCenter;
        case (3):
            return (int)Qt::AlignVCenter | Qt::AlignLeft;
        case (4):
            return (int)Qt::AlignVCenter | Qt::AlignHCenter;
        case (5):
            return (int)Qt::AlignVCenter | Qt::AlignHCenter;
        case (6):
            return (int)Qt::AlignVCenter | Qt::AlignLeft;
        }
        break;
    }

    return QVariant();
}

//==================================================================
//
//==================================================================
Qt::ItemFlags Model::flags(const QModelIndex &index) const
{
    if ((index.row() > 0) && (index.column() == 0))
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }
    if (index.column() == 1)
        return Qt::ItemIsEnabled
             | Qt::ItemIsSelectable; // column with register number selected
    if (index.column() == 2)
        return Qt::ItemIsEnabled
             | Qt::ItemIsSelectable; // column with register type selected
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

//==================================================================
//
//==================================================================
QVariant Model::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
    if (orientation != Qt::Horizontal) return QVariant();
    switch (role)
    {
    case (Qt::DisplayRole):
        switch (section)
        {
        case (0):
            return "Адрес";
        case (1):
            return "Номер\nрегистра";
        case (2):
            return "Тип\nрегистра";
        case (3):
            return "Поле";
        case (4):
            return "Размер";
        case (5):
            return "Кол-во";
        case (6):
            return "Описание";
        }
        break;
    }
    return QVariant();
}

//==================================================================
//
//==================================================================
bool Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int row = index.row();
    int column = index.column();

    switch (role)
    {
    case (Qt::UserRole): {
        if (items[row].addr < 0) break; // no register type if there's no
                                        // address
        PossibleRegType new_reg_type = items[row].rtype.toggle();
        if (new_reg_type == None) // no register number if type is not set
        {
            items[row].rnum = -1;
        }
        break;
    }

        /*all the other roles*/
    default:
        if (row >= items.size()) return false;
        switch (column)
        {
        case (0):
            items[row].addr = value.toString().toInt(0, 16);
            if (items[row].addr <= 0)
            {
                items[row].rtype = RegType(None);
                items[row].rnum = -1; // maybe unnecessary
            }
            break;

        case (1):
            items[row].rnum = value.toString().toInt();
            break;

        case (2):
            items[row].rtype = RegType(value.toString());
            break;

        case (3):
            items[row].field = value.toString();
            break;

        case (4):
            items[row].size = value.toInt();
            if (items[row].size == 0)
            {
                items[row].addr = -1; // wo
                items[row].rnum = -1;
                items[row].rtype = RegType(None);
            }
            break;

        case (5):
            items[row].count = value.toInt();
            if (value.toInt() <= 0)
            {
                items[row].addr = -1; // wo
                items[row].rnum = -1;
                items[row].rtype = RegType(None);
            }
            break;

        case (6):
            items[row].desc = value.toString();
            break;
        }
    }
    recalc_items();
    emit dataChanged(index, index);
    create_undo_point();
    return true;
}

//==================================================================
//
//==================================================================
bool Model::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    items.insert(row, count, Item());
    endInsertRows();
    create_undo_point();
    return true;
}

//==================================================================
//
//==================================================================
bool Model::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    items.remove(row, count);
    recalc_items();
    endRemoveRows();
    create_undo_point();
    return true;
}

//==================================================================
//
//==================================================================
void Model::reload_model()
{
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

//==============================================================================
//
//==============================================================================
void Model::recalc_items()
{
    int i, n, a, a0;

    n = items.count();
    if (n == 0) return;

    a0 = items[0].addr;
    a = a0;
    for (i = 0; i < n; i++)
    {
        Item &item = items[i];
        item.align_flag = false;
        item.error_flag = false;
        item.reg_num_is_big_flag = false;
        if (item.field.isEmpty())
        {
            if (i) item.addr = -1;
            continue;
        } else
        {
            if (item.size < 1) item.error_flag = true;
            if (item.count < 1) item.error_flag = true;
        }
        if (item.error_flag)
        {
            if (i) item.addr = -1;
            continue;
        }
        item.addr = a;
        if (item.rtype.getBody() != None)
        {
            if (item.rtype.getBody() == Analog) item.rnum = item.addr / 2;
            if (item.rtype.getBody() == Digital) item.rnum = item.addr * 8;
            if (item.rnum >= 9999) item.reg_num_is_big_flag = true;
            if ((item.rtype.getBody() == Analog) && (item.addr & 1))
            {
                item.reg_num_is_big_flag = true;
            }
        }
        if (item.size == 2)
        {
            if ((a - a0) & 1) item.align_flag = true;
        } else if (item.size == 3)
        {
            item.align_flag = true;
        } else if (item.size >= 4)
        {
            if ((a - a0) & 3) item.align_flag = true;
        }
        a += item.size * item.count;
    }
    items[n - 1].addr = a;

    emit dataChanged(index(0, 0), index(n, 0));
}

//==============================================================================
//
//==============================================================================
void Model::create_undo_point()
{
    while (undo_buffer_pointer--)
        undo_buffer.removeFirst();
    undo_buffer.insert(0, items);
    while (undo_buffer.count() > 40)
    {
        undo_buffer.removeLast();
    }
    undo_buffer_pointer = 0;
}

//==============================================================================
//
//==============================================================================
void Model::undo()
{
    if ((undo_buffer_pointer + 1) >= undo_buffer.size()) return;
    undo_buffer_pointer++;
    items = undo_buffer.at(undo_buffer_pointer);
    reload_model();
}

//==============================================================================
//
//==============================================================================
void Model::redo()
{
    if (undo_buffer_pointer == 0) return;
    undo_buffer_pointer--;
    items = undo_buffer.at(undo_buffer_pointer);
    reload_model();
}
//==============================================================================
//
//==============================================================================
void Model::clear_undo_list()
{
    undo_buffer_pointer = 0;
    undo_buffer.clear();
}

//##################################################################
//
//##################################################################
ItemDelegate::ItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

//==================================================================
//
//==================================================================
QWidget *ItemDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QWidget *w = QItemDelegate::createEditor(parent, option, index);

    QLineEdit *le = qobject_cast<QLineEdit *>(w);
    if (le)
    {
        Qt::Alignment a = Qt::AlignVCenter | Qt::AlignHCenter;
        switch (index.column())
        {
        case (0):
            a = Qt::AlignVCenter | Qt::AlignHCenter;
            break;
        case (1):
            a = Qt::AlignVCenter | Qt::AlignLeft;
            break;
        case (2):
            a = Qt::AlignVCenter | Qt::AlignLeft;
            break;
        case (3):
            a = Qt::AlignVCenter | Qt::AlignLeft;
            break;
        case (4):
            a = Qt::AlignVCenter | Qt::AlignHCenter;
            break;
        case (5):
            a = Qt::AlignVCenter | Qt::AlignHCenter;
            break;
        case (6):
            a = Qt::AlignVCenter | Qt::AlignLeft;
            break;
        }
        le->setAlignment(a);
        le->setFont(index.data(Qt::FontRole).value<QFont>());
    }
    return w;
}

//==================================================================
//
//==================================================================
void ItemDelegate::drawFocus(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QRect &rect) const
{
    QItemDelegate::drawFocus(painter, option, rect);
}

//##################################################################
//
//##################################################################
RegType::RegType()
{
    body = None;
}

RegType::RegType(PossibleRegType t)
{
    body = t;
}

RegType::RegType(QString str)
{
    if ((str == "") || (str == "N"))
    {
        body = None;
        return;
    }
    if (str == "A")
    {
        body = Analog;
        return;
    }
    if (str == "D")
    {
        body = Digital;
        return;
    }
    return; // make compiler happy
}

PossibleRegType RegType::toggle()
{
    switch (body)
    {
    case (None):
        body = Analog;
        return Analog;
        break;

    case (Analog):
        body = Digital;
        return Digital;
        break;

    case (Digital):
        body = None;
        return None;
        break;
    }
    return None; // make compiler happy
}

QString RegType::toString()
{
    switch (body)
    {
    case (None):
        return "";
        break;

    case (Analog):
        return "A";
        break;

    case (Digital):
        return "D";
        break;
    }
    return "";
}

PossibleRegType RegType::getBody()
{
    return body;
}
