#ifndef FEATURESWIDGET_H
#define FEATURESWIDGET_H

#include "bluetoothfiletransferwidget.h"
#include "../daemon/bluetoothagent.h"
#include "../daemon/bluetoothdbus.h"
#include "../daemon/bluetoothobexagent.h"

#include <QDBusObjectPath>

#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/manager.h>
#include <KF5/BluezQt/bluezqt/initmanagerjob.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/agent.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>
#include <KF5/BluezQt/bluezqt/obexmanager.h>
#include <KF5/BluezQt/bluezqt/initobexmanagerjob.h>
#include <KF5/BluezQt/bluezqt/obexobjectpush.h>
#include <KF5/BluezQt/bluezqt/obexsession.h>
#include <KF5/BluezQt/bluezqt/obextransfer.h>

#include <QWidget>
#include <QSystemTrayIcon>
#include <QAction>
#include <QString>
#include <QMenu>
#include <QGSettings>
#include <QDebug>
#include <QStringList>
#include <QFileDialog>
#include <QVariant>
#include <QProcess>

class BluetoothDbus;

class FeaturesWidget : public QWidget
{
    Q_OBJECT

public:
    FeaturesWidget(QWidget *parent = nullptr);
    ~FeaturesWidget();
    void InitTrayMenu();
    void Pair_device_by_address(QString);
    void Disconnect_device_by_address(QString);
    void Connect_device_by_address(QString);
    void Send_files_by_address(QString);
public slots:
    void TraySignalProcessing(QAction *action);
    void file_transfer_session_add(BluezQt::ObexSessionPtr);
    void file_transfer_creator(QString);
    void close_session();
    void propertyChanged(QString name, QVariantMap map, QStringList list);
    void GSettings_value_chanage(const QString &key);
private:
    QSystemTrayIcon *bluetooth_tray_icon;
    QMenu *tray_Menu;
    QGSettings *settings;

    QStringList paired_device_address;
    QStringList paired_device;
    QString finally_connect_the_device;
    QString Default_Adapter;

    QString selected_file;
    quint64 transfer_file_size = 0;
    bool flag = true;

    BluetoothDbus *session_dbus;
    BluetoothFileTransferWidget *transfer_widget;

    BluezQt::Manager *m_manager = nullptr;
    BluezQt::Adapter *m_adapter = nullptr;
    BluezQt::ObexManager *obex_manager = nullptr;
    BluezQt::AdapterPtr m_localDevice = nullptr;
    BluetoothAgent *bluetoothAgent = nullptr;
    BluetoothObexAgent *bluetoothObexAgent = nullptr;
    BluezQt::ObexObjectPush *opp = nullptr;
    QDBusObjectPath pre_session;
} ;
#endif // FEATURESWIDGET_H
