#ifndef __MBCONFIGWIDGET_H__
#define __MBCONFIGWIDGET_H__

#include <QWidget>
#include <QString>

#include "mk_global.h"

namespace Ui { class MBConfigWidget; }
class MBConfigWidgetPrivate;
class QByteArray;
class QDomDocument;
class QPoint;
class QModelIndex;

//===================================================================
//! Виджет для настройки конфигурации опроса модулей
//===================================================================
class MK_EXPORT MBConfigWidget : public QWidget
{
  Q_OBJECT
  friend class MBConfigWidgetItemDelegate;
public:
  MBConfigWidget( QWidget *parent = 0 );
  ~MBConfigWidget();

  void clearConfiguration();
  void loadConfiguration( const QByteArray &xml );
  void loadConfiguration( const QDomDocument &doc );
  void saveConfiguration( QByteArray &xml );
  void saveConfiguration( QDomDocument &doc );
  QString exportConfiguration();
  void setSlotAttributes(int module, int slot, const QString &attributes);
  void setArduinoOnly(bool isArduino);

private:
  Ui::MBConfigWidget    *ui;
  MBConfigWidgetPrivate *d;
private slots:
  void on_tw1_customContextMenuRequested ( const QPoint& );  
  void tw1_currentRowChanged(const QModelIndex&current,
                             const QModelIndex&previous);
};

#endif
