#ifndef BLUETOOTHDBUS_H
#define BLUETOOTHDBUS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDBusObjectPath>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QWidget>
#include <QDebug>

class BluetoothDbus : public QObject
{
    Q_OBJECT
public:
    explicit BluetoothDbus(QObject *parent = nullptr);
    virtual ~BluetoothDbus();
private:
    int daemonIsNotRunning();

signals:
    void ConnectTheSendingDevice(QString);
    void RemoveTheSendingDevice(QString);
    void DisconnectTheSendingDevice(QString);
    void sendTransferMesg(QStringList);
    void switch_signals(bool);
public slots:
    void connectToDevice(QString);
    void disConnectToDevice(QString);
    void removeDevice(QString);
    QStringList getPairedDevice();
    QString getDevcieByAddress(QString);
    void file_transfer(QStringList);
    void Bluetooth_switch(bool);
};

#endif // BLUETOOTHDBUS_H
