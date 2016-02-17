#include <QtCore>

#include "mbmaster.h"
#include "mbmaster_p.h"
#include "mbcommon.h"
#include "crc.h"
#include "console.h"
#include "abstractserialport.h"

//###################################################################
//
//###################################################################
inline static short swap_short( short a )
{
  return ((unsigned short)a>>8) | ((unsigned short)a<<8);
}

//###################################################################
//
//###################################################################
MBMasterPrivate::MBMasterPrivate( QObject *parent )
  : QThread( parent )
{
  full_time = 0;
  request_counter = answer_counter = error_counter = 0;
  max_packet_length = 128;
  disable_write_transactions = false;
  disable_read_transactions  = false;
  transaction_delay = 0;
  cycle_time = 0;
  auto_skip_mode = true;
  setTerminationEnabled( true );
}

//===================================================================
//
//===================================================================
MBMasterPrivate::~MBMasterPrivate()
{
  clear_configuration();
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::polling_start()
{
 int i=0,j=0,k,m,len,a;
 QByteArray ba;
 MMSlotTransaction mmsp;

 transactions_read.clear();
 transactions_write.clear();
 transactions_write2.clear();
 transactions_write_map.clear();
 request_counter = answer_counter = error_counter = 0;

 //--- конфигурация транзакций на чтение -------------------------------------------------
 for( i=0; i<mmslots.count(); i++ )
 { MMSlot &ps = mmslots[i];

   if( ps.datatype.isRegister() )
   { // classic modbus registers

     if( ps.datatype.isExtendedRegister() )
     { // extended modbus registers

       int regs_per_packet = ((int)((max_packet_length&0x7FFFFFFF)-5))/4;
       k=0;
       while( k < ps.len )
       { int reg_first = 2*k + ps.addr;
         int reg_count = 2*qMin( regs_per_packet, ps.len - k );
         //--------------------------------
         switch( ps.datatype.id() )
         {
           case( MBDataType::DwordsInputRegHiLo   ):
           case( MBDataType::FloatsInputRegHiLo   ):
           case( MBDataType::DwordsInputRegLoHi   ):
           case( MBDataType::FloatsInputRegLoHi   ):
             ba.resize( 6 );
             ba[0] = ps.module.node;
             ba[1] = 0x04;
             ba[2] = reg_first >> 8;
             ba[3] = reg_first;
             ba[4] = reg_count >> 8;
             ba[5] = reg_count;
             CRC::appendCRC16( ba );
             mmsp.exp_length = 2*reg_count+5;
             break;
           case( MBDataType::DwordsHoldingRegHiLo ):
           case( MBDataType::FloatsHoldingRegHiLo ):
           case( MBDataType::DwordsHoldingRegLoHi ):
           case( MBDataType::FloatsHoldingRegLoHi ):
             ba.resize( 6 );
             ba[0] = ps.module.node;
             ba[1] = 0x03;
             ba[2] = reg_first >> 8;
             ba[3] = reg_first;
             ba[4] = reg_count >> 8;
             ba[5] = reg_count;
             CRC::appendCRC16( ba );
             mmsp.exp_length = 2*reg_count+5;
             break;
          default:
            break;
          }
         //--------------------------------
         mmsp.slot    = &ps;
         mmsp.offset  = k;
         mmsp.length  = reg_count/2;
         mmsp.request = ba;
         mmsp.answer.resize( mmsp.exp_length );
         mmsp.execute_flag = true;
         transactions_read << mmsp;
         //--------------------------------
         k = k + reg_count/2;
       }
     } else
     { // standart modbus registers
       int regs_per_packet = 1;
       switch( ps.datatype.id() )
       { case( MBDataType::InputRegisters):
         case( MBDataType::HoldingRegisters):
           regs_per_packet = ((int)((max_packet_length&0x7FFFFFFF)-5))/2;
           break;
         case( MBDataType::DiscreteInputs  ):
         case( MBDataType::Coils  ):
           regs_per_packet = ((int)((max_packet_length&0x7FFFFFFF)-5))*8;
           break;
         default: continue; break;
       }

       k=0;
       while( k < ps.len )
       { int reg_first = k + ps.addr;
         int reg_count = qMin( regs_per_packet, ps.len - k );
         //--------------------------------
         switch( ps.datatype.id() )
         { case( MBDataType::InputRegisters ) :
             ba.resize( 6 );
             ba[0] = ps.module.node;
             ba[1] = 0x04;
             ba[2] = reg_first >> 8;
             ba[3] = reg_first;
             ba[4] = reg_count >> 8;
             ba[5] = reg_count;
             CRC::appendCRC16( ba );
             mmsp.exp_length = 2*reg_count+5;
             break;
           case( MBDataType::HoldingRegisters ) :
             ba.resize( 6 );
             ba[0] = ps.module.node;
             ba[1] = 0x03;
             ba[2] = reg_first >> 8;
             ba[3] = reg_first;
             ba[4] = reg_count >> 8;
             ba[5] = reg_count;
             CRC::appendCRC16( ba );
             mmsp.exp_length = 2*reg_count+5;
             break;
           case( MBDataType::DiscreteInputs ):
             ba.resize( 6 );
             ba[0] = ps.module.node;
             ba[1] = 0x02;
             ba[2] = reg_first >> 8;
             ba[3] = reg_first;
             ba[4] = reg_count >> 8;
             ba[5] = reg_count;
             CRC::appendCRC16( ba );
             mmsp.exp_length = ((reg_count-1)/8+1)+5;
             break;
           case( MBDataType::Coils ):
             ba.resize( 6 );
             ba[0] = ps.module.node;
             ba[1] = 0x01;
             ba[2] = reg_first >> 8;
             ba[3] = reg_first;
             ba[4] = reg_count >> 8;
             ba[5] = reg_count;
             CRC::appendCRC16( ba );
             mmsp.exp_length = ((reg_count-1)/8+1)+5;
             break;
           default :
             break;
         }
         //--------------------------------
         mmsp.slot    = &ps;
         mmsp.offset  = k;
         mmsp.length  = reg_count;
         mmsp.request = ba;
         mmsp.answer.resize( mmsp.exp_length );
         mmsp.execute_flag = true;

         transactions_read << mmsp;
         //--------------------------------
         k = k + reg_count;
       }
     }
   } else if (ps.datatype.isArduino())
   {
     j = ps.len;
     switch( ps.datatype.id() )
     { case( MBDataType::BitsArduino   )          : j = j/8; break;
       case( MBDataType::BytesArduino  )          : j = j*1; break;
       case( MBDataType::WordsArduino  )          : j = j*2; break;
       case( MBDataType::DwordsArduino )          : j = j*4; break;
       case( MBDataType::FloatsArduino )          : j = j*4; break;
       default: continue; break;
     }

     k=0;
     while( k < j )
     { len = qMin( j-k, (int)(16));
       a = k + ps.addr;
       //--------------------------------
       ba.resize(4);
       ba[0] = 0x00;
       ba[1] = a;
       ba[2] = a >> 8;
       ba[3] = len;
       mmsp.exp_length = len + 4;
       //--------------------------------
       mmsp.slot = &ps;
       switch( ps.datatype.id() )
       { case( MBDataType::BitsArduino   )           : mmsp.offset = k*8; mmsp.length = len*8; break;
         case( MBDataType::BytesArduino  )           : mmsp.offset = k/1; mmsp.length = len/1; break;
         case( MBDataType::WordsArduino  )           : mmsp.offset = k/2; mmsp.length = len/2; break;
         case( MBDataType::DwordsArduino )           : mmsp.offset = k/4; mmsp.length = len/4; break;
         case( MBDataType::FloatsArduino )           : mmsp.offset = k/4; mmsp.length = len/4; break;
         default: break;
       }
       mmsp.request = ba;
       mmsp.answer.resize( mmsp.exp_length );
       mmsp.execute_flag = true;

       transactions_read << mmsp;
       //--------------------------------
       k = k+len;
     }
   } else
   {  // mikkkon

     j = ps.len;
     switch( ps.datatype.id() )
     { case( MBDataType::Bits   )          : j = j/8; break;
       case( MBDataType::Bytes  )          : j = j*1; break;
       case( MBDataType::Words  )          : j = j*2; break;
       case( MBDataType::Dwords )          : j = j*4; break;
       case( MBDataType::Floats )          : j = j*4; break;
       default: continue; break;
     }

     k=0;
     while( k < j )
     { len = qMin( j-k, (int)((max_packet_length&0x7FFFFFFC)-8) );
       a = k + ps.addr;
       //--------------------------------
       ba.resize( 6 );
       ba[0] = ps.module.node;
       ba[1] = 0x43;
       ba[2] = 0x00 | ( ps.module.subnode << 4 );
       ba[3] = a >> 8;
       ba[4] = a;
       ba[5] = len & 0xFF;
       CRC::appendCRC16( ba );
       mmsp.exp_length = len+6;
       //--------------------------------
       mmsp.slot = &ps;
       switch( ps.datatype.id() )
       { case( MBDataType::Bits   )           : mmsp.offset = k*8; mmsp.length = len*8; break;
         case( MBDataType::Bytes  )           : mmsp.offset = k/1; mmsp.length = len/1; break;
         case( MBDataType::Words  )           : mmsp.offset = k/2; mmsp.length = len/2; break;
         case( MBDataType::Dwords )           : mmsp.offset = k/4; mmsp.length = len/4; break;
         case( MBDataType::Floats )           : mmsp.offset = k/4; mmsp.length = len/4; break;
         default: break;
       }
       mmsp.request = ba;
       mmsp.answer.resize( mmsp.exp_length );
       mmsp.execute_flag = true;

       transactions_read << mmsp;
       //--------------------------------
       k = k+len;
     }
   }
 }

 //--- конфигурация транзакций на запись -------------------------------------------------
 m=0;
 j=1;
 for( i=0; i<mmslots.count(); i++ )
 { MMSlot &ps = mmslots[i];
   switch( ps.datatype.id() )
   { case( MBDataType::Bits   )           : j = 1; break;
     case( MBDataType::Bytes  )           : j = 1; break;
     case( MBDataType::Words  )           : j = 2; break;
     case( MBDataType::Dwords )           : j = 4; break;
     case( MBDataType::Floats )           : j = 4; break;
     case( MBDataType::HoldingRegisters ) : j = 2; break;
     case( MBDataType::Coils )            : j = 2; break;
     case( MBDataType::BitsArduino)       : j = 1; break;
     case( MBDataType::BytesArduino)      : j = 1; break;
     case( MBDataType::WordsArduino)      : j = 2; break;
     case( MBDataType::DwordsArduino)     : j = 4; break;
     case( MBDataType::FloatsArduino)     : j = 4; break;
     case( MBDataType::DwordsHoldingRegHiLo ):
     case( MBDataType::FloatsHoldingRegHiLo ):
     case( MBDataType::DwordsHoldingRegLoHi ):
     case( MBDataType::FloatsHoldingRegLoHi ):
       j=4;
       break;
     default:  continue;
   }

   for( k=0; k<ps.len; k++ )
   {
     if( ps.datatype.isRegister() )
     {
       switch( ps.datatype.id() )
       { case( MBDataType::HoldingRegisters ):
           ba.resize( 4 );
           ba[0] = ps.module.node;
           ba[1] = 0x06;
           ba[2] = ( ps.addr + k ) >> 8;
           ba[3] = ( ps.addr + k );
           mmsp.slot = &ps;
           mmsp.offset = k;
           mmsp.length = 1;
           mmsp.request = ba;
           mmsp.exp_length = j + 6;
           mmsp.answer.resize( mmsp.exp_length );
           mmsp.execute_flag = false;
           break;
         case( MBDataType::Coils ):
           ba.resize( 4  );
           ba[0] = ps.module.node;
           ba[1] = 0x05;
           ba[2] = ( ps.addr + k ) >> 8;
           ba[3] = ( ps.addr + k );
           mmsp.slot = &ps;
           mmsp.offset = k;
           mmsp.length = 1;
           mmsp.request = ba;
           mmsp.exp_length = j + 6;
           mmsp.answer.resize( mmsp.exp_length );
           mmsp.execute_flag = false;
           break;
         case( MBDataType::DwordsHoldingRegHiLo ):
         case( MBDataType::FloatsHoldingRegHiLo ):
         case( MBDataType::DwordsHoldingRegLoHi ):
         case( MBDataType::FloatsHoldingRegLoHi ):
           ba.resize( 7 );
           ba[0] = ps.module.node;
           ba[1] = 0x10;
           ba[2] = ( ps.addr + 2*k ) >> 8;
           ba[3] = ( ps.addr + 2*k );
           ba[4] = 0;
           ba[5] = 2;
           ba[6] = 4;
           mmsp.slot = &ps;
           mmsp.offset = k;
           mmsp.length = 1;
           mmsp.request = ba;
           mmsp.exp_length = 8;
           mmsp.answer.resize( mmsp.exp_length );
           mmsp.execute_flag = false;
           break;
       }
     } else if( ps.datatype.isArduino() )
     {
       if( ps.datatype.id() == MBDataType::BitsArduino  )  { a = k/8 + ps.addr; }
       else                                                { a = j*k + ps.addr; }
       ba.resize( 4 );
       ba[0] = 0x01;
       ba[1] = a;
       ba[2] = a >> 8;
       ba[3] = j;

       mmsp.slot = &ps;
       mmsp.offset = k;
       mmsp.length = 1;
       mmsp.request = ba;
       mmsp.exp_length = j + 4;
       mmsp.answer.resize( mmsp.exp_length );
       mmsp.execute_flag = false;
     } 
     else //mikkon
     {
       if( ps.datatype.id() == MBDataType::Bits  )  { a = k/8 + ps.addr; }
       else                                         { a = j*k + ps.addr; }
       ba.resize( 6 );
       ba[0] = ps.module.node;
       ba[1] = 0x43;
       ba[2] = 0x01 | ( ps.module.subnode << 4 );
       ba[3] = a >> 8;
       ba[4] = a;
       ba[5] = j;

       mmsp.slot = &ps;
       mmsp.offset = k;
       mmsp.length = 1;
       mmsp.request = ba;
       mmsp.exp_length = j + 8;
       mmsp.answer.resize( mmsp.exp_length );
       mmsp.execute_flag = false;
     }
     //--------------------------------
     transactions_write << mmsp;
     transactions_write_map.insert( ModuleSlotIndex( ps.module.n, ps.n, k ), m );
     m++;
   }
 }
 //for( i=0; i<transactions_write.count(); i++ )
 //{ Console::Print( Console::Debug, "запись:" + QByteArray2QString( transactions_write[i].request ) + "\n"  );
 //}
 start( QThread::HighPriority );
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::polling_stop()
{
  thread_exit_flag = true;
  wait(5000);
  full_time=0;
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::clear_configuration()
{
  thread_exit_flag = true;
  if( !wait(5000) ) terminate();
  full_time=0;

  QMutexLocker locker(&mutex);

  mmmodules.clear();
  mmslots.clear();
  transactions_read.clear();
  transactions_write.clear();
  transactions_write2.clear();
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::set_module_node(int module_index, int node, int subnode )
{
  int i;
  for(i=0; i<mmslots.count(); i++ )
  { if( mmslots[i].module.n != module_index ) continue;
    mmslots[i].module.node    = node;
    mmslots[i].module.subnode = subnode;
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::add_module( int module_index, int node, int subnode,
                           const QString &name, const QString &description  )
{
  MMModule mm;
  mm.n       = module_index;
  mm.node    = node;
  mm.subnode = subnode;
  mm.name    = name;
  mm.desc    = description;
  mmmodules << mm;
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::add_slot( int module_index, int slot_index, int addr, int len,
                    MBDataType datatype,
                         const QString &description  )
{
  int i;
  MMSlot slot;
  for( i=0; i<mmmodules.count(); i++ )
  { if( mmmodules[i].n != module_index ) continue;
    slot.status   = MMSlot::NotInit;
    slot.n        = slot_index;
    slot.module   = mmmodules[i];
    slot.addr     = addr;
    slot.len      = len;
    slot.datatype = datatype;
    slot.desc     = description;
    slot.data.resize( len );
    mmslots << slot;
    break;
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::setTransport(AbstractSerialPortPtr transport )
{
  this->transport = transport;
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::run()
{
  msleep(200);

  QTime stateChanged_timer;
  QTime full_time_timer;
  QTime data_request_timer;
  int i=0;
  int j,k;

  if(transport) transport->resetLastErrorType();

  if( transactions_read.count() == 0 ) return; //#############

  stateChanged_timer.start();
  data_request_timer.start();
  mmslots_status.resize( mmslots.count() );
  mmslots_status.fill( -1 );
  full_time_timer.start();
  full_time=0;

  Console::Print( Console::Information, "Опрос модулей запущен.\n" );

  thread_exit_flag = false;
  while( !thread_exit_flag ) // цикл опроса
  {
    if( ( full_time > 100 ) && ( data_request_timer.elapsed() > 3*full_time ) )
    { data_request_timer.restart();
      int nk = mmslots.count();
      for( k=0; k<nk; k++ )
      { if( !mmslots[k].data_read_flag ) mmslots[k].status=MMSlot::Skiped;
        mmslots[k].data_read_flag = false;
      }
    }

    // транзакции на чтение
    if( !disable_read_transactions )
    { process_transaction( transactions_read[i] );
    }
    // обновление состояния опроса
    if( stateChanged_timer.elapsed() > 200 )
    { stateChanged_timer.start();
      int nk = mmslots.count();
      bool flag = false;
      for( k=0; k<nk; k++ )
      { if( mmslots_status[k] != mmslots[k].status )
        { mmslots_status[k] = mmslots[k].status;
          flag = true;
        }
      }
      if( flag )
      { emit stateChanged();
      }
    }
    i++;
    if( i < transactions_read.count() )  continue;
    i=0;

    optimize_write();
    optimize_write_holding();
    optimize_write_coils();
    optimize_write_holding_ext();

    if( !disable_write_transactions )
    { // транзакции на запись
      for( j=0; j<transactions_write.count(); j++ )
      { if( !transactions_write[j].execute_flag  ) continue;
        if( process_transaction( transactions_write[j] ) )
        { // OK
          transactions_write[j].execute_flag = false;
        } else
        { // ОШИБКА
          if(    (!transactions_write[j].errortimeout)
              && ( transactions_write[j].errorcode != 0) )
          { // отменяем все транзакции данного слота
            for( k=0; k<transactions_write.count(); k++ )
            { if(  ( transactions_write[k].slot->n        == transactions_write[j].slot->n )
                 &&( transactions_write[k].slot->module.n == transactions_write[j].slot->module.n ) )
              { transactions_write[k].execute_flag = false;
              }
            }
          }
          transactions_write[j].execute_flag = false;
        }
      }
    }

    msleep( 2 );

    while( cycle_time > full_time_timer.elapsed() )
    { msleep( 5 );
      if( thread_exit_flag ) break;
    }
    full_time = full_time_timer.restart(); // полное время опроса
  }
  if(transport) transport->close();
  Console::Print( Console::Information, "Опрос модулей остановлен.\n" );
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::optimize_write()
{
  QVector<int> flags_array;

  int i,j,k,a;
  int addr1,addr2,len1;
  int ni = transactions_write.count();
  flags_array.resize( ni );
  flags_array.fill( 0 );

  // этап 1: анализ
  k=1;
  for( i=0; i<(ni-1); i++ )
  { if( !transactions_write[i].execute_flag   ) goto skip;
    if( transactions_write[i].slot->datatype.isRegister()) goto skip;
    if( !transactions_write[i+1].execute_flag )
    { goto skip;
    } else
    { QByteArray &ba1 = transactions_write[i].request;
      QByteArray &ba2 = transactions_write[i+1].request;
      if( ba1[0] != ba2[0] ) goto skip;
      if( ba1[1] != ba2[1] ) goto skip;
      if( ba1[2] != ba2[2] ) goto skip;
      addr1 = ( (unsigned char)ba1[3] << 8 ) | (unsigned char)ba1[4];
      addr2 = ( (unsigned char)ba2[3] << 8 ) | (unsigned char)ba2[4];
      len1  = (unsigned char)ba1[5];
      a = ba1[2] & 0x0F;
      if( a == 0 ) goto skip; // ошибка -- транзакция на чтение
      if( a == 1 ) // просто запись
      { if( ( addr1 + len1 ) != addr2 ) goto skip;
      } else
      { // запись по AND,OR,XOR
        if( (addr1 != addr2) && ( addr1 + len1 ) != addr2 ) goto skip; // ад
      }
      flags_array[i]   = k;
      flags_array[i+1] = k;
    }
    continue;
  skip:
    k++;
  }

  // этап 2: построение транзакций
  k=0;
  for( i=0; i<ni; i++ )
  { if( flags_array[i] == 0 ) continue;
    { k = flags_array[i];
      j = i;
      while( ( j < ni ) && ( flags_array[j] == k ) ) j++;
      j--;
      //Console::Print( QString("ОПТИМИЗИРОВАННАЯ ЗАПИСЬ [%1;%2] (%3)\n").arg(i).arg(j).arg(k) );
      optimize_write_transaction( i,j );
      i=j;
    }
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::optimize_write_transaction(int i1, int i2)
{
  int i,j,k1,k2;
  QByteArray ba;
  QByteArray ba_answer;
  int addr1,len1;
  int addr2,len2;
  int mode;

  for(i=i1; i<=i2; i++)
  { transactions_write[i].execute_flag = false;
  }

  for(i=i1; i<=i2; i++)
  { QByteArray ba = transactions_write[i].request;
    addr1 = ((unsigned char)ba[3]<<8)|(unsigned char)ba[4];
    len1  =  (unsigned char)ba[5];
    ba.chop(2); // удаляем CRC16
    for( j=i+1; j<=i2; j++ )
    { QByteArray &ba2 = transactions_write[j].request;
      int s = ba2.size()-8;
      if( s < 0 ) return; // ошибка
      addr2 = ((unsigned char)ba2[3]<<8)|(unsigned char)ba2[4];
      len2  =  (unsigned char)ba2[5];
      mode  =  (unsigned char)ba2[2] & 0x0F;
      if( mode == 0 ) return; // ошибка
      if( mode == 1 )
      { // просто запись
        if( ( addr1 + len1 ) != addr2 ) return; // ошибка
        ba.append( QByteArray( ba2.constData()+6, s ) );
      } else
      { // запись по AND,OR,XOR
        if( ( addr1 + len1 ) == addr2 )
        { // смежные транзакции -- просто добавляем
          ba.append( QByteArray( ba2.constData()+6, s ) );
        } else
        { if( addr1 == addr2 )
          { if( len1 != len2 ) return; // ошибка
            k1 = ba.size()-len1;
            k2 = 6;
            if( mode == 3 ) // AND
            { while( k1 < ba.size() ) { ba[k1] = ba[k1] & ba2[k2]; k1++; k2++; }
            }
            if( mode == 5 ) // OR
            { while( k1 < ba.size() ) { ba[k1] = ba[k1] | ba2[k2]; k1++; k2++; }
            }
            if( mode == 7 ) // XOR
            { while( k1 < ba.size() ) { ba[k1] = ba[k1] ^ ba2[k2]; k1++; k2++; }
            }
          }
        }
      }
      //transactions_write[j].execute_flag = false;
      addr1 = addr2;
      len1  = len2;
      //if( ba.size() > 128 ) break;
      if( ba.size() > ((max_packet_length&0x7FFFFFFC)-6) ) break;
    }

    int len = ba.size()-6;
    ba[5] = len;
    CRC::appendCRC16( ba );
    ba_answer.resize( len + 8 );
    ba_answer.fill(0);
    if( !transport  ) return;
    int errorcode;
    request_counter++;
    msleep( transaction_delay );
    transport->query( ba, ba_answer, &errorcode );
    //Console::Print( "ОПТИМИЗИРОВАННАЯ ЗАПИСЬ: " + QByteArray2QString( ba ) + "\n" );
    i = j;
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::optimize_write_holding()
{
  QVector<int> flags_array;

  int i,j,k;
  int addr1,addr2;
  int ni = transactions_write.count();
  flags_array.resize( ni );
  flags_array.fill( 0 );

  // этап 1: анализ
  k=1;
  for( i=0; i<(ni-1); i++ )
  { if( !transactions_write[i].execute_flag   ) goto skip;
    if( transactions_write[i].slot->datatype.id() != MBDataType::HoldingRegisters) goto skip;
    if( !transactions_write[i+1].execute_flag )
    { goto skip;
    } else
    { QByteArray &ba1 = transactions_write[i].request;
      QByteArray &ba2 = transactions_write[i+1].request;
      if( ba1[0] != ba2[0] ) goto skip;
      if( ba1[1] != ba2[1] ) goto skip;
      addr1 = ( (unsigned char)ba1[3]) | ((unsigned char)ba1[2] << 8);
      addr2 = ( (unsigned char)ba2[3]) | ((unsigned char)ba2[2] << 8);
      if( ( addr1 + 1 ) != addr2 ) goto skip;
      flags_array[i]   = k;
      flags_array[i+1] = k;
    }
    continue;
  skip:
    k++;
  }

  // этап 2: построение транзакций
  k=0;
  for( i=0; i<ni; i++ )
  { if( flags_array[i] == 0 ) continue;
    { k = flags_array[i];
      j = i;
      while( ( j < ni ) && ( flags_array[j] == k ) ) j++;
      j--;
      //Console::Print( QString("ОПТИМИЗИРОВАННАЯ ЗАПИСЬ [%1;%2] (%3)\n").arg(i).arg(j).arg(k) );
      optimize_write_transaction_holding( i,j );
      i=j;
    }
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::optimize_write_transaction_holding(int i1, int i2)
{
  int i,j;
  QByteArray ba;
  QByteArray ba_copy;
  QByteArray ba_answer;
  int addr1;
  int s;

  for(i=i1; i<=i2; i++)
  { transactions_write[i].execute_flag = false;
  }

  for(i=i1; i<=i2; )
  { QByteArray ba = transactions_write[i].request;
    ba_copy = ba;
    addr1 = ( (unsigned char)ba[2] << 8)  | ((unsigned char)ba[3]);
    int len;  //number of registers that would fit into one package
    for( len = i2 - i + 1; len > 1; len--)
    { if ((len*2 + 9) <= (max_packet_length - 4)) break;
    }
    if (len > 1) //generating package
    { ba[1] = 0x10;
      ba[4] = len >> 8;
      ba[5] = len;
      ba[6] = (len * 2);
      ba.resize(7);
      for (j = i; j < i+len; j++)
      {  QByteArray &ba2 = transactions_write[j].request;
         s = ba2.size() - 6;
         if (s < 0) return;
         ba.append( QByteArray( ba2.constData() + 4, s) );
      }
      CRC::appendCRC16( ba );
    }
    else ba = ba_copy; //single write
    ba_answer.resize( 8 );
    ba_answer.fill(0);
    if( !transport  ) return;
    int errorcode;
    request_counter++;
    msleep( transaction_delay );
    transport->query( ba, ba_answer, &errorcode );
    i = i + len;
    addr1 = addr1 + len;
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::optimize_write_holding_ext()
{
  QVector<int> flags_array;

  int i,j,k;
  int addr1,addr2,count1;
  int ni = transactions_write.count();
  flags_array.resize( ni );
  flags_array.fill( 0 );

  // этап 1: анализ
  k=1;
  for( i=0; i<(ni-1); i++ )
  { if( !transactions_write[i].execute_flag   ) goto skip;
    switch( transactions_write[i].slot->datatype.id() )
    { case( MBDataType::DwordsHoldingRegHiLo ):
      case( MBDataType::FloatsHoldingRegHiLo ):
      case( MBDataType::DwordsHoldingRegLoHi ):
      case( MBDataType::FloatsHoldingRegLoHi ):
        break;
      default:
        goto skip;
        break;
    }
    if( !transactions_write[i+1].execute_flag )
    { goto skip;
    } else
    { QByteArray &ba1 = transactions_write[i].request;
      QByteArray &ba2 = transactions_write[i+1].request;
      if( ba1[0] != ba2[0] ) goto skip;
      if( ba1[1] != ba2[1] ) goto skip;
      addr1  = ( (unsigned char)ba1[3]) | ((unsigned char)ba1[2] << 8);
      addr2  = ( (unsigned char)ba2[3]) | ((unsigned char)ba2[2] << 8);
      count1 = ( (unsigned char)ba1[5]) | ((unsigned char)ba1[4] << 8);
      if( ( addr1 + count1 ) != addr2 ) goto skip;
      flags_array[i]   = k;
      flags_array[i+1] = k;
    }
    continue;
  skip:
    k++;
  }

  // этап 2: построение транзакций
  k=0;
  for( i=0; i<ni; i++ )
  { if( flags_array[i] == 0 ) continue;
    { k = flags_array[i];
      j = i;
      while( ( j < ni ) && ( flags_array[j] == k ) ) j++;
      j--;
      //Console::Print( Console::Debug, QString("ОПТИМИЗИРОВАННАЯ ЗАПИСЬ [%1;%2] (%3)\n").arg(i).arg(j).arg(k) );
      optimize_write_transaction_holding_ext( i,j );
      i=j;
    }
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::optimize_write_transaction_holding_ext(int i1, int i2)
{
  int i,j;
  QByteArray ba;
  QByteArray ba_copy;
  QByteArray ba_answer;
  int addr1;
  int s;

  for(i=i1; i<=i2; i++)
  { transactions_write[i].execute_flag = false;
  }

  for(i=i1; i<=i2; )
  { QByteArray ba = transactions_write[i].request;
    ba_copy = ba;
    addr1 = ( (unsigned char)ba[2] << 8)  | ((unsigned char)ba[3]);
    int len;  //number of registers that would fit into one package
    for( len = i2 - i + 1; len > 1; len--)
    { if ((len*4 + 9) <= (max_packet_length - 4)) break;
    }
    if (len > 1) //generating package
    { ba[1] = 0x10;
      ba[4] = (2*len) >> 8;
      ba[5] = (2*len);
      ba[6] = (len * 4);
      ba.resize(7);
      for (j = i; j < i+len; j++)
      {  const QByteArray &ba2 = transactions_write[j].request;
         s = ba2.size() - 9;
         if (s < 0) return;
         ba.append( QByteArray( ba2.constData() + 7, s) );
      }
      CRC::appendCRC16( ba );
    }else
    { ba = ba_copy; //single write
    }
    ba_answer.resize( 8 );
    ba_answer.fill(0);
    if( !transport  ) return;
    int errorcode;
    request_counter++;
    msleep( transaction_delay );
    transport->query( ba, ba_answer, &errorcode );
    i = i + len;
    addr1 = addr1 + 2*len;
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::optimize_write_coils()
{
  QVector<int> flags_array;

  int i,j,k;
  int addr1,addr2;
  int ni = transactions_write.count();
  flags_array.resize( ni );
  flags_array.fill( 0 );

  // этап 1: анализ
  k=1;
  for( i=0; i<(ni-1); i++ )
  { if( !transactions_write[i].execute_flag   ) goto skip;
    if( transactions_write[i].slot->datatype.id() != MBDataType::Coils) goto skip;
    if( !transactions_write[i+1].execute_flag )
    { goto skip;
    } else
    { QByteArray &ba1 = transactions_write[i].request;
      QByteArray &ba2 = transactions_write[i+1].request;
      if( ba1[0] != ba2[0] ) goto skip;
      if( ba1[1] != ba2[1] ) goto skip;
      addr1 = ( (unsigned char)ba1[3]) | ((unsigned char)ba1[2] << 8);
      addr2 = ( (unsigned char)ba2[3]) | ((unsigned char)ba2[2] << 8);
      if( ( addr1 + 1 ) != addr2 ) goto skip;
      flags_array[i]   = k;
      flags_array[i+1] = k;
    }
    continue;
  skip:
    k++;
  }

  // этап 2: построение транзакций
  k=0;
  for( i=0; i<ni; i++ )
  { if( flags_array[i] == 0 ) continue;
    { k = flags_array[i];
      j = i;
      while( ( j < ni ) && ( flags_array[j] == k ) ) j++;
      j--;
      //Console::Print( Console::Debug, QString("ОПТИМИЗИРОВАННАЯ ЗАПИСЬ [%1;%2] (%3)\n").arg(i).arg(j).arg(k) );
      optimize_write_transaction_coils( i,j );
      i=j;
    }
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::optimize_write_transaction_coils(int i1, int i2)
{
  int i,j;
  QByteArray ba;
  QByteArray ba_copy;
  QByteArray ba_answer;
  int addr1;
  int n_asterisk; //n* from specifications

  for(i=i1; i<=i2; i++)
  { transactions_write[i].execute_flag = false;
  }

  for(i=i1; i<=i2; )
  { QByteArray ba = transactions_write[i].request;
    ba_copy = ba;
    unsigned char val = ba[4]; //FF or 00 in coils write package
    ba.chop(4);
    addr1 = ( (unsigned char)ba[2] << 8)  | ((unsigned char)ba[3]);
    int len;  //number of coils that would fit into one package
    for( len = i2 - i + 1; len > 1; len--)
    { n_asterisk = ((len - 1)/8 + 1);
      if ((n_asterisk + 9) <= (max_packet_length - 4)) break;
    }
    if (len > 1) //generating package
    {
     n_asterisk = ((len - 1)/8 + 1);
     ba.resize(7 + n_asterisk); //w/o CRC
     ba[1] = 0x0F;
     //ba[2] and ba[3] remains unchanhed
     ba[4] = len >> 8;
     ba[5] = len;
     ba[6] = n_asterisk;
     for (j = 1; j <= n_asterisk; j++)
     { ba[6+j] = val; //might be incorrect
     }
     //work with unused bits
     int unused = 8 * n_asterisk - len;
     ba[6 + j-1] = val >> unused;
     CRC::appendCRC16( ba );
    }
    else ba = ba_copy; //single write
    ba_answer.resize( 6 );
    ba_answer.fill(0);
    if( !transport  ) return;
    int errorcode;
    request_counter++;
    msleep( transaction_delay );
    transport->query( ba, ba_answer, &errorcode );
    i = i + len;
    addr1 = addr1 + len;
  }
}

//===================================================================
//
//===================================================================
inline static float ptr2float( const char *ptr, bool mode )
{
  union
  { char  b[4];
    float f;
  } u;

  if( mode )
  { u.b[0] = ptr[1];
    u.b[1] = ptr[0];
    u.b[2] = ptr[3];
    u.b[3] = ptr[2];
  } else
  { u.b[0] = ptr[3];
    u.b[1] = ptr[2];
    u.b[2] = ptr[1];
    u.b[3] = ptr[0];
  }
  return u.f;
}

//===================================================================
//
//===================================================================
inline static qint32 ptr2dword( const char *ptr, bool mode )
{
  union
  { char   b[4];
    qint32 dw;
  } u;

  if( mode )
  { u.b[0] = ptr[1];
    u.b[1] = ptr[0];
    u.b[2] = ptr[3];
    u.b[3] = ptr[2];
  } else
  { u.b[0] = ptr[3];
    u.b[1] = ptr[2];
    u.b[2] = ptr[0];
    u.b[3] = ptr[1];
  }
  return u.dw;
}

//===================================================================
//
//===================================================================
inline static void float2ptr( float f, char *ptr, bool mode )
{
  union
  { char  b[4];
    float f;
  } u;

  u.f = f;

  if( mode )
  { ptr[0] = u.b[1];
    ptr[1] = u.b[0];
    ptr[2] = u.b[3];
    ptr[3] = u.b[2];
  } else
  { ptr[0] = u.b[3];
    ptr[1] = u.b[2];
    ptr[2] = u.b[1];
    ptr[3] = u.b[0];
  }
}

//===================================================================
//
//===================================================================
inline static void dword2ptr( qint32 dw, char *ptr, bool mode )
{
  union
  { char   b[4];
    qint32 dw;
  } u;

  u.dw = dw;

  if( mode )
  { ptr[0] = u.b[1];
    ptr[1] = u.b[0];
    ptr[2] = u.b[3];
    ptr[3] = u.b[2];
  } else
  { ptr[0] = u.b[3];
    ptr[1] = u.b[2];
    ptr[2] = u.b[1];
    ptr[3] = u.b[0];
  }
}

//===================================================================
//
//===================================================================
bool MBMasterPrivate::process_transaction( MMSlotTransaction &tr )
{
  int i;
  int data_offset;

  if( transport == 0 ) return false;

  if(    ( tr.slot->status == MMSlot::Skiped)
      || ( tr.slot->status == MMSlot::NotInit ) )
  { if( auto_skip_mode && !tr.slot->data_read_flag )
    { goto skiped;
    }
  }

  tr.answer.fill(0);

  {
  bool ok = true;
  const bool isArduino = tr.slot->datatype.isArduino();
  QByteArray answer;
  const QByteArray request = isArduino ? encodeArduinoTransport(tr.request) : tr.request;
  if (isArduino)
  {
    answer.resize(encodedArduinoTransportLength(tr.answer.length()));
  }
  else
  {
    answer = tr.answer;
  }

  request_counter++;
  msleep( transaction_delay );
  i = transport->query( request, answer, &tr.errorcode );

  if (isArduino)
  {
    QByteArray temp = decodeArduinoTransport(answer, ok);
    if (ok) {
      tr.answer = temp;
      i = temp.length();
    }
  }
  else
  {
     tr.answer = answer;
  }

  }

  {

  QMutexLocker locker(&mutex);

  tr.errortimeout = false;
  if( i == 0 )
  { tr.errortimeout = true;
    goto exit;
  }
  if( i != tr.exp_length )            goto exit;
  if( i != tr.answer.length() )       goto exit;


  if( tr.slot->datatype.isRegister() )
  {
    if( tr.answer[0] != tr.request[0] ) goto exit;
    if( tr.answer[1] != tr.request[1] ) goto exit;
    if( CRC::CRC16( tr.answer ) )       goto exit;

    switch( tr.answer[1] )
    {
      case( 0x01 ): // Read Coils
        if( tr.slot->datatype.id() != MBDataType::Coils ) goto exit;
        for(i=0; i<tr.length; i++ )
        {  tr.slot->data[i+tr.offset]  =
               ((*((char*)(tr.answer.data()+3+i/8)))&(1<<(i&7)))?(1):(0);
        }
        break;
      case( 0x02 ): // Read Discrete Inputs
        if( tr.slot->datatype.id() != MBDataType::DiscreteInputs ) goto exit;
        for(i=0; i<tr.length; i++ )
        {  tr.slot->data[i+tr.offset]  =
               ((*((char*)(tr.answer.data()+3+i/8)))&(1<<(i&7)))?(1):(0);
        }
        break;
      case( 0x03 ): // Read Holding Registers
        switch( tr.slot->datatype.id() )
        { case( MBDataType::HoldingRegisters ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = swap_short(*((short*)(tr.answer.data()+3+2*i)));
            }
            break;
          case( MBDataType::FloatsHoldingRegHiLo ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = ptr2float( tr.answer.constData()+3+4*i, false );
            }
            break;
          case( MBDataType::FloatsHoldingRegLoHi ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = ptr2float( tr.answer.constData()+3+4*i, true );
            }
            break;
          case( MBDataType::DwordsHoldingRegHiLo ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = ptr2dword( tr.answer.constData()+3+4*i, false );
            }
            break;
          case( MBDataType::DwordsHoldingRegLoHi ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = ptr2dword( tr.answer.constData()+3+4*i, true );
            }
            break;
          default:
            goto exit;
            break;
        }
        break;
      case( 0x04 ): // Read Input Registers
         switch( tr.slot->datatype.id() )
        { case( MBDataType::InputRegisters ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = swap_short(*((short*)(tr.answer.data()+3+2*i)));
            }
            break;
          case( MBDataType::FloatsInputRegHiLo ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = ptr2float( tr.answer.constData()+3+4*i, false );
            }
            break;
          case( MBDataType::FloatsInputRegLoHi ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = ptr2float( tr.answer.constData()+3+4*i, true );
            }
            break;
          case( MBDataType::DwordsInputRegHiLo ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = ptr2dword( tr.answer.constData()+3+4*i, false );
            }
            break;
          case( MBDataType::DwordsInputRegLoHi ):
            for(i=0; i<tr.length; i++ )
            { tr.slot->data[i+tr.offset] = ptr2dword( tr.answer.constData()+3+4*i, true );
            }
            break;
          default:
            goto exit;
            break;
        }
        break;
      case( 0x05 ): // Write Single Coil
        break;
      case( 0x06 ): // Write Single Register
        break;
      case( 0x0F ): // Write Multiple Coils
        break;
      case( 0x10 ): // Write Multiple registers
        break;
      default:
        goto exit; break;
    }
  }
  else if( tr.slot->datatype.isArduino() )
  {
    if( tr.answer[0] != tr.request[0] ) goto exit;
    if( tr.answer[1] != tr.request[1] ) goto exit;
    if( tr.answer[2] != tr.request[2] ) goto exit;
    if( tr.answer[3] != tr.request[3] ) goto exit;
    data_offset = 4;
    if( tr.request[0] == (char)0x00 )
    { // чтение
      switch( tr.slot->datatype.id() )
      { case( MBDataType::BitsArduino   ):
          for(i=0; i<tr.length; i++ )
          {  tr.slot->data[i+tr.offset]  =
                    ((*((char*)(tr.answer.data()+data_offset+i/8)))&(1<<(i&7)))?(1):(0);
          }
          break;
        case( MBDataType::BytesArduino   ):
          for(i=0; i<tr.length; i++ )
          { tr.slot->data[i+tr.offset]        = *((char*)(tr.answer.data()+data_offset+i));
          }
          break;
        case( MBDataType::WordsArduino  ):
          for(i=0; i<tr.length; i++ )
          { tr.slot->data[i+tr.offset]        = *((short*)(tr.answer.data()+data_offset+2*i));
          }
          break;
        case( MBDataType::DwordsArduino ):
          for(i=0; i<tr.length; i++ )
          { tr.slot->data[i+tr.offset]        = *((int*)(tr.answer.data()+data_offset+4*i));
          }
          break;
        case( MBDataType::FloatsArduino ):
          for(i=0; i<tr.length; i++ )
          { tr.slot->data[i+tr.offset]        = *((float*)(tr.answer.data()+data_offset+4*i));
          }
          break;
        default: break;
      }
    }
  }
  else //mikkon package
  {
    if( (tr.request[1] != (char) 0x41) && (tr.request[1] != (char) 0x43) ) goto exit;  //check operation code
    if( tr.answer[0] != tr.request[0] ) goto exit;
    if( tr.answer[1] != tr.request[1] ) goto exit;
    if( tr.answer[2] != tr.request[2] ) goto exit;
    if( CRC::CRC16( tr.answer ) )       goto exit;
    if( tr.request[2] & 0x0F )
    { // запись
      data_offset = 6;
      if( tr.answer[3] != tr.request[3] ) goto exit;
      if( tr.answer[4] != tr.request[4] ) goto exit;
    } else
    { // чтение
      data_offset = 4;
    }

    if( ( tr.request[2] & 0x0F ) == 0 )
    { // чтение
      switch( tr.slot->datatype.id() )
      { case( MBDataType::Bits   ):
          for(i=0; i<tr.length; i++ )
          {  tr.slot->data[i+tr.offset]  =
                    ((*((char*)(tr.answer.data()+data_offset+i/8)))&(1<<(i&7)))?(1):(0);
          }
          break;
        case( MBDataType::Bytes  ):
          for(i=0; i<tr.length; i++ )
          { tr.slot->data[i+tr.offset]        = *((char*)(tr.answer.data()+data_offset+i));
          }
          break;
        case( MBDataType::Words  ):
          for(i=0; i<tr.length; i++ )
          { tr.slot->data[i+tr.offset]        = *((short*)(tr.answer.data()+data_offset+2*i));
          }
          break;
        case( MBDataType::Dwords ):
          for(i=0; i<tr.length; i++ )
          { tr.slot->data[i+tr.offset]        = *((int*)(tr.answer.data()+data_offset+4*i));
          }
          break;
        case( MBDataType::Floats ):
          for(i=0; i<tr.length; i++ )
          { tr.slot->data[i+tr.offset]        = *((float*)(tr.answer.data()+data_offset+4*i));
          }
          break;
        default: break;
      }
    }
  }

  tr.slot->status=MMSlot::Ok;
  answer_counter++;
  return true;
  }
exit:
  error_counter++;
  tr.slot->status=MMSlot::Bad;
  for(i=0; i<tr.length; i++ )
  { tr.slot->data[i+tr.offset].setStatus( MMValue::Bad );
  }
  return false;
skiped:
  for(i=0; i<tr.length; i++ )
  { tr.slot->data[i+tr.offset].setStatus( MMValue::NotInit );
  }
  return false;
}

//===================================================================
//
//===================================================================
MMSlot MBMasterPrivate::getSlot(int module, int slot ) const
{
  int i,n;
  QMutexLocker locker(&mutex);

  n = mmslots.count();
  for(i=0; i<n; i++ )
  { if( mmslots[i].module.n != module ) continue;
    if( mmslots[i].n != slot )          continue;
    mmslots[i].data_read_flag=true;
    return mmslots[i];
    break;
  }
  return MMSlot();
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::setSlotValue(int module, int slot,int index, const MMValue &value )
{
  char buff[8];
  int  buff_len=0;

  int i,j;
  //Console::Print( QString("Запись: %1/%2/%3 %4\n").arg(module).arg(slot).arg(index).arg(value.toString()) );

  QMutexLocker locker(&mutex);

   i=transactions_write_map.value( ModuleSlotIndex(module,slot,index), -1 );
   if( (i>=0) && (i<transactions_write.count()) )
   {
    //Console::Print( QString("Запись: слот найден!\n") );
    const MBDataType datatype = transactions_write[i].slot->datatype;
    if (datatype.isRegister())
    {
      switch( transactions_write[i].slot->datatype.id() )
      { case( MBDataType::HoldingRegisters ):
        case( MBDataType::Coils ):
          transactions_write[i].request.resize(4);
          break;
        case( MBDataType::DwordsHoldingRegHiLo ):
        case( MBDataType::DwordsHoldingRegLoHi ):
        case( MBDataType::FloatsHoldingRegHiLo ):
        case( MBDataType::FloatsHoldingRegLoHi ):
          transactions_write[i].request.resize(7);
          break;
      }
      switch( transactions_write[i].slot->datatype.id() )
      { case( MBDataType::HoldingRegisters ):
          (*(short*)buff) = (swap_short(value.toInt()) & 0xFFFF );
          buff_len=2;
          break;
        case( MBDataType::Coils ):
          if (value.toInt() == 0) (*(short*)buff) = 0x0000;
          else (*(short*)buff) = 0x00FF;
          buff_len=2;
          break;
        case( MBDataType::DwordsHoldingRegHiLo ):
          dword2ptr( value.toInt(), buff, false );
          //Console::Print( Console::Debug, "запись :" + QByteArray2QString( transactions_write[i].request ) + "\n"  );
          buff_len=4;
          break;
        case( MBDataType::DwordsHoldingRegLoHi ):
          dword2ptr( value.toInt(), buff, true  );
          buff_len=4;
          break;
        case( MBDataType::FloatsHoldingRegHiLo ):
          float2ptr( (float)value.toDouble(), buff, false );
          buff_len=4;
          break;
        case( MBDataType::FloatsHoldingRegLoHi ):
          float2ptr( (float)value.toDouble(), buff, true  );
          buff_len=4;
          break;
      }
      for(j=0; j<buff_len; j++ ) transactions_write[i].request.append( buff[j] );
      CRC::appendCRC16( transactions_write[i].request );
    }
    else if (datatype.isArduino())
    {
      transactions_write[i].request.resize(4);
      //Console::Print(Console::Debug, "запись 1:" + QByteArray2QString( transactions_write[i].request ) + "\n"  );
      switch( transactions_write[i].slot->datatype.id() )
      { case( MBDataType::BitsArduino ):
          if( value.toInt() != 0 )
          { (*(char*)buff) = 1 << (index&7);
             transactions_write[i].request[0] =  0x02; // OR
          } else
          { (*(char*)buff) = 0xFF & (~( 1 << (index&7)));
            transactions_write[i].request[0] =   0x03; // AND
          }
          buff_len=1;
          break;
        case( MBDataType::BytesArduino  )          :  (*(char*)buff)  = (value.toInt() & 0xFF );     buff_len=1; break;
        case( MBDataType::WordsArduino  )          :  (*(short*)buff) = (value.toInt() & 0xFFFF );   buff_len=2; break;
        case( MBDataType::DwordsArduino )          :  (*(int*)buff)   =          value.toInt();      buff_len=4; break;
        case( MBDataType::FloatsArduino )          :  (*(float*)buff) = (float)( value.toDouble() ); buff_len=4; break;
        default: buff_len=0;
      }
      for(j=0; j<buff_len; j++ ) transactions_write[i].request.append( buff[j] );
    }
    else  // mikkon
    {
      transactions_write[i].request.resize(6);
      //Console::Print(Console::Debug, "запись 1:" + QByteArray2QString( transactions_write[i].request ) + "\n"  );
      switch( transactions_write[i].slot->datatype.id() )
      { case( MBDataType::Bits   ):
          if( value.toInt() != 0 )
          { (*(char*)buff) = 1 << (index&7);
             transactions_write[i].request[2] =
                   0x05 | ( transactions_write[i].slot->module.subnode << 4 ); // OR
          } else
          { (*(char*)buff) = 0xFF & (~( 1 << (index&7)));
            transactions_write[i].request[2] =
                  0x03 | ( transactions_write[i].slot->module.subnode << 4 ); // AND
          }
          buff_len=1;
          break;
        case( MBDataType::Bytes  )          :  (*(char*)buff)  = (value.toInt() & 0xFF );     buff_len=1; break;
        case( MBDataType::Words  )          :  (*(short*)buff) = (value.toInt() & 0xFFFF );   buff_len=2; break;
        case( MBDataType::Dwords )          :  (*(int*)buff)   =          value.toInt();      buff_len=4; break;
        case( MBDataType::Floats )          :  (*(float*)buff) = (float)( value.toDouble() ); buff_len=4; break;
        default: buff_len=0;
      }
      for(j=0; j<buff_len; j++ ) transactions_write[i].request.append( buff[j] );
      CRC::appendCRC16( transactions_write[i].request );
    }

    transactions_write[i].execute_flag=true;
    //Console::Print( Console::Debug, "запись 2:" + QByteArray2QString( transactions_write[i].request ) + "\n"  );
  }
}

//===================================================================
//
//===================================================================
MMValue MBMasterPrivate::getSlotValue(int module, int slot,int index ) const
{
  MMValue ret;
  int i,n;
  QMutexLocker locker(&mutex);

  n = mmslots.count();
  for(i=0; i<n; i++ )
  { if( mmslots[i].module.n != module ) continue;
    if( mmslots[i].n != slot )          continue;
    if( index >= mmslots[i].data.count() ) break;
    mmslots[i].data_read_flag=true;
    return mmslots[i].data[index];
  }
  ret.setStatus( MMValue::Bad );
  return ret;
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::setSlotAttributes(int module, int slot, const QString &attributes )
{
  int i,n;
  QMutexLocker locker(&mutex);

  n = mmslots.count();
  for(i=0; i<n; i++ )
  { if( mmslots[i].module.n != module ) continue;
    if( mmslots[i].n        != slot )   continue;
    mmslots[i].attributes = attributes;
    break;
  }
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::setReadTransactionsDisabled( bool disabled )
{
  QMutexLocker locker(&mutex);
  disable_read_transactions = disabled;
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::setWriteTransactionsDisabled( bool disabled )
{
  QMutexLocker locker(&mutex);
  disable_write_transactions = disabled;
}

//===================================================================
//
//===================================================================
bool MBMasterPrivate::readTransactionsDisabled() const
{
  QMutexLocker locker(&mutex);
  return disable_read_transactions;
}

//===================================================================
//
//===================================================================
bool MBMasterPrivate::writeTransactionsDisabled() const
{
  QMutexLocker locker(&mutex);
  return disable_write_transactions ;
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::setMaximumPacketLength( int packet_length )
{
  if( packet_length < 16  ) packet_length = 16;
  if( packet_length > 256 ) packet_length = 256;
  max_packet_length = packet_length;
}

//===================================================================
//
//===================================================================
int MBMasterPrivate::maximumPacketLength() const
{
  return max_packet_length;
}


//===================================================================
// Расшифровка ModuleDefinition
//===================================================================
MikkonModuleDefinition MBMasterPrivate::decodeModuleDefinition( const QByteArray &ba )
{
#include "packed_struct_begin.h"
  struct PACKED_STRUCT_HEADER ModuleDefinition
  { quint8   StructureLength;              // Structure Length(=128)
    quint16  ID;                           // Module ID
    quint8   MajorVersion;                 // Version Major
    quint8   MinorVersion;                 //         Minor
    char     ReleaseDate[8];               // Release Date
    char     ReleaseMadeBy[3];             // Release Made by XYZ (Initials)
    char     ProductName[40];              // Product Name
    char     ProductDescription[40];       // Product Description
    char     CompanyName[16];              // Company Name
    char     LegalCopyright[16];           // Legal Copyright
  } *st;
  MikkonModuleDefinition md;
#include "packed_struct_end.h"
  if( ba.count() < (int)sizeof(st) ) return md;

  st = (ModuleDefinition*)ba.data();
  md.id                  = st->ID;
  md.majorVersion        = st->MajorVersion;
  md.minorVersion        = st->MinorVersion;
  md.releaseDate         = QString::fromLatin1( st->ReleaseDate,        sizeof( st->ReleaseDate        ) );
  md.releaseMadeBy       = QString::fromLatin1( st->ReleaseMadeBy,      sizeof( st->ReleaseMadeBy      ) );
  md.productName         = QString::fromLatin1( st->ProductName,        sizeof( st->ProductName        ) );
  md.productDescription  = QString::fromLatin1( st->ProductDescription, sizeof( st->ProductDescription ) );
  md.companyName         = QString::fromLatin1( st->CompanyName,        sizeof( st->CompanyName        ) );
  md.legalCopyright      = QString::fromLatin1( st->LegalCopyright,     sizeof( st->LegalCopyright     ) );
  return md;
}

//###################################################################
//
//###################################################################
MBMaster::MBMaster( QObject *parent )
  : QObject( parent ), d( new MBMasterPrivate(0) )
{
  d->moveToThread( d );
  connect( d, SIGNAL( stateChanged() ), this, SIGNAL( stateChanged() ) );
}

//===================================================================
//
//===================================================================
MBMaster::~MBMaster()
{
  delete d;
}

//===================================================================
//! Привязка к транспорту
//===================================================================
void MBMaster::setTransport(AbstractSerialPortPtr transport )
{
  d->setTransport( transport );
}

//===================================================================
//! Очистка конфигурации
//===================================================================
void MBMaster::clear_configuration()
{
  d->clear_configuration();
}

//===================================================================
//! Поменять адрес модуля
//===================================================================
void MBMaster::set_module_node(int module_index, int node, int subnode )
{
  d->set_module_node( module_index, node, subnode );
}

//===================================================================
//! Добавить модуль
/**
  \param module_index   Номер модуля
  \param node           Адрес
  \param subnode        Дополнительный адрес
  \param name           Имя модуля
  \param description    Описание модуля
*/
//===================================================================
void MBMaster::add_module( int module_index, int node, int subnode,
                                     const QString &name ,
                                     const QString &description )
{
  d->add_module(  module_index,  node, subnode,
                                     name ,  description );
}

//===================================================================
//! Добавить слот
/**
  \param module_index   Номер модуля
  \param slot_index     Номер слота
  \param addr           Начальный адрес
  \param len            Длина (Количество запрашиваемых данных)
  \param datatype       Тип данных
  \param description    Описание
*/
//===================================================================
void MBMaster::add_slot( int module_index, int slot_index, int addr, int len,
                    MBDataType datatype, const QString &description  )
{
  d->add_slot( module_index, slot_index, addr, len,
                     datatype, description  );
}

//===================================================================
//! Запустить опрос
//===================================================================
void MBMaster::polling_start()
{
  d->polling_start();
}

//===================================================================
//! Остановить опрос
//===================================================================
void MBMaster::polling_stop()
{
  d->polling_stop();
}

//===================================================================
//! Прочитать все данные из слота
/**
  \param module Номер модуля
  \param slot   Номер слота
*/
//===================================================================
MMSlot MBMaster::getSlot(int module, int slot ) const
{
  return d->getSlot( module,  slot );
}

//===================================================================
//! Прочитать значение
/**
  \param module Номер модуля
  \param slot   Номер слота
  \param index  Номер сигнала
*/
//===================================================================
MMValue MBMaster::getSlotValue(int module, int slot,int index ) const
{
  return d->getSlotValue( module, slot, index );
}

//===================================================================
//! Записать значение
/**
  \param module Номер модуля
  \param slot   Номер слота
  \param index  Номер сигнала
  \param value  Значение
*/
//===================================================================
void MBMaster::setSlotValue(int module, int slot,int index,
                                const MMValue &value )
{
  d->setSlotValue( module,  slot, index, value );
}

//===================================================================
//! Установить аттрибуты слота
/**
  \param module      Номер модуля
  \param slot        Номер слота
  \param attributes  Аттрибуты
*/
//===================================================================
void MBMaster::setSlotAttributes(int module, int slot, const QString &attributes )
{
  d->setSlotAttributes( module,  slot,  attributes );
}

//===================================================================
//! Запретить транзакции на чтение
/**
  \param disabled
    - \b true - транзакции на чтение запрещены,
    - \b false - разрешены
*/
//===================================================================
void MBMaster::setReadTransactionsDisabled( bool disabled )
{
  d->setReadTransactionsDisabled( disabled );
}

//===================================================================
//! Запретить транзакции на запись
/**
  \param disabled
    - \b true  - транзакции на запись запрещены,
    - \b false - разрешены
*/
//===================================================================
void MBMaster::setWriteTransactionsDisabled( bool disabled )
{
  d->setWriteTransactionsDisabled( disabled );
}

//===================================================================
//! Статус запрета транзакции на чтение
/**
  \return
    - \b true  - транзакции на чтение запрещены,
    - \b false - разрешены
*/
//===================================================================
bool MBMaster::readTransactionsDisabled() const
{
  return d->readTransactionsDisabled();
}

//===================================================================
//! Статус запрета на запись
/**
  \return
    - \b true  - транзакции на запись запрещены,
    - \b false - разрешены
*/
//===================================================================
bool MBMaster::writeTransactionsDisabled() const
{
  return d->writeTransactionsDisabled();
}

//===================================================================
//! Установить максимальную длину пакета
/**
  \param packet_length - максимальная длина пакета (байт)
*/
//===================================================================
void MBMaster::setMaximumPacketLength( int packet_length )
{
  return d->setMaximumPacketLength( packet_length );
}

//===================================================================
//! Максимальная длина пакета
/**
  \return максимальная длина пакета (байт)
*/
//===================================================================
int MBMaster::maximumPacketLength() const
{
  return d->maximumPacketLength();
}

//===================================================================
//! Общее кол-во запросов
//===================================================================
int MBMaster::request_counter() const
{ return d->request_counter;
}

//===================================================================
//! Общее кол-во ответов
//===================================================================
int MBMaster::answer_counter() const
{ return d->answer_counter;
}

//===================================================================
//! Общее кол-во ошибок
//===================================================================
int MBMaster::error_counter() const
{ return d->error_counter;
}

//===================================================================
//! Время (в мс) полного цикла опроса
//===================================================================
int MBMaster::full_time() const
{ return d->full_time;
}

//===================================================================
//! Установка времени ожидания между запросами
/**
  \param packet_length - время ожидания (мс)
*/
//===================================================================
void MBMaster::setTransactionDelay( int delay )
{ if( delay < 0 )     delay = 0;
  if( delay > 10000 ) delay = 10000;
  d->transaction_delay = delay;
}

//===================================================================
//! Текущее значение времени ожидания между запросами
/**
  \return время ожидания (мс)
*/
//===================================================================
int  MBMaster::transactionDelay()
{ return d->transaction_delay;
}

//===================================================================
//! Установка времени полного цикла опроса
/**
  \param packet_length - времени полного цикла опроса (мс)
*/
//===================================================================
void MBMaster::setCycleTime( int time )
{ if( time < 0 ) time = 0;
  d->cycle_time = time;
}

//===================================================================
//! Текущее значение времени полного цикла опроса
/**
  \return времени полного цикла опроса (мс)
*/
//===================================================================
int  MBMaster::cycleTime()
{ return d->cycle_time;
}

//===================================================================
//! Установка режима автоматического пропуска неиспользуемых пакетов
/**
  \param autoSkipMode
       - \b true  -  неиспользуемые пакеты не опрашиваются (по умолчанию),
       - \b false -  опрашиваются все пакеты
*/
//===================================================================
void MBMaster::setAutoSkipMode( bool autoSkipMode )
{
  d->auto_skip_mode = autoSkipMode;
}

//===================================================================
//! Текущее значение режима автоматического пропуска неиспользуемых пакетов
/**
  \return
     - \b true  - неиспользуемые пакеты не опрашиваются (по умолчанию),
     - \b false - опрашиваются все пакеты
*/
//===================================================================
bool MBMaster::autoSkipMode()
{
  return d->auto_skip_mode;
}


//===================================================================
//! Расшифровка ModuleDefinition
//===================================================================
MikkonModuleDefinition
            MBMaster::decodeModuleDefinition( const QByteArray& ba )
{
  return MBMasterPrivate::decodeModuleDefinition( ba );
}

