#ifndef SYSDBUSREGISTER_H
#define SYSDBUSREGISTER_H

#include <string>
#include <glib.h>
#include <glib/gprintf.h>

#include <QtDBus/QDBusContext>
#include <QObject>
#include <QCoreApplication>
#include <QFile>
#include <QDateTime>

#define LIST_PATH "/etc/pairDevice.list"

class SysDbusRegister : public QObject, protected QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.bluetooth.interface")
public:
    SysDbusRegister();
    ~SysDbusRegister();
private:

public slots:
    Q_SCRIPTABLE int exitService();
    Q_SCRIPTABLE QString writeKeyFile(QString,QString);
    Q_SCRIPTABLE QString getKeyFilePath();
};

#endif // SYSDBUSREGISTER_H
