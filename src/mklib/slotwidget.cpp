#include "slotwidget.h"

#include "mbmasterxml.h"

#include <console.h>

#include <qwt_legend.h>
#include <qwt_math.h>
#include <qwt_painter.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_series_data.h>

#include <QClipboard>
#include <QDomDocument>
#include <QFile>
#include <QMessageBox>

#include "ui/ui_slotdialog.h"
#include "ui/ui_slotwidget.h"

//###################################################################
///Окно с представлением слота (используется по двойному нажатию на слот)
//###################################################################
SlotWidget::SlotWidget(QWidget *parent, MBMasterXMLPtr mm, int module, int slot,
                       bool min_flag)
    : QMainWindow(parent)
    , ui(new Ui::SlotWidget)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint
                   | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, true);
    resize(800, 400);

    module_n = module;
    slot_n = slot;
    this->mm = mm;
    kx = ky = 1;
    ax = 0;
    minimize_flag = min_flag;

    const MMSlot slot2 = mm->getSlot(module_n, slot_n);
    // меню "экспорт"
    QAction *act = new QAction(this);
    act->setText("Экспорт");
    ui->menubar->addAction(act);
    connect(act, SIGNAL(triggered()), SLOT(export_data()));

    // импортирование настроек из строки
    QRegExp rx("^(.+)=(.+)$");
    foreach (QString s, slot2.attributes.split(';'))
    {
        if (rx.indexIn(s) == 0) { settings[rx.cap(1)] = rx.cap(2); }
    }

    QFile qss(":/qss/res/toolbuttons.qss");
    qss.open(QFile::ReadOnly);
    QString tb_qss = QLatin1String(qss.readAll());
    qss.close();

    //кнопка Пауза
    pb_pause = new QToolButton;
    pb_pause->setText("Пауза");
    pb_pause->setToolTip("Пауза");
    pb_pause->setCursor(QCursor(Qt::ArrowCursor));
    pb_pause->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    pb_pause->setStyleSheet(tb_qss);
    pb_pause->setIcon(QIcon(":/icons/res/pause.png"));
    connect(pb_pause, SIGNAL(clicked()), this, SLOT(pb_pause_clicked()));

    pause_flag = false;

    //кнопка Минимизация таблицы опроса
    pb_minimize = new QToolButton;
    pb_minimize->setCursor(QCursor(Qt::ArrowCursor));
    pb_minimize->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    pb_minimize->setStyleSheet(tb_qss);
    connect(pb_minimize, SIGNAL(clicked()), this, SLOT(pb_minimize_clicked()));
    updateMinimizeButtonState(minimize_flag);

    toolWidget = new QWidget(ui->plot);
    QHBoxLayout *layout = new QHBoxLayout;

    layout->addWidget(pb_pause);
    layout->addWidget(pb_minimize);
    layout->setSpacing(5);
    layout->setContentsMargins(0, 0, 0, 0);
    toolWidget->setLayout(layout);
    toolWidget->move(5, 5);

    // обработка меню
    connect(ui->action_view_table, SIGNAL(triggered()), this,
            SLOT(view_changed()));
    connect(ui->action_view_plot1, SIGNAL(triggered()), this,
            SLOT(view_changed()));
    connect(ui->action_view_plot2, SIGNAL(triggered()), this,
            SLOT(view_changed()));
    connect(ui->action_view_text, SIGNAL(triggered()), this,
            SLOT(view_changed()));

    connect(ui->action_stat_show, SIGNAL(toggled(bool)), this,
            SLOT(slot_statistic_show(bool)));
    connect(ui->chb_line_trans_use, SIGNAL(toggled(bool)), this,
            SLOT(slot_line_trans_used(bool)));

    connect(ui->action_format_bin, SIGNAL(triggered()), this,
            SLOT(format_changed()));
    connect(ui->action_format_dec, SIGNAL(triggered()), this,
            SLOT(format_changed()));
    connect(ui->action_format_unsigned, SIGNAL(triggered()), this,
            SLOT(format_changed()));
    connect(ui->action_format_hex, SIGNAL(triggered()), this,
            SLOT(format_changed()));
    connect(ui->action_format_float, SIGNAL(triggered()), this,
            SLOT(format_changed()));

    // название окна
    QString str;
    str = "Cлот: " + slot2.desc;
    str += " [ " + QString::number(slot2.module.n) + "."
         + QString::number(slot2.n);
    str += " | " + slot2.datatype.toName() + " | "
         + QString::asprintf("0x%X", slot2.addr);
    str += " | " + QString::number(slot2.len);
    str += " | " + slot2.module.name + " | ";
    str += "узел " + QString::number(slot2.module.node);
    if (slot2.module.subnode)
        str += "." + QString::number(slot2.module.subnode);
    str += " ]" + slot2.module.desc;
    setWindowTitle(str);

    // возможные варианты меню
    switch (slot2.datatype.id())
    {
    case (MBDataType::Floats):
    case (MBDataType::Doubles):
        ui->action_format_bin->setEnabled(false);
        ui->action_format_dec->setEnabled(false);
        ui->action_format_unsigned->setEnabled(false);
        ui->action_format_hex->setEnabled(false);
        ui->action_format_float->setEnabled(true);
        ui->action_format_float->setChecked(true);
        break;
    case (MBDataType::Bits):
        ui->action_format_bin->setEnabled(false);
        ui->action_format_dec->setEnabled(true);
        ui->action_format_unsigned->setEnabled(false);
        ui->action_format_hex->setEnabled(false);
        ui->action_format_float->setEnabled(false);
        ui->action_format_dec->setChecked(true);
        break;
    default:
        ui->action_format_float->setEnabled(false);
        ui->action_format_dec->setChecked(true);
        break;
    }

    // конфигурация таблицы
    configure_table();

    // конфигурация графиков
    ui->plot->setCanvasBackground(QColor("linen"));
    ui->plot->enableAxis(QwtPlot::yRight, false);
    ui->plot->enableAxis(QwtPlot::yLeft, true);
    ui->plot->setAxisAutoScale(QwtPlot::yLeft);

    plot_curve = new QwtPlotCurve("");
    plot_curve->setPen(QPen(Qt::darkBlue, 2));
    plot_curve->setStyle(QwtPlotCurve::Steps);
    plot_curve->attach(ui->plot);

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->enableYMin(true);
    grid->setMajorPen(QPen(Qt::black, 1, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray, 1, Qt::DotLine));
    grid->attach(ui->plot);

    ui->plot2->setCanvasBackground(QColor("linen"));
    ui->plot2->enableAxis(QwtPlot::yRight, false);
    ui->plot2->enableAxis(QwtPlot::yLeft, true);
    ui->plot2->setAxisAutoScale(QwtPlot::yLeft);

    plot2_curve = new QwtPlotCurve("");
    plot2_curve->setPen(QPen(Qt::darkBlue, 2));
    plot2_curve->setStyle(QwtPlotCurve::Steps);
    plot2_curve->attach(ui->plot2);

    QwtPlotGrid *grid2 = new QwtPlotGrid;
    grid2->enableXMin(true);
    grid2->enableYMin(true);
    grid2->setMajorPen(QPen(Qt::black, 1, Qt::DotLine));
    grid2->setMinorPen(QPen(Qt::gray, 1, Qt::DotLine));
    grid2->attach(ui->plot2);
    //настройка гисограммы
    for (int i = 0; i < hist_plot_data_y_len; i++)
        hist_plot_data_x[i] = i;

    ui->hist->setCanvasBackground(QColor("linen"));
    ui->hist->enableAxis(QwtPlot::xTop, false);
    ui->hist->enableAxis(QwtPlot::xBottom, false);
    ui->hist->enableAxis(QwtPlot::yRight, false);
    ui->hist->enableAxis(QwtPlot::yLeft, false);

    QwtPlotCurve *hist_data = new QwtPlotCurve("Curve 2");
    hist_data->setPen(QPen{Qt::NoPen});
    hist_data->setStyle(QwtPlotCurve::Steps);
    hist_data->setBrush(QColor("lightslategrey"));
    hist_data->setRawSamples(hist_plot_data_x, hist_plot_data_y,
                             hist_plot_data_y_len);
    hist_data->attach(ui->hist);
    // применение настроек
    QMap<QString, QString> settings2 = settings;

    str = settings2["view"];
    if (str == "table" || str.isEmpty())
        ui->action_view_table->activate(QAction::Trigger);
    if (str == "plot") ui->action_view_plot1->activate(QAction::Trigger);
    if (str == "scaled_plot") ui->action_view_plot2->activate(QAction::Trigger);
    if (str == "text") ui->action_view_text->activate(QAction::Trigger);
    str = settings2["format"];
    if (str == "dec") ui->action_format_dec->activate(QAction::Trigger);
    if (str == "bin") ui->action_format_bin->activate(QAction::Trigger);
    if (str == "unsigned")
        ui->action_format_unsigned->activate(QAction::Trigger);
    if (str == "hex") ui->action_format_hex->activate(QAction::Trigger);
    if (str == "float") ui->action_format_float->activate(QAction::Trigger);
    applay_settings();

    int a;
    bool ok;
    ui->statGroupBox->setVisible(false);
    if (settings.contains("statistic_show"))
    {
        a = settings["statistic_show"].toInt(&ok);
        if (ok && a)
        {
            ui->action_stat_show->setChecked(true);
            // slot_statistic_show( false );
        }
    }

    ui->text->setReadOnly(true);

    // таймер перерисовки графиков
    startTimer(1000);
}

//===================================================================
//
//===================================================================
SlotWidget::~SlotWidget()
{
    delete ui;
}

//===================================================================
/// Обработка меню "Вид"
//===================================================================
void SlotWidget::view_changed()
{
    QAction *act = (QAction *)sender();
    ui->action_view_table->setChecked(act == ui->action_view_table);
    ui->action_view_plot1->setChecked(act == ui->action_view_plot1);
    ui->action_view_plot2->setChecked(act == ui->action_view_plot2);
    ui->action_view_text->setChecked(act == ui->action_view_text);

    if (act == ui->action_view_table)
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->action_stat_show->setEnabled(false);
        ui->statGroupBox->setVisible(false);
    } else if (act == ui->action_view_plot1)
    {
        ui->stackedWidget->setCurrentIndex(1);
        ui->action_stat_show->setEnabled(true);
        ui->statGroupBox->setVisible(ui->action_stat_show->isChecked());
        toolWidget->setParent(ui->plot->canvas());
        toolWidget->show();
    } else if (act == ui->action_view_plot2)
    {
        ui->stackedWidget->setCurrentIndex(2);
        ui->action_stat_show->setEnabled(true);
        ui->statGroupBox->setVisible(ui->action_stat_show->isChecked());
        toolWidget->setParent(ui->plot2->canvas());
        toolWidget->show();
    } else if (act == ui->action_view_text)
    {
        ui->stackedWidget->setCurrentIndex(3);
        ui->action_stat_show->setEnabled(false);
        ui->statGroupBox->setVisible(false);
    }
    save_settings();
}

//==============================================================================
/// Обработка меню "Формат"
//==============================================================================
void SlotWidget::format_changed()
{
    QAction *act = (QAction *)sender();
    ui->action_format_bin->setChecked(act == ui->action_format_bin);
    ui->action_format_dec->setChecked(act == ui->action_format_dec);
    ui->action_format_unsigned->setChecked(act == ui->action_format_unsigned);
    ui->action_format_hex->setChecked(act == ui->action_format_hex);
    ui->action_format_float->setChecked(act == ui->action_format_float);

    configure_table();
    save_settings();
}

//==============================================================================
/// Экспорт данных в буфер обмена
//==============================================================================
void SlotWidget::export_data()
{
    QString str;
    int i, j, n, m;
    QwtPlotCurve *curve = 0;

    switch (ui->stackedWidget->currentIndex())
    {
    case (0): // экспорт таблицы
        n = ui->table->rowCount();
        m = ui->table->columnCount();
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < m; j++)
            {
                QTableWidgetItem *titem = ui->table->item(i, j);
                if (!titem) continue;
                if (!(titem->flags() & Qt::ItemIsEditable)) continue;
                str += titem->text() + "\n";
            }
        }
        str.chop(1);
        break;
    case (1): // график
        curve = plot_curve;
        break;
    case (2): // масштабированный график
        curve = plot2_curve;
        break;
    }

    if (curve)
    { // экспорт содержания графиков
        const QwtSeriesData<QPointF> *d = curve->data();
        n = d->size();
        for (i = 1; i < n; i++)
        {
            str += QString::number(d->sample(i - 1).x()) + "\t";
            str += QString::number(d->sample(i).y(), 'g', 8) + "\n";
        }
        str.chop(1);
    }

    QApplication::clipboard()->setText(str);
    QMessageBox::information(this, "Слот",
                             "Данные скопированы в буфер обмена.");
}
//==============================================================================
//
//==============================================================================
void SlotWidget::pb_pause_clicked()
{
    if (pause_flag)
    {
        pause_flag = false;
        pb_pause->setText("Пауза");
        pb_pause->setToolTip("Пауза");
        pb_pause->setIcon(QIcon(":/icons/res/pause.png"));
    } else
    {
        pause_flag = true;
        pb_pause->setText("Пуск");
        pb_pause->setToolTip("Пуск");
        pb_pause->setIcon(QIcon(":/icons/res/play.png"));
    }
}
//==============================================================================
//
//==============================================================================
void SlotWidget::pb_minimize_clicked()
{
    emit signalMinimize(!minimize_flag);
}

//==============================================================================
/// Подготовка и выдача (сигналом) текущих настроек окна
//==============================================================================
void SlotWidget::save_settings()
{
    // текущий формат отображения таблицы
    if (ui->action_format_dec->isChecked())
        settings["format"] = "dec";
    else if (ui->action_format_unsigned->isChecked())
        settings["format"] = "unsigned";
    else if (ui->action_format_hex->isChecked())
        settings["format"] = "hex";
    else if (ui->action_format_bin->isChecked())
        settings["format"] = "bin";
    else if (ui->action_format_float->isChecked())
        settings["format"] = "float";

    // текущий вид
    switch (ui->stackedWidget->currentIndex())
    {
    case (0):
        settings["view"] = "table";
        break;
    case (1):
        settings["view"] = "plot";
        break;
    case (2):
        settings["view"] = "scaled_plot";
        break;
    case (3):
        settings["view"] = "text";
        break;
    }

    // упаковка всех настроек в строку
    QString str;
    foreach (QString key, settings.keys())
    {
        str += key + "=" + settings[key].remove(';') + ";";
    }
    if (str.endsWith(";")) str.chop(1);
    // вызов сигнала
    emit attributes_saved(module_n, slot_n, str);
}

//==============================================================================
/// Вызов диалога настройки масштабирования графика
//==============================================================================
void SlotWidget::on_action_set_scale_triggered()
{
    SlotDialog dialog(this, 0, &settings);
    if (dialog.exec() != QDialog::Accepted) return;

    applay_settings();
    ui->action_view_plot2->activate(QAction::Trigger);
    save_settings();
}

//==============================================================================
/// Применение настроек масштабирования к графику и таблице
//==============================================================================
void SlotWidget::applay_settings()
{
    bool ok;
    double a, b, c, d;
    int ia, ib, ic;
    QString str;
    QRegExp rx1("^(.+)/(.+)$");
    QRegExp rx2("^(.+)/(.+)/(.+)$");
    QRegExp rx3("^(.+)/(.+)/(.+)/(.+)$");

    // настройка таблицы
    configure_table();

    //-------- настройка масштабированного графика ---------

    scale_mode_x = scale_mode_y = None;
    kx = ky = 1;
    ax = 0;
    plot2_unsigned = false;

    // название осей
    ui->plot2->setAxisTitle(QwtPlot::xBottom, settings["xname"]);
    ui->plot2->setAxisTitle(QwtPlot::yLeft, settings["yname"]);

    // масштаб - коэффициент
    if (settings.contains("xscale_coeff"))
    {
        a = settings["xscale_coeff"].toDouble(&ok);
        if (ok)
        {
            scale_mode_x = Coeff;
            kx = a;
        }
    }
    if (settings.contains("yscale_coeff"))
    {
        a = settings["yscale_coeff"].toDouble(&ok);
        if (ok)
        {
            scale_mode_y = Coeff;
            ky = a;
        }
    }

    // масштаб - интервал
    str = settings["xscale_interval"];
    if (rx1.indexIn(str) == 0)
    {
        a = rx1.cap(1).toDouble(&ok);
        if (!ok) goto lab1;
        b = rx1.cap(2).toDouble(&ok);
        if (!ok) goto lab1;
        scale_mode_x = Interval;
        ax = a;
        bx = b;
    }
lab1:;
    // масштаб - база модуля
    str = settings["xscale_base"];
    if (rx2.indexIn(str) == 0)
    {
        ia = rx2.cap(1).toInt(&ok);
        if (!ok) goto lab2;
        ib = rx2.cap(2).toInt(&ok);
        if (!ok) goto lab2;
        ic = rx2.cap(3).toInt(&ok);
        if (!ok) goto lab2;
        scale_mode_x = Base;
        scale_base_x.m = ia;
        scale_base_x.s = ib;
        scale_base_x.n = ic;
    }
lab2:;
    str = settings["yscale_base"];
    if (rx2.indexIn(str) == 0)
    {
        ia = rx2.cap(1).toInt(&ok);
        if (!ok) goto lab3;
        ib = rx2.cap(2).toInt(&ok);
        if (!ok) goto lab3;
        ic = rx2.cap(3).toInt(&ok);
        if (!ok) goto lab3;
        scale_mode_y = Base;
        scale_base_y.m = ia;
        scale_base_y.s = ib;
        scale_base_y.n = ic;
    }
lab3:;
    plot2_unsigned = (settings["yunsigned"] == "1");

    str = settings["isr_method"];
    eb_ref = 0.0;

    if (str == "0")
    {
        isr_mode = ISRPress;
        str = settings["isr_value"];
        eb_ref = str.toDouble(&ok);
        if (!ok) eb_ref = 0.0;
    } else if (str == "1")
    {
        isr_mode = ISRBase;
        str = settings["isr_value"];
        ia = 0;
        ib = 0;
        ic = 0;
        if (rx2.indexIn(str) == 0)
        {
            ia = rx2.cap(1).toInt(&ok);
            if (!ok) ia = 0;
            ib = rx2.cap(2).toInt(&ok);
            if (!ok) ib = 0;
            ic = rx2.cap(3).toInt(&ok);
            if (!ok) ic = 0;

            if (!ia || !ib || !ic)
            {
                ia = 0;
                ib = 0;
                ic = 0;
            }
        }

        isr_base.m = ia;
        isr_base.s = ib;
        isr_base.n = ic;
    } else
    {
        isr_mode = ISRPress;
        eb_ref = 0.0;
    }

    use_line_trans = (settings["use_line_trans"] == "1");

    ui->chb_line_trans_use->setChecked(use_line_trans);

    str = settings["line_trans_coords"];
    if (rx3.indexIn(str) == 0)
    {
        a = rx3.cap(1).toDouble(&ok);
        if (!ok) a = 0.;
        b = rx3.cap(2).toDouble(&ok);
        if (!ok) b = 0.;
        c = rx3.cap(3).toDouble(&ok);
        if (!ok) c = 0.;
        d = rx3.cap(4).toDouble(&ok);
        if (!ok) d = 0.;
        line_trans_x1 = a;
        line_trans_y1 = b;
        line_trans_x2 = c;
        line_trans_y2 = d;
    }

    if (settings.contains("pen_width"))
    {
        ia = settings["pen_width"].toInt(&ok);
        if (ok)
        {
            plot_curve->setPen(QPen(Qt::darkBlue, ia));
            plot2_curve->setPen(QPen(Qt::darkBlue, ia));
        }
    }
}

//==============================================================================
/// Настройка таблицы
//==============================================================================
void SlotWidget::configure_table()
{
    int i, j, k;
    char *cf;

    if (!mm) return;

    const MMSlot slot = mm->getSlot(module_n, slot_n);

    // формат ячеек
    cf = (char *)"";
    if (ui->action_format_dec->isChecked()) cf = (char *)"";
    if (ui->action_format_unsigned->isChecked()) cf = (char *)"unsigned";
    if (ui->action_format_hex->isChecked()) cf = (char *)"hex";
    if (ui->action_format_bin->isChecked()) cf = (char *)"bin";
    if (ui->action_format_float->isChecked()) cf = (char *)"float";

    // заполнение XML документа о таблице
    QDomDocument doc;
    QDomElement tag_table = doc.createElement("Table");
    tag_table.setAttribute("ColumnCount", 10);
    tag_table.setAttribute("RowCount", ((slot.len - 1) / 10) + 1);
    tag_table.setAttribute("Labels", "0;1;2;3;4;5;6;7;8;9");
    tag_table.setAttribute("LabelsSize", "70;70;70;70;70;70;70;70;70;70");
    doc.appendChild(tag_table);
    i = 0;
    while (1)
    {
        for (j = 0; j < 10; j++)
        {
            k = 10 * i + j;
            if (k >= slot.len) goto exit;
            QDomElement tag_item = doc.createElement("Item");
            tag_item.setAttribute("Row", QString::number(i));
            tag_item.setAttribute("Column", QString::number(j));
            tag_item.setAttribute(
                "Assign",
                QString("%1/%2/%3").arg(slot.module.n).arg(slot.n).arg(k + 1));
            tag_item.setAttribute("Format", cf);
            tag_table.appendChild(tag_item);
        }
        i++;
    }
exit:
    // настройка таблицы
    ui->table->setMode(MKTable::Edit);
    ui->table->setMBMaster(mm);
    ui->table->loadConfiguration(doc);
    for (i = 0; i < ui->table->rowCount(); i++)
    {
        ui->table->setVerticalHeaderItem(
            i, new QTableWidgetItem(QString::number(10 * i)));
    }
    ui->table->verticalHeader()->show();
    ui->table->setMode(MKTable::Polling);
}

//==============================================================================
/// Таймер для обновления графиков
//==============================================================================
void SlotWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if (!mm) return;
    if (pause_flag) return;

    //----------- подготовка данных --------------------

    int i, mask;
    bool b_unsigned = ui->action_format_unsigned->isChecked()
                   || ui->action_format_hex->isChecked()
                   || ui->action_format_bin->isChecked();


    MMSlot ss = mm->getSlot(module_n, slot_n);

    switch (ss.datatype.id())
    {
    case (MBDataType::Bytes):
        mask = 0xFF;
        break;
    case (MBDataType::Words):
        mask = 0xFFFF;
        break;
    default:
        mask = 0xFFFFFFFF;
        break;
    }

    if (b_unsigned
        && ((ss.datatype.id() == MBDataType::Floats)
            || (ss.datatype.id() == MBDataType::Doubles)))
        b_unsigned = false;

    int n = ss.data.size() + 1;
    plot_data_x.resize(n);
    plot_data_y.resize(n);
    for (i = 1; i < n; i++)
    {
        plot_data_y[i]
            = b_unsigned ? (double)((unsigned int)ss.data[i - 1].toInt() & mask)
                         : ss.data[i - 1].toDouble();
        plot_data_x[i] = i;
    }

    //------------  вывод графика ---------------

    plot_data_x[0] = 0;
    if (use_line_trans)
    {
        //------------  линейное преобразование графика ---------------
        for (i = 1; i < n; i++)
        {
            plot_data_y[i] = line_trans_y1
                           + ((line_trans_y2 - line_trans_y1)
                              / (line_trans_x2 - line_trans_x1))
                                 * (plot_data_y[i] - line_trans_x1);
        }
    }

    plot_data_y[0] = plot_data_y[1];
    plot_curve->setSamples(plot_data_x.constData(), plot_data_y.constData(), n);
    ui->plot->replot();
    if (ui->action_view_plot1->isChecked() && ui->action_stat_show->isChecked())
        calc_statistic();

    //------------ вывод масштабированного графика -------------

    for (i = 1; i < n; i++)
    {
        plot_data_y[i]
            = plot2_unsigned
                ? (double)((unsigned int)ss.data[i - 1].toInt() & mask)
                : ss.data[i - 1].toDouble();
        plot_data_x[i] = i;
    }

    // подготовка оси X
    switch (scale_mode_x)
    {
    case (None):
        kx = 1.0;
        break;
    case (Coeff):
        break;
    case (Interval):
        kx = (bx - ax) / (n - 1);
        break;
    case (Base):
        MMValue v = mm->getSlotValue(scale_base_x.m, scale_base_x.s,
                                     scale_base_x.n - 1);
        if (v.isValid())
        {
            kx = v.toDouble();
            if (kx == 0) kx = 1;
            ax = 0;
        }
        break;
    }
    // подготовка оси Y
    switch (scale_mode_y)
    {
    case (None):
        ky = 1.0;
        break;
    case (Coeff):
        break;
    case (Interval):
        break;
    case (Base):
        MMValue v = mm->getSlotValue(scale_base_y.m, scale_base_y.s,
                                     scale_base_y.n - 1);
        if (v.isValid())
        {
            ky = v.toDouble();
            if (ky == 0) ky = 1;
        }
        break;
    }

    // масштабирование и вывод
    for (i = 0; i < n; i++)
    {
        plot_data_x[i] *= kx;
        plot_data_x[i] += ax;
        plot_data_y[i] *= ky;
    }

    //------------  линейное преобразование масштабированного
    //графика-------------

    if (use_line_trans)
    {
        for (i = 1; i < n; i++)
        {
            plot_data_y[i] = line_trans_y1
                           + ((line_trans_y2 - line_trans_y1)
                              / (line_trans_x2 - line_trans_x1))
                                 * (plot_data_y[i] - line_trans_x1);
        }
    }

    plot_data_y[0] = plot_data_y[1];
    plot2_curve->setSamples(plot_data_x.constData(), plot_data_y.constData(),
                            n);
    ui->plot2->replot();
    if (ui->action_view_plot2->isChecked() && ui->action_stat_show->isChecked())
        calc_statistic();

    if (ui->text->isVisible())
    {
        QString str;
        for (int i = 0; i < ss.data.size(); ++i)
        {
            str += QChar(ss.data[i].toInt());
        }
        ui->text->setPlainText(str);
    }
}
//==============================================================================
// Расчет статистики
//==============================================================================
void SlotWidget::calc_statistic()
{
    y_mean = 0;
    y_std = 0;
    y_min = y_max = plot_data_y.value(0);

    foreach (double y, plot_data_y)
    {
        if (y > y_max) y_max = y;
        if (y < y_min) y_min = y;

        y_mean += y;
        y_std += y * y;
    }

    y_mean /= plot_data_y.size();
    y_std /= plot_data_y.size();
    y_std -= y_mean * y_mean;
    y_std = sqrt(y_std);

    ui->le_mean->setText(QString::number(y_mean));
    ui->le_diff->setText(QString::number(y_max - y_min));
    ui->le_std->setText(QString::number(y_std));

    if ((y_mean != 0) && (y_max != y_min))
    {
        if ((isr_mode == ISRBase) && (isr_base.m * isr_base.s * isr_base.n > 0))
        {
            MMValue v
                = mm->getSlotValue(isr_base.m, isr_base.s, isr_base.n - 1);
            if (v.isValid())
            {
                eb_ref = v.toDouble();
            } else
            {
                eb_ref = 0.0;
            }
        }

        if (eb_ref != 0.0)
        {
            ui->le_noise_percent->setText(
                QString::number(fabs((y_max - y_min) / eb_ref) * 100));
            ui->le_noise_db->setText(QString::number(
                (6.020599913 * (log(eb_ref / (y_max - y_min)) / log(2.0))
                 + 1.76),
                'f', 1));
            //-20*log10( fabs( (y_max-y_min) / y_mean ) ),'f' ,1 ) );old variant
            ui->le_eff_bits->setText(QString::number(
                log(eb_ref / (y_max - y_min)) / log(2.0), 'f', 1));
            //( -20*log10( fabs( (y_max-y_min) / y_mean ) )) / 6.0205999,'f' ,1
            //) ); старый расчет эфф. разрядов
        } else
        {
            ui->le_noise_percent->setText("---");
            ui->le_noise_db->setText("---");
            ui->le_eff_bits->setText("---");
        }
    } else
    {
        ui->le_noise_percent->clear();
        ui->le_noise_db->clear();
        ui->le_eff_bits->clear();
    }

    // расчет гистограммы
    double a;
    int i, j;
    a = (y_max - y_min) / (hist_plot_data_y_len - 1);
    memset(hist_plot_data_y, 0, sizeof(hist_plot_data_y));
    if (a)
    {
        for (i = 0; i < plot_data_y.size(); i++)
        {
            j = (int)((plot_data_y[i] - y_min) / a) + 1;
            if (j < 0) j = 0;
            if (j > (hist_plot_data_y_len - 1)) j = (hist_plot_data_y_len - 1);
            hist_plot_data_y[j] += 1;
        }
    }
    ui->hist->replot();
}
//==============================================================================
//
//==============================================================================
void SlotWidget::slot_line_trans_used(bool us)
{
    if (us)
    {
        QRegExp rx("^(.+)/(.+)/(.+)/(.+)$");
        QString str = settings["line_trans_coords"];
        if (rx.indexIn(str) != 0)
        {
            QMessageBox::warning(
                this, "Линейное преобразование",
                "Операция прервана: не указана одна или несколько координат "
                "линейного преобразования.");
            ui->chb_line_trans_use->setChecked(false);
            return;
        }
    }
    use_line_trans = us;
    settings.remove("use_line_trans");
    if (us) settings["use_line_trans"] = "1";
}
//==============================================================================
//
//==============================================================================
void SlotWidget::slot_statistic_show(bool isVisible)
{
    ui->statGroupBox->setVisible(isVisible);

    if (settings.contains("statistic_show"))
    {
        settings.remove("statistic_show");
    }

    if (isVisible)
    {
        settings["statistic_show"] = "1";
    } else
    {
        settings["statistic_show"] = "0";
    }

    save_settings();
}
//==============================================================================
//
//==============================================================================
void SlotWidget::updateMinimizeButtonState(bool state)
{
    minimize_flag = state;
    if (minimize_flag)
    {
        pb_minimize->setText("Развернуть");
        pb_minimize->setToolTip("Развернуть таблицу");
        pb_minimize->setIcon(QIcon(":/icons/res/arrow_down.png"));
    } else
    {
        pb_minimize->setText("Свернуть");
        pb_minimize->setToolTip("Свернуть таблицу");
        pb_minimize->setIcon(QIcon(":/icons/res/arrow_up.png"));
    }
}
//==============================================================================
//
//==============================================================================
void SlotWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if (windowState() == Qt::WindowMinimized) { emit signalOstsClose(); }
        if (windowState() == Qt::WindowNoState)
        {
            emit signalOstsOpen();
            if (minimize_flag) signalMinimize(true);
        }
    }
}
//==============================================================================
//
//==============================================================================
void SlotWidget::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    if (windowState() != Qt::WindowMinimized) emit signalOstsClose();
}
//###################################################################
/// Диалог о масштабировании графика
//###################################################################
SlotDialog::SlotDialog(SlotWidget *slotw, QWidget *parent,
                       QMap<QString, QString> *settings)
    : QDialog(parent)
    , ui(new Ui::SlotDialog)
{
    ui->setupUi(this);
    setWindowTitle("Масштабирование графика");

    slotwidget = slotw;

    this->settings = settings;

    ui->xscale_int_a->setValidator(new QDoubleValidator(this));
    ui->xscale_int_b->setValidator(new QDoubleValidator(this));
    ui->xscale_coeff_value->setValidator(new QDoubleValidator(this));
    ui->yscale_coeff_value->setValidator(new QDoubleValidator(this));

    ui->le_input_signal_range->setValidator(new QDoubleValidator(this));

    ui->le_x1->setValidator(new QDoubleValidator(this));
    ui->le_x2->setValidator(new QDoubleValidator(this));
    ui->le_y1->setValidator(new QDoubleValidator(this));
    ui->le_y2->setValidator(new QDoubleValidator(this));


    connect(ui->xscale_none, SIGNAL(toggled(bool)), this,
            SLOT(radio_buttons()));
    connect(ui->yscale_none, SIGNAL(toggled(bool)), this,
            SLOT(radio_buttons()));
    connect(ui->xscale_coeff, SIGNAL(toggled(bool)), this,
            SLOT(radio_buttons()));
    connect(ui->yscale_coeff, SIGNAL(toggled(bool)), this,
            SLOT(radio_buttons()));
    connect(ui->xscale_int, SIGNAL(toggled(bool)), this, SLOT(radio_buttons()));
    connect(ui->xscale_base, SIGNAL(toggled(bool)), this,
            SLOT(radio_buttons()));
    connect(ui->yscale_base, SIGNAL(toggled(bool)), this,
            SLOT(radio_buttons()));
    connect(ui->chb_line_trans, SIGNAL(toggled(bool)), this,
            SLOT(radio_buttons()));
    connect(ui->rb_isr_base, SIGNAL(toggled(bool)), this,
            SLOT(radio_buttons()));
    connect(ui->rb_isr_press, SIGNAL(toggled(bool)), this,
            SLOT(radio_buttons()));

    connect(ui->pb_line_trans_correct, SIGNAL(clicked()), this,
            SLOT(linear_transformation_correct()));

    ui->xscale_none->setChecked(true);
    ui->yscale_none->setChecked(true);

    // чтение настроек
    if (!settings) return;
    const QMap<QString, QString> &csettings = *settings;

    QRegExp rx1("^(.+)/(.+)$");
    QRegExp rx2("^(.+)/(.+)/(.+)$");
    QRegExp rx3("^(.+)/(.+)/(.+)/(.+)$");
    QString str;

    // названия
    ui->xname->setText(csettings["xname"]);
    ui->yname->setText(csettings["yname"]);

    // масштаб - коэффициент
    if (csettings.contains("xscale_coeff"))
    {
        ui->xscale_coeff->setChecked(true);
        ui->xscale_coeff_value->setText(csettings["xscale_coeff"]);
    }
    if (csettings.contains("yscale_coeff"))
    {
        ui->yscale_coeff->setChecked(true);
        ui->yscale_coeff_value->setText(csettings["yscale_coeff"]);
    }

    // масштаб - интервал
    str = csettings["xscale_interval"];
    if (rx1.indexIn(str) == 0)
    {
        ui->xscale_int->setChecked(true);
        ui->xscale_int_a->setText(rx1.cap(1));
        ui->xscale_int_b->setText(rx1.cap(2));
    }

    // масштаб - база
    str = csettings["xscale_base"];
    if (rx2.indexIn(str) == 0)
    {
        ui->xscale_base->setChecked(true);
        ui->xscale_base_m->setValue(rx2.cap(1).toInt());
        ui->xscale_base_s->setValue(rx2.cap(2).toInt());
        ui->xscale_base_n->setValue(rx2.cap(3).toInt());
    }
    str = csettings["yscale_base"];
    if (rx2.indexIn(str) == 0)
    {
        ui->yscale_base->setChecked(true);
        ui->yscale_base_m->setValue(rx2.cap(1).toInt());
        ui->yscale_base_s->setValue(rx2.cap(2).toInt());
        ui->yscale_base_n->setValue(rx2.cap(3).toInt());
    }
    ui->yunsigned->setChecked(csettings["yunsigned"] == "1");

    //диапазон входного сигнала
    str = csettings["isr_method"];
    if (str == "0")
    {
        ui->rb_isr_press->setChecked(true);
        ui->le_input_signal_range->setText(csettings["isr_value"]);
    } else if (str == "1")
    {
        ui->rb_isr_base->setChecked(true);
        str = csettings["isr_value"];
        if (rx2.indexIn(str) == 0)
        {
            ui->sb_isr_m->setValue(rx2.cap(1).toInt());
            ui->sb_isr_s->setValue(rx2.cap(2).toInt());
            ui->sb_isr_n->setValue(rx2.cap(3).toInt());
        }
    } else
    {
        ui->rb_isr_press->setChecked(true);
        ui->le_input_signal_range->setText("0.0");
    }
    //линейное преобразование
    ui->chb_line_trans->setChecked(csettings["use_line_trans"] == "1");

    str = csettings["line_trans_coords"];
    if (rx3.indexIn(str) == 0)
    {
        ui->le_x1->setText(rx3.cap(1));
        ui->le_y1->setText(rx3.cap(2));
        ui->le_x2->setText(rx3.cap(3));
        ui->le_y2->setText(rx3.cap(4));
    }
    if (csettings.contains("pen_width"))
    {
        ui->sb_pen_width->setValue(csettings["pen_width"].toInt());
    }
}

//==============================================================================
// Destructor
//==============================================================================
SlotDialog::~SlotDialog()
{
    delete ui;
}

//==============================================================================
// Обработка включения/отключения различных пунктов диалога
//==============================================================================
void SlotDialog::radio_buttons()
{
    ui->xscale_coeff_value->setEnabled(ui->xscale_coeff->isChecked());
    ui->xscale_int_a->setEnabled(ui->xscale_int->isChecked());
    ui->xscale_int_b->setEnabled(ui->xscale_int->isChecked());
    ui->xscale_base_m->setEnabled(ui->xscale_base->isChecked());
    ui->xscale_base_s->setEnabled(ui->xscale_base->isChecked());
    ui->xscale_base_n->setEnabled(ui->xscale_base->isChecked());
    ui->yscale_coeff_value->setEnabled(ui->yscale_coeff->isChecked());
    ui->yscale_base_m->setEnabled(ui->yscale_base->isChecked());
    ui->yscale_base_s->setEnabled(ui->yscale_base->isChecked());
    ui->yscale_base_n->setEnabled(ui->yscale_base->isChecked());
    ui->le_x1->setEnabled(ui->chb_line_trans->isChecked());
    ui->le_x2->setEnabled(ui->chb_line_trans->isChecked());
    ui->le_y1->setEnabled(ui->chb_line_trans->isChecked());
    ui->le_y2->setEnabled(ui->chb_line_trans->isChecked());
    ui->pb_line_trans_correct->setEnabled(ui->chb_line_trans->isChecked());
    ui->le_input_signal_range->setEnabled(ui->rb_isr_press->isChecked());
    ui->sb_isr_m->setEnabled(ui->rb_isr_base->isChecked());
    ui->sb_isr_s->setEnabled(ui->rb_isr_base->isChecked());
    ui->sb_isr_n->setEnabled(ui->rb_isr_base->isChecked());
}

//==============================================================================
// Нажатие кнопки OK
//==============================================================================
void SlotDialog::accept()
{
    if (!settings) return;

    if (ui->rb_isr_press->isChecked()
        && ui->le_input_signal_range->text().isEmpty())
    {
        QMessageBox::warning(
            this, "Диапазон входного сигнала",
            "Операция прервана: не указан диапазон входного сигнала.");
        return;
    }
    if (ui->chb_line_trans->isChecked())
    {
        QString str_x1, str_x2, str_y1, str_y2;
        str_x1 = ui->le_x1->text();
        str_x2 = ui->le_x2->text();
        str_y1 = ui->le_y1->text();
        str_y2 = ui->le_y2->text();
        if (str_x1.isEmpty() || str_x2.isEmpty() || str_y1.isEmpty()
            || str_y2.isEmpty())
        {
            QMessageBox::warning(
                this, "Линейное преобразование",
                "Операция прервана: не указана одна или несколько координат "
                "линейного преобразования.");
            return;
        }
        if (str_x1 == str_x2 || str_y1 == str_y2)
        {
            QMessageBox::warning(
                this, "Линейное преобразование",
                "Операция прервана: обнаружено равенство координат одной из "
                "осей линейного преобразования.");
            return;
        }
    }

    QMap<QString, QString> &isettings = *settings;

    QString str;

    isettings.remove("xname");
    isettings.remove("yname");
    isettings.remove("xscale_coeff");
    isettings.remove("yscale_coeff");
    isettings.remove("xscale_interval");
    isettings.remove("xscale_base");
    isettings.remove("yscale_base");
    isettings.remove("yunsigned");
    isettings.remove("isr_method");
    isettings.remove("isr_value");
    isettings.remove("use_line_trans");
    isettings.remove("line_trans_coords");
    isettings.remove("pen_width");


    // названия осей
    str = ui->xname->text();
    if (!str.isEmpty()) isettings["xname"] = str;
    str = ui->yname->text();
    if (!str.isEmpty()) isettings["yname"] = str;

    // масштаб - коэффициент
    if (ui->xscale_coeff->isChecked())
    {
        isettings["xscale_coeff"] = ui->xscale_coeff_value->text();
    }
    if (ui->yscale_coeff->isChecked())
    {
        isettings["yscale_coeff"] = ui->yscale_coeff_value->text();
    }
    // масштаб - интервал
    if (ui->xscale_int->isChecked())
    {
        isettings["xscale_interval"]
            = ui->xscale_int_a->text() + "/" + ui->xscale_int_b->text();
    }
    // масштаб - база
    if (ui->xscale_base->isChecked())
    {
        isettings["xscale_base"] = QString::asprintf(
            "%d/%d/%d", ui->xscale_base_m->value(), ui->xscale_base_s->value(),
            ui->xscale_base_n->value());
    }
    if (ui->yscale_base->isChecked())
    {
        isettings["yscale_base"] = QString::asprintf(
            "%d/%d/%d", ui->yscale_base_m->value(), ui->yscale_base_s->value(),
            ui->yscale_base_n->value());
    }
    // беззнаковые числа
    if (ui->yunsigned->isChecked()) { isettings["yunsigned"] = "1"; }

    if (ui->rb_isr_press->isChecked())
    {
        isettings["isr_method"] = "0";
        isettings["isr_value"] = ui->le_input_signal_range->text();
    } else
    {
        isettings["isr_method"] = "1";
        isettings["isr_value"]
            = QString::asprintf("%d/%d/%d", ui->sb_isr_m->value(),
                                ui->sb_isr_s->value(), ui->sb_isr_n->value());
    }

    if (ui->chb_line_trans->isChecked()) { isettings["use_line_trans"] = "1"; }

    isettings["line_trans_coords"] = ui->le_x1->text() + "/" + ui->le_y1->text()
                                   + "/" + ui->le_x2->text() + "/"
                                   + ui->le_y2->text();

    isettings["pen_width"] = QString::number(ui->sb_pen_width->value());

    done(QDialog::Accepted);
}
//==============================================================================
// Нажатие кнопки Скорректировать ноль
//==============================================================================
void SlotDialog::linear_transformation_correct()
{
    ui->le_x1->setText(slotwidget->get_mean());
    ui->le_x2->setText(
        QString::number((ui->le_x1->text().toDouble() + 1.), 'f', 8));
}
