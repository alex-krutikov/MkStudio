#include <QtGui>

#include "main.h"
#include "mainwindow.h"
#include "thread.h"
#include "misc.h"

#include "console.h"

namespace {

//==============================================================================
// format int ==> 0x00000000
//==============================================================================
QString toHex( DWORD i )
{
  QString str;
  str.sprintf("0x%08X", (int)i );
  return str;
}

//==============================================================================
// Блокирующее исполнение функтора в главном потоке приложения
//==============================================================================
template <typename F>
static void execInMainThread(F &&fun)
{
   QObject src;
   QObject::connect(&src, &QObject::destroyed, qApp, std::forward<F>(fun),
                    Qt::BlockingQueuedConnection);
}

} // namespace

//==============================================================================
// Поток выполнения команд
//==============================================================================
Thread::Thread()
{
  node                  = 0;
  flash_begin_ptr       = 0;
  flash_end_ptr         = 0;
  flash_sector_size     = 0;
  flash_buff_limit      = 0;
  erase_delay           = 500;
  loader_bin_length     = 0;
  max_packet_size       = 128;
}

//==============================================================================
// Modbus Master -- установка максимальной длины пакета
//==============================================================================
void Thread::setupMaxPacketSize( int max_packet_size )
{

  if( max_packet_size < 32 )  max_packet_size=32;
  if( max_packet_size > 255 ) max_packet_size=255;

  this->max_packet_size = max_packet_size;
}

//==============================================================================
// Поток выполнения команд -- точка входа в нить
//==============================================================================
void Thread::run()
{
  setTerminationEnabled(true);
  loader_bin_length = 0;
  switch( mode )
  { case(1): // чтение из модуля
      if( !mb_reset()        ) break;
      if( !mb_load_module_type() ) break;
      if( !mb_passwd()       ) break;
      if( !mb_load_flash()   ) break;
      if( !mb_verify_flash() ) break;
      if( !mb_reset()        ) break;
      if( !file_save()       ) break;
      Console::Print( Console::Information, "\nЧтение микропрограммы завершилась успешно.\n");
      break;
    case(2): // прошиваем модуль
      if( !file_load()       ) break;
      if( !mb_reset()        ) break;
      if( !mb_load_module_type()  ) break;
      if( !mb_check_module_type() ) break;
      if( !mb_passwd()       ) break;
      if( !mb_write_flash()  ) break;
      if( !mb_verify_flash() ) break;
      if( !mb_reset()        ) break;
      Console::Print(Console::Information, "\nЗапись микропрограммы завершилась успешно.\n");
      break;
    case(3): // RESET
      if( !mb_reset()             ) break;
      if( !mb_check_module_type() ) break;
      break;
    case(4): // информация о модуле
      if( !mb_read_information() ) break;
      break;
    case(5): // проверка модуля
      if( !file_load()       ) break;
      if( !mb_reset()        ) break;
      if( !mb_load_module_type()  ) break;
      if( !mb_check_module_type() ) break;
      if( !mb_passwd()       ) break;
      if( !mb_verify_flash() ) break;
      if( !mb_reset()        ) break;
      Console::Print( Console::Information, "\nПроверка микропрограммы завершилась успешно.\n");
      break;
    case(6): // стереть модуль
      if( !mb_reset()        ) break;
      if( !mb_load_module_type()  ) break;
      //if( !mb_check_module_type() ) break;
      if( !mb_passwd()       ) break;
      if( !mb_erase_flash()  ) break;
      if( !mb_reset()        ) break;
      Console::Print( Console::Information, "\nМикропрограмма стерта.\n");
      break;
    case(7): // прошиваем загрузчик
      if( !file_load_loader_bin() ) break;
      if( !mb_reset()             ) break;
      if( !mb_load_module_type()  ) break;
      if( !mb_check_module_type() ) break;
      if( !mb_passwd()            ) break;
      if( !mb_write_flash()       ) break;
      if( !mb_verify_flash()      ) break;
      if( !mb_loader_change()     ) break;
      Console::Print( Console::Information, "\n\nЗапись загрузчика прошла успешно.\n");
      Console::Print( Console::Information, "Для работы модуля необходимо записать в него микропрограмму.\n");
      break;
    case(8): // прошиваем модуль прошивкой из интернета
      if( !mb_reset()             ) break;
      if( !mb_load_module_type()  ) break;
      if( !mb_download_firmware() ) break;
      if( !mb_reset()        ) break;
      if( !mb_load_module_type()  ) break;
      if( !mb_check_module_type() ) break;
      if( !mb_passwd()       ) break;
      if( !mb_write_flash()  ) break;
      if( !mb_verify_flash() ) break;
      if( !mb_reset()        ) break;
      Console::Print(Console::Information, "\nЗапись микропрограммы завершилась успешно.\n");
      break;
  }
  Console::Print( Console::Information, tr("\n\n") );
  mode=0;
}

//==============================================================================
// Поток выполнения команд -- Запись файла с прошивкой
//==============================================================================
int Thread::file_save()
{
  WORD crc16;
  DWORD crc32;
  BYTE temp_buff[1024];
  int i,len;

  const char *s2 = QDateTime::currentDateTime()
                      .toString("yyyyMMddhhmmss").toLatin1();

  Console::Print( Console::Information, tr("Запись файла...") );

  memset( temp_buff, 0, sizeof( temp_buff ));
  memcpy( &temp_buff[ 0], &info_buffer[0], 16 ); // имя модуля
  memcpy( &temp_buff[16], s2, strlen(s2) );
  memcpy( &temp_buff[48], &info_buffer[16], 16 ); // тип процессора
  memcpy( &temp_buff[64], &info_buffer[32], 16 ); // дата реализации
  memcpy( &temp_buff[80], &info_buffer[48], 3*4 ); // начальный и конечный адрес flash
                                                   // размер сектора

// 20060710 ksy
// !!!! ВНИМАНИЕ !!!!
//
// в mmpm-dos добавлено в заголовке:
// temp_buff[48] - [63] - тип процессора, что прочитан функцией 45/8
// (temp_buff[0] - [15] - также пишется имя модуля, что прочитано ф-цией 45/8)
// temp_buff[64] - [79] - дата реализации, что прочитана функцией 45/8
// temp_buff[80] - [83] - начальный адрес flash
// temp_buff[84] - [87] - конечный адрес flash
// temp_buff[88] - [91] - размер сектора flash
//
// все это необходимо учесть при проверке  !!!
//
// !!!!!!!!!!!!!!!!!!

  len = flash_end_ptr-flash_begin_ptr;
  crc16 = get_crc16( buffer + 0x80, len - 0x80 );

  buffer[0x10] = 1;
  buffer[0x11] = crc16;
  buffer[0x12] = crc16 >> 8;

  crc32 = 0xFFFFFFFF;
  for(i=0; i<1024;    i++) crc32 = update_crc32( crc32, temp_buff[i] );
  for(i=0; i<len;     i++) crc32 = update_crc32( crc32, buffer[i]    );

  memcpy(&temp_buff[0x20], &crc32, sizeof(crc32));

  if (QFileInfo(firmware_filename).isFile())
      firmware_filename = QFileInfo(firmware_filename).absolutePath() + QDir::separator();

  if (firmware_filename.isEmpty())
      firmware_filename += QDir().homePath() + QDir::separator();

  QString init_filename = QString::fromLatin1((char*)&info_buffer[0], 16 ).split(QChar('\0')).value(0);
  firmware_filename += init_filename;

  mainwindow->flag4=1; // синхронизация с GUI потоком (см. MainWindow::update() )
  while( mainwindow->flag4 == 1 ) msleep(100);
  QString filename = firmware_filename;

  if( filename.isEmpty() ) return 0;
  if( ! filename.contains(".bin",Qt::CaseInsensitive ) ) filename += ".bin";

  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate ))
  {
    Console::Print( Console::Information, tr("\n\n ОШИБКА! Не могу создать файл!\n") );
    return 0;
  }

  file.write( (const char*)temp_buff, 1024 );
  file.write( (const char*)buffer, buffer_len );
  file.flush();
  file.close();
  Console::Print( Console::Information, tr("OK.\n") );
  return 1;
}

//==============================================================================
//  Поток выполнения команд -- Чтение файла с прошивкой
//==============================================================================
int Thread::file_load()
{
  DWORD i;
  DWORD crc32;
  DWORD crc32_from_file;
  DWORD size;
  QFile file;

  Console::Print( Console::Information, "Чтение файла... \n" );

  memset( buffer, 0, sizeof( buffer ));
  memset( file_info_buffer, 0, sizeof( file_info_buffer ));

  QString filename = firmware_filename;
  mainwindow->flag3=1; // синхронизация с GUI потоком (см. MainWindow::update() )
  while( mainwindow->flag3 == 1 ) msleep(100);

  if( firmware_filename.isEmpty() )
  {
      firmware_filename = filename;
      goto error;
  }

  file.setFileName( firmware_filename );
  if (!file.open(QIODevice::ReadOnly) )
  { Console::Print( Console::Error, tr("\n\n ОШИБКА! Не могу открыть файл!\n") );
    goto error;
  }

  size=file.size();
  if( size <= 1024 )
  { Console::Print( Console::Error, tr("\n\n ОШИБКА! Размер файла меньше 1K!\n") );
    goto error;
  }
  else if( size >= (1024*1024) )
  { Console::Print( Console::Error, tr("\n\n ОШИБКА! Размер файла больше 1000K!\n") );
    goto error;
  }
  buffer_len = size-1024;

  file.read( (char*)file_info_buffer, 1024 );
  file.read( (char*)buffer, buffer_len );
  file.close();

  memcpy(&crc32_from_file, &file_info_buffer[0x20], sizeof(crc32_from_file));
  memset(&file_info_buffer[0x20], 0, sizeof(DWORD));

  crc32 = 0xFFFFFFFF;
  for(i=0; i<1024;       i++) crc32 = update_crc32( crc32, file_info_buffer[i] );
  for(i=0; i<buffer_len; i++) crc32 = update_crc32( crc32, buffer[i]    );
  if( crc32 !=  crc32_from_file )
  { if( file_load_raw_bin( firmware_filename ) == 0 )
    { goto error;
    }
  }

  Console::Print( Console::Information, tr("OK.\n") );
  return 1;

error:
  memset( buffer, 0, sizeof( buffer ));
  memset( file_info_buffer, 0, sizeof( file_info_buffer ));
  return 0;
}

//==============================================================================
// Поток выполнения команд
// Чтение файла с прошивкой из RAW bin-файла прошивки
//==============================================================================
int Thread::file_load_raw_bin( const QString &filename )
{
  // запрос подтверждения пользователя
  int i;
  char c;

  mainwindow->flag2 = 1; // синхронизация с GUI потоком (см. MainWindow::update() )
  while( mainwindow->flag2 == 1 ) msleep(100);
  if( mainwindow->flag2 == 0 ) return 0;

  memset( buffer, 0, sizeof( buffer ));
  memset( file_info_buffer, 0, sizeof( file_info_buffer ));

  QFile file( filename );
  if( !file.open( QIODevice::ReadOnly ) )
  { Console::Print( Console::Error, tr("\n\n ОШИБКА! Не могу открыть файл!\n") );
    return 0;
  }
  if( file.size() > sizeof( buffer ) )
  { Console::Print( Console::Error, tr("\n\n ОШИБКА! Файл слишком большой!\n") );
    return 0;
  }
  if( file.read( (char*)buffer, file.size() ) != file.size() )
  { Console::Print( Console::Error, "\n\n ОШИБКА! Ошибка при чтении файла!\n" );
    return 0;
  }
  file.close();

  Console::Print( Console::Information, "Информация из файла с микропрограммой:\n");
  QString str;
  for(i=0; i<128; i++ )
  {
    c = buffer[i];
    if((c==0)&&(buffer[i+1]==0)) continue;
    if(c==0)
    { str.append("\n  ");
      continue;
    }
    str.append(c);
  }
  Console::Print( Console::Information, "  " + str + "\n");

  memcpy( file_info_buffer, buffer, 16 );

  return 1;
}

//==============================================================================
// Поток выполнения команд
// Чтение файла с прошивкой из RAW bin-файла прошивки загрузчика
//==============================================================================
int Thread::file_load_loader_bin()
{
  Console::Print( Console::Information, "Чтение файла... \n" );

  memset( buffer, 0, sizeof( buffer ));
  memset( file_info_buffer, 0, sizeof( file_info_buffer ));
  loader_bin_length = 0;

  mainwindow->flag3=1;  // синхронизация с GUI потоком (см. MainWindow::update() )
  while( mainwindow->flag3 == 1 ) msleep(100);
  QString filename = firmware_filename;

  if( filename.isEmpty() )
  { Console::Print( Console::Information, "Отмена");
    return 0;
  }

  QFile file( filename );
  if( !file.open( QIODevice::ReadOnly ) )
  { Console::Print( Console::Error, tr("\n\n ОШИБКА! Не могу открыть файл!\n") );
    return 0;
  }
  loader_bin_length = file.size();
  if( file.size() > sizeof( buffer ) )
  { Console::Print( Console::Error, tr("\n\n ОШИБКА! Файл слишком большой!\n") );
    return 0;
  }
  if( file.read( (char*)buffer, file.size() ) != file.size() )
  { Console::Print( Console::Error, "\n\n ОШИБКА! Ошибка при чтении файла!\n" );
    return 0;
  }
  file.close();

  memcpy( file_info_buffer, buffer+0x40, 16 );

  return 1;
}

//==============================================================================
// Поток выполнения команд -- Сброс модуля
//==============================================================================
int Thread::mb_reset()
{
  int i;
  DWORD code;

  Console::Print( Console::Information, tr("Сброс модуля... ") );
  if( ! modbus->func_45_00_reset_req( node, &code ) )
  { Console::Print( Console::Error, tr("Ошибка связи!\n") );
    return 0;
  }
  for( i=0; i<node; i++ )  { code = irnd( code );  }

  if( ! modbus->func_45_01_reset_ans( node, code ) )
  { Console::Print( Console::Error, tr("Ошибка связи!\n") );
    return 0;
  }
  Console::Print( Console::Information, tr("OK.\n") );
  return 1;
}

//==============================================================================
// Поток выполнения команд -- запрос пароля
//==============================================================================
int Thread::mb_passwd()
{
  DWORD passw, answer;

  Console::Print( Console::Information, tr("Запрос пароля... ") );
  if( !modbus->func_45_02_passw_req( node, &passw ) )
  { Console::Print( Console::Error, tr("Ошибка связи!\n") );
    return 0;
  }

  temp_str =  tr("Контрольное число: %1\n\nВведите пароль:").arg(passw);
  temp_str2 = QString::number(passw);
  mainwindow -> flag=1; // синхронизация с GUI потоком (см. MainWindow::update() )
  while( mainwindow -> flag ) msleep(100);
  if( temp_str.isEmpty() ) return 0;
  answer = temp_str.toInt();

  if( !modbus->func_45_03_passw_ans( node, answer ) )
  { Console::Print( Console::Error, tr("Ошибка связи!\n") );
    return 0;
  }
  Console::Print( Console::Information, tr("OK.\n") );
  return 1;
}

//==============================================================================
// Поток выполнения команд -- считать микропрограмму
//==============================================================================
int Thread::mb_load_flash()
{
  DWORD addr;
  DWORD data_size;

  Console::Print( Console::Information, tr("Чтение FLASH... ") );
  memset( buffer,0,sizeof( buffer ) );
  addr = flash_begin_ptr;
  while( addr < flash_end_ptr )
  {
    data_size = qMin( max_packet_size-6, flash_end_ptr - addr );
    if( !modbus->func_45_04_flash_read( node, addr,
                                          data_size, &buffer[addr - flash_begin_ptr] ) )
    { Console::Print( Console::Error, tr("Ошибка связи!\n") );
      return 0;
    }
    addr += data_size;
    mainwindow -> pb_value = 50*(addr-flash_begin_ptr)
                                 /(flash_end_ptr-flash_begin_ptr);
  }
  buffer_len = flash_end_ptr - flash_begin_ptr;
  Console::Print( Console::Information, tr("OK.\n") );
  return 1;
}

//==============================================================================
// Поток выполнения команд -- проверить микропрограмму
//==============================================================================
int Thread::mb_verify_flash()
{
  DWORD addr;
  DWORD data_size;
  DWORD i;
  BYTE temp_buffer[256];

  Console::Print( Console::Information, tr("Проверка...") );

  addr = flash_begin_ptr;
  while( addr < flash_end_ptr )
  {
    data_size = qMin( max_packet_size-6, flash_end_ptr - addr );
    if( !modbus->func_45_04_flash_read( node, addr, data_size, temp_buffer ) )
    { Console::Print( Console::Error, tr("Ошибка связи!\n") );
      return 0;
    }
    for( i=0; i<data_size; i++ )
    { if( temp_buffer[i] != buffer[addr+i-flash_begin_ptr] )
      { if(    ( (addr + i) >= ( flash_begin_ptr + 0x10 ) )
            && ( (addr + i) <= ( flash_begin_ptr + 0x12 ) ) )
        { // CRC16 прошивки может отсутствовать при прошивке по JTAG
          continue;
        }
        Console::Print( Console::Error, tr("OШИБКА ВЕРИФИКАЦИИ! (адрес: 0x%1 данные: [%2] [%3])\n")
                       .arg((int)(addr+i),0,16)
                       .arg((int)temp_buffer[i],0,16 )
                       .arg((int)buffer[addr+i-flash_begin_ptr],0,16)  );
        return 0;
      }
    }
    addr += data_size;
    mainwindow -> pb_value = 50*(addr-flash_begin_ptr)
                                  /(flash_end_ptr-flash_begin_ptr)+50;
  }

  Console::Print( Console::Information, tr("OK.\n") );
  return 1;
}

//==============================================================================
// Поток выполнения команд -- стереть flash
//==============================================================================
int Thread::mb_erase_flash()
{
  DWORD sec1,sec2;

  sec1 = (flash_begin_ptr)/flash_sector_size;
  sec2 = (flash_end_ptr)/flash_sector_size-1;

  Console::Print( Console::Information, tr("Задержка %1 мс...\n").arg( erase_delay ) );
  msleep( erase_delay );
  Console::Print( Console::Information, tr("OK.\n") );


  Console::Print( Console::Information, tr("Стирание модуля...") );

  Console::Print( Console::Information, tr("  стирание FLASH (%1 - %2) ... ")
                                    .arg(toHex(flash_begin_ptr))
                                    .arg(toHex(flash_end_ptr)));

  // стираем
  if( !modbus->func_45_05_flash_erase( node, sec1,sec2 ) )
  { Console::Print( Console::Error, tr("Ошибка связи!\n") );
      return 0;
  }
  msleep(1000);
  Console::Print( Console::Information, tr("OK.\n"));

  return 1;
}

//==============================================================================
// Поток выполнения команд -- записать микропрограмму
//==============================================================================
int Thread::mb_write_flash()
{
  DWORD sec1,sec2;

  sec1 = (flash_begin_ptr)/flash_sector_size;
  sec2 = (flash_end_ptr)/flash_sector_size-1;

  DWORD addr;
  DWORD data_size;
  Console::Print( Console::Information, tr("Прошивка модуля... \n") );

  Console::Print( Console::Information, tr("  стирание FLASH (сектора %1-%2)... ").arg(sec1).arg(sec2) );

  // стираем
  if( !modbus->func_45_05_flash_erase( node, sec1,sec2 ) )
  { Console::Print( Console::Error, tr("Ошибка связи!\n") );
      return 0;
  }
  msleep(1000);
  Console::Print( Console::Information, tr("OK.\n"));

  addr = flash_begin_ptr;
  while( addr < flash_end_ptr )
  {
    if(( addr & (flash_sector_size-1) ) == 0 )
    {  Console::Print( Console::Information, tr("  сектор %1... ").arg(((addr-flash_begin_ptr)/flash_sector_size) ));
    }

    data_size = qMin( flash_buff_limit, max_packet_size-8 );
    data_size = qMin( data_size, flash_sector_size-(addr&(flash_sector_size-1)) );

    if( !modbus->func_45_06_buffer_write( node, &buffer[addr-flash_begin_ptr],
                                                addr & (flash_sector_size-1),
                                                data_size ) )
    { Console::Print( Console::Error, tr("Ошибка связи!\n") );
      return 0;
    }
    addr += data_size;
    if(( addr & (flash_sector_size-1) ) == 0 )
    { // пишем сектор
      if( !modbus->func_45_07_flash_write( node,
                                addr-flash_sector_size, flash_sector_size ) )
      { Console::Print( Console::Error, tr("Ошибка связи!\n") );
        return 0;
      }
      Console::Print( Console::Information, tr("OK.\n"));
    }
    mainwindow -> pb_value = 50*(addr-flash_begin_ptr)
                             /(flash_end_ptr-flash_begin_ptr);
  }
  return 1;
}

//==============================================================================
// Поток выполнения команд -- считать информацию о модуле
//==============================================================================
int Thread::mb_read_information()
{
  struct __attribute__ ((packed))
  {
    BYTE  StructureLength;              // Structure Length(=128)
    WORD  ID;                           // Module ID
    BYTE  MajorVersion;                 // Version Major
    BYTE  MinorVersion;                 //         Minor
    char  ReleaseDate[8];               // Release Date
    char  ReleaseMadeBy[3];             // Release Made by XYZ (Initials)
    char  ProductName[40];              // Product Name
    char  ProductDescription[40];       // Product Description
    char  CompanyName[16];              // Company Name
    char  LegalCopyright[16];           // Legal Copyright
    char reserv[32];
  } info;

  Console::Print( Console::Information, tr("Информация о микропрограмме модуля...") );
  if( !modbus->func_43_00_read( node, 0, 128, (BYTE*)&info, true ) )
  { Console::Print( Console::Error, tr("Ошибка связи!\n") );
      return 0;
  }
  Console::Print( Console::Information, tr("OK.\n") );
  Console::Print( Console::Information, (tr("  ID           : 0x%1\n")
        .arg( info.ID,1,16 )) );
  Console::Print( Console::Information, tr("  Версия       : %1.%2\n")
        .arg( info.MajorVersion ).arg( info.MinorVersion ) );
  Console::Print( Console::Information, tr("  Дата выпуска : %1\n")
        .arg( QString::fromLatin1( (const char *)&info.ReleaseDate,8 ) ) );
  Console::Print( Console::Information, tr("  Автор        : %1\n")
        .arg( QString::fromLatin1( (const char *)&info.ReleaseMadeBy,3 ) ) );
  Console::Print( Console::Information, tr("  Наименование : %1\n")
        .arg( QString::fromLatin1( (const char *)&info.ProductName,40 ) ) );
  Console::Print( Console::Information, tr("  Описание     : %1\n")
        .arg( QString::fromLatin1( (const char *)&info.ProductDescription,40 ) ) );
  Console::Print( Console::Information, tr("  Производитель: %1\n")
        .arg( QString::fromLatin1( (const char *)&info.CompanyName,16 ) ) );
  Console::Print( Console::Information, tr("  Права        : %1\n")
        .arg( QString::fromLatin1( (const char *)&info.LegalCopyright,16 ) ) );
  return 1;
}

//==============================================================================
// Поток выполнения команд -- считать тип модуля
//==============================================================================
int Thread::mb_load_module_type()
{
  flash_begin_ptr = flash_end_ptr = flash_sector_size = 0;

  Console::Print( Console::Information, tr("Чтение типа модуля... ") );
  memset( info_buffer, 0, sizeof( info_buffer ) );
  if( !modbus->func_45_08_module_info( node, info_buffer, 128 ) )
  {  Console::Print( Console::Error, tr("Ошибка связи!\n") );
     return 0;
  }
  Console::Print( Console::Information, tr("OK.\n") );

  Console::Print( Console::Information, tr("  Наименование модуля     : %1\n")
        .arg( QString::fromLatin1( (const char *)&info_buffer[ 0],15 ) ) );
  Console::Print( Console::Information, tr("  Тип процессора          : %1\n")
        .arg( QString::fromLatin1( (const char *)&info_buffer[16],15 ) ) );
  Console::Print( Console::Information, tr("  Дата выпуска загрузчика : %1\n")
        .arg( QString::fromLatin1( (const char *)&info_buffer[32],15 ) ) );

  memcpy(&flash_begin_ptr, &info_buffer[48], sizeof(flash_begin_ptr));
  memcpy(&flash_end_ptr, &info_buffer[52], sizeof(flash_end_ptr));
  memcpy(&flash_sector_size, &info_buffer[56], sizeof(flash_sector_size));
  memcpy(&flash_buff_limit, &info_buffer[60], sizeof(flash_buff_limit));

  if( loader_bin_length )
  { flash_end_ptr = flash_begin_ptr + ((loader_bin_length/flash_sector_size)+1)*flash_sector_size;
  }

  if( flash_buff_limit > flash_sector_size ) flash_buff_limit = flash_sector_size;
  if( flash_buff_limit > 128               ) flash_buff_limit = 128;
  if( flash_buff_limit == 0                ) flash_buff_limit = 128;
  flash_buff_limit &= ~3;

  Console::Print( Console::Information, tr("  Адрес начала FLASH      : %1\n")
        .arg( toHex( flash_begin_ptr)  ) );
  Console::Print( Console::Information, tr("  Адрес конца  FLASH      : %1\n")
        .arg( toHex( flash_end_ptr)  ) );
  Console::Print( Console::Information, tr("  Размер сектора FLASH    : %1\n")
        .arg( toHex( flash_sector_size )  ) );
  Console::Print( Console::Information, tr("  Порции заполнения       : %1\n")
        .arg( toHex( flash_buff_limit )  ) );

  if( loader_bin_length == 0 )
  {
    if( buffer[0x10] == 0 )
    {  Console::Print( Console::Information, "Добваление CRC16 в микропрограмму\n");
       int len = flash_end_ptr-flash_begin_ptr;
       WORD crc16 = get_crc16( buffer + 0x80, len - 0x80 );
       buffer[0x10] = 1;
       buffer[0x11] = crc16;
       buffer[0x12] = crc16 >> 8;
    }
  }

  return 1;
}

//==============================================================================
// Поток выполнения команд -- проверить тип модуля
//==============================================================================
int Thread::mb_check_module_type()
{
  int i;

  Console::Print( Console::Information, tr("Проверка типа модуля... ") );
  for(i=0; i<15; i++ )
  { if( file_info_buffer[i] != info_buffer[i] )
    { Console::Print( Console::Error, tr("ОШИБКА! Тип модуля не соответствует микропрограмме!.\n") );
      Console::Print( Console::Error, tr("           Тип модуля: \"%1\"\n")
         .arg( QString::fromLatin1( (const char *)&info_buffer[ 0],15 ) ) );
      Console::Print( Console::Error, tr("   Тип микропрограммы: \"%1\"\n")
         .arg( QString::fromLatin1( (const char *)&file_info_buffer[ 0],15 ) ) );
      memset( info_buffer, 0, sizeof( info_buffer ) );
      return 0;
    }
  }
  Console::Print( Console::Information, tr("OK.\n") );
  return 1;
}

//==============================================================================
// Поток выполнения команд -- запись загрузчика
//==============================================================================
int Thread::mb_loader_change()
{
  Console::Print( Console::Information, "Запись загрузчика..." );

  WORD crc16 = get_crc16( buffer, loader_bin_length );
  if( !modbus->func_45_09_loader_change( node,crc16, loader_bin_length ) )
  { Console::Print( Console::Error, tr("OШИБКА записи загрузчика (команда 43_09)!\n") );
    return 0;
  }
  return 1;
}

//==============================================================================
//
//==============================================================================
int Thread::mb_download_firmware()
{
    const QString module_name = QString::fromLatin1((const char *)&info_buffer[0], 15)
                                   .split(QChar('\0')).value(0);

    QByteArray ba;

    execInMainThread(
                [module_name, &ba] {
                download_firmware_gui(module_name, &ba);
    } );

    if (ba.isEmpty())
        return 0;

    memset(buffer, 0, sizeof(buffer));
    memset(file_info_buffer, 0, sizeof(file_info_buffer));

    quint32 size=ba.size();
    if( size <= 1024 )
    { Console::Print( Console::Error, tr("\n\n ОШИБКА! Размер файла меньше 1K!\n") );
      return 0;
    }
    else if( size >= (1024*1024) )
    { Console::Print( Console::Error, tr("\n\n ОШИБКА! Размер файла больше 1000K!\n") );
      return 0;
    }
    buffer_len = size-1024;

    QBuffer ba_buffer(&ba);
    ba_buffer.open(QIODevice::ReadOnly);
    ba_buffer.read( (char*)file_info_buffer, 1024 );
    ba_buffer.read( (char*)buffer, buffer_len );

    quint32 crc32_from_file;
    quint32 crc32 = 0xFFFFFFFF;
    memcpy(&crc32_from_file, &file_info_buffer[0x20], sizeof(crc32_from_file));
    memset(&file_info_buffer[0x20], 0, sizeof(DWORD));
    for(unsigned int i=0; i<1024;       i++) crc32 = update_crc32( crc32, file_info_buffer[i] );
    for(unsigned int i=0; i<buffer_len; i++) crc32 = update_crc32( crc32, buffer[i]    );
    if( crc32 !=  crc32_from_file )
    {
        memset( buffer, 0, sizeof( buffer ));
        memset( file_info_buffer, 0, sizeof( file_info_buffer ));
        Console::Print( Console::Error, tr("\n\n ОШИБКА! Поврежден файл микопрограммы (CRC error)\n") );
        return 0;
    }

    Console::Print(Console::Error, tr("Микропрограмма получена из интернета.\n") );

    return 1;
}
