#include "mkpicker.h"

#include "plot.h"

#include <qwt_painter.h>
#include <qwt_plot.h>

#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPointer>
#include <QTextDocumentFragment>

#include <cmath>
//##############################################################################
//
//##############################################################################

QwtText MKPicker::trackerText(const QPoint &pos) const
{
    return trackerText(invTransform(pos));
}
//==============================================================================
//тескст над курсором
//==============================================================================
QwtText MKPicker::trackerText(const QwtDoublePoint &pos) const
{
    if (!atCanvas || pos.x() < 0.0 || !trackerDataVisible) return QwtText("");
    QString text;
    int plot_num = plt->plotsCount();
    qreal y1 = 0.0, y2 = 0.0, y3 = 0.0, y4 = 0.0;
    y1 = plt->getYcoord(pos.x(), 0);
    if (plot_num > 1) y2 = plt->getYcoord(pos.x(), 1);
    if (plot_num > 2) y3 = plt->getYcoord(pos.x(), 2);
    if (plot_num > 3) y4 = plt->getYcoord(pos.x(), 3);

    switch (rubberBand())
    {
    case HLineRubberBand:
        text.sprintf("%.7g", pos.y());
        break;
    case VLineRubberBand:
        text = createLabel(pos.x());
        break;
    default: {
        text = createLabel(pos.x()) + "\n" + QString::number(y1, 'g', 7);
        if (plot_num > 1) text += "\n" + QString::number(y2, 'g', 7);
        if (plot_num > 2) text += "\n" + QString::number(y3, 'g', 7);
        if (plot_num > 3) text += "\n" + QString::number(y4, 'g', 7);
    }
    }
    return QwtText(text);
}
//==============================================================================
//отрисовка перекрестия
//==============================================================================
void MKPicker::drawRubberBand(QPainter *painter) const
{
    const QRect &pRect = pickRect();
    const QPoint pos = trackerPosition();
    QPen pen;
    pen.setColor(Qt::red);
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);
    QwtPainter::drawLine(painter, pos.x(), pRect.top(), pos.x(),
                         pRect.bottom());
    QwtPainter::drawLine(painter, pRect.left(), pos.y(), pRect.right(),
                         pos.y());
}
//==============================================================================
//
//==============================================================================
void MKPicker::setWindow(QString window)
{
    this->window = window;
}
//==============================================================================
//
//==============================================================================
QString MKPicker::createLabel(qreal x) const
{
    if (x < 0.0) return "---";

    double min, sec, tick;
    QString tick_str, sec_str;

    if (window == "sec")
    {
        tick = modf(x, &sec) * 1000;

        tick_str = QString::number((int)(tick));

        if (tick < 10.0)
        {
            tick_str = "00";
        } else if (tick < 100.0)
        {
            tick_str = "0" + tick_str;
        }

        tick_str.resize(2);

        return QString::number((int)sec) + "." + tick_str;
    } else if (window == "min")
    {
        tick = modf(modf(x, &min) * 60, &sec) * 1000;

        tick_str = QString::number((int)(tick));

        if (tick < 10.0)
        {
            tick_str = "00";
        } else if (tick < 100.0)
        {
            tick_str = "0" + tick_str;
        }

        tick_str.resize(2);

        sec_str = QString::number((int)(sec));

        if (sec_str.length() == 1) sec_str = "0" + sec_str;

        return QString::number((int)(min)) + ":" + sec_str + "." + tick_str;
    } else
        return "0:0.0";
}
//==============================================================================
//
//==============================================================================
void MKPicker::widgetMouseMoveEvent(QMouseEvent *event)
{
    QwtPicker::widgetMouseMoveEvent(event);
    if ((event->buttons() == Qt::LeftButton)
        && (event->modifiers() & Qt::ControlModifier))
        plt->moveCanvas();
    if (atCanvas) refresh();
}
//==============================================================================
//
//==============================================================================
void MKPicker::timerEvent(QTimerEvent *timer)
{
    Q_UNUSED(timer);
    if (canvas()->underMouse())
    {
        atCanvas = true;
    } else
    {
        atCanvas = false;
    }
}
