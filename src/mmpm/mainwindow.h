#ifndef __mainwindow__h__
#define __mainwindow__h__

#include "ui_mainwindow.h"
#include "ui_initdialog.h"
#include "ui_optdialog.h"
#include "ui_helpdialog.h"
#include "ui_selectsourcedialog.h"
#include "ui_downloaddialog.h"

#include <windows.h>

#include <QMainWindow>
#include <QThread>
#include <QObject>
#include <QMutex>
#include <QByteArray>

class HelpDialog;

void download_firmware_gui(const QString &module_name, QByteArray *out);

//==============================================================================
// Главное окно
//==============================================================================
class MainWindow : public QMainWindow,
                   public Ui::MainWindow
{
  Q_OBJECT
public:
  MainWindow();
  void closeEvent( QCloseEvent *event );
  int pb_value;
  volatile BYTE flag;  // флаг для синхронизации GUI
  volatile BYTE flag2; // флаг для синхронизации GUI
  volatile BYTE flag3; // флаг для синхронизации GUI
  volatile BYTE flag4; // флаг для синхронизации GUI
private:
  bool set_node();
  QTimer *timer;
  QStatusBar *statusbar;
  QLabel *status_requests;
  QLabel *status_answers;
  QLabel *status_errors;
private slots:
  void update();
  void on_pb1_clicked();
  void on_pb2_clicked();
  void on_pb3_clicked();
  void on_pb4_clicked();
  void on_pb5_clicked();
  void on_pb6_clicked();
  void on_pb7_clicked();
  void on_pb8_clicked();
  void on_pb10_clicked();
  void on_pb11_clicked();
};

//==============================================================================
// Начальный диалог
//==============================================================================
class InitDialog : public QDialog,
                   public Ui::InitDialog
{
  Q_OBJECT
  static const char ini_port_name[];
  static const char ini_port_speed[];
  static const char ini_port_host[];
  static const char ini_port_timeout[];
  static const char ini_port_max_packet_size[];
public:
  InitDialog( QWidget *parent=0);
private:
  void accept();
private slots:
  void on_pb_help_clicked();
  void on_cb_portname_currentIndexChanged(int);
};

//==============================================================================
// Диалог настроек
//==============================================================================
class OptDialog : public QDialog,
                  public Ui::OptDialog
{
public:
  OptDialog( QWidget *parent=0);
private:
  void accept();
};

//==============================================================================
// Начальный диалог
//==============================================================================
class HelpDialog : public QDialog,
                   public Ui::HelpDialog
{
public:
  HelpDialog( QWidget *parent=0);
};

//==============================================================================
//
//==============================================================================
class SelectSourceDialog : public QDialog,
                           public Ui::SelectSourceDialog
{
public:
  enum Result {FromFile = Accepted + 1, FromInternet};

  SelectSourceDialog( QWidget *parent=0);
};

//==============================================================================
//
//==============================================================================
class FirmwareVersionsDialog : public QDialog,
                       public Ui::DownloadDialog
{
public:
  FirmwareVersionsDialog(const QStringList &versions, QString *out, QWidget *parent=0);
  void accept();

private:
  QString *m_out;
};



#endif
