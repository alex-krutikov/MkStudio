#include <QtCore>

#include "serialport_p.h"

#include "console.h"
#include "serialport.h"
#include "mbcommon.h"

#define _UNICODE
#include <winuser.h>
#include <windows.h>
#include <tchar.h>
#include <setupapi.h>
#include <dbt.h>

extern "C" WINUSERAPI HDEVNOTIFY WINAPI RegisterDeviceNotificationW(HANDLE,LPVOID,DWORD);
extern "C" WINUSERAPI BOOL WINAPI UnregisterDeviceNotification(HANDLE);

//===================================================================
// Оконная функция
//===================================================================
LRESULT CALLBACK SerialPortWndProc(HWND hwnd, UINT Message, WPARAM wparam,LPARAM lparam)
{
  SerialPortPrivate *spp = (SerialPortPrivate*)GetWindowLongPtr( hwnd, GWL_USERDATA );
  if( Message == WM_DEVICECHANGE )
  { Console::Print(Console::Information, "WM_DEVICECHANGE  wParam=" + QString::number( wparam , 16) + " \n" );
    spp->close();
  }
  return DefWindowProc(hwnd,Message,wparam,lparam);
}

//===================================================================
// Инициализация
//===================================================================
SerialPortPrivate::SerialPortPrivate( SerialPort *sp_arg )
  : sp( sp_arg )
{
  hport = 0;
  last_error_id = 0;

  if (perf_cnt_ok = QueryPerformanceCounter(&freq))
  { perf_cnt_ok = QueryPerformanceFrequency(&freq);
  }

  WNDCLASS w;
  memset(&w,0,sizeof(WNDCLASS));
  w.style = CS_HREDRAW | CS_VREDRAW;
  w.lpfnWndProc = SerialPortWndProc;
  w.hInstance = 0;
  w.hbrBackground = (WHITE_BRUSH);
  w.lpszClassName = _T("SerialPortNotifyWindows");
  RegisterClass(&w);
  hwnd = CreateWindow(_T("SerialPortNotifyWindows"),_T("SerialPortNotifyWindow"),0,
		0,0,0,0,NULL,NULL,0,NULL);
  SetWindowLongPtr( hwnd, GWL_USERDATA, (LONG)this );
  ShowWindow(hwnd,SW_HIDE);
}

//===================================================================
// Завершение
//===================================================================
SerialPortPrivate::~SerialPortPrivate()
{
  close();
  CloseWindow( hwnd );
}

//===================================================================
// Открытие порта
//===================================================================
bool SerialPortPrivate::open()
{
  close();

  Sleep(100);

  if( GetVersion() & 0x80000000 )
  { // Windows 9X
    hport = CreateFileA( ("\\\\.\\"+sp->portname).toAscii(),
                    GENERIC_READ|GENERIC_WRITE,
                    0,NULL,OPEN_EXISTING,0,NULL );

  } else
  { // Windows NT/XP...
    hport = CreateFileW( (WCHAR*)("\\\\.\\"+sp->portname).utf16(),
                    GENERIC_READ|GENERIC_WRITE,
                    0,NULL,OPEN_EXISTING,0,NULL );
  }

  if(hport==INVALID_HANDLE_VALUE)
  { if( last_error_id != 1 )
    { last_error_id = 1;
      Console::Print( Console::Error, "ERROR Can't open port!\n" );
      sp->lastError_str="Can't open port!";
    }
    hport=0;
    return false;
  }

  // Инициализируем COM-порт.
  DCB dcb;
  memset( &dcb, 0, sizeof( dcb) );
  dcb.DCBlength=sizeof(DCB);
  dcb.BaudRate=sp->portspeed;
  dcb.fBinary=1; //8 бит.
  dcb.fParity=0; // без паритета.
  dcb.fOutxCtsFlow=0;
  dcb.fOutxDsrFlow=0;
  dcb.fDtrControl=DTR_CONTROL_DISABLE;
  dcb.fDsrSensitivity=0;
  dcb.fTXContinueOnXoff=1;
  dcb.fOutX=0;
  dcb.fInX=0;
  dcb.fErrorChar=0;
  dcb.fNull=0;
  dcb.fRtsControl = ( GetVersion() & 0x80000000 )
                    ? RTS_CONTROL_DISABLE  // Windows 95/98/ME
                    : RTS_CONTROL_TOGGLE;  // Windows NT/2000/XP...
  dcb.fAbortOnError=0;
  dcb.wReserved=0;
  dcb.XonLim=100;
  dcb.XoffLim=10;
  dcb.ByteSize=8;
  dcb.StopBits=0;	// 1 стоп.

  if(!SetCommState(hport,&dcb))
  {	CloseHandle(hport);
        hport=0;
        sp->lastError_str="Can't init port!";
	  return false;
  }

  // Устанавливаем таймауты.
  COMMTIMEOUTS ct;
  memset( &ct, 0 , sizeof(ct) );
  ct.ReadTotalTimeoutConstant = 100;
  SetCommTimeouts(hport,&ct);

  // Сбрасываем порт.
  PurgeComm(hport,PURGE_TXCLEAR|PURGE_RXCLEAR);

  // Устанавливаем размер буферов приема и передачи.
  SetupComm(hport,512,512);

  return true;
}

//===================================================================
// Переоткрытие порта
//===================================================================
bool SerialPortPrivate::reopen()
{
  if( hport == 0 ) return open();
  return false;
}

//===================================================================
// Закрытие порта
//===================================================================
void SerialPortPrivate::close()
{
  if( hport )
  { CloseHandle( hport );
    hport = 0;
  }
}

//===================================================================
// Посылка запроса и получение ответа MODBUS RTU
//===================================================================
int SerialPortPrivate::query( const QByteArray &request,
                                  QByteArray &answer, int *errorcode)
{
  bool ok;
  DWORD i,j,answer_size;

  if( hport == 0 )
  { reopen();
    return 0;
  }

  if( sp->answer_timeout != sp->current_answer_timeout )
  { COMMTIMEOUTS ct;
    memset( &ct, 0 , sizeof(ct) );
    ct.ReadTotalTimeoutConstant = sp->answer_timeout;
    SetCommTimeouts(hport,&ct);
    sp->current_answer_timeout =  sp->answer_timeout;
  }

  PurgeComm(hport,PURGE_TXCLEAR|PURGE_RXCLEAR);

  Console::Print( Console::ModbusPacket, "MODBUS: Request: "+QByteArray2QString( request )+"\n");

  // задержка перед запросом 4 байтовых интервала на выбранной скорости
  usleep((8 * 10 * 1000000 / sp->speed()));

  // запрос
  ok = WriteFile( hport, request.data(), request.length(),  &j, 0 );
  if(!ok)
  { if( last_error_id != 2 )
    { last_error_id = 2;
      Console::Print( Console::Error, "MODBUS: ERROR: WriteFile (return value).\n" );
    }
    return -1;
  }
  if((int)j != request.length() )
  { Console::Print( Console::Error, "MODBUS: ERROR: WriteFile (wrong length).\n" );
    return 0;
  }

  // ответ: чтение первых 5 байт
  answer_size = answer.length();
  if(errorcode) *errorcode=0;
  i=0;
  if( answer_size > 5 )
  { ok = ReadFile( hport, answer.data(), 5, &i, 0 );
    if(!ok)
    { if( last_error_id != 3 )
      { last_error_id = 3;
        Console::Print( Console::Error, "MODBUS: ERROR: ReadFile (return value).\n" );
      }
      return 0;
    }
    if((i==5)&&(answer[1]&0x80)) goto error1;
    if(i==0) goto error2;
  }
  // ответ: чтение оставшихся байт
  j=i;
  while(1)
  { ok = ReadFile( hport, answer.data()+j, answer_size-j, &i, 0 );
    if(!ok)
    { if( last_error_id != 3 )
      { last_error_id = 3;
        Console::Print( Console::Error, "MODBUS: ERROR: ReadFile (return value).\n" );
      }
      return 0;
    }
    j += i;
    if( i == 0           ) break;
    if( j == answer_size ) break;
  }

  // анализ ошибок
  if( j == 0 )                goto error2;
  else if(j != answer_size )  goto error3;

  Console::Print( Console::ModbusPacket, "MODBUS:  Answer: "+QByteArray2QString( answer )+"\n");

  if( last_error_id )
  {  Console::Print( Console::ModbusError, "MODBUS: OK.\n" );
     last_error_id = 0;
  }
  return j;
error1:
  {
  Console::Print( Console::ModbusError, "MODBUS: Error flag in answer. " );
  QByteArray ba = answer;
  ba.resize( 5 );
  Console::Print( Console::ModbusError, " [ "+QByteArray2QString( ba, 1 )+" ]\n");
  if(errorcode) *errorcode=answer[2];
  return 5;
  }
error2:
  {
  if( last_error_id != 4 )
  Console::Print( Console::ModbusError, "MODBUS: TimeOut.\n" );
  last_error_id = 4;
  return 0;
  }
error3:
   {
   Console::Print( Console::ModbusError, QString("MODBUS: ERROR: ReadFile Wrong answer length. (expected=%1, real=%2) ")
                              .arg(answer.length()).arg(j) );
   QByteArray ba = answer;
   ba.resize( j );
   Console::Print( Console::ModbusError, "Answer: "+QByteArray2QString( ba, 1 )+"\n");
   return j;
   }
}

//===================================================================
// Микросекундная задержка
//===================================================================
void SerialPortPrivate::usleep(DWORD us)
{
  LARGE_INTEGER curr, stop;

  if( perf_cnt_ok )
  { QueryPerformanceCounter(&curr);
    stop.QuadPart = curr.QuadPart + (freq.QuadPart * 1000 * us / 1000000000);
    do
    { Sleep(0);
      QueryPerformanceCounter(&curr);
    } while (curr.QuadPart < stop.QuadPart);
  } else
  { Sleep((us + 999) / 1000);
  }
}


//===================================================================
// Поиск присутствующих в системе COM портов
//  return: список строк в формате [название порта];[описание]
//===================================================================
QStringList SerialPortPrivate::queryComPorts()
{
  if( GetVersion() & 0x80000000 )
  { // Windows98
    QStringList sl;
    for(int i=1;i<=9;i++) sl << QString("COM%1;Serial Port %2").arg(i).arg(i);
    return sl;
  }

  QString str;
  QRegExp regexp("^COM([0-9]{1,2})");
  QMap<int, QString> map;

  const GUID GUID_DEVINTERFACE_COMPORT = {0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73};

  typedef HKEY (__stdcall SETUPDIOPENDEVREGKEY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, REGSAM);
  typedef BOOL (__stdcall SETUPDIDESTROYDEVICEINFOLIST)(HDEVINFO);
  typedef BOOL (__stdcall SETUPDIENUMDEVICEINFO)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
  typedef HDEVINFO (__stdcall SETUPDIGETCLASSDEVS)(LPGUID, LPCTSTR, HWND, DWORD);
  typedef BOOL (__stdcall SETUPDIGETDEVICEREGISTRYPROPERTY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, PDWORD, PBYTE, DWORD, PDWORD);

  //Get the various function pointers we require from setupapi.dll
  HINSTANCE hSetupAPI = LoadLibrary(_T("SETUPAPI.DLL"));
  if (hSetupAPI == NULL)
    return QStringList();

  SETUPDIOPENDEVREGKEY* lpfnLPSETUPDIOPENDEVREGKEY = reinterpret_cast<SETUPDIOPENDEVREGKEY*>(GetProcAddress(hSetupAPI, "SetupDiOpenDevRegKey"));
  #ifdef _UNICODE
  SETUPDIGETCLASSDEVS* lpfnSETUPDIGETCLASSDEVS = reinterpret_cast<SETUPDIGETCLASSDEVS*>(GetProcAddress(hSetupAPI, "SetupDiGetClassDevsW"));
  SETUPDIGETDEVICEREGISTRYPROPERTY* lpfnSETUPDIGETDEVICEREGISTRYPROPERTY = reinterpret_cast<SETUPDIGETDEVICEREGISTRYPROPERTY*>(GetProcAddress(hSetupAPI, "SetupDiGetDeviceRegistryPropertyW"));
  #else
  SETUPDIGETCLASSDEVS* lpfnSETUPDIGETCLASSDEVS = reinterpret_cast<SETUPDIGETCLASSDEVS*>(GetProcAddress(hSetupAPI, "SetupDiGetClassDevsA"));
  SETUPDIGETDEVICEREGISTRYPROPERTY* lpfnSETUPDIGETDEVICEREGISTRYPROPERTY = reinterpret_cast<SETUPDIGETDEVICEREGISTRYPROPERTY*>(GetProcAddress(hSetupAPI, "SetupDiGetDeviceRegistryPropertyA"));
  #endif
  SETUPDIDESTROYDEVICEINFOLIST* lpfnSETUPDIDESTROYDEVICEINFOLIST = reinterpret_cast<SETUPDIDESTROYDEVICEINFOLIST*>(GetProcAddress(hSetupAPI, "SetupDiDestroyDeviceInfoList"));
  SETUPDIENUMDEVICEINFO* lpfnSETUPDIENUMDEVICEINFO = reinterpret_cast<SETUPDIENUMDEVICEINFO*>(GetProcAddress(hSetupAPI, "SetupDiEnumDeviceInfo"));

  if ((lpfnLPSETUPDIOPENDEVREGKEY == NULL) || (lpfnSETUPDIDESTROYDEVICEINFOLIST == NULL) ||
      (lpfnSETUPDIENUMDEVICEINFO == NULL) || (lpfnSETUPDIGETCLASSDEVS == NULL) || (lpfnSETUPDIGETDEVICEREGISTRYPROPERTY == NULL))
  { //Unload the setup dll
    FreeLibrary(hSetupAPI);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return QStringList();
  }

  //Now create a "device information set" which is required to enumerate all the ports
  GUID guid = GUID_DEVINTERFACE_COMPORT;
  HDEVINFO hDevInfoSet = lpfnSETUPDIGETCLASSDEVS(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_ALLCLASSES);
  //HDEVINFO hDevInfoSet = lpfnSETUPDIGETCLASSDEVS(&guid, NULL, NULL, DIGCF_DEVICEINTERFACE );
  if (hDevInfoSet == INVALID_HANDLE_VALUE)
  { //Unload the setup dll
    FreeLibrary(hSetupAPI);
    return QStringList();
  }

  //Finally do the enumeration
  BOOL bMoreItems = TRUE;
  int nIndex = 0;
  SP_DEVINFO_DATA devInfo;
  while (bMoreItems)
  { //Enumerate the current device
    devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
    bMoreItems = lpfnSETUPDIENUMDEVICEINFO(hDevInfoSet, nIndex, &devInfo);
    if (bMoreItems)
    { //Did we find a serial port for this device
      BOOL bAdded = FALSE;
      //Get the registry key which stores the ports settings
      HKEY hDeviceKey = lpfnLPSETUPDIOPENDEVREGKEY(hDevInfoSet, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
      if (hDeviceKey != INVALID_HANDLE_VALUE )
      { //Read in the name of the port
        TCHAR pszPortName[256];
        DWORD dwSize = sizeof(pszPortName);
        DWORD dwType = 0;
  	    if ((RegQueryValueEx(hDeviceKey, _T("PortName"), NULL, &dwType, reinterpret_cast<LPBYTE>(pszPortName), &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
        { //If it looks like "COMX" then
          //add it to the array which will be returned
          size_t nLen = _tcslen(pszPortName);
          if (nLen > 3)
          {  str = QString::fromWCharArray( pszPortName );
             //if( str.startsWith("COM") )
             if( regexp.exactMatch( str ) )
               bAdded = TRUE;
          }
        }
        //Close the key now that we are finished with it
        RegCloseKey(hDeviceKey);
      }

      //If the port was a serial port, then also try to get its friendly name
      if (bAdded)
      { TCHAR pszFriendlyName[256];
        DWORD dwSize = sizeof(pszFriendlyName);
        DWORD dwType = 0;
        if (lpfnSETUPDIGETDEVICEREGISTRYPROPERTY(hDevInfoSet, &devInfo, SPDRP_DEVICEDESC, &dwType, reinterpret_cast<PBYTE>(pszFriendlyName), dwSize, &dwSize) && (dwType == REG_SZ))
        str +=";"+QString::fromWCharArray( pszFriendlyName );
        if( regexp.indexIn(str) > -1 ) map[ regexp.cap(1).toInt() ] = str;
      }
    }
    ++nIndex;
  }
  //Free up the "device information set" now that we are finished with it
  lpfnSETUPDIDESTROYDEVICEINFOLIST(hDevInfoSet);
  //Unload the setup dll
  FreeLibrary(hSetupAPI);
  //Return the success indicator

  return  map.values();
}

