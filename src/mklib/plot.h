#ifndef __PLOT_H__
#define __PLOT_H__

#include "mbmasterxml.h"
#include "mk_global.h"
#include "mkpicker.h"

#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>

#include <QDialog>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QVector>
#include <QWidget>

#include <memory>

class QTimerEvent;
class QTableWidgetItem;
class QToolButton;
class QFile;
class QTextStream;
class QwtPlotZoomer;
class QwtPlotPicker;
class MKPicker;
class PlotRawSeriesData;

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------

namespace Ui {
class Plot;
class PlotISRDialog;
} // namespace Ui

class MK_EXPORT Plot : public QWidget
{
    Q_OBJECT
public:
    Plot(QString title, QList<QTableWidgetItem *> mktableItemList,
         bool min_flag, QWidget *parent = 0,
         MBMasterXMLPtr mbmaster = MBMasterXMLPtr());
    virtual ~Plot();
    QSize sizeHint() const;
    double noise() const;
    void setPlotList(QString list);
    int plotsCount() { return plots_count; }
    qreal getYcoord(qreal xcoord, int y_data_index);
    void moveCanvas();
private slots:
    void pb_clear_clicked();
    void pb_pause_clicked();
    void pb_export_clicked();
    void pb_minimize_clicked();
    void pb_crosshair_toggled(bool);
    void pb_grid_toggled(bool);
    void pb_picker_data_toggled(bool);
    void tb_isr_set_clicked();
    void updateMinimizeButtonState(bool state);
    void on_cb_speed_currentIndexChanged(int);
    void on_sb_tact_valueChanged(int);
    void on_sb_plot_pen_w_valueChanged(int);
    void change_current_plot(int);
    void on_le_input_signal_range_textEdited(QString signal);
    void params_change();
    void calc_statistic();
    bool load_recorder_params();
    bool save_recorder_params();
    void set_file_for_write();
    void file_write_start_stop();
    void setWindow(int);
    void updateISRparams();

private:
    Ui::Plot *ui;
    QToolButton *pb_clear;
    QToolButton *pb_pause;
    QToolButton *pb_export;
    QToolButton *pb_minimize;
    QToolButton *pb_grid;
    QToolButton *pb_crosshair;
    QToolButton *pb_picker_data;

    std::unique_ptr<QwtPlotCurve> plot_data1;
    std::unique_ptr<QwtPlotCurve> plot_data2;
    std::unique_ptr<QwtPlotCurve> plot_data3;
    std::unique_ptr<QwtPlotCurve> plot_data4;
    std::unique_ptr<QwtPlotCurve> hist_data;

    int y_data_len;
    int plots_count;

    std::unique_ptr<double[]> a_value;
    std::unique_ptr<double[]> x_data;
    std::unique_ptr<double[]> y_data1;
    std::unique_ptr<double[]> y_data2;
    std::unique_ptr<double[]> y_data3;
    std::unique_ptr<double[]> y_data4;

    PlotRawSeriesData *plot_series_data1{nullptr};
    PlotRawSeriesData *plot_series_data2{nullptr};
    PlotRawSeriesData *plot_series_data3{nullptr};
    PlotRawSeriesData *plot_series_data4{nullptr};

    double *y_data_stat;

    double y_max;
    double y_min;
    double y_mean;
    double y_std;
    static const int y_hist_data_len = 17;
    double y_hist_data[y_hist_data_len];
    double x_hist_data[y_hist_data_len];

    QVector<int> module_index;
    QVector<int> slot_index;
    QVector<int> value_index;

    bool firstrun_flag;
    bool pause_flag;
    bool minimize_flag;
    bool hide_minimize_flag;
    bool file_write_flag;
    bool isr_use_base_flag;
    bool start_flag;
    int points_counter;

    std::unique_ptr<double[]> avr_a;

    int avr_counter;
    MBMasterXMLPtr mbmaster;
    double y_min1, y_min2, y_min3, y_min4;
    double y_max1, y_max2, y_max3, y_max4;
    double eb_ref;
    struct
    {
        int m, s, n;
    } isr_base;
    QList<QTableWidgetItem *> mktableItemList;
    QStringList isr_params; // isr - Input Signal Range
    QFile file;
    QTextStream file_stream;
    int file_write_counter;
    QString titleStr;
    int timerId, timerInterval;
    quint64 time_append;
    QwtPlotZoomer *zoomer;
    MKPicker *picker;
    QwtPlotGrid *grid;
signals:
    void signalMinimize(bool is_minimize);
    void signalPlotClose();
    void signalPlotOpen();

protected:
    void timerEvent(QTimerEvent *event);
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *event);
};
//==============================================================================
// Диалог настройки диапазона входного сигнала
//==============================================================================
class MK_EXPORT PlotISRDialog : public QDialog // ISR - Input Signal Range
{
    Q_OBJECT
public:
    PlotISRDialog(QStringList *isr_list, unsigned int isr_index,
                  QWidget *parent = 0);
    virtual ~PlotISRDialog();
private slots:
    void accept();
    void config_widgets();

private:
    Ui::PlotISRDialog *ui;
    QStringList *isr_param_list;
    unsigned int isr_param_index;
};

#endif
