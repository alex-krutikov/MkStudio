#ifndef __interface__h_
#define __interface__h_

class QString;

class MKViewPluginInterface
{
public:
    virtual ~MKViewPluginInterface() {}
    virtual QString echo(const QString &message) = 0;
    virtual QString moduleName() = 0;
    virtual void helpWindow(QWidget *parent = 0) = 0;
    virtual void mainWindow(QWidget *parent, const QString &portname,
                            int portspeed, int node, int subnode)
        = 0;
};

Q_DECLARE_INTERFACE(MKViewPluginInterface,
                    "inkommet.mkview.plugininterface/2.0");

#endif
