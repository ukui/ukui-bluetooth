#ifndef BLUETOOTHOBEXAGENT_H
#define BLUETOOTHOBEXAGENT_H

#include "fileReceive/filereceivingpopupwidget.h"

#include <QObject>

#include <KF5/BluezQt/bluezqt/obexagent.h>
#include <KF5/BluezQt/bluezqt/obexfiletransfer.h>
#include <KF5/BluezQt/bluezqt/obexmanager.h>
#include <KF5/BluezQt/bluezqt/obexfiletransferentry.h>
#include <KF5/BluezQt/bluezqt/obextransfer.h>
#include <KF5/BluezQt/bluezqt/obexsession.h>
#include <KF5/BluezQt/bluezqt/request.h>

#include <QDBusObjectPath>
#include <QMetaObject>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QMessageBox>
#include <QDebug>

class FileReceivingPopupWidget;

class BluetoothObexAgent : public BluezQt::ObexAgent
{
    Q_OBJECT
public:
    explicit BluetoothObexAgent(QObject *parent = nullptr);
    void authorizePush (BluezQt::ObexTransferPtr transfer,BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request);
    QDBusObjectPath objectPath() const override;
    void cancel ();
    void release ();

    int daemonIsNotRunning();
    void receiveDisConnectSignal(QString address);


};

#endif // BLUETOOTHOBEXAGENT_H
