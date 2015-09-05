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


  int request_counter; //!< Общее кол-во запросов
  int answer_counter;  //!< Общее кол-во ответов
  int error_counter;   //!< Общее кол-во ошибок
  int full_time;       //!< Время (в мс) полного цикла опроса

//! Расшифровка ModuleDefinition
  static MikkonModuleDefinition decodeModuleDefinition( const QByteArray& );
signals:
  void stateChanged();

private:
  void run();
  bool process_transaction( MMSlotTransaction &tr );
  void optimize_write();
  void optimize_write_transaction(int i1, int i2);
  void optimize_write_holding();
  void optimize_write_transaction_holding(int i1, int i2);
  void optimize_write_holding_ext();
  void optimize_write_transaction_holding_ext(int i1, int i2);
  void optimize_write_coils();
  void optimize_write_transaction_coils(int i1, int i2);
private:
  int answer_timeout;
  QVector<MMModule> mmmodules; //!< модули
  QVector<MMSlot>   mmslots;   //!< слоты
  QVector<int>      mmslots_status; //!< состояние слотов
  QVector<MMSlotTransaction> transactions_read;    //!< транзакции на чтение
  QVector<MMSlotTransaction> transactions_write;   //!< транзакции на запись
  QVector<MMSlotTransaction> transactions_write2;  //!< транзакции на запись "0" для битов
  mutable QMutex mutex;   //!< синхронизация обращения к данным и настройкам
  AbstractSerialPort *transport;  //!< ссылка на транспорт
  volatile bool thread_exit_flag;  //!< флаг выхода из цикла опроса
  bool disable_write_transactions; //!< флаг запрета транзакций на запись
  bool disable_read_transactions;  //!< флаг запрета транзакций на чтение
  int max_packet_length;           //!< максимальная длина пакета
  int transaction_delay;
  int cycle_time;
  bool auto_skip_mode;

  // служебная структура для быстрого поиска транзакци на запись
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
  // ассациативный контейнер для быстрого поиска транзакци на запись
  QMap<ModuleSlotIndex,int> transactions_write_map;
};

#endif
