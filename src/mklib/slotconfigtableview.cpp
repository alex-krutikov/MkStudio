#include <QtGui>

#include "slotconfigtableview.h"
#include "mbcommon.h"

//###################################################################
//
//###################################################################
void SlotConfigTableView::handleClick(QModelIndex ind)
{
  if (ind.column() == 3)
  {
    if (ind.data(Qt::DisplayRole).toString() == "") return;  //no need to show editor in such case
    emit edit(ind);
  }
}


//###################################################################
//
//###################################################################
MBConfigWidgetItemDelegate::MBConfigWidgetItemDelegate( QWidget *parent )
  : QItemDelegate( parent )
{
}

//==============================================================================
//
//==============================================================================
QWidget* MBConfigWidgetItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                            const QModelIndex &index ) const
{
  if( index.data(Qt::DisplayRole).toString().isEmpty() ) return 0;  //no need to show editor in such case

  QComboBox *w = new QComboBox(parent);

  QPalette palette = w->palette();
  palette.setColor( QPalette::Base, QColor("linen") );
  w->setPalette( palette );

  QString current_text = index.data( Qt::DisplayRole ).toString();
  MBDataType mb(1);
  for (int i = 1; i < mb.length(); i++)
  {
    w->addItem( mb.toName(), i );
    mb.step();
  }
  w->setCurrentIndex( w->findText( current_text ) );
  connect( w,   SIGNAL(currentIndexChanged(int)),
           this,  SLOT( commitAndCloseEditor() ) );  //otherwise user needs to click
                                                     //outside of combobox after choosing
                                                     // an element to commit data
  return w;

}

//==============================================================================
//
//==============================================================================
void MBConfigWidgetItemDelegate::updateEditorGeometry ( QWidget * editor,
             const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  QItemDelegate::updateEditorGeometry(editor,option,index);

  QComboBox *w = qobject_cast<QComboBox*>(editor);
  if (!w) return;
  w->showPopup();
}

//==============================================================================
//
//==============================================================================
void MBConfigWidgetItemDelegate::commitAndCloseEditor()
{
  QWidget *editor = qobject_cast<QWidget*>( sender() );
  if( editor == 0 ) return;

  QComboBox *w = qobject_cast<QComboBox*>(editor);
  if (!w) return;
  if (w->currentIndex() == -1) return;
  emit commitData( editor );
  emit closeEditor( editor);
}

//==============================================================================
//
//==============================================================================
void MBConfigWidgetItemDelegate::setModelData ( QWidget * editor, QAbstractItemModel * model,
                                       const QModelIndex &index ) const
{
  QComboBox *w = qobject_cast<QComboBox*>(editor);
  if (!w) return;
  int cur_ind = w->currentIndex();
  if (cur_ind == -1) return;

  model->setData(index, w->itemData(cur_ind), Qt::EditRole);
}