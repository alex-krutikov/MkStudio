#ifndef __SERVER_H__
#define __SERVER_H__

#include <QObject>

class Server : public QObject
{
  Q_OBJECT
public:
  Server();
  virtual ~Server();
};

#endif
