#ifndef __thread__h_
#define __thread__h_

#include "mmpm_types.h"

#include <QThread>

//==============================================================================
// Поток выполнения команд
//==============================================================================
class Thread : public QThread
{
public:
    Thread();
    void setupMaxPacketSize(int max_packet_size);
    int mode;
    BYTE node;
    DWORD erase_delay;

private:
    void run();
    int mb_reset();
    int mb_passwd();
    int mb_load_flash();
    int mb_verify_flash();
    int mb_write_flash();
    int mb_erase_flash();
    int file_save();
    int file_load();
    int file_load_raw_bin(const QString &filename);
    int file_load_loader_bin();
    int mb_read_information();
    int mb_load_module_type();
    int mb_check_module_type();
    int mb_loader_change();
    int mb_download_firmware();

    size_t buffer_not_empty_size() const;

    int loader_bin_length;
    BYTE buffer[1024 * 1024];
    BYTE info_buffer[128];
    BYTE file_info_buffer[1024];

    DWORD buffer_len;
    DWORD flash_begin_ptr;
    DWORD flash_end_ptr;
    DWORD flash_sector_size;
    DWORD flash_buff_limit;
    DWORD max_packet_size;
};

#endif
