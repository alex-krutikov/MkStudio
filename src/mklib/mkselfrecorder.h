#ifndef __SELFRECORDER__H_
#define __SELFRECORDER__H_

#include <QWidget>

class QPen;
class QString;
class QColor;
class QwtPlot;

//==============================================================
//! Самописец
//==============================================================
class MKSelfRecorder : public QWidget
{
  Q_OBJECT
public:
  MKSelfRecorder( QWidget *parent = 0 );

//! Добавить кривую на график
  void addCurve( int curveId, const QPen &pen,
                      const QString &description );

//! Обновить данные
  void updateData( int curveId, double data );

//! Установить цвет фона
  void setBackgroundColor( const QColor &color  );

private:
  QwtPlot *plot;

};

#endif
