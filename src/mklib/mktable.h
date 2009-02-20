#ifndef __TABLE_H__
#define __TABLE_H__

#include <QTableWidget>
#include <QTimer>
#include <QItemDelegate>
#include <QWidget>
#include <QModelIndex>
#include <QListWidget>
#include <QVector>

class MBMasterXML;
class QMouseEvent;
class QDomDocument;
class MBDataType;
class MMValue;
class MKTableItemDelegate;

//---------------------------------------------------------------------------
//! ����������� �������
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

  void setMBMaster( MBMasterXML *mm ) { mbmaster = mm; }
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
  MBMasterXML *mbmaster;
  MKTableItemDelegate *delegate;

  struct MKTable_SS_Enum
  { QMap<int,QString> map_display;
    QMap<int,QString> map_menu;
  };

  struct MKTableAssignData;

  QVector<MKTableAssignData> assign_data;
  QVector<MKTable_SS_Enum>   ss_enum;
  QMap<QString,int> ss_enum_name2index;

  QString settingssheet_str;
private slots:
  void refresh();
signals:
  void item_changed_by_editor();
};

/*! \page xml_table_desc ������ XML ��������� �������

  �������� ����������� � ��� \b Table.

  \b Table �������� ��������� ���������:
  \li \b ColumnCount
  \li \b RowCount
  \li \b Labels
  \li \b LabelsSize

  ������ ���� \b Table ����������� ���� \b Item � ��������� ������ �������.

  \b Item �������� ��������� ���������:
  \li \b Column
  \li \b Row
  \li \b Text
  \li \b Font
  \li \b Assign

  �������� \b Font ����� ���������� � ���� ����������� ";" ��������:
  \li \b bold         ���������� �����
  \li \b underline    �������������
  \li \b align_left   ������������ � ������ ����
  \li \b align_center ������������ �� ������
  \li \b align_right  ������������ ������� ����

  �������� \b Assign ������ ������� ������ ������ � ������. �� �������� � ����:

  \b \c "[����� ������]/[����� �����]/[����� �������]".

  ���� ������� �� ����� ��� �� ������������� �������, ���������, ��� ������ ��������
  ����� � �� ���������� � ������.

  ������:

\verbatim
\endverbatim

*/

#endif
