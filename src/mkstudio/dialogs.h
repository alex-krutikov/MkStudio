#ifndef __DIALOGS__H_
#define __DIALOGS__H_

#include "abstractserialport.h"
#include "mbmasterxml.h"

class QwtPlotCurve;

#include "ui_initdialog.h"
//==============================================================================
/// Окно выбора порта и скорости
//==============================================================================
class InitDialog : public QDialog,
                   public Ui::InitDialog
{
  Q_OBJECT
public:
  InitDialog( QWidget *parent = 0);
  void accept();
  AbstractSerialPortPtr port() const {return m_port;}
private slots:
  void on_cb_portname_currentIndexChanged(int);
private:
  AbstractSerialPortPtr  m_port;
};

#include "ui_assigndialog.h"
//==============================================================================
/// Диалог "Привязка параметров"
//==============================================================================
class AssignDialog : public QDialog,
                     public Ui::AssignDialog
{
  Q_OBJECT
public:
  AssignDialog( QWidget *parent = 0,
                const QString &assign = QString(),
                const QString &format = QString(),
                const QString &ss     = QString(),
                const QStringList &sl = QStringList(),
                const bool &ss_confirm = bool() );
  QString assign;
  QString format;
  QString ss;
  bool    ss_confirm;
private:
  void accept();
};

#include "ui_settingsdialog.h"
//==============================================================================
/// Диалог "Настройки"
//==============================================================================
class SettingsDialog : public QDialog,
                       public Ui::SettingsDialog
{
  Q_OBJECT
public:
  SettingsDialog( QWidget *parent = 0 );
};

#include "ui_texteditdialog.h"
//==============================================================================
/// Диалог "Редактирование текста"
//==============================================================================
class TextEditDialog : public QDialog,
                       public Ui::TextEditDialog
{
  Q_OBJECT
public:
  TextEditDialog( QWidget *parent = 0 );
};

#include "ui_unitedslots.h"
//==============================================================================
/// Диалог "Редактирование текста"
//==============================================================================
class UnitedSlots : public QWidget,
                    public Ui::UnitedSlots
{
  Q_OBJECT
public:
  UnitedSlots( QWidget *parent = 0, MBMasterXMLPtr mm = MBMasterXMLPtr());
  void timerEvent ( QTimerEvent * event );
private:
  static const int curves_num = 10;
  MBMasterXMLPtr mm;
  QwtPlotCurve *curves[curves_num];
};

//==============================================================================
/// Диалог настройки горячих клавиш
//==============================================================================
#include "ui_shortcuts.h"
#include "shortcut.h"
#include <QDomDocument>

class ShortCutsDialog : public QDialog, public Ui::ShortCutsDialog
{
    Q_OBJECT
public:
    ShortCutsDialog(QWidget *parent = 0, QString config_file = "");
    void keyPressEvent(QKeyEvent *);
    QList<QDomElement> shortcutList;
    QHash<int, ShortCut*> hotkey;

private slots:
    void tableItemChanged(QTableWidgetItem *);
    void refreshTable(QTableWidgetItem *);
    void fitTableRowHeight();
    void separatorChanged(int);
    void fillDomEelement(QDomElement *, QString, QString);
    void updateTableItem(QTableWidgetItem *);
    void accept();

private:
    QTableWidget *tw;
    QTableWidgetItem *blankItem;
    enum{keyCodeRole = Qt::UserRole, keyNameRole = Qt::UserRole+1,
         assignRole = Qt::UserRole+2, actionRole = Qt::UserRole+3,
         blankRole = Qt::UserRole+4, separatorRole = Qt::UserRole+5};
    enum{NameCol = 0, KeyCol = 1, AssignCol = 2, SeparatorCol = 3};
    enum{DataCol = 0};
};

//==============================================================================
/// Делегат для редактирования ячейки привязки (в настройках горячих клавиш)
//==============================================================================

#include <QItemDelegate>

class AssignDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    AssignDelegate(QObject *parent = 0);
    QWidget *createEditor (QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
private slots:
  void commitAndCloseEditor();
private:
  bool eventFilter(QObject*, QEvent*);
};
#endif
