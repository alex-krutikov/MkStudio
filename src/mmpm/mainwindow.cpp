#include "mainwindow.h"

#include "main.h"
#include "modbus.h"

#include "console.h"
#include "serialport.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDomDocument>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QScreen>
#include <QScrollBar>
#include <QSettings>
#include <QStatusBar>
#include <QTimer>

bool get_firmwares_list(QWidget *parent, const QString &module_name,
                        QStringList *out, QString *errorMessage)
{
    QDialog dialog(parent);
    dialog.setWindowFlags(Qt::Tool);
    QLabel *label = new QLabel(&dialog);
    label->setText("Подключаюсь к серверу...");
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    dialog.setLayout(layout);

    QString url = firmware_server_url + "/get?name=" + module_name;

    QNetworkReply *reply = network_manager.get(QNetworkRequest(QUrl(url)));
    QObject::connect(reply, SIGNAL(finished()), &dialog, SLOT(accept()));
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &dialog,
                     SLOT(accept()));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)), &dialog,
                     SLOT(accept()));

    dialog.exec();

    reply->deleteLater();

    if (dialog.result() != QDialog::Accepted) return false;

    switch (reply->error())
    {
    case QNetworkReply::NoError:
        break;
    default:
        *errorMessage = "Ошибка при подключении к серверу.";
        return true;
    }

    if (!reply->isFinished()) return false;

    QByteArray data = reply->readAll();

    QDomDocument doc;
    bool ok = doc.setContent(data);
    if (!ok)
    {
        *errorMessage = "Ошибка. Сервер выдал некорректные данные.";
        return true;
    }
    QDomElement docRootElement = doc.documentElement();

    QStringList ret;

    QDomNode node = docRootElement.firstChild();
    if (docRootElement.tagName() == "versions")
    {
        while (!node.isNull())
        {
            QDomElement e = node.toElement();
            if (!e.isNull())
            {
                if (e.tagName() == "item") { ret << e.text(); }
            }
            node = node.nextSibling();
        }
    }

    *out = ret;
    return true;
}

//==============================================================================
//
//==============================================================================
bool download_firmware(QWidget *parent, const QString &module_name,
                       const QString &firmware_name, QByteArray *out,
                       QString *error)
{
    QString url = firmware_server_url + "/" + module_name + "/" + firmware_name
                + ".bin";

    QProgressDialog progressDialog(parent);
    progressDialog.setWindowFlags(Qt::Tool);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setWindowTitle("MMpM");
    progressDialog.setLabelText("Загрузка прошивки");
    progressDialog.setMinimumDuration(500);

    QEventLoop loop;

    QNetworkReply *reply = network_manager.get(QNetworkRequest(QUrl(url)));

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(
        reply,
        static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(
            &QNetworkReply::errorOccurred),
        &loop, &QEventLoop::quit);
    QObject::connect(reply, &QNetworkReply::sslErrors, &loop,
                     &QEventLoop::quit);

    QObject::connect(&progressDialog, &QProgressDialog::canceled, &loop,
                     &QEventLoop::quit);

    QObject::connect(reply, &QNetworkReply::downloadProgress, &progressDialog,
                     [&progressDialog](qint64 a, qint64 b) {
                         progressDialog.setMaximum(b);
                         progressDialog.setValue(a);
                     });

    loop.exec();

    reply->deleteLater();

    if (progressDialog.wasCanceled()) return false;

    if (reply->isRunning() || (reply->error() != QNetworkReply::NoError))
    {
        *error = "Ошибка при загрузке прошивки";
        return true;
    }

    *out = reply->readAll();
    return true;
}

//==============================================================================
//
//==============================================================================
void download_firmware_gui(const QString &module_name, QByteArray *out)
{
    QStringList versions;
    QString errorMessage;
    QString firmware_name;
    QByteArray ba;

    bool ok
        = get_firmwares_list(mainwindow, module_name, &versions, &errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::information(mainwindow, "MMpM", errorMessage);
        return;
    }

    if (!ok) return;

    if (versions.isEmpty())
    {
        QMessageBox::information(
            mainwindow, "MMpM",
            QString("Микропограмма для модуля \"%1\" не найдена на сервере.")
                .arg(module_name));
        return;
    }

    FirmwareVersionsDialog dialog(versions, &firmware_name, mainwindow);
    dialog.exec();

    if (dialog.result() != QDialog::Accepted) return;

    ok = download_firmware(mainwindow, module_name, firmware_name, &ba,
                           &errorMessage);

    if (!errorMessage.isEmpty())
    {
        QMessageBox::information(mainwindow, "MMpM", errorMessage);
        return;
    }

    if (!ok) return;

    *out = ba;
}

//==============================================================================
// Главное окно
//==============================================================================
MainWindow::MainWindow()
{
    setupUi(this);
    setWindowTitle("Менеджер микропрограмм модулей ПТК УМИКОН");
    te->setLineWrapMode(QPlainTextEdit::NoWrap);
    pb8->hide();
    pb11->hide();

    Settings settings;
    if (settings.contains("pos") && settings.contains("size"))
    {
        const QPoint pos = settings.value("pos").toPoint();
        const QSize size = settings.value("size").toSize();
        move(pos);
        resize(size);
    } else
    {
        QRect rs = QGuiApplication::screens().value(0)->availableGeometry();
        QRect rw;
        rw.setSize(QSize(800, 500));
        if (rw.height() > (rs.height() - 70))
        {
            rw.setHeight(rs.height() - 50);
        }
        if (rw.width() > (rs.width() - 50)) { rw.setWidth(rs.width() - 70); }
        rw.moveCenter(rs.center());
        setGeometry(rw);
    }

    firmware_filename = settings.value("firmware_filename").toString();
    firmware_server_url
        = settings.value("firmware_server_url", DEFAULT_SERVER_URL).toString();

    Console::setMessageTypes(Console::Error | Console::Warning
                             | Console::Information | Console::Debug);
    flag = 0;
    flag2 = 0;
    flag3 = 0;
    flag4 = 0;

    //------- строка статуса -----------------
    statusbar = new QStatusBar();
    status_requests = new QLabel;
    status_answers = new QLabel;
    status_errors = new QLabel;
    statusbar->addPermanentWidget(status_requests);
    statusbar->addPermanentWidget(status_answers);
    statusbar->addPermanentWidget(status_errors);
    setStatusBar(statusbar);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(10);

    pb_value = 0;
    flag = 0;
}

//==============================================================================
// Главное окно -- установка адреса модуля
//==============================================================================
bool MainWindow::set_node()
{
    bool ok;
    BYTE ret;

    if (secret_mode)
    {
        ret = QInputDialog::getInt(this, tr("MMpM"), tr("Адрес модуля:"),
                                   thread1->node, 1, 127, 1, &ok);
        if (!ok)
        {
            thread1->mode = 0;
            return false;
        }
        thread1->node = ret;
        Console::Print(Console::Information,
                       tr("Адрес модуля: %1\n").arg(thread1->node));
    } else
    {
        thread1->node = 127;
    }
    return true;
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Прочитать"
//==============================================================================
void MainWindow::on_pb1_clicked()
{
    if (thread1->isRunning()) return;
    if (!set_node()) return;
    thread1->mode = 1;
    thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Записать"
//==============================================================================
void MainWindow::on_pb2_clicked()
{
    if (thread1->isRunning()) return;
    if (!set_node()) return;
    SelectSourceDialog selectSourcedialog(this);
    int res = selectSourcedialog.exec();
    if (res == SelectSourceDialog::FromFile)
    {
        thread1->mode = 2;
        thread1->start();
        return;
    } else if (res == SelectSourceDialog::FromInternet)
    {
        thread1->mode = 8;
        thread1->start();
        return;
    }

    thread1->mode = 0;
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Опции"
//==============================================================================
void MainWindow::on_pb3_clicked()
{
    OptDialog dialog(this);
    dialog.exec();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Сброс"
//==============================================================================
void MainWindow::on_pb4_clicked()
{
    if (thread1->isRunning()) return;
    if (!set_node()) return;
    thread1->mode = 3;
    thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Информация"
//==============================================================================
void MainWindow::on_pb5_clicked()
{
    if (thread1->isRunning()) return;
    if (!set_node()) return;
    thread1->mode = 4;
    thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Проверить"
//==============================================================================
void MainWindow::on_pb6_clicked()
{
    if (thread1->isRunning()) return;
    if (!set_node()) return;
    thread1->mode = 5;
    thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Помощь"
//==============================================================================
void MainWindow::on_pb7_clicked()
{
    // HelpDialog *dialog = new HelpDialog( this );
    helpdialog->show();
    helpdialog->raise();
    helpdialog->activateWindow();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "СТЕРЕТЬ"
//==============================================================================
void MainWindow::on_pb8_clicked()
{
    if (thread1->isRunning()) return;
    if (!set_node()) return;
    thread1->mode = 6;
    thread1->start();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Очистить"
//==============================================================================
void MainWindow::on_pb10_clicked()
{
    te->clear();
}

//==============================================================================
// Главное окно -- Нажатие кнопкки "Запись загрузчика"
//==============================================================================
void MainWindow::on_pb11_clicked()
{
    if (QMessageBox::warning(
            this, "MMpM",
            "<big><b>ВНИМАНИЕ!</b>"
            "<p>Запись загрузчика является потенциально опасной операцией! "
            "Вы должны четко представлять, что делаете. В случае записи "
            "некорректного или "
            "не соответствующего типу модуля загрузчика, а также при "
            "отключении питания во время "
            "перезаписи, восстановление модуля может быть осуществленно "
            "<b>только у изготовителя!</b></p>"
            "<p>Продолжить?</p></big>",
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel)
        != QMessageBox::Yes)
        return;

    if (thread1->isRunning()) return;
    if (!set_node()) return;
    thread1->mode = 7;
    thread1->start();
}

//==============================================================================
//
//==============================================================================
void MainWindow::closeEvent(QCloseEvent *)
{
    Settings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("firmware_filename",
                      QFileInfo(firmware_filename).absolutePath());
}

//==============================================================================
//
//==============================================================================
static WORD irnd(WORD value)
{ //                       8#31105    8#15415   8#77777
    return ((((DWORD)value * 0x3245) + 0x1B0D) & 0x7FFF);
}


//==============================================================================
//
//==============================================================================
static DWORD passw2answer(DWORD pass)
{
    const DWORD pow10[10] = {1000000000, 100000000, 10000000, 1000000, 100000,
                             10000,      1000,      100,      10,      1};
    int i;
    DWORD sum;

    sum = 0;
    for (i = 0; i < 10; i++)
    {
        while (pass >= pow10[i])
        {
            sum++;
            pass -= pow10[i];
        }
    }
    return sum;
}

//==============================================================================
// Главное окно -- таймер 100 ms
//==============================================================================
void MainWindow::update()
{
    bool ok;

    status_requests->setText(QString(" Запросы: %1 ").arg(modbus->req_counter));
    status_answers->setText(QString(" Ответы:  %1 ").arg(modbus->ans_counter));
    status_errors->setText(QString(" Ошибки:  %1 ").arg(modbus->err_counter));

    progressBar->setValue(pb_value);

    if (thread1->mode)
    {
        pb1->setEnabled(false);
        pb2->setEnabled(false);
        pb4->setEnabled(false);
        pb5->setEnabled(false);
        pb6->setEnabled(false);
        pb8->setEnabled(false);
        pb11->setEnabled(false);
    } else
    {
        pb1->setEnabled(true);
        pb2->setEnabled(true);
        pb4->setEnabled(true);
        pb5->setEnabled(true);
        pb6->setEnabled(true);
        pb8->setEnabled(true);
        pb11->setEnabled(true);
        pb_value = 0;
    }

    if (flag)
    {
        timer->stop();
        if (secret_mode2 == 1)
        {
            if (secret_mode == 1)
            {
                temp_str2.setNum(irnd(temp_str2.toULong()));
            } else
            {
                temp_str2.setNum(passw2answer(temp_str2.toULong()));
            }
        } else
        {
            temp_str2.clear();
        }

        temp_str = QInputDialog::getText(this, tr("MMpM"), temp_str,
                                         QLineEdit::Normal, temp_str2, &ok);
        if (!ok) temp_str.clear();
        timer->start();
        flag = 0;
    }

    if (flag2 == 1)
    {
        QMessageBox::StandardButton button;
        timer->stop();
        button = QMessageBox::warning(
            this, "MMpM",
            "<p>Файл микропрограммы подготовлен в сторонней программе и не "
            "соответствует формату MMpM !"
            "Тем не менее, его можно попробовать безопасно записать в "
            "модуль.</p>"
            "<p>Продолжить?</p>",
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
        timer->start();
        flag2 = 0;
        if (button == QMessageBox::Yes)
        {
            flag2 = 2;
        } else
        {
            Console::Print(Console::Information, "Отмена\n");
        }
    }

    if (flag3 == 1)
    {
        timer->stop();
        if (firmware_filename.isEmpty())
            firmware_filename += QDir().homePath() + QDir::separator();

        firmware_filename = QFileDialog::getOpenFileName(
            0, "Загрузить файл", firmware_filename, "*.bin");
        timer->start();
        flag3 = 0;
    }

    if (flag4 == 1)
    {
        timer->stop();

        firmware_filename = QFileDialog::getSaveFileName(
            0, tr("Записать файл"), firmware_filename, tr("*.bin"));
        timer->start();
        flag4 = 0;
    }

    modbus->subnode = le->text().toInt();

    //////////////////////

    QString console_data = Console::takeMessage();
    if (!console_data.isEmpty()) { te->insertPlainText(console_data); }

    if (mainwindow->cb_autoscroll->isChecked())
    {
        QScrollBar *sb = te->verticalScrollBar();
        if (!sb->isSliderDown()) sb->setValue(sb->maximum());
    }
}

//==============================================================================
const char InitDialog::ini_port_name[] = "ComPortName";
const char InitDialog::ini_port_speed[] = "ComPortSpeed";
const char InitDialog::ini_port_host[] = "ModbusTcpHost";
const char InitDialog::ini_port_timeout[] = "ComPortTimeout";
const char InitDialog::ini_port_max_packet_size[] = "MaxPacketSize";
//==============================================================================
// Начальный диалог
//==============================================================================
InitDialog::InitDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    setWindowTitle("MMpM - Настройка связи");

    Settings settings;

    QString str, str2;
    QStringList sl;
    int i;
    //----------------------------------------------------------------------
    cb_portname->clear();
    QStringList ports = SerialPort::queryComPorts();
    foreach (str, ports)
    {
        str2 = str;
        str2.replace(';', "        ( ");
        cb_portname->addItem(str2 + " )", str.section(';', 0, 0));
    }
    cb_portname->addItem("Modbus TCP", "===TCP===");
    i = cb_portname->findData(settings.value(ini_port_name).toString());
    if (i >= 0) cb_portname->setCurrentIndex(i);
    //----------------------------------------------------------------------
    cb_portspeed->addItems(QStringList() << "300"
                                         << "600"
                                         << "1200"
                                         << "2400"
                                         << "4800"
                                         << "9600"
                                         << "19200"
                                         << "38400"
                                         << "57600"
                                         << "115200"
                                         << "230400"
                                         << "460800"
                                         << "921600");
    i = cb_portspeed->findText(
        settings.value(ini_port_speed, "115200").toString());
    if (i >= 0) cb_portspeed->setCurrentIndex(i);
    //----------------------------------------------------------------------
    le_tcp_server->setText(
        settings.value(ini_port_host, "localhost").toString());
    //----------------------------------------------------------------------
    sb_timeout->setValue(settings.value(ini_port_timeout, 200).toInt());
    //----------------------------------------------------------------------
    sb_max_packet_size->setValue(
        settings.value(ini_port_max_packet_size, 128).toInt());
    //----------------------------------------------------------------------
    setFocus();
}

//==============================================================================
// Переключение ComboBox'а порта
//==============================================================================
void InitDialog::on_cb_portname_currentIndexChanged(int)
{
    QString str = cb_portname->itemData(cb_portname->currentIndex()).toString();
    bool b = (str == "===TCP===");

    cb_portspeed->setHidden(b);
    l_speed->setHidden(b);
    l_server->setHidden(!b);
    le_tcp_server->setHidden(!b);
}

//==============================================================================
// Начальный диалог -- нажатие OK
//==============================================================================
void InitDialog::accept()
{
    QString str = cb_portname->itemData(cb_portname->currentIndex()).toString();

    if (str == "===TCP===")
    {
        modbus->setupTcpHost(le_tcp_server->text());
    } else if (modbus->setupPort(str, cb_portspeed->currentText().toInt()) == 0)
    {
        QMessageBox::critical(this, "MMpM", tr("Ошибка открытия порта!"));
        reject();
        return;
    }
    modbus->setupTimeOut(sb_timeout->value());
    thread1->setupMaxPacketSize(sb_max_packet_size->value());

    Settings settings;
    settings.setValue(ini_port_name, str);
    settings.setValue(ini_port_speed, cb_portspeed->currentText());
    settings.setValue(ini_port_host, le_tcp_server->text());
    settings.setValue(ini_port_timeout, sb_timeout->value());
    settings.setValue(ini_port_max_packet_size, sb_max_packet_size->value());

    done(1);
}

//==============================================================================
// Начальный диалог -- нажатие OK
//==============================================================================
void InitDialog::on_pb_help_clicked()
{
    helpdialog->exec();
    helpdialog->activateWindow();
}

//==============================================================================
// Диалог настроек
//==============================================================================
OptDialog::OptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    setWindowTitle(tr("Опции"));

    le_server_url->setText(firmware_server_url);

    cb_modbus_packets->setChecked(Console::messageTypes()
                                  & Console::ModbusPacket);

    sb_delay->setValue(thread1->erase_delay);

    if (!secret_mode2) cb1->hide();
    if (!secret_mode2) cb2->hide();

    if (!mainwindow->pb8->isHidden()) cb1->setChecked(true);
    if (!mainwindow->pb11->isHidden()) cb2->setChecked(true);
    setFocus();
}

//==============================================================================
// Диалог настроек -- нажатие OK
//==============================================================================
void OptDialog::accept()
{
    Console::setMessageTypes(Console::ModbusPacket,
                             cb_modbus_packets->isChecked());

    firmware_server_url = le_server_url->text();

    thread1->erase_delay = sb_delay->value();

    if (cb1->isChecked() && secret_mode2)
    {
        mainwindow->pb8->show();
    } else
    {
        mainwindow->pb8->hide();
    }
    if (cb2->isChecked() && secret_mode2)
    {
        mainwindow->pb11->show();
    } else
    {
        mainwindow->pb11->hide();
    }

    Settings settings;
    settings.setValue("firmware_server_url", firmware_server_url);

    done(1);
}

//==============================================================================
// Диалог помощи
//==============================================================================
HelpDialog::HelpDialog(QWidget *)
{
    setupUi(this);
    setWindowTitle("Помощь MMpM версия 2.6 от 20091020");
    resize(700, 400);

    QFont f = font();
    f.setPointSize(9);
    textBrowser->setFont(f);
    textBrowser->setSource(QUrl::fromLocalFile(":/html/help/help.html"));
}

//==============================================================================
//
//==============================================================================
SelectSourceDialog::SelectSourceDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect(pb_from_file, &QPushButton::clicked, this,
            [this] { done(FromFile); });
    connect(pb_from_internet, &QPushButton::clicked, this,
            [this] { done(FromInternet); });
}

//==============================================================================
//
//==============================================================================
FirmwareVersionsDialog::FirmwareVersionsDialog(const QStringList &versions,
                                               QString *out, QWidget *parent)
    : QDialog(parent)
    , m_out(out)
{
    setupUi(this);

    lw->addItems(versions);
    lw->selectionModel()->select(lw->model()->index(0, 0),
                                 QItemSelectionModel::Select);

    connect(buttonBox, &QDialogButtonBox::accepted, this,
            &FirmwareVersionsDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this,
            &FirmwareVersionsDialog::reject);
    connect(lw, &QListWidget::doubleClicked, this,
            &FirmwareVersionsDialog::accept);
}

//==============================================================================
//
//==============================================================================
void FirmwareVersionsDialog::accept()
{
    *m_out = lw->selectedItems().value(0)->text();
    QDialog::accept();
}
