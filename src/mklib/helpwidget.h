#ifndef __HELPWIDGET_H__
#define __HELPWIDGET_H__

#include <QWidget>

class QUrl;
namespace Ui { class HelpWidget; }

class HelpWidget : public QWidget
{
  Q_OBJECT
public:
  HelpWidget(QWidget *parent = 0);
  virtual ~HelpWidget();
  void setContents( const QUrl &url );
private:
  Ui::HelpWidget *ui;
};


#endif
