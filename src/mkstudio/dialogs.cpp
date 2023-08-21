#include "dialogs.h"

#include "utils.h"

#include <mbmasterxml.h>
#include <mbtcp.h>
#include <mbudp.h>
#include <serialport.h>

#include <QRegExp>
#include <qwt_legend.h>
#include <qwt_math.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>

#include <QCompleter>
#include <QFile>
#include <QKeyEvent>
#include <QMessageBox>

//##############################################################################
/// Окно выбора порта и скорости
//##############################################################################
InitDialog::InitDialog(QWidget *parent, bool isArduinoOnly)
    : QDialog(parent = 0)
    , arduinoOnly(isArduinoOnly)
{
    setupUi(this);
    setWindowTitle("MKStudio");

    Utils::Settings settings;

    errorLabel->clear();

    resize(0, 0); // resize to minimal size

    QString str, str2, portname;
    QStringList sl;
    int i;
    //----------------------------------------------------------------------
    portname = settings.value("portname").toString();
    cb_portname->clear();
    QStringList ports = SerialPort::queryComPorts();
    foreach (str, ports)
    {
        str2 = str;
        str2.replace(';', "        ( ");
        cb_portname->addItem(str2 + " )", str.section(';', 0, 0));
    }

    if (!isArduinoOnly)
    {
        cb_portname->addItem("Modbus TCP", "===TCP===");
        cb_portname->addItem("Modbus UDP", "===UDP===");
    }

    i = cb_portname->findData(portname);
    if (i >= 0) cb_portname->setCurrentIndex(i);
    //----------------------------------------------------------------------
    cb_portspeed->addItems(QStringList() << "300"
                                         << "600"
                                         << "1200"
                                         << "2400"
                                         << "4800"
                                         << "9600"
                                         << "19200"
                                         << "38400"
                                         << "57600"
                                         << "115200"
                                         << "230400"
                                         << "460800"
                                         << "921600");
    i = cb_portspeed->findText(
        settings.value("portspeed", "115200").toString());
    if (i >= 0) cb_portspeed->setCurrentIndex(i);
    //----------------------------------------------------------------------
    le_tcp_server->setText(settings.value("tcpserver").toString());
    //----------------------------------------------------------------------
    sb_timeout->setValue(settings.value(portname + "_timeout", 200).toInt());
    //----------------------------------------------------------------------
    sb_max_len->setValue(settings.value(portname + "_datalength", 128).toInt());
    //----------------------------------------------------------------------
    sb_tr_delay->setValue(settings.value("transaction_delay").toInt());
    sb_cycle_time->setValue(settings.value("cycle_time").toInt());
    //----------------------------------------------------------------------
    setFocus();
}

//==============================================================================
/// Переключение ComboBox'а порта
//==============================================================================
void InitDialog::on_cb_portname_currentIndexChanged(int)
{
    QString str = cb_portname->itemData(cb_portname->currentIndex()).toString();
    bool b = ((str == "===TCP===") || (str == "===UDP==="));

    cb_portspeed->setHidden(b);
    l_speed->setHidden(b);
    l_server->setHidden(!b);
    le_tcp_server->setHidden(!b);

    Utils::Settings settings;
    sb_timeout->setValue(settings.value(str + "_timeout", 200).toInt());
    sb_max_len->setValue(settings.value(str + "_datalength", 128).toInt());
}

//==============================================================================
/// Окно выбора порта и скорости -- нажатие OK
//==============================================================================
void InitDialog::accept()
{
    QString str = cb_portname->itemData(cb_portname->currentIndex()).toString();
    QString str2 = le_tcp_server->text().simplified();
    int timeout = sb_timeout->value();

    Utils::Settings settings;
    settings.setValue("portname", str);
    settings.setValue("portspeed", cb_portspeed->currentText());
    settings.setValue("tcpserver", str2);
    settings.setValue(str + "_timeout", timeout);
    settings.setValue(str + "_datalength", sb_max_len->value());
    settings.setValue("transaction_delay", sb_tr_delay->value());
    settings.setValue("cycle_time", sb_cycle_time->value());

    if (str == "===TCP===")
    {
        str = le_tcp_server->text();
        m_port.reset(new MbTcpPort);
    } else if (str == "===UDP===")
    {
        str = le_tcp_server->text();
        m_port.reset(new MbUdpPort);
    } else
    {
        m_port.reset(new SerialPort);
    }

    if (!str.isEmpty())
    {
        m_port->setName(str);
        m_port->setSpeed(cb_portspeed->currentText().toInt());
        m_port->setAnswerTimeout(timeout);
    }

    if (!m_port->open())
    {
        m_port.reset();
        errorLabel->setText("Не могу открыть порт. Возможно, что порт "
                            "используется другим приложением.");
        return;
    }

    done(QDialog::Accepted);
}

bool InitDialog::portsFound() const
{
    return (cb_portname->count() > 0);
}

//##############################################################################
/// Диалог "Привязка параметров"
//##############################################################################
AssignDialog::AssignDialog(QWidget *parent, const QString &assign_arg,
                           const QString &format_arg, const QString &ss_arg,
                           const QStringList &sl_arg,
                           const bool &ss_confirm_arg)
    : QDialog(parent)
    , assign(assign_arg)
    , format(format_arg)
    , ss(ss_arg)
    , ss_confirm(ss_confirm_arg)
{
    setupUi(this);
    QRegExp rx("^(\\d+)/(\\d+)/(\\d+)$");
    rx.indexIn(assign);
    le1->setText(rx.cap(1));
    le2->setText(rx.cap(2));
    le3->setText(rx.cap(3));
    le1->setFocus();

    if (format.contains("unsigned")) cb_format->setCurrentIndex(1);
    if (format.contains("hex")) cb_format->setCurrentIndex(2);
    if (format.contains("bin")) cb_format->setCurrentIndex(3);

    le_ss->setText(ss);
    cbox_confirm->setChecked(ss_confirm);
    QCompleter *completer = new QCompleter(sl_arg, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    le_ss->setCompleter(completer);

    cb_fill_row->addItems(QStringList() << "модуль"
                                        << "слот"
                                        << "номер");
    cb_fill_column->addItems(QStringList() << "модуль"
                                           << "слот"
                                           << "номер");
    cb_fill_row->setCurrentIndex(2);
    cb_fill_column->setCurrentIndex(2);
    cb_fill_row->setEnabled(false);
    cb_fill_column->setEnabled(false);

    connect(cbox_fill_row, SIGNAL(toggled(bool)), cb_fill_row,
            SLOT(setEnabled(bool)));
    connect(cbox_fill_column, SIGNAL(toggled(bool)), cb_fill_column,
            SLOT(setEnabled(bool)));
}

//==============================================================================
/// Диалог "Привязка параметров" -- нажатие ОК
//==============================================================================
void AssignDialog::accept()
{
    if (cbox_fill_row->isChecked() && cbox_fill_column->isChecked()
        && (cb_fill_row->currentIndex() == cb_fill_column->currentIndex()))
    {
        QMessageBox::warning(this, Utils::AppInfo::title(),
                             "Заполнение ячеек задано неверно. Для строк и "
                             "столбцов заданы одинаковые параметры.");
        return;
    }

    bool m_ok, s_ok, n_ok;
    int im = le1->text().toInt(&m_ok);
    int is = le2->text().toInt(&s_ok);
    int in = le3->text().toInt(&n_ok);

    if (!m_ok || (im < 1))
    {
        QMessageBox::warning(this, Utils::AppInfo::title(),
                             "Поле \"Модуль\" должно содержать номер модуля "
                             "(целое положительное число).");
        return;
    }
    if (!s_ok || (is < 1))
    {
        QMessageBox::warning(this, Utils::AppInfo::title(),
                             "Поле \"Слот\" должно содержать номер слота "
                             "(целое положительное число).");
        return;
    }
    if (!n_ok || (in < 1))
    {
        QMessageBox::warning(
            this, Utils::AppInfo::title(),
            "Поле \"Номер\" должно содержать целое положительное число.");
        return;
    }

    assign = QString::asprintf("%d/%d/%d", im, is, in);


    switch (cb_format->currentIndex())
    {
    case (0):
        format = "dec";
        break;
    case (1):
        format = "unsigned";
        break;
    case (2):
        format = "hex";
        break;
    case (3):
        format = "bin";
        break;
    }

    ss = le_ss->text();
    ss_confirm = cbox_confirm->isChecked();

    done(QDialog::Accepted);
}

//##############################################################################
/// Диалог "Настройки"
//##############################################################################
SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setWindowTitle("Настройки MKStudio");
}

//##############################################################################
/// Диалог "Редактирование текста" (используется при редактировании
/// содержимого заголовка таблицы)
//##############################################################################
TextEditDialog::TextEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setWindowTitle(Utils::AppInfo::title());
}

//##############################################################################
/// Диалог "Настройки"
//##############################################################################
UnitedSlots::UnitedSlots(QWidget *parent, MBMasterXMLPtr mm)
    : QWidget(parent)
    , mm(mm)
{
    int i;
    QTableWidgetItem *titem;
    QColor color;

    setupUi(this);
    setWindowTitle(Utils::AppInfo::title());

    setWindowFlags(Qt::Tool);
    setAttribute(Qt::WA_DeleteOnClose, true);

    tw->verticalHeader()->setDefaultSectionSize(font().pointSize() + 11);
    tw->setColumnCount(2);
    tw->setHorizontalHeaderLabels(QStringList() << "Цвет"
                                                << "Слот");
    tw->horizontalHeader()->resizeSection(0, 50);
    tw->horizontalHeader()->setStretchLastSection(true);
    tw->verticalHeader()->hide();
    tw->setRowCount(curves_num);

    plot->setCanvasBackground(QColor("linen"));
    plot->enableAxis(QwtPlot::yRight, false);
    plot->enableAxis(QwtPlot::yLeft, true);

    for (i = 0; i < curves_num; i++)
    {
        color = QColor((Qt::GlobalColor)(i + 8));
        titem = new QTableWidgetItem;
        titem->setFlags(Qt::NoItemFlags);
        titem->setBackground(QBrush{color});
        tw->setItem(i, 0, titem);

        curves[i] = new QwtPlotCurve("");
        curves[i]->setPen(QPen(color, 2));
        curves[i]->setStyle(QwtPlotCurve::Steps);
        curves[i]->attach(plot);
    }

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->setMajorPen(QPen(Qt::black, 1, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray, 1, Qt::DotLine));
    grid->attach(plot);

    startTimer(1000);
}

//==============================================================================
///
//==============================================================================
void UnitedSlots::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    int i, j, k;
    bool ok;
    QTableWidgetItem *titem;
    QVector<double> plot_data_x;
    QVector<double> plot_data_y;

    for (i = 0; i < curves_num; i++)
    {
        titem = tw->item(i, 1);
        if (titem == 0) continue;
        j = titem->text().toInt(&ok);
        if (!ok) continue;
        if (j < 1) continue;

        MMSlot ss = mm->getSlot(1, j - 1);

        int n = ss.data.size() + 1;
        if (n == 1) continue;
        plot_data_x.resize(n);
        plot_data_y.resize(n);

        for (k = 1; k < n; k++)
        {
            plot_data_y[k] = ss.data[k - 1].toDouble();
            plot_data_x[k] = k;
        }
        plot_data_x[0] = 0;
        plot_data_y[0] = plot_data_y[1];
        curves[i]->setSamples(plot_data_x.constData(), plot_data_y.constData(),
                              n);
    }
    plot->replot();
}
//==============================================================================
///
//==============================================================================
ShortCutsDialog::ShortCutsDialog(QWidget *parent, QString config_file)
    : QDialog(parent)
{
    QTableWidgetItem *twItem;
    QDomElement element;
    QDomNodeList list;
    QDomDocument doc("MKStudio");

    setupUi(this);
    setWindowTitle("Горячие клавиши");

    QFile file(config_file);

    if (file.open(QIODevice::ReadOnly))
    {
        if (doc.setContent(&file))
        {
            file.close();

            if (doc.doctype().name() != "MKStudio")
            {
                QMessageBox::information(
                    this, Utils::AppInfo::title(),
                    "Файл не является конфигурацией MKStudio.");
            } else
            {
                list = doc.elementsByTagName("Table");

                if (list.size())
                {
                    QDomDocument doc_table;
                    doc_table.appendChild(
                        doc_table.importNode(list.item(0), true));

                    QDomElement root = doc_table.documentElement();
                    QDomNodeList shortcut_list
                        = root.elementsByTagName("ShortCut");

                    if (!shortcut_list.size())
                    {
                        QDomElement write0
                            = doc_table.createElement("ShortCut");
                        fillDomEelement(&write0, "Записать 0", "WriteValue");
                        root.appendChild(write0);

                        QDomElement write1
                            = doc_table.createElement("ShortCut");
                        fillDomEelement(&write1, "Записать 1", "WriteValue");
                        root.appendChild(write1);

                        QDomElement copy = doc_table.createElement("ShortCut");
                        fillDomEelement(&copy, "Копировать в буфер обмена",
                                        "CopyToClipBoard");
                        root.appendChild(copy);
                    }

                    shortcut_list = root.elementsByTagName("ShortCut");

                    twItem = new QTableWidgetItem();
                    twItem->setTextAlignment(Qt::AlignCenter);
                    twItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                    table->setItem(0, KeyCol, twItem);
                    twItem = new QTableWidgetItem();
                    twItem->setTextAlignment(Qt::AlignCenter);
                    twItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                    table->setItem(1, KeyCol, twItem);
                    twItem = new QTableWidgetItem();
                    twItem->setTextAlignment(Qt::AlignCenter);
                    twItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
                    table->setItem(2, KeyCol, twItem);

                    for (int i = 0; i < shortcut_list.count(); i++)
                    {
                        element = shortcut_list.item(i).toElement();
                        twItem = new QTableWidgetItem();

                        twItem->setText(element.attribute("Name"));
                        twItem->setData(keyCodeRole,
                                        element.attribute("KeyCode").toInt());
                        twItem->setData(keyNameRole,
                                        element.attribute("KeyName"));
                        table->item(i, KeyCol)->setText(
                            element.attribute("KeyName"));
                        twItem->setData(actionRole,
                                        element.attribute("Action"));

                        QString assign = element.attribute("Assign");
                        twItem->setData(assignRole, assign);
                        twItem->setTextAlignment(Qt::AlignCenter);
                        table->setItem(i, NameCol, twItem);
                        QStringList assignlist = assign.split(";");

                        /* Если привязок больше, чем одна - писать в столбик(tw)
                         */
                        if (assignlist.size() == 1 && i != 2)
                        {
                            twItem = new QTableWidgetItem(assign);
                            twItem->setTextAlignment(Qt::AlignCenter);
                            table->setItem(i, AssignCol, twItem);
                        } else
                        {
                            int j = 0;
                            tw = new QTableWidget();
                            tw->setRowCount(assignlist.size() + 1);
                            tw->setColumnCount(1);
                            tw->verticalHeader()->setHidden(true);
                            tw->horizontalHeader()->setHidden(true);
                            tw->verticalHeader()->setDefaultSectionSize(20);
                            tw->horizontalHeader()->setStretchLastSection(true);
                            tw->setVerticalScrollBarPolicy(
                                Qt::ScrollBarAsNeeded);
                            tw->setHorizontalScrollBarPolicy(
                                Qt::ScrollBarAsNeeded);
                            tw->setFrameShape(QFrame::NoFrame);
                            tw->setGridStyle(Qt::NoPen);

                            for (j = 0; j < assignlist.size(); j++)
                            {
                                twItem = new QTableWidgetItem(assignlist.at(j));
                                twItem->setTextAlignment(Qt::AlignCenter);
                                tw->setItem(j, 0, twItem);
                            }

                            blankItem = new QTableWidgetItem();
                            blankItem->setTextAlignment(Qt::AlignCenter);
                            blankItem->setData(blankRole, true);
                            tw->setItem(j, 0, blankItem);

                            connect(tw, SIGNAL(itemChanged(QTableWidgetItem *)),
                                    this,
                                    SLOT(tableItemChanged(QTableWidgetItem *)));

                            table->setCellWidget(i, AssignCol, tw);

                            QComboBox *separator = new QComboBox();
                            separator->setMaximumHeight(20);
                            separator->addItem("Табуляция", "\t");
                            separator->addItem("Пробел", " ");
                            separator->addItem("Возврат коретки", "\n");
                            separator->addItem(";", ";");
                            separator->addItem(":", ":");

                            table->setCellWidget(2, SeparatorCol, separator);

                            if (element.attribute("Separator").isEmpty())
                                table->item(2, DataCol)
                                    ->setData(separatorRole, "\t");
                            else
                            {
                                twItem->setData(separatorRole,
                                                element.attribute("Separator"));
                                separator->setCurrentIndex(separator->findData(
                                    element.attribute("Separator")));
                            }

                            if (tw->rowCount() == 2 || tw->rowCount() == 1)
                                table->cellWidget(2, SeparatorCol)
                                    ->setEnabled(false);

                            connect(separator, SIGNAL(currentIndexChanged(int)),
                                    this, SLOT(separatorChanged(int)));

                            fitTableRowHeight();
                        }
                    }
                    table->resizeColumnsToContents();
                    table->setColumnWidth(AssignCol, 120);
                    connect(table, SIGNAL(itemChanged(QTableWidgetItem *)),
                            this, SLOT(updateTableItem(QTableWidgetItem *)));
                }
            }
        }
    }
    file.close();
    table->setItemDelegateForColumn(AssignCol, new AssignDelegate);
    tw->setItemDelegateForColumn(0, new AssignDelegate);
}
//==============================================================================
///
//==============================================================================
void ShortCutsDialog::updateTableItem(QTableWidgetItem *item)
{
    if (item->column() == 2)
    {
        table->item(item->row(), DataCol)->setData(assignRole, item->text());
    }
}
//==============================================================================
///
//==============================================================================
void ShortCutsDialog::fillDomEelement(QDomElement *el, QString name,
                                      QString action)
{
    el->setAttribute("Name", name);
    el->setAttribute("KeyCode", "");
    el->setAttribute("KeyName", "");
    el->setAttribute("Action", action);
    el->setAttribute("Separator", "");
    el->setAttribute("Assign", "");
}
//==============================================================================
///
//==============================================================================
void ShortCutsDialog::separatorChanged(int index)
{
    table->item(2, DataCol)
        ->setData(separatorRole,
                  qobject_cast<QComboBox *>(table->cellWidget(2, SeparatorCol))
                      ->itemData(index, Qt::UserRole)
                      .toString());
}
//==============================================================================
///
//==============================================================================
void ShortCutsDialog::tableItemChanged(QTableWidgetItem *itm)
{
    if (itm->data(blankRole).toBool() && !itm->text().isEmpty())
    {
        itm->setData(blankRole, false);
        tw->insertRow(tw->rowCount());
        QTableWidgetItem *twItem = new QTableWidgetItem();
        twItem->setTextAlignment(Qt::AlignCenter);
        twItem->setData(blankRole, true);
        tw->setItem(tw->rowCount() - 1, 0, twItem);
        blankItem = twItem;

        fitTableRowHeight();
    } else if (!itm->data(blankRole).toBool() && itm->text().isEmpty())
    {
        refreshTable(itm);

        fitTableRowHeight();
    }

    QString str;

    for (int i = 0; i < (tw->rowCount() - 1); i++)
    {
        if (i) str += ";";
        str += tw->item(i, 0)->text();
    }
    table->item(2, DataCol)->setData(assignRole, str);

    if (tw->rowCount() == 2 || tw->rowCount() == 1)
        table->cellWidget(2, SeparatorCol)->setEnabled(false);
    else
        table->cellWidget(2, SeparatorCol)->setEnabled(true);
}
//==============================================================================
///
//==============================================================================
void ShortCutsDialog::fitTableRowHeight()
{
    table->setRowHeight(2, tw->rowCount() * tw->rowHeight(0) + 1);
}
//==============================================================================
///
//==============================================================================
void ShortCutsDialog::refreshTable(QTableWidgetItem *itm)
{
    tw->blockSignals(true);
    int row = itm->row();

    for (int i = row; i < (tw->rowCount() - 1); i++)
    {
        tw->item(i, 0)->setText(tw->item(i + 1, 0)->text());
    }
    tw->removeRow(blankItem->row() - 1);
    tw->blockSignals(false);
}
//==============================================================================
///
//==============================================================================
void ShortCutsDialog::keyPressEvent(QKeyEvent *event)
{
    if (table->currentItem()->column() == 1)
    {
        QString keyStr;

        table->item(table->currentItem()->row(), 0)
            ->setData(keyCodeRole, event->key());

        if (event->text().isEmpty())
        {
            switch (event->key())
            {
            /* F1, F5 заняты */
            case Qt::Key_F2:
                keyStr = "F2";
                break;
            case Qt::Key_F3:
                keyStr = "F3";
                break;
            case Qt::Key_F4:
                keyStr = "F4";
                break;
            case Qt::Key_F6:
                keyStr = "F6";
                break;
            case Qt::Key_F7:
                keyStr = "F7";
                break;
            case Qt::Key_F8:
                keyStr = "F8";
                break;
            case Qt::Key_F9:
                keyStr = "F9";
                break;
            case Qt::Key_F10:
                keyStr = "F10";
                break;
            case Qt::Key_F11:
                keyStr = "F11";
                break;
            case Qt::Key_F12:
                keyStr = "F12";
                break;
            case Qt::Key_Home:
                keyStr = "Home";
                break;
            case Qt::Key_End:
                keyStr = "End";
                break;
            case Qt::Key_Control:
                keyStr = "Ctrl";
                break;
            case Qt::Key_Alt:
                keyStr = "Alt";
                break;
            default:
                keyStr = "";
                break;
            }
            table->currentItem()->setText(keyStr);
            table->item(table->currentItem()->row(), DataCol)
                ->setData(keyNameRole, keyStr);
        } else
        {
            table->currentItem()->setText(event->text());
            table->item(table->currentItem()->row(), DataCol)
                ->setData(keyNameRole, event->text());
        }
    } else
    {
        QDialog::keyPressEvent(event);
    }
}
//==============================================================================
///
//==============================================================================
void ShortCutsDialog::accept()
{
    QDomDocument doc;
    ShortCut *sc;

    if (table->cellWidget(2, SeparatorCol)->isEnabled())
    {
        sc = new ShortCut(
            ShortCut::CopyToClipBoard,
            table->item(2, DataCol)->data(keyCodeRole).toInt(),
            table->item(2, DataCol)->data(keyNameRole).toString(),
            table->item(2, DataCol)->data(assignRole).toString(),
            table->item(2, DataCol)->data(separatorRole).toString());
    } else
    {
        sc = new ShortCut(ShortCut::CopyToClipBoard,
                          table->item(2, DataCol)->data(keyCodeRole).toInt(),
                          table->item(2, DataCol)->data(keyNameRole).toString(),
                          table->item(2, DataCol)->data(assignRole).toString());
    }

    hotkey.insert(table->item(2, 0)->data(keyCodeRole).toInt(), sc);

    sc = new ShortCut(ShortCut::WriteValue,
                      table->item(0, DataCol)->data(keyCodeRole).toInt(),
                      table->item(0, DataCol)->data(keyNameRole).toString(),
                      table->item(0, DataCol)->data(assignRole).toString(), 0);
    hotkey.insert(table->item(0, 0)->data(keyCodeRole).toInt(), sc);

    sc = new ShortCut(ShortCut::WriteValue,
                      table->item(1, DataCol)->data(keyCodeRole).toInt(),
                      table->item(1, DataCol)->data(keyNameRole).toString(),
                      table->item(1, DataCol)->data(assignRole).toString(), 1);
    hotkey.insert(table->item(1, 0)->data(keyCodeRole).toInt(), sc);

    for (int i = 0; i < table->rowCount(); i++)
    {
        QDomElement shortcut_item = doc.createElement("ShortCut");
        shortcut_item.setAttribute("Name", table->item(i, DataCol)->text());
        shortcut_item.setAttribute(
            "KeyCode", table->item(i, DataCol)->data(keyCodeRole).toInt());
        shortcut_item.setAttribute(
            "KeyName", table->item(i, DataCol)->data(keyNameRole).toString());
        shortcut_item.setAttribute(
            "Action", table->item(i, DataCol)->data(actionRole).toString());
        shortcut_item.setAttribute(
            "Separator",
            table->item(i, DataCol)->data(separatorRole).toString());
        shortcut_item.setAttribute(
            "Assign", table->item(i, DataCol)->data(assignRole).toString());

        shortcutList.append(shortcut_item);
    }
    QDialog::accept();
}
//==============================================================================
///
//==============================================================================
AssignDelegate::AssignDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}
//==============================================================================
///
//==============================================================================
QWidget *AssignDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QLineEdit *line = new QLineEdit(parent);
    QRegularExpressionValidator *assignExp = new QRegularExpressionValidator(
        QRegularExpression("(\\d+)(\\/)(\\d+)(\\/)([1-9]\\d+)"), parent);
    line->setValidator(assignExp);
    line->setFrame(false);
    line->setAlignment(Qt::AlignCenter);

    connect(line, SIGNAL(editingFinished()), this,
            SLOT(commitAndCloseEditor()));

    return line;
}
//==============================================================================
///
//==============================================================================
void AssignDelegate::setEditorData(QWidget *editor,
                                   const QModelIndex &index) const
{
    QString str = index.model()->data(index, Qt::DisplayRole).toString();

    QLineEdit *line = qobject_cast<QLineEdit *>(editor);
    line->installEventFilter(const_cast<AssignDelegate *>(this));
    line->setText(str);
}
//==============================================================================
///
//==============================================================================
void AssignDelegate::commitAndCloseEditor()
{
    QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}
//==============================================================================
///
//==============================================================================
bool AssignDelegate::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Return)
        {
            QLineEdit *editor = qobject_cast<QLineEdit *>(obj);
            commitData(editor);
            closeEditor(editor);
            return true;
        }
    }
    return QItemDelegate::eventFilter(obj, event);
}
