#ifndef __SELFRECORDER__H_
#define __SELFRECORDER__H_

#include <QWidget>

class QPen;
class QString;
class QColor;
class QwtPlot;

//==============================================================
//! ���������
//==============================================================
class MKSelfRecorder : public QWidget
{
  Q_OBJECT
public:
  MKSelfRecorder( QWidget *parent = 0 );

//! �������� ������ �� ������
  void addCurve( int curveId, const QPen &pen,
                      const QString &description );

//! �������� ������
  void updateData( int curveId, double data );

//! ���������� ���� ����
  void setBackgroundColor( const QColor &color  );

private:
  QwtPlot *plot;

};

#endif
