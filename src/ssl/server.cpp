#include <QtCore>
#include <QtNetwork>

#include "server.h"
#include "console.h"

QSslCertificate cert;

//=============================================================================
//
//=============================================================================
Server::Server()
{
/*
 QFile file("d:/certreq.pem");
 bool ok = file.open( QIODevice::ReadOnly );
 cert = QSslCertificate( &file );

 Console::Print( Console::Debug, ok?"1":"0");
 Console::Print( Console::Debug, cert.isNull()?"0":"1");
*/

  foreach (QSslCertificate cert, QSslCertificate::fromPath("d:/1/*.pem", QSsl::Pem,
                                                          QRegExp::Wildcard))
  {

  Console::Print( Console::Debug, "ok\n" );

  }

}


//=============================================================================
//
//=============================================================================
Server::~Server()
{
}
