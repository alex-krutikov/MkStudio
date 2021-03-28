#ifndef __modbus_h__
#define __modbus_h__

#include "mmpm_types.h"

#include <QObject>

class AbstractSerialPort;
class QString;

//==============================================================================
// Modbus Master
//==============================================================================
class ModbusMaster : public QObject
{
    friend class MainWindow;

public:
    ModbusMaster(QObject *parent = 0);
    virtual ~ModbusMaster();
    int setupPort(const QString &name = QString(), int speed = 0);
    int setupTcpHost(const QString &host);
    void setupTimeOut(int timeout);
    inline QString port() const { return portname; }

    int func_43_00_read(BYTE node, WORD addr, BYTE len, BYTE *ptr,
                        bool slowMode = false);
    int func_43_01_set(BYTE node, BYTE *ptr, WORD addr, BYTE len, int mode = 0);
    int func_43_02_set_and(BYTE node, BYTE *ptr, WORD addr, BYTE len);
    int func_43_03_set_or(BYTE node, BYTE *ptr, WORD addr, BYTE len);
    int func_43_04_set_xor(BYTE node, BYTE *ptr, WORD addr, BYTE len);

    int func_45_00_reset_req(BYTE node, DWORD *code);
    int func_45_01_reset_ans(BYTE node, DWORD code);
    int func_45_02_passw_req(BYTE node, DWORD *passw);
    int func_45_03_passw_ans(BYTE node, DWORD passw);
    int func_45_04_flash_read(BYTE node, DWORD addr, BYTE len, BYTE *ptr);
    int func_45_05_flash_erase(BYTE node, DWORD sector_begin, DWORD sector_end);
    int func_45_06_buffer_write(BYTE node, BYTE *ptr, WORD addr, BYTE len);
    int func_45_07_flash_write(BYTE node, DWORD addr, DWORD len);
    int func_45_08_module_info(BYTE node, BYTE *ptr, DWORD len);
    int func_45_09_loader_change(BYTE node, WORD crc16, DWORD len);

    BYTE func_number;
    bool single_mode;

    DWORD req_counter;
    DWORD ans_counter;
    DWORD err_counter;

    BYTE subnode;

private:
    int go();
    void setArray(QString &str, BYTE *data, DWORD len);

    QString portname;
    int portspeed;
    AbstractSerialPort *serialport;

    BYTE req[384];
    BYTE ans[384];
    DWORD req_len;
    DWORD ans_len;
    DWORD ans_expect_len;
};

#endif
