#ifndef __misk__h_
#define __misk__h_

#include "mmpm_types.h"

WORD get_crc16(BYTE *ptr, DWORD DataLen);

void make_crc32table();
DWORD get_crc32(BYTE *ptr, DWORD len);
DWORD update_crc32(DWORD crc32, BYTE b);
WORD irnd(WORD value);

#endif
