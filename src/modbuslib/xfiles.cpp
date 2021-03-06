#include "xfiles.h"

#include "abstractserialport.h"
#include "console.h"
#include "crc.h"
#include "mbcommon.h"

//=============================================================================
// Служебная функция преобразования результата
//=============================================================================
static XFiles::Result fromFResult(int fresult)
{
    enum FRESULT
    {
        FR_OK = 0,          /* 0 */
        FR_NOT_READY,       /* 1 */
        FR_NO_FILE,         /* 2 */
        FR_NO_PATH,         /* 3 */
        FR_INVALID_NAME,    /* 4 */
        FR_INVALID_DRIVE,   /* 5 */
        FR_DENIED,          /* 6 */
        FR_EXIST,           /* 7 */
        FR_RW_ERROR,        /* 8 */
        FR_WRITE_PROTECTED, /* 9 */
        FR_NOT_ENABLED,     /* 10 */
        FR_NO_FILESYSTEM,   /* 11 */
        FR_INVALID_OBJECT,  /* 12 */
        FR_MKFS_ABORTED     /* 13 */
    };

    XFiles::Result ret = XFiles::UnknownError;
    switch ((FRESULT)fresult)
    {
    case (FR_OK):
        ret = XFiles::Ok;
        break;
    case (FR_NOT_READY):
        ret = XFiles::NotReady;
        break;
    case (FR_NO_FILE):
        ret = XFiles::NoFile;
        break;
    case (FR_NO_PATH):
        ret = XFiles::NoPath;
        break;
    case (FR_INVALID_NAME):
        ret = XFiles::InvalidName;
        break;
    case (FR_INVALID_DRIVE):
        ret = XFiles::InvalidDrive;
        break;
    case (FR_DENIED):
        ret = XFiles::Denied;
        break;
    case (FR_EXIST):
        ret = XFiles::Exist;
        break;
    case (FR_RW_ERROR):
        ret = XFiles::RWError;
        break;
    case (FR_WRITE_PROTECTED):
        ret = XFiles::WriteProtected;
        break;
    case (FR_NOT_ENABLED):
        ret = XFiles::NotEnabled;
        break;
    case (FR_NO_FILESYSTEM):
        ret = XFiles::NoFilesysytem;
        break;
    case (FR_INVALID_OBJECT):
        ret = XFiles::InvalidObject;
        break;
    case (FR_MKFS_ABORTED):
        ret = XFiles::MkfsAborted;
        break;
    default:
        ret = XFiles::UnknownError;
        break;
    }
    return ret;
}

//=============================================================================
// Служебный класс
//=============================================================================
class XFilesPrivate
{
    friend class XFiles;

    XFiles::Result query(const QByteArray &req, QByteArray &ans,
                         QByteArray &result, int res_offset = 0);

    AbstractSerialPort *port;
    int node;
    int packet_data_length;
};

//=============================================================================
// Запрос/ответ
//=============================================================================
XFiles::Result XFilesPrivate::query(const QByteArray &req, QByteArray &ans,
                                    QByteArray &result, int res_offset)
{
    XFiles::Result ret = XFiles::Ok;
    int j, tr = 4;
    int req_size = req.size();

    while (--tr)
    {
        Console::Print(Console::ModbusPacket,
                       "# Req: " + QByteArray2QString(req) + "\n");
        j = port->query(req, ans);
        QByteArray zzz = ans;
        zzz.resize(j);
        Console::Print(Console::ModbusPacket,
                       "# Ans: " + QByteArray2QString(zzz) + "\n");

        if (j > 2 && ans[1] & 0x80)
        {
            Console::Print(Console::ModbusError,
                           "# Ans: Error flag in answer.\n");
            ret = XFiles::ErrorFlagInAnswer;
            continue;
            // return XFiles::ErrorFlagInAnswer;
        }

        if (j != ans.size())
        {
            if (!j)
            {
                ret = XFiles::NoAnswer;
                Console::Print(Console::ModbusError,
                               "# Ans: Error. No answer.\n");
            } else
            {
                ret = XFiles::WrongAnswerLength;
                Console::Print(Console::ModbusError,
                               "# Ans: Error. Wrong answer length.\n");
            }
            continue;
        }
        if (CRC::CRC16(ans))
        {
            ret = XFiles::CRCError;
            Console::Print(Console::ModbusError, "# Ans: CRC Error.\n");
            continue;
        }
        if (!res_offset) res_offset = req_size - 2;
        if (ans.left(res_offset) != req.left(res_offset))
        {
            ret = XFiles::AnswerNotMatch;
            Console::Print(Console::ModbusError,
                           "# Ans: Error. Answer does not match request.\n");
            continue;
        }
        result = ans.mid(res_offset, j - res_offset + 2);
        ret = result.size() ? fromFResult(result[0]) : ret;
        break;
    }
    return ret;
}

//=============================================================================
//
//=============================================================================
XFiles::XFiles()
    : d(new XFilesPrivate)
{
    d->port = 0;
}

//=============================================================================
//
//=============================================================================
XFiles::~XFiles()
{
    delete d;
}

//=============================================================================
//! Привязка к транспорту.
//
//! \param port транспорт
//=============================================================================
void XFiles::setTransport(AbstractSerialPort *port)
{
    d->port = port;
}

//=============================================================================
//! Задать узел
//
//! \param node номер узла
//=============================================================================
void XFiles::setNode(int node)
{
    d->node = node;
}

//=============================================================================
//! Открыть каталог
//
//! \param path путь
//! \param[out] id дескриптор каталога
//! \return код завершения операции
//=============================================================================
XFiles::Result XFiles::openDirectory(const QString &path, int *id)
{
    Result ret;
    int dir_size, req_size;

    QByteArray dir = ("0:" + path).toLocal8Bit();
    if (path != "/" && dir.endsWith('/')) dir.chop(1);
    dir.append((char)0);
    dir_size = dir.size();

    if (dir_size > 200) return InvalidName;

    Console::Print(Console::Information, "# XFILES 40 - Open a Directory.\n");

    QByteArray req, ans, res;
    req.resize(5);
    req[0] = d->node;
    req[1] = 0x45;
    req[2] = 40;
    req[3] = 10;
    req[4] = 0;
    req[5] = dir_size;
    req.append(dir);
    CRC::appendCRC16(req);

    req_size = req.size();
    ans.resize(req_size + 2);

    ret = d->query(req, ans, res);

    if (ret != Ok) return ret;
    if (id) (*id) = res[1];
    return ret;
}

//=============================================================================
//! Прочесть содержимое каталога
//
//! \param id дескриптор каталога
//! \param[out] fi_list список содержимого каталога
//! \return код завершения операции
//=============================================================================
XFiles::Result XFiles::readDirectory(int id, QList<XFilesFileInfo> &fi_list)
{
    Result ret;
    XFilesFileInfo fi;

#include "packed_struct_begin.h"
    struct PACKED_STRUCT_HEADER
    {
        int size;
        short date;
        short time;
        char attr;
        char filenamesize;
        char filename[13];
    } st;
#include "packed_struct_end.h"

    Console::Print(Console::Information, "# XFILES 41 - Read a Directory.\n");

    QByteArray req, ans, res;

    req.resize(4);
    req[0] = d->node;
    req[1] = 0x45;
    req[2] = 41;
    req[3] = id;
    CRC::appendCRC16(req);
    ans.resize(30);

    fi_list.clear();
    while (1)
    {
        ret = d->query(req, ans, res);
        if (ret != XFiles::Ok) break;
        memcpy(&st, res.constData() + 1, sizeof(st));
        fi.name = QString::fromLocal8Bit(st.filename);
        fi.size = st.size;
        fi.attr = st.attr;
        fi.mtime
            = QDateTime(QDate(1980 + ((st.date >> 9) & 0x7F),
                              ((st.date >> 5) & 0x0F), ((st.date) & 0x1F)),
                        QTime(((st.time >> 11) & 0x1F), ((st.time >> 5) & 0x2F),
                              ((st.time) & 0x1F) * 2));

        if (fi.name.isEmpty()) break;
        fi_list << fi;
    }

    if (ret != XFiles::Ok) fi_list.clear();
    return ret;
}

//=============================================================================
//! Открыть файл
//
//! \param filename имя файла с полным путем
//! \param mode режим доступа
//! \param[out] id дескриптор файла
//! \return код завершения операции
//=============================================================================
XFiles::Result XFiles::openFile(const QString &filename, int mode, int *id)
{
    XFiles::Result ret;
    QByteArray path = filename.toLocal8Bit();
    path.append((char)0);
    int path_size = path.size();

    if (path_size > 100) return XFiles::InvalidName;

    Console::Print(Console::Information,
                   "# XFILES 33 - Open/Create a File." + filename + "\n");

    QByteArray req, ans, res;
    req.resize(5);
    req[0] = d->node;
    req[1] = 0x45;
    req[2] = 33;
    req[3] = 10;
    req[4] = mode;
    req[5] = path_size;
    req.append(path);
    CRC::appendCRC16(req);

    int req_size = req.size();
    ans.resize(req_size + 2);

    ret = d->query(req, ans, res);
    if (ret == XFiles::Ok && id) (*id) = res[1];
    return ret;
}

//=============================================================================
//! Прочитать из файла
//
//! \param id      дескриптор файла
//! \param offset  смещение относительно начала файла
//! \param len     длина данных
//! \param[out] ba приемник данных
//! \return код завершения операции
//=============================================================================
XFiles::Result XFiles::readFile(int id, int offset, int len, QByteArray &ba)
{
    Console::Print(Console::Information, "# XFILES 35 - Read File.\n");

    XFiles::Result ret;
    QByteArray req, ans, res;
    req.resize(9);
    req[0] = d->node;
    req[1] = 0x45;
    req[2] = 35;
    req[3] = id;
    req[4] = offset;
    req[5] = offset >> 8;
    req[6] = offset >> 16;
    req[7] = offset >> 24;
    req[8] = len;
    CRC::appendCRC16(req);

    int req_size = req.size();
    ans.resize(req_size + 2 + len);

    ret = d->query(req, ans, res);
    if (ret != XFiles::Ok) return ret;
    int real_len = (unsigned char)res[1];
    res.remove(0, 2);
    res.resize(real_len);
    ba = res;
    return ret;
}

//=============================================================================
//! Записать в файл
//
//! \param id        дескриптор файла
//! \param offset    смещение относительно начала файла
//! \param ba        источник данных
//! \param[out] wlen количество реально записанных байт
//! \return код завершения операции
//=============================================================================
XFiles::Result XFiles::writeFile(int id, int offset, const QByteArray &ba,
                                 int *wlen)
{
    Console::Print(Console::Information, "# XFILES 36 - Write File.\n");

    int len = ba.size();
    if (len < 0 || len > 200) return XFiles::WrongDataLength;

    XFiles::Result ret;

    QByteArray req, ans, res;
    req.resize(9);
    req[0] = d->node;
    req[1] = 0x45;
    req[2] = 36;
    req[3] = id;
    req[4] = offset;
    req[5] = offset >> 8;
    req[6] = offset >> 16;
    req[7] = offset >> 24;
    req[8] = len;
    req.append(ba);
    CRC::appendCRC16(req);

    ans.resize(13);

    ret = d->query(req, ans, res, 9);
    if (ret != XFiles::Ok) return ret;
    if (wlen) (*wlen) = (unsigned char)res[1];
    return ret;
}

//=============================================================================
//! Получить информацию о файле
//
//! \param filename   полное имя файла
//! \param[out] fi    информация о файле
//! \return код завершения операции
//=============================================================================
XFiles::Result XFiles::getFileInfo(const QString &filename, XFilesFileInfo &fi)
{
    Result ret;

    QByteArray path = filename.toLocal8Bit();
    path.append((char)0);
    int path_size = path.size();

    if (path_size > 100) return XFiles::InvalidName;

#include "packed_struct_begin.h"
    struct PACKED_STRUCT_HEADER
    {
        int size;
        short date;
        short time;
        char attr;
        char filenamesize;
        char filename[13];
    } st;
#include "packed_struct_end.h"

    Console::Print(Console::Information, "# XFILES 43 - Get File Status.\n");

    QByteArray req, ans, res;

    req.resize(4);
    req[0] = d->node;
    req[1] = 0x45;
    req[2] = 43;
    req[3] = path_size;
    req.append(path);
    CRC::appendCRC16(req);
    ans.resize(req.size() + 1 + sizeof(st));

    ret = d->query(req, ans, res);
    if (ret != XFiles::Ok) return ret;
    memcpy(&st, res.constData() + 1, sizeof(st));
    fi.name = QString::fromLocal8Bit(st.filename);
    fi.size = st.size;
    fi.attr = st.attr;
    fi.mtime
        = QDateTime(QDate(1980 + ((st.date >> 9) & 0x7F),
                          ((st.date >> 5) & 0x0F), ((st.date) & 0x1F)),
                    QTime(((st.time >> 11) & 0x1F), ((st.time >> 5) & 0x2F),
                          ((st.time) & 0x1F) * 2));

    return ret;
}

//=============================================================================
//! Закрыть файл
//
//! \param id   дескриптор файла
//! \return код завершения операции
//=============================================================================
XFiles::Result XFiles::closeFile(int id)
{
    Console::Print(Console::Information, "# XFILES 34 - Close a File.\n");
    XFiles::Result ret;
    QByteArray req, ans, res;
    req.resize(4);
    req[0] = d->node;
    req[1] = 0x45;
    req[2] = 34;
    req[3] = id;
    CRC::appendCRC16(req);

    int req_size = req.size();
    ans.resize(req_size + 1);

    ret = d->query(req, ans, res);
    return ret;
}

//=============================================================================
//! Удалить файл
//
//! \param path имя файла с полным путем
//! \return код завершения операции
//=============================================================================
XFiles::Result XFiles::removeFile(const QString &path)
{
    int path_ba_size, req_size;
    XFiles::Result ret;

    QByteArray path_ba = path.toLocal8Bit();
    path_ba.append((char)0);
    path_ba_size = path_ba.size();

    if (path_ba_size > 100) return XFiles::InvalidName;
    ;

    Console::Print(Console::Information,
                   "# XFILES 45 - Remove a File or Directory.\n");

    QByteArray req, ans, res;
    req.resize(4);
    req[0] = d->node;
    req[1] = 0x45;
    req[2] = 45;
    req[3] = path_ba_size;
    req.append(path_ba);
    CRC::appendCRC16(req);

    req_size = req.size();
    ans.resize(req_size + 1);

    ret = d->query(req, ans, res);
    return ret;
}

//=============================================================================
//! Создать каталог
//
//! \param path полный путь
//! \return код завершения операции
//=============================================================================
XFiles::Result XFiles::createDirectory(const QString &path)
{
    int path_ba_size, req_size;
    XFiles::Result ret;

    QByteArray path_ba = path.toLocal8Bit();
    path_ba.append((char)0);
    path_ba_size = path_ba.size();

    if (path_ba_size > 100) return XFiles::InvalidName;

    Console::Print(Console::Information, "# XFILES 45 - Create a Directory.\n");

    QByteArray req, ans, res;
    req.resize(4);
    req[0] = d->node;
    req[1] = 0x45;
    req[2] = 44;
    req[3] = path_ba_size;
    req.append(path_ba);
    CRC::appendCRC16(req);

    req_size = req.size();
    ans.resize(req_size + 1);

    ret = d->query(req, ans, res);
    return ret;
}

//=============================================================================
//! Получить название ошибки по ее коду
//
//! \param result ошибка
//! \return строка с названием ошибки
//=============================================================================
QString XFiles::decodeResult(Result result)
{
    switch (result)
    {
    case (Ok):
        return "OK";
    case (NotReady):
        return "NotReady";
    case (NoFile):
        return "NoFile";
    case (NoPath):
        return "NoPath";
    case (InvalidName):
        return "InvalidName";
    case (InvalidDrive):
        return "InvalidDrive";
    case (Denied):
        return "Denied";
    case (Exist):
        return "Exist";
    case (RWError):
        return "RWError";
    case (WriteProtected):
        return "WriteProtected";
    case (NotEnabled):
        return "NotEnabled";
    case (NoFilesysytem):
        return "NoFilesysytem";
    case (InvalidObject):
        return "InvalidObject";
    case (MkfsAborted):
        return "MkfsAborted";
    case (NoAnswer):
        return "NoAnswer";
    case (UnknownError):
        return "UnknownError";
    case (WrongDataLength):
        return "WrongDataLength";
    case (WrongAnswerLength):
        return "WrongAnswerLength";
    case (ErrorFlagInAnswer):
        return "ErrorFlagInAnswer";
    case (AnswerNotMatch):
        return "AnswerNotMatch";
    case (CRCError):
        return "CRCError";
    default:
        break;
    }
    return "Unknown result";
}
