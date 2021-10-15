#include "modbus.h"
#include "main.h"
#include "misc.h"

#include "console.h"
#include "global_exit_manager.h"
#include "mbtcp.h"
#include "serialport.h"

//==============================================================================
// Modbus Master
//==============================================================================
ModbusMaster::ModbusMaster(QObject *parent)
    : QObject(parent)
{
    req_counter = 0;
    ans_counter = 0;
    err_counter = 0;
    func_number = 0x43;
    single_mode = false;
    subnode = 0;

    serialport = new SerialPort;
}

//==============================================================================
// Modbus Master
//==============================================================================
ModbusMaster::~ModbusMaster()
{
    delete serialport;
}

//==============================================================================
// Modbus Master -- настройка COM порта
//==============================================================================
int ModbusMaster::setupPort(const QString &name, int speed)
{
    delete serialport;
    serialport = new SerialPort;

    if (!name.isEmpty()) portname = name;
    if (speed) portspeed = speed;

    serialport->setName(portname);
    serialport->setSpeed(speed);

    return 1;
}

//==============================================================================
// Modbus Master -- настройка ModbusTCP порта
//==============================================================================
int ModbusMaster::setupTcpHost(const QString &host)
{
    delete serialport;
    serialport = new MbTcpPort;

    if (!host.isEmpty()) portname = host;
    serialport->setName(host);

    return 1;
}

//==============================================================================
// Modbus Master -- установка тайм-аутов
//==============================================================================
void ModbusMaster::setupTimeOut(int timeout)
{
    serialport->setAnswerTimeout(timeout);
}

//==============================================================================
// Modbus Master -- запрос/ответ
//==============================================================================
int ModbusMaster::go()
{
    QString str;
    int attempts = 0;

    if (req_len >= 2)
    {
        qint16 crc16 = get_crc16(req, req_len - 2);
        req[req_len - 2] = crc16;
        req[req_len - 1] = crc16 >> 8;
    }
    if ((req[2] & 0x0F) == 9) { attempts = 100; }

    QByteArray ba_req
        = QByteArray((char *)req, qMin((size_t)req_len, sizeof(req)));
    QByteArray ba_ans;
    ba_ans.resize(ans_expect_len);

    while (attempts++ < 50)
    {
        GlobalExitManager::instance().check();

        req_counter++;
        memset(ans, 0, sizeof(ans));
        ba_ans.fill(0);
        ans_len = serialport->query(ba_req, ba_ans);
        memcpy(ans, ba_ans.constData(), qMin((size_t)ans_len, sizeof(ans)));

        if (ans_len) ++ans_counter;

        //-------------------------
        if (ans_len && (get_crc16(ans, ans_len) == 0) && (req[0] == ans[0])
            && (req[1] == 0x45) && (ans[1] == 0xC5) && (req[2] == 0x07)
            && (ans[2] == 0x07))
        {
            continue;
        }
        //-------------------------
        if (ans_len && (get_crc16(ans, ans_len) == 0) && (ans[0] == req[0])
            && (ans[1] == req[1]) && (ans[2] == req[2]))
        {
            return 1;
        } else
        {
            Console::Print(Console::ModbusError,
                           tr("  Ошибка пакета (попытка %1)\n").arg(attempts));
        }
        err_counter++;
        if (single_mode) return 0;
    }

    if ((req[2] & 0x0F) != 9)
    {
        Console::Print(Console::Information, tr("  ОШИБКА СВЯЗИ!"));
    }
    return 0;
}


//==============================================================================
// Modbus Master -- массив ==> строка шестнадцатеричных чисел
//==============================================================================
void ModbusMaster::setArray(QString &str, BYTE *data, DWORD len)
{
    DWORD i;
    QString s;

    str = "";
    for (i = 0; i < len; i++)
    {
        s = QString::asprintf(" %2.2X", *(data + i));
        str = str + s;
    }
}

//==============================================================================
// Modbus Master -- функция 0x43 -- чтение данных
//==============================================================================
int ModbusMaster::func_43_00_read(BYTE node, WORD addr, BYTE len, BYTE *ptr,
                                  bool slowMode)
{
    req[0] = node;
    req[1] = 0x43;
    if (slowMode) req[1] = 0x41;
    req[2] = 0x00 | (subnode << 4);
    req[3] = addr >> 8;
    req[4] = addr;
    req[5] = len;
    req_len = 8;
    ans_expect_len = (DWORD)(6 + len);
    if (!go()) return 0;
    // Console::Print( tr("  ModBus: %1 %2 \n").arg(len).arg(ans_len));
    if (ans_len != (DWORD)(6 + len)) return 0;
    memcpy(ptr, &ans[4], len);
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x43 -- запись данных
//==============================================================================
int ModbusMaster::func_43_01_set(BYTE node, BYTE *ptr, WORD addr, BYTE len,
                                 int mode)
{
    req[0] = node;
    req[1] = func_number;
    req[2] = (subnode << 4);
    switch (mode)
    {
    case (1):
        req[2] |= 0x03;
        break; // and
    case (2):
        req[2] |= 0x05;
        break; // or
    case (3):
        req[2] |= 0x07;
        break; // xor
    default:
        req[2] |= 0x01;
        break; // set
    }
    req[3] = addr >> 8;
    req[4] = addr;
    req[5] = len;
    memcpy(&req[6], ptr, len);
    req_len = 8 + len;
    ans_expect_len = (DWORD)(6 + len);
    if (!go()) return 0;
    if (ans[3] != len) return 0;
    if (ans_len != (DWORD)(6 + len)) return 0;
    memcpy(ptr, &ans[4], len);
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x43 -- запись данных по AND
//==============================================================================
int ModbusMaster::func_43_02_set_and(BYTE node, BYTE *ptr, WORD addr, BYTE len)
{
    return func_43_01_set(node, ptr, addr, len, 1);
}

//==============================================================================
//  Modbus Master -- функция 0x43 -- запись данных по OR
//==============================================================================
int ModbusMaster::func_43_03_set_or(BYTE node, BYTE *ptr, WORD addr, BYTE len)
{
    return func_43_01_set(node, ptr, addr, len, 2);
}

//==============================================================================
//  Modbus Master -- функция 0x43 -- запись данных по XOR
//==============================================================================
int ModbusMaster::func_43_04_set_xor(BYTE node, BYTE *ptr, WORD addr, BYTE len)
{
    return func_43_01_set(node, ptr, addr, len, 3);
}

//==============================================================================
// Modbus Master -- функция 0x45 -- запрос на RESET
//==============================================================================
int ModbusMaster::func_45_00_reset_req(BYTE node, DWORD *code)
{
    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x00 | (subnode << 4);
    req_len = 5;
    ans_expect_len = 9;
    if (!go()) return 0;
    if (ans_len != 9) return 0;
    memcpy(code, &ans[3], sizeof(*code));
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x45 -- ответ на запрос по RESET
//==============================================================================
int ModbusMaster::func_45_01_reset_ans(BYTE node, DWORD code)
{
    int i;

    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x01 | (subnode << 4);
    memcpy(&req[3], &code, sizeof(DWORD));
    req_len = 9;
    ans_expect_len = 9;
    if (!go()) return 0;
    if (ans_len != 9) return 0;
    for (i = 0; i < 9; i++)
        if (req[i] != ans[i]) return 0;
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x45 -- запрос на ввод пароля
//==============================================================================
int ModbusMaster::func_45_02_passw_req(BYTE node, DWORD *passw)
{
    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x02 | (subnode << 4);
    req_len = 5;
    ans_expect_len = 9;
    if (!go()) return 0;
    if (ans_len != 9) return 0;
    memcpy(passw, &ans[3], sizeof(*passw));
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x45 -- ответ на запрос на ввод пароля
//==============================================================================
int ModbusMaster::func_45_03_passw_ans(BYTE node, DWORD passw)
{
    int i;

    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x03 | (subnode << 4);
    memcpy(&req[3], &passw, sizeof(DWORD));
    req_len = 9;
    ans_expect_len = 9;
    if (!go()) return 0;
    if (ans_len != 9) return 0;
    for (i = 0; i < 9; i++)
        if (req[i] != ans[i]) return 0;
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x45 -- чтение микропрограммы из FLASH модуля
//==============================================================================
int ModbusMaster::func_45_04_flash_read(BYTE node, DWORD addr, BYTE len,
                                        BYTE *ptr)
{
    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x04 | (subnode << 4);
    memcpy(&req[3], &addr, sizeof(DWORD));
    req[7] = len;
    req_len = 10;
    ans_expect_len = (DWORD)(6 + len);
    if (!go()) return 0;
    if (ans_len != (DWORD)(6 + len)) return 0;
    if (ans[3] != len) return 0;
    memcpy(ptr, &ans[4], len);
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x45 -- стирание секторов FLASH модуля
//==============================================================================
int ModbusMaster::func_45_05_flash_erase(BYTE node, DWORD sector_begin,
                                         DWORD sector_end)
{
    int i;

    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x05 | (subnode << 4);
    memcpy(&req[3], &sector_begin, sizeof(DWORD));
    memcpy(&req[7], &sector_end, sizeof(DWORD));
    req_len = 13;
    ans_expect_len = 13;
    if (!go()) return 0;
    if (ans_len != 13) return 0;
    for (i = 0; i < 13; i++)
        if (req[i] != ans[i]) return 0;
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x45 -- запись микропрограммы во временный буфер
//==============================================================================
int ModbusMaster::func_45_06_buffer_write(BYTE node, BYTE *ptr, WORD addr,
                                          BYTE len)
{
    int i;

    // Console::Print( tr("<буфер ptr=%1 addr=%2 len=%3> ").arg((int)ptr,
    // 8,16,QChar('0') )
    //                                                 .arg(addr,
    //                                                 8,16,QChar('0') )
    //                                                 .arg(len, 8,16,QChar('0')
    //                                                 ));

    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x06 | (subnode << 4);
    req[3] = addr;
    req[4] = addr >> 8;
    req[5] = len;
    memcpy(&req[6], ptr, len);
    req_len = 8 + len;
    ans_expect_len = (DWORD)(6 + len);
    if (!go()) return 0;
    if (ans_len != (DWORD)(len + 6)) return 0;
    if (ans[3] != len) return 0;
    for (i = 0; i < len; i++)
        if (req[i + 6] != ans[i + 4]) return 0;
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x45 -- запись микропрограммы в FLASH модуля из
//                                  временного буфера
//==============================================================================
int ModbusMaster::func_45_07_flash_write(BYTE node, DWORD addr, DWORD len)
{
    int i;

    // Console::Print( tr("<пишем сектор addr=%1 len=%2> ").arg(addr,
    // 8,16,QChar('0') )
    //                                                 .arg(len, 8,16,QChar('0')
    //                                                 ));

    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x07 | (subnode << 4);
    memcpy(&req[3], &addr, sizeof(DWORD));
    memcpy(&req[7], &len, sizeof(DWORD));
    req_len = 13;
    ans_expect_len = 13;
    if (!go()) return 0;
    if (ans_len != 13) return 0;
    for (i = 0; i < 13; i++)
        if (req[i] != ans[i]) return 0;
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x45 -- получение информации о модуле
//==============================================================================
int ModbusMaster::func_45_08_module_info(BYTE node, BYTE *ptr, DWORD len)
{
    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x08 | (subnode << 4);
    req[3] = 128;
    req_len = 6;
    ans_expect_len = (DWORD)(6 + 128);
    if (!go()) return 0;
    if (ans_len != (DWORD)(6 + 128)) return 0;
    if (ans[3] != 128) return 0;
    memcpy(ptr, &ans[4], len);
    return 1;
}

//==============================================================================
// Modbus Master -- функция 0x45 -- перезапись загрузчика
//==============================================================================
int ModbusMaster::func_45_09_loader_change(BYTE node, WORD crc16, DWORD len)
{
    req[0] = node;
    req[1] = 0x45;
    req[2] = 0x09 | (subnode << 4);
    req[3] = crc16;
    req[4] = crc16 >> 8;
    req[5] = 0;
    req[6] = 0;
    req[7] = len;
    req[8] = len >> 8;
    req[9] = len >> 16;
    req[10] = len >> 24;
    req_len = 13;
    ans_expect_len = (DWORD)(6 + 13);
    if (!go()) return 1;
    return 0;
}
