#ifndef __MBMASTER__H_
#define __MBMASTER__H_

/*! \file  mbmaster.h
    \brief Мастер Modbus RTU

    Опрос модулей MIKKON по протоколу Modbus RTU.
*/

#include <QtCore>

#include "mbcommon.h"
#include "serialport.h"

#include "mbtypes.h"

class QDomDocument;

//===================================================================
//! Опрос Модулей MIKKON
/**
    Система опроса модулей MIKKON через последовательный порт по
    протоколу Modbus RTU.

    См. \ref xml_opros_desc
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


  int request_counter; //!< Общее кол-во запросов
  int answer_counter;  //!< Общее кол-во ответов
  int error_counter;   //!< Общее кол-во ошибок
  int full_time;       //!< Время (в мс) полного цикла опроса

//! Расшифровка ModuleDefinition
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
  QVector<MMModule> mmmodules; //!< модули
  QVector<MMSlot>   mmslots;   //!< слоты
  QVector<int>      mmslots_status; //!< состояние слотов
  QVector<MMSlotTransaction> transactions_read;    //!< транзакции на чтение
  QVector<MMSlotTransaction> transactions_write;   //!< транзакции на запись
  QVector<MMSlotTransaction> transactions_write2;  //!< транзакции на запись "0" для битов
  mutable QMutex mutex;   //!< синхронизация обращения к данным и настройкам
  SerialPort *transport;  //!< ссылка на транспорт
  volatile bool thread_exit_flag;  //!< флаг выхода из цикла опроса
  bool disable_write_transactions; //!< флаг запрета транзакций на запись
  bool disable_read_transactions;  //!< флаг запрета транзакций на чтение
  int max_packet_length;           //!< максимальная длина пакета

  //! служебная структура для быстрого поиска транзакци на запись
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
  //! ассациативный контейнер для быстрого поиска транзакци на запись
  QMap<ModuleSlotIndex,int> transactions_write_map;
};

/*! \page xml_opros_desc Формат XML документа конфигурации опроса модулей

  Документ заключается в тег \b MBConfig и состоит из тегов \b Module, в которых описываются модули.

  \b Module содержит следующие аттрибуты:
  \li \b N    Уникальный для данной конфигурации номер модуля.
  \li \b Node Адрес в формате [адрес].[подадрес].
  \li \b Name Имя модуля.
  \li \b Desc Описание модуля.

  Внутри описания модуля расположены теги \b Slot с описания слотов опроса.

  \b Slot содержит следующие аттрибуты:
  \li \b N            Уникальный для данного модуля номер слота
  \li \b Addr         Начальный адрес данных (шестнадцатеричное число)
  \li \b Length       Количество данных
  \li \b Type         Тип данных
  \li \b Desc         Описание

  Аттрибут \b Type может содержать следующие значения:
  \li \b bits
  \li \b bytes
  \li \b words
  \li \b dwords
  \li \b floats

  Пример:

\verbatim
  <MBConfig>
    <Module Node="1" Desc="" N="1" Name="DI300" >
      <Slot Desc="Дискретные входы" Length="64" Type="bits" N="1" Addr="2" />
      <Slot Desc="Режим" Length="32" Type="bits" N="2" Addr="A" />
      <Slot Desc="Счетчики" Length="64" Type="dwords" N="3" Addr="E" />
      <Slot Desc="Сброс счетчиков" Length="64" Type="bits" N="4" Addr="10E" />
      <Slot Desc="Режимы фильтрации" Length="64" Type="bytes" N="5" Addr="116" />
      <Slot Desc="Частота" Length="64" Type="dwords" N="6" Addr="156" />
      <Slot Desc="Период экс. сглаживания" Length="64" Type="words" N="7" Addr="256" />
    </Module>
  </MBConfig>
\endverbatim

*/

#endif
