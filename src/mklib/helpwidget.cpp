#include "helpwidget.h"

#include "ui/ui_helpwidget.h"

HelpWidget::HelpWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HelpWidget)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icons/res/help2.png"));

    QFont f = font();
    f.setPixelSize(16);
    ui->tb->setFont(f);

    ui->tb->setOpenExternalLinks(true);

    connect(ui->pb_backward, SIGNAL(clicked()), ui->tb, SLOT(backward()));
    connect(ui->pb_forward, SIGNAL(clicked()), ui->tb, SLOT(forward()));
    connect(ui->pb_home, SIGNAL(clicked()), ui->tb, SLOT(home()));

    connect(ui->tb, SIGNAL(backwardAvailable(bool)), ui->pb_backward,
            SLOT(setEnabled(bool)));
    connect(ui->tb, SIGNAL(forwardAvailable(bool)), ui->pb_forward,
            SLOT(setEnabled(bool)));
}

HelpWidget::~HelpWidget()
{
    delete ui;
}

void HelpWidget::setContents(const QUrl &url)
{
    ui->tb->setSource(url);
}
