#ifndef __mainwindow__h__
#define __mainwindow__h__

//#ifdef Q_OS_WIN32
  //#include <windows.h>
  //#include <dbt.h>
//#endif

#include<QtGui>

struct TableClipboardItem
{
  QTableWidgetItem item;
  int row,col;
};

typedef QVector<TableClipboardItem> TableClipboard;

#include "ui_mainwindow.h"
//==============================================================================
/// Главное окно
//==============================================================================
class MainWindow : public QMainWindow,
                   public Ui::MainWindow
{
  Q_OBJECT
public:
  MainWindow();
private:
  void closeEvent( QCloseEvent *event );
  void setAlign( int alignment );
  bool load_conf( const QString &filename );
  bool save_conf( const QString &filename );
  void fill_empty_items();
  void applay_undo_point();
  void clear_undo_list();
  void timerEvent(QTimerEvent *event );
  void toClipboard();
  void fromClipboard();
private slots:
  void on_action_tw_rows_triggered();
  void on_action_tw_columns_triggered();
  void action_tw_header_double_clicked(int logicalIndex);
  void on_tw_itemSelectionChanged();
  void on_action_new_triggered();
  void on_action_load_triggered();
  void on_action_save_triggered();
  void on_action_save_as_triggered();
  void on_action_play_triggered();
  void on_action_bold_triggered();
  void on_action_underline_triggered();
  void on_action_italic_triggered();
  void on_action_align_left_triggered();
  void on_action_align_center_triggered();
  void on_action_align_right_triggered();
  void on_action_link_triggered();
  void on_action_settings_triggered();
  void on_action_row_add_triggered();
  void on_action_row_delete_triggered();
  void on_action_column_add_triggered();
  void on_action_column_delete_triggered();
  void on_action_cell_add_right_triggered();
  void on_action_cell_add_down_triggered();
  void on_action_cell_delete_left_triggered();
  void on_action_cell_delete_up_triggered();
  void on_action_copy_triggered();
  void on_action_cut_triggered();
  void on_action_paste_triggered();
  void on_action_foreground_color_triggered();
  void on_action_background_color_triggered();
  void on_action_undo_triggered();
  void on_action_redo_triggered();
  void on_action_values_save_triggered();
  void on_action_values_load_triggered();
  void on_action_export_config_triggered();
  void on_action_help_triggered();
  void on_action_help_about_triggered();
  void on_action_united_graph_triggered();
  void del_key_pressed();
  void copy_to_console();
  void context_menu();
  void slot_attributes_saved( int module, int slot, QString attributes );
  //void on_action_cells_span_triggered();
public slots:
  void make_undo_point();
private:
  QLabel *status_requests;
  QLabel *status_answers;
  QLabel *status_errors;
  QLabel *full_time;
  QString current_config_filename;
  QString current_config_values_filename;
  bool play_mode;
  TableClipboard table_clipboard;
  QList<QByteArray> undo_list;
  int undo_list_current_point;
  bool is_undo_enabled, is_redo_enabled;
};

#endif
