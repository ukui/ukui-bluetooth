#ifndef FEATURESWIDGET_H
#define FEATURESWIDGET_H

#include "../fileSend/bluetoothfiletransferwidget.h"
#include "../agent/bluetoothagent.h"
#include "daemon/bluetoothdbus.h"
#include "../agent/bluetoothobexagent.h"

#include <string>
#include <glib.h>
#include <glib/gprintf.h>

#include <QDBusObjectPath>

#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/manager.h>
#include <KF5/BluezQt/bluezqt/initmanagerjob.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/agent.h>
#include <KF5/BluezQt/bluezqt/battery.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>
#include <KF5/BluezQt/bluezqt/obexmanager.h>
#include <KF5/BluezQt/bluezqt/initobexmanagerjob.h>
#include <KF5/BluezQt/bluezqt/obexobjectpush.h>
#include <KF5/BluezQt/bluezqt/obexsession.h>
#include <KF5/BluezQt/bluezqt/obextransfer.h>

#include <iostream>
#include <QApplication>
#include <QTimer>
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
#include <QPalette>
#include <QTextCodec>
#include <QRect>
#include <QUrl>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QDBusMessage>

#define LIST_PATH "/etc/pairDevice.list"

using namespace std;
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
    void Remove_device_by_address(QString);
    void Connect_device_by_address(QString);
    void Send_files_by_address(QString);
    void Turn_on_or_off_bluetooth(bool);
    void Connect_device(BluezQt::DevicePtr);
    void Open_bluetooth_settings();
    void SendNotifyMessage(QString);
    void NotifyOnOff();

    void Dbus_file_transfer(QUrl);
    void Monitor_sleep_signal();

    void Connect_the_last_connected_device();
    void adapterChangeFUN();
    void createPairDeviceFile();
    void writeDeviceInfoToFile(const QString& devAddress,const QString& devName, const BluezQt::Device::Type type);
    QStringList getDeviceConnectTimeList();
    bool exit_flag = false;
signals:
    void ProgramExit();
public slots:
    void TraySignalProcessing(QAction *action);
    void file_transfer_session_add(BluezQt::ObexSessionPtr);
    void file_transfer_creator(QString);
    void close_session();
    void propertyChanged(QString name, QVariantMap map, QStringList list);
    void GSettings_value_chanage(const QString &key);
    void Dbus_bluetooth_switch(bool);
    void Monitor_sleep_Slot(bool);
    void adapterPoweredChanged(bool value);
    void adapterDeviceRemove(BluezQt::DevicePtr ptr);
private:
    QSystemTrayIcon *bluetooth_tray_icon;
    QMenu *tray_Menu;
    QGSettings *settings;

    QStringList paired_device_address;
    QStringList paired_device;
    QString finally_connect_the_device;
    QString Default_Adapter;
    QString File_save_path;
    QString cur_adapter_address;
    QStringList adapter_list;
    QUrl selected_file;
    quint64 transfer_file_size = 0;
    bool flag = true;

    BluetoothDbus *session_dbus;
    BluetoothFileTransferWidget *transfer_widget;

    BluezQt::Manager *m_manager = nullptr;
    BluezQt::Adapter *m_adapter = nullptr;
    BluezQt::ObexManager *obex_manager = nullptr;
    BluetoothAgent *bluetoothAgent = nullptr;
    BluetoothObexAgent *bluetoothObexAgent = nullptr;
    BluezQt::ObexObjectPush *opp = nullptr;
    BluezQt::ObexTransferPtr filePtr = nullptr;
    QDBusObjectPath pre_session;
    bool dev_remove_flag = false;
    bool sleep_flag = false;
    bool dev_connected_when_sleep = false;
    bool dev_disconnected_flag = true;
    QString pair_device_file;
    int dev_callbak_flag = 0;
} ;
#endif // FEATURESWIDGET_H
