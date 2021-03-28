#ifndef __XFILES_H__
#define __XFILES_H__

#include "mbl_global.h"

#include <QDateTime>
#include <QList>
#include <QString>

class XFilesPrivate;
class AbstractSerialPort;
class QByteArray;
//=============================================================================
//
//=============================================================================
struct MBL_EXPORT XFilesFileInfo
{
    enum
    {
        AM_RDO = 0x01, // Read only
        AM_HID = 0x02, // Hidden
        AM_SYS = 0x04, // System
        AM_VOL = 0x08, // Volume label
        AM_LFN = 0x0F, // LFN entry
        AM_DIR = 0x10, // Directory
        AM_ARC = 0x20, // Archive
    };

    QString name;
    int size;
    int attr;
    QDateTime mtime;
    bool isDir() const { return ((attr & AM_DIR) != 0); }
};

//=============================================================================
//! Работа с файловой системой микроконтроллеров по протоколу XFILES
/**
    \ingroup API_group
*/
//=============================================================================
class MBL_EXPORT XFiles
{
public:
    //! Коды ошибок
    enum Result
    {
        Ok = 0,
        NotReady,
        NoFile,
        NoPath,
        InvalidName,
        InvalidDrive,
        Denied,
        Exist,
        RWError,
        WriteProtected,
        NotEnabled,
        NoFilesysytem,
        InvalidObject,
        MkfsAborted,
        UnknownError,
        WrongDataLength,
        NoAnswer,
        WrongAnswerLength,
        ErrorFlagInAnswer,
        AnswerNotMatch,
        CRCError
    };

    enum
    {
        FA_READ = 0x01,
        FA_OPEN_EXISTING = 0x00,
        FA_WRITE = 0x02,
        FA_CREATE_NEW = 0x04,
        FA_CREATE_ALWAYS = 0x08,
        FA_OPEN_ALWAYS = 0x10,
    };

    XFiles();
    virtual ~XFiles();

    /** @name Настройка
     *
     */
    //@{
    void setTransport(AbstractSerialPort *port);
    void setNode(int node);
    //@}

    /** @name Работа с файловой системой
     *
     */
    //@{
    Result openDirectory(const QString &path, int *id);
    Result readDirectory(int id, QList<XFilesFileInfo> &fi_list);
    Result openFile(const QString &filename, int mode, int *id);
    Result readFile(int id, int offset, int len, QByteArray &ba);
    Result writeFile(int id, int offset, const QByteArray &ba, int *wlen);
    Result getFileInfo(const QString &filename, XFilesFileInfo &fi);
    Result closeFile(int id);
    Result removeFile(const QString &path);
    Result createDirectory(const QString &path);
    //@}
    static QString decodeResult(Result result);

private:
    XFilesPrivate *d;
};

#endif
