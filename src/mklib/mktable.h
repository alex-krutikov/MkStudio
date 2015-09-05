#ifndef __TABLE_H__
#define __TABLE_H__

#include <QTableWidget>
#include <QTimer>
#include <QItemDelegate>
#include <QWidget>
#include <QModelIndex>
#include <QListWidget>
#include <QVector>
#include <QDomDocument>

#include "mk_global.h"
#include "shortcut.h"

class MBMasterXML;
class QMouseEvent;
class MBDataType;
class MMValue;
class MKTableItemDelegate;

//---------------------------------------------------------------------------
//! Настроечная таблица
//---------------------------------------------------------------------------
class MK_EXPORT MKTable : public QTableWidget
{
  Q_OBJECT
  // ???? Q_ENUMS(ShortCut::SCut)
  friend class MKTableItemDelegate;
public:
  enum Mode { Edit = 0, Polling };
  enum { IndexRole          = Qt::UserRole,
         FormatRole         = Qt::UserRole+1,
         AssignRole         = Qt::UserRole+2,
         SSSelectorRole     = Qt::UserRole+3,
         SSConfirmEditRole  = Qt::UserRole+4,
         RecorderParamsRole = Qt::UserRole+5};
  MKTable( QWidget *parent = 0 );
  ~MKTable();
  bool loadConfiguration( const QDomDocument &doc );
  void saveConfiguration( QDomDocument &doc );
  QStringList getHorizontalHeaderLabels();

  void setMBMaster( MBMasterXML *mm ) { mbmaster = mm; }
  void setMode( enum Mode m );
  void setSettingsSheet( const QString &ss);
  QString settingsSheet();
  
  void setOptimizeReadMode( bool optimizeRead );
  bool optimizeReadMode();

  QSize sizeHint() const;
  bool getCurrentItemConfirmEdit(){return currentItemConfirmEdit;}
  void closeAllRecorders();
  QList<QDomElement> shortcutList;
  QHash<int, ShortCut*> hotkey;

private:
  void mouseMoveEvent( QMouseEvent *event );
  void mousePressEvent( QMouseEvent *event );
  void mouseReleaseEvent( QMouseEvent *event );
  void mouseDoubleClickEvent( QMouseEvent *event );
  void keyPressEvent( QKeyEvent *event );
  void contextMenuEvent( QContextMenuEvent *e );
  void ss_process();
  static QString format_output( const MMValue &value,
                         const MBDataType &datatype, int type );
  enum Mode mode;
  QTimer timer;
  MBMasterXML *mbmaster;
  MKTableItemDelegate *delegate;
  bool currentItemConfirmEdit;

  /*struct MKTable_SS_Enum
  { QMap<int,QString> map_display;
    QMap<int,QString> map_menu;
  };*/
  struct MapEmul
  {
    int key;
    QString value;
  };

  struct MKTable_SS_Enum
  { QVector<MapEmul> map_display;
    QVector<MapEmul> map_menu;
  };
  struct MKTableAssignData;

  QVector<MKTableAssignData> assign_data;
  QVector<MKTable_SS_Enum>   ss_enum;
  QMap<QString,int> ss_enum_name2index;

  QString settingssheet_str;
  bool optimize_read_mode;
  quint32 plots_count;
private slots:
  void refresh();
  void slotMinimize(bool);
  void slotPlotClose();
  void slotPlotOpen();
  void loadShortCuts(const QDomDocument &);
signals:
  void item_changed_by_editor();
  void signalCloseAllRecorders();
  void signalMinimizeStateChange( bool state );
  void signalMinimizeAllHide( bool state );
};

/*! \page xml_table_desc Формат XML документа таблицы

  Документ заключается в тег \b Table.

  \b Table содержит следующие аттрибуты:
  \li \b ColumnCount
  \li \b RowCount
  \li \b Labels
  \li \b LabelsSize

  Внутри тега \b Table расположены теги \b Item с описанием ячейки таблицы.

  \b Item содержит следующие аттрибуты:
  \li \b Column
  \li \b Row
  \li \b Text
  \li \b Font
  \li \b Assign

  Аттрибут \b Font может задаваться в виде разделенных ";" значений:
  \li \b bold         полужирный шрифт
  \li \b underline    подчеркивание
  \li \b align_left   выравнивание к левому краю
  \li \b align_center выравнивание по центру
  \li \b align_right  выравнивание правому краю

  Аттрибут \b Assign задает привзку данной ячейки к опросу. Он задается в виде:

  \b \c "[номер модуля]/[номер слота]/[номер сигнала]".

  Если атрибут не задан или не соответствует формату, считается, что ячейка содержит
  текст и не отностится к опросу.

  Пример:

\verbatim
\endverbatim

*/

#endif
