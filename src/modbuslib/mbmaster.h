#ifndef __MBMASTER__H_
#define __MBMASTER__H_

#include "mbcommon.h"
#include "mbtypes.h"

class MBMasterPrivate;
class SerialPort;

//===================================================================
//! Опрос Модулей MIKKON
/**
    Система опроса модулей MIKKON через последовательный порт по
    протоколу Modbus RTU.

    См. \ref xml_opros_desc
*/
//===================================================================
class MBMaster : public QObject
{
  friend class MBMasterXML;
  friend class MBMasterPrivate;
  friend class MBMasterPrivateXML;
  friend class MBMasterWidget;
  friend class MBMasterWidgetTableModel;

  Q_OBJECT
public:
  MBMaster( QObject *parent = 0);

  void setTransport( SerialPort *transport );
  void clear_configuration();
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


  int request_counter() const;
  int answer_counter() const;
  int error_counter() const;
  int full_time() const;

//! Расшифровка ModuleDefinition
  static MikkonModuleDefinition decodeModuleDefinition( const QByteArray& );

signals:
  void stateChanged();
private:
  MBMasterPrivate *d;
};

#endif
