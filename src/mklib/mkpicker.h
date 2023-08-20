#ifndef MKPICKER_H
#define MKPICKER_H

#include <qwt_plot_canvas.h>
#include <qwt_plot_picker.h>

#include <QTimer>

class Plot;

class MKPicker : public QwtPlotPicker
{
    Q_OBJECT

public:
    explicit MKPicker(int xAxis, int yAxis, int selectionFlags,
                      RubberBand rubberBand, DisplayMode trackerMode,
                      QWidget *parent)
        : QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, parent)
    {
    }

    QwtText trackerText(const QPoint &) const;
    QwtText trackerText(const QPointF &) const;
    void drawRubberBand(QPainter *) const;
    void setPlot(Plot *plot) { plt = plot; }
    void refresh() { begin(); }
    void setTrakerDataVisible(bool show) { trackerDataVisible = show; }
    double xAxisCoordFromPlot(QPoint pos)
    {
        return (double)invTransform(pos).x();
    }

private:
    QString window;
    Plot *plt;
    bool atCanvas;
    bool trackerDataVisible;
public slots:
    void setWindow(QString window);
private slots:
    QString createLabel(qreal) const;

protected:
    void widgetMouseMoveEvent(QMouseEvent *);
    void timerEvent(QTimerEvent *);
};

#endif // MKPICKER_H
