#include "consolewidget.h"

#include <console.h>

#include <QScrollBar>

#include "ui/ui_consolewidget.h"

//###################################################################
//
//###################################################################
ConsoleWidget::ConsoleWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ConsoleWidget)
{
    ui->setupUi(this);
    div_counter = 0;
    ui->te->setFont(QFont("Courier New", 10));
    ui->te->setLineWrapMode(QTextEdit::NoWrap);
    ui->bcb_autoscroll->setChecked(true);
    ui->te->setReadOnly(true);
    startTimer(200);
}

//===================================================================
//
//===================================================================
ConsoleWidget::~ConsoleWidget()
{
    delete ui;
}

//===================================================================
/// Функция таймера
//===================================================================
void ConsoleWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    ui->te->insertPlainText(Console::takeMessage());

    // лимитирование размера
    if (div_counter++ > 25) // 5 секунд
    {
        div_counter = 0;
        QTextDocument *doc = ui->te->document();
        if (doc->blockCount() > 10000)
        {
            doc->setMaximumBlockCount(5000);
            doc->setMaximumBlockCount(0);
        }
    }
    // автопрокрутка
    if (ui->bcb_autoscroll->isChecked())
    {
        QScrollBar *sb = ui->te->verticalScrollBar();
        if (!sb->isSliderDown()) sb->setValue(sb->maximum());
    }
}

//===================================================================
/// Очистка консоли
//===================================================================
void ConsoleWidget::on_pb_clear_clicked()
{
    ui->te->clear();
}
