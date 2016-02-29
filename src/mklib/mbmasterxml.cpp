#include "mbmasterxml.h"

#include "console.h"
#include "slotwidget.h"

#include <mbmaster_p.h>

#include <QDomElement>

//###################################################################
//
//###################################################################
MBMasterXML::MBMasterXML( QObject *parent )
  : MBMaster( parent )
{
}

//===================================================================
//
//===================================================================
void MBMasterXML::load_configuration( QDomDocument &doc )
{
  bool ok;
  int i,j;

  MMSlot    ss;
  MMModule mm;
  MMSlotTransaction mmsp;

  clear_configuration();
  d->full_time=0;

  QMutexLocker locker(&d->mutex);

  d->mmmodules.clear();
  d->mmslots.clear();

  QDomElement element;
  QDomNodeList slot_list;
  QString str;
  QRegExp rx("(\\d+)(?:\\D*(\\d+))?");

  QDomElement docElem = doc.documentElement();
  QDomNodeList module_list = doc.elementsByTagName("Module");
  for(i=0; i<module_list.count(); i++ )
  { element = module_list.item( i ).toElement();
    mm.n = element.attribute("N").toInt();
    rx.indexIn( element.attribute("Node") );
    mm.node    = rx.cap(1).toInt();
    mm.subnode = rx.cap(2).toInt();
/*
    Console::Print( QString("NODE = %1, SUBNODE = %2  <%3> <%4> <%5> <%6>")
            .arg( mm.node ).arg( mm.subnode ).arg( element.attribute("Node") )
            .arg(rx.numCaptures() ).arg(rx.cap(1) ).arg(rx.cap(2) ) );
*/
    mm.name = element.attribute("Name");
    mm.desc = element.attribute("Desc");
/*
    Console::Print( QString("  Модуль: %1       Название: %2 Адрес: %3 Описание: %4\n")
                   .arg(mm.n,2)
                   .arg(mm.name)
                   .arg(mm.node,2)
                   .arg(mm.desc) );
*/
    slot_list = element.elementsByTagName("Slot");
    for(j=0; j<slot_list.count(); j++ )
    { element = slot_list.item( j ).toElement();
      ss.status = MMSlot::NotInit;
      ss.n    = element.attribute("N").toInt();
      str = element.attribute("Type");
      ss.datatype.fromTextId( str );
      if( ss.datatype.isRegister() ) ss.addr = element.attribute("Addr").toInt(&ok,10);
      else ss.addr = element.attribute("Addr").toInt(&ok,16);
      ss.len  = element.attribute("Length").toInt();      
      str = element.attribute("Operation");
      ss.data.resize( ss.len );
      ss.desc       = element.attribute("Desc");
      ss.attributes = element.attribute("Attributes");

      ss.module = mm;
/*
      Console::Print( QString("    слот:   %1 Адрес: 0x%2 Кол-во: %3 Тип: %4 Операция: \"%5\" Описание: %6\n")
                     .arg(ss.n,2)
                     .arg(ss.addr,3,16)
                     .arg(ss.len,2)
                     .arg( ss.datatype.toName(), 9)
                     .arg( ss.optype.toName(), 19)
                     .arg( ss.desc )  );
*/
      d->mmslots << ss;
    }
 }

 Console::Print( Console::Information, "Конфигурации MBMasterPrivateXML загружена.\n" );
}

//===================================================================
//
//===================================================================
void MBMasterXML::load_configuration( const QByteArray &xml )
{
  QDomDocument doc;
  doc.setContent( xml, false );
  load_configuration( doc );
}

//===================================================================
//
//===================================================================
void MBMasterXML::saveValues( QDomDocument &doc )
{
  int i,j;
  QDomElement root = doc.createElement("MBMasterPrivateValues");
  doc.appendChild(root);
  QString str;

  for(i=0; i<d->mmslots.size(); i++ )
  { QDomElement slot = doc.createElement("Slot");
    slot.setAttribute("Module",d->mmslots[i].module.n);
    slot.setAttribute("Slot",d->mmslots[i].n);
    slot.setAttribute("Attribute", d->mmslots[i].attributes);
    str.clear();
    for( j=0; j<d->mmslots[i].data.size(); j++ )
    { if( d->mmslots[i].data[j].status() == MMValue::Ok ) str += d->mmslots[i].data[j].toString();
      str+=";";
    }
    str.chop(1);
    slot.setAttribute("Values",str);
    root.appendChild(slot);
  }
}

//===================================================================
//
//===================================================================
void MBMasterXML::loadValues( const QDomDocument &doc )
{
  int i,j,mm,ss;
  QDomElement element;
  QDomNodeList slot_list;
  QString str,attr;

  QDomElement docElem = doc.documentElement();
  QDomNodeList module_list = doc.elementsByTagName("Slot");
  for(i=0; i<module_list.count(); i++ )
  { element = module_list.item( i ).toElement();
    mm = element.attribute("Module").toInt();
    ss = element.attribute("Slot").toInt();
    str = element.attribute("Values");
    attr = element.attribute("Attribute");
    QStringList sl = str.split( QChar(';') );
    for(j=0; j<sl.size(); j++ )
    { bool ok;
      MMValue value;
      int a = sl.at(j).toInt(&ok);
      if(ok)
      { value = a;
      } else
      { double x = sl.at(j).toDouble(&ok);
        if(ok)
        { value = x;
        }
      }
      setSlotValue(mm,ss,j,value);
    }
    setSlotAttributes(mm,ss,attr);
  }
}
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
