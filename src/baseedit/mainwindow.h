#ifndef __mainwindow_h_
#define __mainwindow_h_

#include <QMainWindow>
#include <QStringList>

class Model;
class QAction;
class QActionGroup;
class QCloseEvent;
class QString;

#include "model.h"

//==============================================================================
// главное окно
//==============================================================================
#include "ui_mainwindow.h"
class MainWindow : public QMainWindow,
                   public Ui::MainWindow
{
  Q_OBJECT
public:
  MainWindow();
  void create_undo_point();
  void load_file( const QString &filename );
private slots:
  void on_action_new_triggered();
  void on_action_load_triggered();
  void on_action_save_triggered();
  void on_action_save_as_triggered();
  void on_action_export_triggered();
  void on_action_export_h_triggered();
  void on_action_import_triggered();
  void on_action_copy_triggered();
  void on_action_paste_triggered();
  void on_action_undo_triggered();
  void on_action_redo_triggered();
  void on_action_row_insert_triggered();
  void on_action_row_delete_triggered();
  void on_action_about_triggered();
  void update_recent_files();
  void recent_files_activated( QAction* act );
  void on_action_HTML_export_triggered();
  void on_action_Markdown_export_triggered();

  void on_action_show_modbus_toggled();
  void check_tw_activation(const QModelIndex&);
private:

  void file_save();
  void file_load();

  void closeEvent ( QCloseEvent * event );

  Model *model;
  QString current_file_name;
  QStringList recent_files;
  QActionGroup *recent_files_group;

  Items clipboard_buffer;
};

#endif
