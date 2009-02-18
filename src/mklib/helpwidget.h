#ifndef __HELPWIDGET_H__
#define __HELPWIDGET_H__

#include <QWidget>

class QUrl;

#include "ui_helpwidget.h"
class HelpWidget : public QWidget,
                   public Ui::HelpWidget
{
  Q_OBJECT
public:
  HelpWidget(QWidget *parent = 0);
  void setContents( const QUrl &url );
private:
};


#endif
