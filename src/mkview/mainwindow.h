#ifndef __mainwindow__h__
#define __mainwindow__h__

#include "mbmasterxml.h"
#include "serialport.h"
#include "ui/ui_initdialog.h"
#include "ui/ui_settings.h"

class AbstractSerialPort;
class MKViewPluginInterface;
class MKTable;
class InfoLabel;

//=======================================================================================
//
//=======================================================================================
class InitDialog
    : public QDialog
    , public Ui::InitDialog
{
    Q_OBJECT
public:
    InitDialog(QWidget *parent = 0);
    void accept();
    QString filename();
    QString portname();
    int portspeed();
    QString module_node();
    QString module_name();
    QString host() const;
    int int_module_node();
    int int_module_subnode();

private:
    void setModulesComboBox(QComboBox *cb);
private slots:
    void on_cb_portname_currentIndexChanged(int);
    void on_tb_modulehelp_clicked();
    void on_tb_moduleinfo_clicked();
    void on_pb_update_clicked();
    void on_pb_settings_clicked();
};

//=======================================================================================
//
//=======================================================================================
class SettingsDialog
    : public QDialog
    , public Ui::SettingsDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);
    void accept();
};

//=======================================================================================
//
//=======================================================================================
#include "ui/ui_mainwindowxml.h"
class MainWindowXml
    : public QMainWindow
    , public Ui::MainWindowXml
{
    Q_OBJECT
public:
    MainWindowXml(QWidget *parent = 0, const QString &portname = QString(),
                  int portspeed = 115200, const QString &host = "localhost",
                  const QString &xml_filename = QString(), int node = 1,
                  int subnode = 0);
    void closeEvent(QCloseEvent *event);
private slots:
    void group_update();
    void on_action_font_increase_triggered();
    void on_action_font_decrease_triggered();

private:
    MKTable *tw;
    QStatusBar *statusbar;
    InfoLabel *status_requests;
    InfoLabel *status_answers;
    InfoLabel *status_errors;
    InfoLabel *full_time;
    MBMasterXMLPtr mbmaster;
};

//=======================================================================================
//
//=======================================================================================
#include "ui/ui_midialog.h"
class ModuleInfoDialog
    : public QDialog
    , public Ui::ModuleInfoDialog
{
    Q_OBJECT
public:
    ModuleInfoDialog(QWidget *parent = 0, const QByteArray &ba = QByteArray());
};


#endif
