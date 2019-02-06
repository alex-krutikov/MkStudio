#ifndef __UTILS_H__
#define __UTILS_H__

#include <QString>

#include <memory>

class QByteArray;

class ProtocolAnalyserPrivate;


inline static QString intToHex(int a, int width = 0)
{
    return "0x" + QString("%1").arg(a, width, 16, QLatin1Char('0')).toUpper();
}

class ProtocolAnalyser
{
public:
    ProtocolAnalyser();
    ~ProtocolAnalyser();

    void reset();
    void setRequest(const QByteArray &request);
    void setResponse(const QByteArray &response);

    int expectedResponseLength() const;

private:
    std::unique_ptr<ProtocolAnalyserPrivate> d;
};

#endif
