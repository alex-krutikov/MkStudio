#ifndef __MBMASTER_P__H_
#define __MBMASTER_P__H_

#include <QThread>

#include "mbcommon.h"
#include "mbtypes.h"

class AbstractSerialPort;

class MBMasterPrivate : public QThread
{
  friend class MBMasterPrivateXML;
  friend class MBMasterXML;
  friend class MBMaster;
  friend class MBMasterWidget;
  friend class MBMasterWidgetTableModel;

  Q_OBJECT
private:
  MBMasterPrivate( QObject *parent);
  virtual ~MBMasterPrivate();

  void setTransport( AbstractSerialPort *transport );
  void clear_configuration();

  void set_module_node(int module_index, int node, int subnode );
  void add_module( int module_index, int node, int subnode,
                                     const QString &name,
                                     const QString &description );
  void add_slot( int module_index, int slot_index, int addr, int len,
                    MBDataType datatype, const QString &description );

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
signals:
  void stateChanged();

private:
  void run();
  bool process_transaction( MMSlotTransaction &tr );
  void optimize_write();
  void optimize_write_transaction(int i1, int i2);
private:
  int answer_timeout;
  QVector<MMModule> mmmodules; //!< ������
  QVector<MMSlot>   mmslots;   //!< �����
  QVector<int>      mmslots_status; //!< ��������� ������
  QVector<MMSlotTransaction> transactions_read;    //!< ���������� �� ������
  QVector<MMSlotTransaction> transactions_write;   //!< ���������� �� ������
  QVector<MMSlotTransaction> transactions_write2;  //!< ���������� �� ������ "0" ��� �����
  mutable QMutex mutex;   //!< ������������� ��������� � ������ � ����������
  AbstractSerialPort *transport;  //!< ������ �� ���������
  volatile bool thread_exit_flag;  //!< ���� ������ �� ����� ������
  bool disable_write_transactions; //!< ���� ������� ���������� �� ������
  bool disable_read_transactions;  //!< ���� ������� ���������� �� ������
  int max_packet_length;           //!< ������������ ����� ������
  int transaction_delay;
  int cycle_time;

  // ��������� ��������� ��� �������� ������ ��������� �� ������
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
  // ������������� ��������� ��� �������� ������ ��������� �� ������
  QMap<ModuleSlotIndex,int> transactions_write_map;
};

#endif
