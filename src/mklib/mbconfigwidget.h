#ifndef __MBCONFIGWIDGET_H__
#define __MBCONFIGWIDGET_H__

#include <QWidget>
#include <QAbstractTableModel>
#include <QString>
#include <QByteArray>
#include <QtXml>

#include "mbcommon.h"

#define MODULES_MAX_N   50
#define SLOTS_MAX_N    100

namespace Ui { class MBConfigWidget; }

class MBConfigWidgetPrivate;

//===================================================================
//! Виджет для настройки конфигурации опроса модулей
//===================================================================
class MBConfigWidget : public QWidget
{
  Q_OBJECT
public:
  MBConfigWidget( QWidget *parent = 0 );
  ~MBConfigWidget();
//! Очистить конфигурацию
  void clearConfiguration();

//! Загрузить конфигурацию
/*!
   \param xml массив данных с конфигурацией (текст xml-описания)
*/
  void loadConfiguration( QByteArray &xml );

//! Загрузить конфигурацию
/*!
   \param doc XML документ с описанием конфигурации
*/
  void loadConfiguration( QDomDocument &doc );

//! Получить текущую конфигурацию
/*!
   \param xml массив данных с конфигурацией (текст xml-описания)
*/
  void saveConfiguration( QByteArray &xml );

//! Получить текущую конфигурацию
/*!
   \param doc XML документ с описанием конфигурации
*/
  void saveConfiguration( QDomDocument &doc );

//! Экспортировать текущую конфигурацию в текстовый файл
/*!
   \return Текстовое описание конфигурации
*/
  QString exportConfiguration();

//! Задать аттрибуты слота
/*!
   \param module     Номер модуля
   \param slot       Номер слота
   \param attributes Аттрибуты
*/
  void setSlotAttributes(int module, int slot, const QString &attributes);

private:
  Ui::MBConfigWidget    *ui;
  MBConfigWidgetPrivate *d;
private slots:
  void on_tw1_customContextMenuRequested ( const QPoint& );
  void on_tw2_activated( const QModelIndex& );
  void tw1_currentRowChanged(const QModelIndex&current,
                             const QModelIndex&previous);
};


#endif
