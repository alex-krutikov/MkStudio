#include <QtCore>
#include <QtXml>

#include "mbmasterxml.h"
#include "mbmaster_p.h"
#include "console.h"

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
    Console::Print( QString("  ������: %1       ��������: %2 �����: %3 ��������: %4\n")
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
      ss.addr = element.attribute("Addr").toInt(&ok,16);
      ss.len  = element.attribute("Length").toInt();
      str = element.attribute("Type");
      ss.datatype.fromTextId( str );
      str = element.attribute("Operation");
      ss.data.resize( ss.len );
      ss.desc       = element.attribute("Desc");
      ss.attributes = element.attribute("Attributes");

      ss.module = mm;
/*
      Console::Print( QString("    ����:   %1 �����: 0x%2 ���-��: %3 ���: %4 ��������: \"%5\" ��������: %6\n")
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

 Console::Print( "������������ MBMasterPrivateXML ���������.\n" );
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
  QString str;

  QDomElement docElem = doc.documentElement();
  QDomNodeList module_list = doc.elementsByTagName("Slot");
  for(i=0; i<module_list.count(); i++ )
  { element = module_list.item( i ).toElement();
    mm = element.attribute("Module").toInt();
    ss = element.attribute("Slot").toInt();
    str = element.attribute("Values");
    QStringList sl = str.split( QChar(';') );
    for(j=0; j<sl.size(); j++ )
    { bool ok;
      int a = sl.at(j).toInt(&ok);
      if(ok)
      { setSlotValue(mm,ss,j,a);
      } else
      { double x=sl.at(j).toDouble(&ok);
        if(ok) setSlotValue(mm,ss,j,x);
      }
    }
  }
}


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
