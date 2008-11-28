#ifndef __TABLE_H__
#define __TABLE_H__

#include <QTableWidget>
#include <QTimer>
#include <QItemDelegate>
#include <QWidget>
#include <QModelIndex>
#include <QListWidget>

#include "mbtypes.h"

class MBMaster;
class QMouseEvent;
class QDomDocument;

class MKTableItemDelegate;
class MBDataType;

class QToolButton;

//---------------------------------------------------------------------------
//! Настроечная таблица
//---------------------------------------------------------------------------
class MKTable : public QTableWidget
{
  Q_OBJECT
  friend class MKTableItemDelegate;
public:
  enum Mode { Edit = 0, Polling };
  enum { IndexRole      = Qt::UserRole,
         FormatRole     = Qt::UserRole+1,
         AssignRole     = Qt::UserRole+2,
         SSSelectorRole = Qt::UserRole+3 };
  MKTable( QWidget *parent = 0 );
  ~MKTable();
  bool loadConfiguration( const QDomDocument &doc );
  void saveConfiguration( QDomDocument &doc );
  QStringList getHorizontalHeaderLabels();

  void setMBMaster( MBMaster *mm ) { mbmaster = mm; }
  void setMode( enum Mode m );
  void setSettingsSheet( const QString &ss);
  QString settingsSheet();

  QSize sizeHint() const;
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
  MBMaster *mbmaster;
  MKTableItemDelegate *delegate;

  struct MKTable_SS_Enum
  { QMap<int,QString> map_display;
    QMap<int,QString> map_menu;
  };

  // служебная структура - привязка ячеек
  struct MKTableAssignData
  { int m_index,s_index,i_index; // модуль,слот,индекс
    int row,column;              // ячейка
    MMValue current_value;       // текущее значение
    int status;                  // достоверность значения
    int format;                  // формат выдачи
    int ss_enum_index;           // индекс в векторе enum
    static bool lessThan(const MKTableAssignData &s1, const MKTableAssignData &s2);
  };

  QVector<MKTableAssignData> assign_data;
  QVector<MKTable_SS_Enum>   ss_enum;
  QMap<QString,int> ss_enum_name2index;

  QString settingssheet_str;
private slots:
  void refresh();
signals:
  void item_changed_by_editor();
};

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class MKTableItemDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  MKTableItemDelegate( MKTable *parent = 0 );
  void paint ( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
  void setModelData ( QWidget * editor, QAbstractItemModel * model, const QModelIndex &index ) const;
  QWidget *createEditor ( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
  void setEditorData( QWidget *editor, const QModelIndex &index ) const;
  void updateEditorGeometry ( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
private slots:
  void commitAndCloseEditor();
private:
  bool eventFilter(QObject *obj, QEvent *ev);
  MKTable *table;
};

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class MKTableItemCBWidget : public QListWidget
{
  Q_OBJECT
public:
  MKTableItemCBWidget( QWidget *parent = 0 );
  void highlightcursor();
signals:
  void editingFinished();
protected:
  void mousePressEvent( QMouseEvent * event );
  void mouseMoveEvent ( QMouseEvent * event );
  void leaveEvent ( QEvent * event );

   QStyleOptionViewItem viewOptions() const
   { QStyleOptionViewItem option = QListView::viewOptions();
     option.showDecorationSelected = true;
     return option;
   }
private:
  int row_under_mouse;
};

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
#include "ui/ui_plot.h"

class Plot : public QWidget,
             public Ui::Plot
{
  Q_OBJECT
public:
  Plot( QWidget *parent=0, MBMaster *mbmaster=0, const QString &config = QString() );
  void timerEvent ( QTimerEvent * event );
  QSize sizeHint () const;
  double noise() const;
private slots:
  void pb_clear_clicked();
  void pb_pause_clicked();
  void pb_export_clicked();
  void on_cb_speed_currentIndexChanged( int );
private:
  QToolButton *pb_pause;
  QToolButton *pb_export;
  static const int y_data_len = 200;
  double y_data[ y_data_len ];
  double x_data[ y_data_len ];
  double y_max;
  double y_min;
  double y_mean;
  double y_std;
  static const int y_hist_data_len = 17;
  double y_hist_data[ y_hist_data_len ];
  double x_hist_data[ y_hist_data_len ];
  int module_index;
  int slot_index;
  int value_index;
  bool firstrun_flag;
  bool pause_flag;
  int points_counter;
  double avr_a;
  int avr_counter;
  MBMaster *mbmaster;
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
