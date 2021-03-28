#include "mainwindow.h"
#include "info.h"
#include "main.h"
#include "model.h"

#include <QClipboard>
#include <QDomDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QTextCodec>
#include <QTextStream>

//#######################################################################################
// главное окно
//#######################################################################################
MainWindow::MainWindow()
{
    setupUi(this);

    Settings settings;
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(600, 400)).toSize();
    restoreState(settings.value("mainwindow").toByteArray());
    current_file_name = settings.value("filename").toString();
    recent_files = settings.value("recent_files").toStringList();

    recent_files_group = new QActionGroup(this);
    connect(recent_files_group, SIGNAL(triggered(QAction *)), this,
            SLOT(recent_files_activated(QAction *)));

    setWindowTitle(app_header);

    resize(size);
    move(pos);

    items.resize(100);

    tw->verticalHeader()->hide();
    tw->verticalHeader()->setDefaultSectionSize(22);
    tw->horizontalHeader()->setStretchLastSection(true);
    tw->horizontalHeader()->setHighlightSections(false);
    tw->horizontalHeader()->setSectionsClickable(false);
    tw->setSelectionBehavior(QAbstractItemView::SelectRows);
    tw->setSelectionMode(QAbstractItemView::ContiguousSelection);

    tw->setItemDelegate(new ItemDelegate(this));

    model = new Model(this);
    tw->setModel(model);

    tw->horizontalHeader()->resizeSection(0, 70);
    tw->horizontalHeader()->resizeSection(1, 70);
    tw->horizontalHeader()->resizeSection(2, 70);
    tw->horizontalHeader()->resizeSection(3, 400);
    tw->horizontalHeader()->resizeSection(4, 50);
    tw->horizontalHeader()->resizeSection(5, 50);

    connect(tw, SIGNAL(activated(const QModelIndex &)), tw,
            SLOT(edit(const QModelIndex &)));
    tw->setColumnHidden(1, !action_show_modbus->isChecked());
    tw->setColumnHidden(2, !action_show_modbus->isChecked());

    file_load();

    connect(tw, SIGNAL(activated(const QModelIndex &)), this,
            SLOT(check_tw_activation(const QModelIndex &))); // WO!
    update_recent_files();
}

//==============================================================================
//
//==============================================================================
void MainWindow::update_recent_files()
{
    recent_files = recent_files.mid(0, 10);

    if (!current_file_name.isEmpty())
    {
        recent_files.removeAll(current_file_name);
        recent_files.push_front(current_file_name);
    }

    QList<QAction *> al = menu_file->actions();

    foreach (QAction *act, recent_files_group->actions())
    {
        act->deleteLater();
    }

    foreach (QString fn, recent_files)
    {
        if (QFile::exists(fn))
        {
            QAction *act = new QAction(fn, this);
            recent_files_group->addAction(act);
            menu_file->addAction(act);
        }
    }
}

//==============================================================================
//
//==============================================================================
void MainWindow::recent_files_activated(QAction *act)
{
    load_file(act->text());
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_new_triggered()
{
    current_file_name.clear();
    setWindowTitle(app_header);
    items.clear();
    items.resize(20);
    model->reload_model();
    model->clear_undo_list();
}

//==============================================================================
//
//==============================================================================
void MainWindow::load_file(const QString &filename)
{
    current_file_name = filename;
    file_load();
    update_recent_files();
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_load_triggered()
{
    QString str = QFileDialog::getOpenFileName(this, app_header,
                                               current_file_name, "*.base");
    if (str.isEmpty()) return;
    current_file_name = str;
    file_load();
    update_recent_files();
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_save_triggered()
{
    if (current_file_name.isEmpty())
    {
        on_action_save_as_triggered();
    } else
    {
        file_save();
    }
    update_recent_files();
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_save_as_triggered()
{
    QString str = QFileDialog::getSaveFileName(this, app_header,
                                               current_file_name, "*.base");
    if (str.isEmpty()) return;

    if (!str.endsWith(".base")) str += ".base";

    current_file_name = str;
    setWindowTitle(app_header + " " + current_file_name);
    file_save();
    update_recent_files();
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_export_triggered()
{
    QString s;

    QString filename = current_file_name;
    if (filename.endsWith(".base")) filename.chop(5);
    QFile file(filename + ".inc");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate
                   | QIODevice::Text))
        return;
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("Windows-1251"));

    foreach (Item item, items)
    {
        if (item.addr >= 0)
        {
            QString part2;
            if (item.rtype.getBody() != None)
            {
                part2 += "  ( " + item.rtype.toString() + " "
                       + QString::number(item.rnum) + " ";
                if (item.reg_num_is_big_flag) part2 += "ERROR ";
                part2 += ")";
            }
            out << QString().sprintf("\n  // 0x%4.4X", item.addr);
            out << part2 << endl;
        }
        s = item.field.simplified();
        if (!s.isEmpty())
        {
            if ((!s.contains("#")) && (!s.contains(";")) && (!s.endsWith(",")))
            {
                s += ";";
            }
            out << "  " << s << endl;
        }
        if (!item.desc.isEmpty()) { out << "      // " << item.desc << endl; }
    }

    QMessageBox::information(
        this, app_header,
        QString("База экспортированна в %1.inc").arg(filename));
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_export_h_triggered()
{
    QString s;

    QString filename = current_file_name;
    if (filename.endsWith(".base")) filename.chop(5);
    QString basename = QFileInfo(filename).baseName();
    QFile file(filename + ".h");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate
                   | QIODevice::Text))
        return;
    QTextStream out(&file);
    out.setCodec(QTextCodec::codecForName("utf-8"));

    out << QString("#ifndef __BASE_%1__\n").arg(basename.toUpper());
    out << QString("#define __BASE_%1__\n").arg(basename.toUpper());
    out << "\n"
           "#include <stdint.h>\n"
           "\n";
    out << QString("struct Base_%1\n").arg(basename);
    out << "{\n";

    foreach (Item item, items)
    {
        if (item.addr >= 0)
        {
            QString part2;
            if (item.rtype.getBody() != None)
            {
                part2 += "  ( " + item.rtype.toString() + " "
                       + QString::number(item.rnum) + " ";
                if (item.reg_num_is_big_flag) part2 += "ERROR ";
                part2 += ")";
            }
            out << QString().sprintf("\n  // 0x%4.4X", item.addr);
            out << part2 << endl;
        }
        s = item.field.simplified();
        if (!s.isEmpty())
        {
            if ((!s.contains("#")) && (!s.contains(";")) && (!s.endsWith(",")))
            {
                s += ";";
            }
            out << "  " << s << endl;
        }
        if (!item.desc.isEmpty()) { out << "      // " << item.desc << endl; }
    }

    out << "};\n"
           "\n"
           "#endif\n"
           "\n";


    QMessageBox::information(
        this, app_header, QString("База экспортированна в %1.h").arg(filename));
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_import_triggered()
{
    QRegExp rx("\\s*//\\s*0x[0-9A-F]{4}\\s*");

    QString s1;
    QString filename = QFileDialog::getOpenFileName(this, app_header,
                                                    current_file_name, "*.*");
    if (filename.isEmpty()) return;

    QModelIndexList si = tw->selectionModel()->selectedRows();
    if (si.count() == 0)
    {
        tw->selectRow(0);
        si = tw->selectionModel()->selectedRows();
    }
    if (si.count() == 0) return;
    int row = si.at(0).row();

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    QTextStream in(&file);
    in.setCodec(QTextCodec::codecForName("Windows-1251"));

    Item item;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.simplified().isEmpty()) continue;
        if (rx.exactMatch(line)) continue;
        item = Item();
        s1 = line.section("//", 0, 0).simplified();
        if (s1.endsWith(";")) s1.chop(1);
        item.field = s1;
        item.desc = line.section("//", 1, -1);
        items.insert(row, item);
        row++;
    }

    model->recalc_items();
    model->reload_model();
    model->create_undo_point();
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_copy_triggered()
{
    QString str;

    QModelIndexList si = tw->selectionModel()->selectedRows();
    if (si.count() == 0) return;
    QVector<int> v;
    foreach (QModelIndex mi, si)
    {
        v << mi.row();
    }
    qSort(v);

    clipboard_buffer = items.mid(v[0], v.count());
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_paste_triggered()
{
    QModelIndexList si = tw->selectionModel()->selectedRows();
    if (si.count() == 0) return;
    QVector<int> v;
    foreach (QModelIndex mi, si)
    {
        v << mi.row();
    }
    qSort(v);
    int row = v[0];
    model->insertRows(row, clipboard_buffer.count());
    foreach (Item it, clipboard_buffer)
    {
        items[row] = it;
        row++;
    }
    model->recalc_items();
    model->create_undo_point();
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_undo_triggered()
{
    model->undo();
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_redo_triggered()
{
    model->redo();
}

//==============================================================================
//
//==============================================================================
void MainWindow::file_save()
{
    int i;
    QDomDocument doc;

    QDomElement root = doc.createElement("Base");
    for (i = 0; i < items.size(); i++)
    {
        QDomElement row = doc.createElement("Row");
        if (items[i].addr >= 0) row.setAttribute("Addr", items[i].addr);
        if (items[i].rnum >= 0)
            row.setAttribute("Register_number", items[i].rnum); // wo
        if (items[i].rtype.getBody() != None)
            row.setAttribute("Register_type", items[i].rtype.toString()); // wo
        if (!items[i].field.isEmpty())
            row.setAttribute("Field", items[i].field);
        if (items[i].size > 0) row.setAttribute("Size", items[i].size);
        if (items[i].count > 0) row.setAttribute("Count", items[i].count);
        if (!items[i].desc.isEmpty()) row.setAttribute("Desc", items[i].desc);
        root.appendChild(row);
    }
    doc.appendChild(root);

    QFile file(current_file_name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    file.write("<?xml version=\"1.0\" encoding=\"cp-1251\"?>\n");
    file.write(codec->fromUnicode(doc.toString(2)));
}

//==============================================================================
//
//==============================================================================
void MainWindow::file_load()
{
    int i;

    QDomDocument doc;
    QDomNodeList list;
    QDomElement element;

    if (current_file_name.isEmpty()) return;

    QFile file(current_file_name);
    if (!file.open(QIODevice::ReadOnly))
    {
        current_file_name.clear();
        setWindowTitle(app_header + " " + current_file_name);
        return;
    }
    if (!doc.setContent(&file))
    {
        QMessageBox::critical(
            this, app_header,
            QString("Ошибка при загрузке файла %1.").arg(current_file_name));
        current_file_name.clear();
        setWindowTitle(app_header + " " + current_file_name);
        return;
    }
    file.close();

    list = doc.elementsByTagName("Row");
    items.clear();
    items.resize(list.count());
    for (i = 0; i < list.count(); i++)
    {
        element = list.item(i).toElement();
        items[i].addr = element.attribute("Addr", "-1").toInt();
        items[i].rnum = element.attribute("Register_number", "-1").toInt();
        items[i].rtype = RegType(element.attribute("Register_type", ""));
        items[i].field = element.attribute("Field");
        items[i].size = element.attribute("Size").toInt();
        items[i].count = element.attribute("Count").toInt();
        items[i].desc = element.attribute("Desc");
    }
    setWindowTitle(app_header + " " + current_file_name);

    model->recalc_items();
    model->reload_model();
    model->clear_undo_list();
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_row_insert_triggered()
{
    QModelIndexList si = tw->selectionModel()->selectedRows();
    if (si.count() == 0) return;
    int row = si.at(0).row();

    model->insertRows(row, 1);
    tw->selectRow(row);
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_row_delete_triggered()
{
    QModelIndexList si = tw->selectionModel()->selectedRows();
    if (si.count() == 0) return;

    QVector<int> v;
    foreach (QModelIndex mi, si)
    {
        v << mi.row();
    }
    qSort(v);

    model->removeRows(v[0], v.count());
    tw->selectRow(v[0]);
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_HTML_export_triggered()
{
    QString str;

    QAbstractItemModel *model = tw->model();
    int rows = model->rowCount();
    int columns = model->columnCount();

    int i, j;

    str += "<table border=\"0\" rules=\"none\" cellpadding=\"5\" "
           "cellspacing=\"0\" >\n";

    for (i = 0; i < rows; i++)
    {
        str += "  <tr>\n";
        for (j = 0; j < columns; j++)
        {
            QModelIndex index = model->index(i, j);
            QString disp_role = model->data(index, Qt::DisplayRole).toString();
            QVariant background_role = model->data(index, Qt::BackgroundRole);
            QVariant foreground_role = model->data(index, Qt::ForegroundRole);
            int alignment_role
                = model->data(index, Qt::TextAlignmentRole).toInt();
            QVariant font_role = model->data(index, Qt::FontRole);

            const QFont f = font_role.value<QFont>();

            if (f.bold()) disp_role = "<b>" + disp_role + "</b>";
            if (f.italic()) disp_role = "<i>" + disp_role + "</i>";
            if (foreground_role.isValid())
            {
                disp_role = "<font color=\"" + foreground_role.toString()
                          + "\">" + disp_role + "</font>";
            }

            str += "    <td";

            if (background_role.isValid()
                && background_role != QColor(240, 240, 240))
            {
                str += " bgcolor=\"" + background_role.toString() + "\"";
            }

            if (alignment_role & Qt::AlignRight) str += " align=\"right\"";
            if (alignment_role & Qt::AlignHCenter) str += " align=\"center\"";

            str += ">";
            str += disp_role;
            str += "</td>\n";
        }
        str += "  </tr>\n";
    }
    str += "</table>\n";

    QMimeData *md = new QMimeData;
    md->setHtml(str);
    md->setText(str);
    QApplication::clipboard()->setMimeData(md);

    QMessageBox::information(this, app_header,
                             "База скопирована в буфер обмена в формате HTML.");
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_Markdown_export_triggered()
{
    QString out;

    QAbstractItemModel *model = tw->model();
    int rows = model->rowCount();
    int columns = model->columnCount();

    out += "\n|";

    for (int i = 0; i < columns; i++)
    {
        if (tw->isColumnHidden(i)) continue;
        out += tw->model()->headerData(i, Qt::Horizontal).toString() + " |";
    }

    out += "\n|";

    for (int i = 0; i < columns; i++)
    {
        if (tw->isColumnHidden(i)) continue;
        out += "---|";
    }

    out += "\n";

    for (int i = 0; i < rows; i++)
    {
        out += "| ";
        for (int j = 0; j < columns; j++)
        {
            if (tw->isColumnHidden(j)) continue;

            QModelIndex index = model->index(i, j);
            QString disp_role = model->data(index, Qt::DisplayRole).toString();
            out += disp_role;
            out += " |";
        }
        out += "\n";
    }

    QMimeData *md = new QMimeData;
    md->setText(out);
    QApplication::clipboard()->setMimeData(md);

    QMessageBox::information(
        this, app_header,
        "База скопирована в буфер обмена в формате Markdown.");
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_about_triggered()
{
    QMessageBox::about(
        this, app_header,
        "<body>"
        "Программа <b>BaseEdit</b> <br/><br/> "
        "Версия: "
            + ProgramInformation::version()
            + "<br><br>"
              "ЗАО \"Инкоммет\" <br><br>"
              "<table border=0 cellpadding=1 cellspacing=1> "
              "  <tbody><tr><td>тел.</td><td>(495) 171-97-49</td></tr> "
              "         <tr><td>тел./факс</td><td>(495) 737-56-36</td></tr>"
              "         "
              "<tr><td>e-mail&nbsp;&nbsp;</td><td>mail@inkommet.ru</td></tr>"
              "         <tr><td>url</td><td><a href=\"http://www.inkommet.ru\">"
              "http://www.inkommet.ru</a></td>"
              "    </tr></tbody></table>"
              "</body>");
}

//==============================================================================
//
//==============================================================================
void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    Settings settings;
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("mainwindow", saveState());
    settings.setValue("filename", current_file_name);
    settings.setValue("recent_files", recent_files);
}

//==============================================================================
//
//==============================================================================
void MainWindow::on_action_show_modbus_toggled()
{
    bool b = action_show_modbus->isChecked();
    tw->setColumnHidden(1, !b);
    tw->setColumnHidden(2, !b);
}

//==============================================================================
//
//==============================================================================
void MainWindow::check_tw_activation(const QModelIndex &ind)
{
    int row = ind.row();
    int col = ind.column();
    if (row == 0) return;
    if (col == 2) // processing type
    {
        // QVariant str = model->data(ind);
        model->setData(ind, 1, Qt::UserRole);
    }
}
