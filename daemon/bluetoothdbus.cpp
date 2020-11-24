#include "bluetoothdbus.h"

BluetoothDbus::BluetoothDbus(QObject *parent)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    if(daemonIsNotRunning()){
        QDBusConnection bus = QDBusConnection::sessionBus();
        // 在session bus上注册名为"com.kylin_user_guide.hotel"的service

        if (!bus.registerService("org.ukui.bluetooth")) {  //注意命名规则-和_
                qDebug() << bus.lastError().message();
                exit(1);
        }
        // "QDBusConnection::ExportAllSlots"表示把类Hotel的所有Slot都导出为这个Object的method
        qDebug() << Q_FUNC_INFO << __LINE__ << bus.registerObject("/org/ukui/bluetooth", "org.ukui.bluetooth", this,QDBusConnection::ExportAllSlots|QDBusConnection::ExportAllSignals);
    }
}

BluetoothDbus::~BluetoothDbus()
{

}

int BluetoothDbus::daemonIsNotRunning()
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected())
        return 0;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", "org.ukui.bluetooth");
    return reply.value() == "";
}

void BluetoothDbus::connectToDevice(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    emit this->sendDevAddress(address);
}

QStringList BluetoothDbus::getPairedDevice()
{

}

QString BluetoothDbus::getDevcieByAddress(QString)
{

}

void BluetoothDbus::file_transfer(QString dev, QString file)
{
    qDebug() << Q_FUNC_INFO << dev << file;
    emit this->sendTransferMesg(dev,file);
}
