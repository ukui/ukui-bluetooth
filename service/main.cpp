#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>

#include "sysdbusregister.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    a.setOrganizationName("Kylin Team");
    a.setApplicationName("system-bus-bluetooth-service");

    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerService("com.bluetooth.systemdbus")){
        qCritical() << "QDbus register service failed reason:" << systemBus.lastError();
        //exit(1);
    }

    if (!systemBus.registerObject("/", "com.bluetooth.interface", new SysDbusRegister(), QDBusConnection::ExportAllSlots)){
        qCritical() << "QDbus register object failed reason:" << systemBus.lastError();
        //exit(2);
    }
//    qDebug() << "ok";
    return a.exec();
}
