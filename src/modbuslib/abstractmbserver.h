#ifndef __ABSTRACTMBSERVER_H__
#define __ABSTRACTMBSERVER_H__


class AbstractMBServer
{
public:
    virtual void setReplyDelay(int delay) = 0;

public:
    virtual ~AbstractMBServer() {};
};

#endif
