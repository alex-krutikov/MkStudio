#ifndef __MBMASTER__H_
#define __MBMASTER__H_

/*! \file  mbmaster.h
    \brief ������ Modbus RTU

    ����� ������� MIKKON �� ��������� Modbus RTU.
*/

#include <QtCore>

#include "mbcommon.h"
#include "serialport.h"

#include "mbtypes.h"

class QDomDocument;

//===================================================================
//! ����� ������� MIKKON
/**
    ������� ������ ������� MIKKON ����� ���������������� ���� ��
    ��������� Modbus RTU.

    ��. \ref xml_opros_desc
*/
//===================================================================
class MBMaster : public QThread
{
  friend class MBMasterWidget;
  friend class MBMasterWidgetTableModel;

  Q_OBJECT
public:
  MBMaster( QObject *parent = 0);
  virtual ~MBMaster();

  void setTransport( SerialPort *transport );
  void clear_configuration();
  void load_configuration( QDomDocument &doc );
  void load_configuration( const QByteArray &xml );

  void saveValues( QDomDocument &doc );
  void loadValues( const QDomDocument &doc );
  void set_module_node(int module_index, int node, int subnode=0 );
  void add_module( int module_index, int node, int subnode=0,
                                     const QString &name = QString(),
                                     const QString &description = QString() );
  void add_slot( int module_index, int slot_index, int addr, int len,
                    MBDataType datatype, const QString &description = QString() );

  void polling_start();
  void polling_stop();

  MMSlot getSlot(int module, int slot ) const;
  MMValue getSlotValue(int module, int slot,int index ) const;
  void setSlotValue(int module, int slot,int index,
                                const MMValue &value );

  void setSlotAttributes(int module, int slot, const QString &attributes );


  void setReadTransactionsDisabled( bool disabled );
  void setWriteTransactionsDisabled( bool disabled );


  bool readTransactionsDisabled() const;
  bool writeTransactionsDisabled() const;


  void setMaximumPacketLength( int packet_length );
  int maximumPacketLength() const;


  int request_counter; //!< ����� ���-�� ��������
  int answer_counter;  //!< ����� ���-�� �������
  int error_counter;   //!< ����� ���-�� ������
  int full_time;       //!< ����� (� ��) ������� ����� ������

//! ����������� ModuleDefinition
  static MikkonModuleDefinition decodeModuleDefinition( const QByteArray& );

private:
  void run();
  bool process_transaction( MMSlotTransaction &tr );
  void optimize_write();
  void optimize_write_transaction(int i1, int i2);
signals:
  void stateChanged();
  void slotUpdated(int module, int slot);
private:
  int answer_timeout;
  QVector<MMModule> mmmodules; //!< ������
  QVector<MMSlot>   mmslots;   //!< �����
  QVector<int>      mmslots_status; //!< ��������� ������
  QVector<MMSlotTransaction> transactions_read;    //!< ���������� �� ������
  QVector<MMSlotTransaction> transactions_write;   //!< ���������� �� ������
  QVector<MMSlotTransaction> transactions_write2;  //!< ���������� �� ������ "0" ��� �����
  mutable QMutex mutex;   //!< ������������� ��������� � ������ � ����������
  SerialPort *transport;  //!< ������ �� ���������
  volatile bool thread_exit_flag;  //!< ���� ������ �� ����� ������
  bool disable_write_transactions; //!< ���� ������� ���������� �� ������
  bool disable_read_transactions;  //!< ���� ������� ���������� �� ������
  int max_packet_length;           //!< ������������ ����� ������

  //! ��������� ��������� ��� �������� ������ ��������� �� ������
  struct ModuleSlotIndex
  { int module,slot,index;
    inline ModuleSlotIndex(int module, int slot, int index)
    { this->module = module;
      this->slot   = slot;
      this->index  = index;
    }
    inline bool operator<(const ModuleSlotIndex &s) const
    { if( module > s.module ) return false;
      if( module == s.module )
      { if( slot >  s.slot ) return false;
        if( slot == s.slot )
        { if( index >=  s.index ) return false;
        }
      }
      return true;
    }
  };
  //! ������������� ��������� ��� �������� ������ ��������� �� ������
  QMap<ModuleSlotIndex,int> transactions_write_map;
};

/*! \page xml_opros_desc ������ XML ��������� ������������ ������ �������

  �������� ����������� � ��� \b MBConfig � ������� �� ����� \b Module, � ������� ����������� ������.

  \b Module �������� ��������� ���������:
  \li \b N    ���������� ��� ������ ������������ ����� ������.
  \li \b Node ����� � ������� [�����].[��������].
  \li \b Name ��� ������.
  \li \b Desc �������� ������.

  ������ �������� ������ ����������� ���� \b Slot � �������� ������ ������.

  \b Slot �������� ��������� ���������:
  \li \b N            ���������� ��� ������� ������ ����� �����
  \li \b Addr         ��������� ����� ������ (����������������� �����)
  \li \b Length       ���������� ������
  \li \b Type         ��� ������
  \li \b Desc         ��������

  �������� \b Type ����� ��������� ��������� ��������:
  \li \b bits
  \li \b bytes
  \li \b words
  \li \b dwords
  \li \b floats

  ������:

\verbatim
  <MBConfig>
    <Module Node="1" Desc="" N="1" Name="DI300" >
      <Slot Desc="���������� �����" Length="64" Type="bits" N="1" Addr="2" />
      <Slot Desc="�����" Length="32" Type="bits" N="2" Addr="A" />
      <Slot Desc="��������" Length="64" Type="dwords" N="3" Addr="E" />
      <Slot Desc="����� ���������" Length="64" Type="bits" N="4" Addr="10E" />
      <Slot Desc="������ ����������" Length="64" Type="bytes" N="5" Addr="116" />
      <Slot Desc="�������" Length="64" Type="dwords" N="6" Addr="156" />
      <Slot Desc="������ ���. �����������" Length="64" Type="words" N="7" Addr="256" />
    </Module>
  </MBConfig>
\endverbatim

*/

#endif
