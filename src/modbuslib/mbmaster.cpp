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
MBMasterPrivate::MBMasterPrivate( QObject *parent )
  : QThread( parent )
{
  transport = 0;
  full_time = 0;
  request_counter = answer_counter = error_counter = 0;
  max_packet_length = 128;
  disable_write_transactions = false;
  disable_read_transactions  = false;
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

 //--- ������������ ���������� �� ������ -------------------------------------------------
 for( i=0; i<mmslots.count(); i++ )
 { MMSlot &ps = mmslots[i];
   j = ps.len;
   switch( ps.datatype.id() )
   { case( MBDataType::Bits   ): j = j/8; break;
     case( MBDataType::Bytes  ): j = j*1; break;
     case( MBDataType::Words  ): j = j*2; break;
     case( MBDataType::Dwords ): j = j*4; break;
     case( MBDataType::Floats ): j = j*4; break;
     default: break;
   }

   k=0;
   while( k < j )
   { //len = qMin( j-k, 128 );
     len = qMin( j-k, (int)((max_packet_length&0x7FFFFFFC)-8) );
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
     //--------------------------------
     mmsp.slot = &ps;
     switch( ps.datatype.id() )
     { case( MBDataType::Bits   ): mmsp.offset = k*8; mmsp.length = len*8; break;
       case( MBDataType::Bytes  ): mmsp.offset = k/1; mmsp.length = len/1; break;
       case( MBDataType::Words  ): mmsp.offset = k/2; mmsp.length = len/2; break;
       case( MBDataType::Dwords ): mmsp.offset = k/4; mmsp.length = len/4; break;
       case( MBDataType::Floats ): mmsp.offset = k/4; mmsp.length = len/4; break;
       default: break;
     }
     mmsp.request = ba;
     mmsp.exp_length = len+6;
     mmsp.answer.resize( mmsp.exp_length );
     mmsp.execute_flag = true;

     transactions_read << mmsp;
     //--------------------------------
     k = k+len;
   }
 }

 //--- ������������ ���������� �� ������ -------------------------------------------------
 m=0;
 for( i=0; i<mmslots.count(); i++ )
 { MMSlot &ps = mmslots[i];
   switch( ps.datatype.id() )
   { case( MBDataType::Bits   ): j = 1; break;
     case( MBDataType::Bytes  ): j = 1; break;
     case( MBDataType::Words  ): j = 2; break;
     case( MBDataType::Dwords ): j = 4; break;
     case( MBDataType::Floats ): j = 4; break;
     default:  break;
   }

   for( k=0; k<ps.len; k++ )
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
     //--------------------------------
     mmsp.slot = &ps;
     mmsp.offset = k;
     mmsp.length = 1;
     mmsp.request = ba;
     mmsp.exp_length = j + 8;
     mmsp.answer.resize( mmsp.exp_length );
     mmsp.execute_flag = false;
     transactions_write << mmsp;
     transactions_write_map.insert( ModuleSlotIndex( ps.module.n, ps.n, k ), m );
     m++;
   }
 }
 //for( i=0; i<transactions_write.count(); i++ )
 //{ Console::Print("������:" + QByteArray2QString( transactions_write[i].request ) + "\n"  );
 //}
 start();
 setPriority( QThread::HighestPriority );
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
void MBMasterPrivate::setTransport( AbstractSerialPort *transport )
{
  this->transport = transport;
}

//===================================================================
//
//===================================================================
void MBMasterPrivate::run()
{
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

  Console::Print( "����� ������� �������.\n" );

  thread_exit_flag = false;
  while( !thread_exit_flag ) // ���� ������
  {
    if( data_request_timer.elapsed() > 3000 )
    { data_request_timer.restart();
      int nk = mmslots.count();
      for( k=0; k<nk; k++ )
      { if( !mmslots[k].data_read_flag ) mmslots[k].status=MMSlot::Skiped;
        mmslots[k].data_read_flag = false;
      }
    }

    // ���������� �� ������
    if( !disable_read_transactions )
    { process_transaction( transactions_read[i] );
    }
    // ���������� ��������� ������
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

    full_time = full_time_timer.restart(); // ������ ����� ������

    msleep(25);

    optimize_write();

    if( !disable_write_transactions )
    { // ���������� �� ������
      for( j=0; j<transactions_write.count(); j++ )
      { if( !transactions_write[j].execute_flag  ) continue;
        if( process_transaction( transactions_write[j] ) )
        { // OK
          transactions_write[j].execute_flag = false;
        } else
        { // ������
          if(    (!transactions_write[j].errortimeout)
              && ( transactions_write[j].errorcode != 0) )
          { // �������� ��� ���������� ������� �����
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
    msleep(25);
  }
  Console::Print( "����� ������� ����������.\n" );
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

  // ���� 1: ������
  k=1;
  for( i=0; i<(ni-1); i++ )
  { if( !transactions_write[i].execute_flag   ) goto skip;
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
      if( a == 0 ) goto skip; // ������ -- ���������� �� ������
      if( a == 1 ) // ������ ������
      { if( ( addr1 + len1 ) != addr2 ) goto skip;
      } else
      { // ������ �� AND,OR,XOR
        if( (addr1 != addr2) && ( addr1 + len1 ) != addr2 ) goto skip; // ��
      }
      flags_array[i]   = k;
      flags_array[i+1] = k;
    }
    continue;
  skip:
    k++;
  }

  // ���� 2: ���������� ����������
  k=0;
  for( i=0; i<ni; i++ )
  { if( flags_array[i] == 0 ) continue;
    { k = flags_array[i];
      j = i;
      while( ( j < ni ) && ( flags_array[j] == k ) ) j++;
      j--;
      //Console::Print( QString("���������������� ������ [%1;%2] (%3)\n").arg(i).arg(j).arg(k) );
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
    ba.chop(2); // ������� CRC16
    for( j=i+1; j<=i2; j++ )
    { QByteArray &ba2 = transactions_write[j].request;
      int s = ba2.size()-8;
      if( s < 0 ) return; // ������
      addr2 = ((unsigned char)ba2[3]<<8)|(unsigned char)ba2[4];
      len2  =  (unsigned char)ba2[5];
      mode  =  (unsigned char)ba2[2] & 0x0F;
      if( mode == 0 ) return; // ������
      if( mode == 1 )
      { // ������ ������
        if( ( addr1 + len1 ) != addr2 ) return; // ������
        ba.append( QByteArray( ba2.constData()+6, s ) );
      } else
      { // ������ �� AND,OR,XOR
        if( ( addr1 + len1 ) == addr2 )
        { // ������� ���������� -- ������ ���������
          ba.append( QByteArray( ba2.constData()+6, s ) );
        } else
        { if( addr1 == addr2 )
          { if( len1 != len2 ) return; // ������
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
      if( ba.size() > ((max_packet_length&0x7FFFFFFC)-8) ) break;
    }

    int len = ba.size()-6;
    ba[5] = len;
    CRC::appendCRC16( ba );
    ba_answer.resize( len + 8 );
    ba_answer.fill(0);
    if( !transport  ) return;
    int errorcode;
    transport->query( ba, ba_answer, &errorcode );
    //Console::Print( "���������������� ������: " + QByteArray2QString( ba ) + "\n" );
    i = j;
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
  { if( !tr.slot->data_read_flag )
    { goto skiped;
    }
  }

  tr.answer.fill(0);

  request_counter++;
  i = transport->query( tr.request, tr.answer, &tr.errorcode );

  {

  QMutexLocker locker(&mutex);

  tr.errortimeout = false;
  if( i == 0 )
  { tr.errortimeout = true;
    goto exit;
  }
  if( i != tr.exp_length )            goto exit;
  if( i != tr.answer.length() )       goto exit;
  if( tr.answer[0] != tr.request[0] ) goto exit;
  if( tr.answer[1] != tr.request[1] ) goto exit;
  if( tr.answer[2] != tr.request[2] ) goto exit;
  if( CRC::CRC16( tr.answer ) )       goto exit;
  if( tr.request[2] & 0x0F )
  { // ������
    data_offset = 6;
    if( tr.answer[3] != tr.request[3] ) goto exit;
    if( tr.answer[4] != tr.request[4] ) goto exit;
  } else
  { // ������
    data_offset = 4;
  }

  if( ( tr.request[2] & 0x0F ) == 0 )
  { // ������
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

  tr.slot->status=MMSlot::Ok;
  answer_counter++;
  return true;
  }
exit:
  error_counter++;
  tr.slot->status=MMSlot::Bad;
skiped:
  for(i=0; i<tr.length; i++ )
  { tr.slot->data[i+tr.offset].setStatus( MMValue::Bad );
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
  //Console::Print( QString("������: %1/%2/%3 %4\n").arg(module).arg(slot).arg(index).arg(value.toString()) );

  QMutexLocker locker(&mutex);

   i=transactions_write_map.value( ModuleSlotIndex(module,slot,index), -1 );
   if( (i>=0) && (i<transactions_write.count()) )
   {
    //Console::Print( QString("������: ���� ������!\n") );
    transactions_write[i].request.resize(6);
    //Console::Print("������ 1:" + QByteArray2QString( transactions_write[i].request ) + "\n"  );
    switch( transactions_write[i].slot->datatype.id() )
    {  case( MBDataType::Bits   ):
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
       case( MBDataType::Bytes  ):  (*(char*)buff)  = (value.toInt() & 0xFF );     buff_len=1; break;
       case( MBDataType::Words  ):  (*(short*)buff) = (value.toInt() & 0xFFFF );   buff_len=2; break;
       case( MBDataType::Dwords ):  (*(int*)buff)   =          value.toInt();      buff_len=4; break;
       case( MBDataType::Floats ):  (*(float*)buff) = (float)( value.toDouble() ); buff_len=4; break;
       default: buff_len=0;
    }

    for(j=0; j<buff_len; j++ ) transactions_write[i].request.append( buff[j] );
    CRC::appendCRC16( transactions_write[i].request );
    transactions_write[i].execute_flag=true;
    //Console::Print("������ 2:" + QByteArray2QString( transactions_write[i].request ) + "\n"  );
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
// ����������� ModuleDefinition
//===================================================================
MikkonModuleDefinition MBMasterPrivate::decodeModuleDefinition( const QByteArray &ba )
{
  //struct PACKED ModuleDefinition
  struct __attribute__(( packed )) ModuleDefinition
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
  : QObject( parent ), d( new MBMasterPrivate(this) )
{
  connect( d, SIGNAL( stateChanged() ), this, SIGNAL( stateChanged() ) );
}

//===================================================================
//! �������� � ����������
//===================================================================
void MBMaster::setTransport( AbstractSerialPort *transport )
{
  d->setTransport( transport );
}

//===================================================================
//! ������� ������������
//===================================================================
void MBMaster::clear_configuration()
{
  d->clear_configuration();
}

//===================================================================
//! �������� ����� ������
//===================================================================
void MBMaster::set_module_node(int module_index, int node, int subnode )
{
  d->set_module_node( module_index, node, subnode );
}

//===================================================================
//! �������� ������
/**
  \param module_index   ����� ������
  \param node           �����
  \param subnode        �������������� �����
  \param name           ��� ������
  \param description    �������� ������
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
//! �������� ����
/**
  \param module_index   ����� ������
  \param slot_index     ����� �����
  \param addr           ��������� �����
  \param len            ����� (���������� ������������� ������)
  \param datatype       ��� ������
  \param description    ��������
*/
//===================================================================
void MBMaster::add_slot( int module_index, int slot_index, int addr, int len,
                    MBDataType datatype, const QString &description  )
{
  d->add_slot( module_index, slot_index, addr, len,
                     datatype, description  );
}

//===================================================================
//! ��������� �����
//===================================================================
void MBMaster::polling_start()
{
  d->polling_start();
}

//===================================================================
//! ���������� �����
//===================================================================
void MBMaster::polling_stop()
{
  d->polling_stop();
}

//===================================================================
//! ��������� ��� ������ �� �����
/**
  \param module ����� ������
  \param slot   ����� �����
*/
//===================================================================
MMSlot MBMaster::getSlot(int module, int slot ) const
{
  return d->getSlot( module,  slot );
}

//===================================================================
//! ��������� ��������
/**
  \param module ����� ������
  \param slot   ����� �����
  \param index  ����� �������
*/
//===================================================================
MMValue MBMaster::getSlotValue(int module, int slot,int index ) const
{
  return d->getSlotValue( module, slot, index );
}

//===================================================================
//! �������� ��������
/**
  \param module ����� ������
  \param slot   ����� �����
  \param index  ����� �������
  \param value  ��������
*/
//===================================================================
void MBMaster::setSlotValue(int module, int slot,int index,
                                const MMValue &value )
{
  d->setSlotValue( module,  slot, index, value );
}

//===================================================================
//! ���������� ��������� �����
/**
  \param module      ����� ������
  \param slot        ����� �����
  \param attributes  ���������
*/
//===================================================================
void MBMaster::setSlotAttributes(int module, int slot, const QString &attributes )
{
  d->setSlotAttributes( module,  slot,  attributes );
}

//===================================================================
//! ��������� ���������� �� ������
/**
  \param disabled \b true - ���������� �� ������ ���������, \b false - ���������
*/
//===================================================================
void MBMaster::setReadTransactionsDisabled( bool disabled )
{
  d->setReadTransactionsDisabled( disabled );
}

//===================================================================
//! ��������� ���������� �� ������
/**
  \param disabled \b true - ���������� �� ������ ���������, \b false - ���������
*/
//===================================================================
void MBMaster::setWriteTransactionsDisabled( bool disabled )
{
  d->setWriteTransactionsDisabled( disabled );
}

//===================================================================
//! ������ ������� ���������� �� ������
/**
  \return \b true - ���������� �� ������ ���������, \b false - ���������
*/
//===================================================================
bool MBMaster::readTransactionsDisabled() const
{
  return d->readTransactionsDisabled();
}

//===================================================================
//! ������ ������� �� ������
/**
  \return \b true - ���������� �� ������ ���������, \b false - ���������
*/
//===================================================================
bool MBMaster::writeTransactionsDisabled() const
{
  return d->writeTransactionsDisabled();
}

//===================================================================
//! ���������� ������������ ����� ������
/**
  \param packet_length - ������������ ����� ������ (����)
*/
//===================================================================
void MBMaster::setMaximumPacketLength( int packet_length )
{
  return d->setMaximumPacketLength( packet_length );
}

//===================================================================
//! ������������ ����� ������
/**
  \return ������������ ����� ������ (����)
*/
//===================================================================
int MBMaster::maximumPacketLength() const
{
  return d->maximumPacketLength();
}

//===================================================================
//! ����� ���-�� ��������
//===================================================================
int MBMaster::request_counter() const
{ return d->request_counter;
}

//===================================================================
//! ����� ���-�� �������
//===================================================================
int MBMaster::answer_counter() const
{ return d->answer_counter;
}

//===================================================================
//! ����� ���-�� ������
//===================================================================
int MBMaster::error_counter() const
{ return d->error_counter;
}

//===================================================================
//! ����� (� ��) ������� ����� ������
//===================================================================
int MBMaster::full_time() const
{ return d->full_time;
}
//===================================================================
//! ����������� ModuleDefinition
//===================================================================
MikkonModuleDefinition
            MBMaster::decodeModuleDefinition( const QByteArray& ba )
{
  return MBMasterPrivate::decodeModuleDefinition( ba );
}

