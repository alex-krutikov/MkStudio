#ifndef ABSTRACTSCALEDRAW_H
#define ABSTRACTSCALEDRAW_H

#include "qwt_scale_draw.h"
#include <qlocale.h>


class ScaleDraw : public QwtScaleDraw
{
public:

    // Форматирование меток шкалы
    QwtText label(double value) const
    {
        return QLocale::system().toString(value, 'g', 7);
    }
};

#endif // ABSTRACTSCALEDRAW_H
