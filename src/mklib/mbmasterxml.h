#ifndef __MBMASTERXML__H_
#define __MBMASTERXML__H_

#include "mbcommon.h"
#include "mbtypes.h"
#include "mbmaster.h"

#include "mk_global.h"

class QDomDocument;
class MBMasterPrivateXML;
class QByteArray;

//===================================================================
//! Опрос Модулей MIKKON
/**
    Система опроса модулей MIKKON через последовательный порт по
    протоколу Modbus RTU.

    См. \ref xml_opros_desc
*/
//===================================================================
class MK_EXPORT MBMasterXML : public MBMaster
{
  friend class MBMasterPrivateXML;
  friend class MBMasterWidget;
  friend class MBMasterWidgetTableModel;

  Q_OBJECT
public:
  MBMasterXML( QObject *parent = 0);

  void load_configuration( QDomDocument &doc );
  void load_configuration( const QByteArray &xml );

  void saveValues( QDomDocument &doc );
  void loadValues( const QDomDocument &doc );

};

#endif
