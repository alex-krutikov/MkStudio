#ifndef __MBMASTERXML_P__H_
#define __MBMASTERXML_P__H_

#include <QThread>

#include "mbcommon.h"

class MBMasterPrivateXML : public QThread
{
  friend class MBMasterXML;
  friend class MBMasterWidget;
  friend class MBMasterWidgetTableModel;
private:
  MBMasterPrivateXML( QObject *parent);
  virtual ~MBMasterPrivateXML();

  void setTransport( SerialPort *transport );
  void clear_configuration();
  void load_configuration( QDomDocument &doc );
  void load_configuration( const QByteArray &xml );

  void saveValues( QDomDocument &doc );
  void loadValues( const QDomDocument &doc );
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

#endif
