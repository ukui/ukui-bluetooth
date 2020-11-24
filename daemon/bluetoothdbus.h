#ifndef BLUETOOTHDBUS_H
#define BLUETOOTHDBUS_H

#include <QObject>
#include <QString>
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
    void sendDevAddress(QString);
    void sendTransferMesg(QString,QString);
public slots:
    void connectToDevice(QString);
    QStringList getPairedDevice();
    QString getDevcieByAddress(QString);
    void file_transfer(QString,QString);
};

#endif // BLUETOOTHDBUS_H
