/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2021 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef FEATURESWIDGET_H
#define FEATURESWIDGET_H

#include "../fileSend/bluetoothfiletransferwidget.h"
#include "../agent/bluetoothagent.h"
#include "daemon/bluetoothdbus.h"
#include "../agent/bluetoothobexagent.h"
#include "../component/switchaction.h"

#include <string>
#include <glib.h>
#include <glib/gprintf.h>

#include <QDBusObjectPath>

#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/manager.h>
#include <KF5/BluezQt/bluezqt/initmanagerjob.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/agent.h>

#ifdef BATTERY
#include <KF5/BluezQt/bluezqt/battery.h>
#endif

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
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QRect>
#include <QUrl>
#include <QProcess>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QFileInfo>
#include <QWidgetAction>
#include <QDBusMessage>
#include <QMessageBox>

#define LIST_PATH "/etc/pairDevice.list"
#define RFKILL_DEV "/dev/rfkill"

using namespace std;
class BluetoothDbus;

class FeaturesWidget : public QWidget
{
    Q_OBJECT

public:
    FeaturesWidget(QWidget *parent = nullptr);
    ~FeaturesWidget();
    void showBluetoothTrayIcon();
    void InitTrayMenu();
    void setWidgetPosition();
    void InitAgentAndDbus();
    void M_registerAgent();

    void Pair_device_by_address(QString);
    void Disconnect_device_by_address(QString);
    void Remove_device_by_address(QString);
    void Connect_device_by_address(QString);
    void Connect_device_audio(QString);
    void Send_files_by_address(QString);
    void Turn_on_or_off_bluetooth(bool);
    void Connect_device(BluezQt::DevicePtr);
    void Open_bluetooth_settings();
    void SendNotifyMessage(QString);
    void NotifyOnOff();

    void Dbus_file_transfer(QStringList);
    void Monitor_sleep_signal();
    bool Connect_device_name_white_list(QString dev_name);
    void Connect_the_last_connected_device();
    void adapterChangeFUN();
    void writeDeviceInfoToFile(const QString& devAddress,const QString& devName, const BluezQt::Device::Type type);
    void removeDeviceInfoToFile(const QString& devAddress);
    void setTrayIcon(bool);
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
    void Remove_device_by_devicePtr(BluezQt::DevicePtr ptr);
    void Start_notify_timer(BluezQt::DevicePtr);
    void Stop_notify_timer(BluezQt::DevicePtr);
    void showWarnningMesgBox();

private:
    QList<QString>  is_connected;
    QSystemTrayIcon *bluetooth_tray_icon = nullptr;
    QMenu           *tray_Menu           = nullptr;
    SwitchAction    *m_action            = nullptr;
    QGSettings      *settings            = nullptr;

    QStringList paired_device_address;
    QStringList paired_device;
    QString     finally_connect_the_device;
    QString     Default_Adapter;
    QString     File_save_path;
    QString     cur_adapter_address;
    QStringList adapter_list;
    QString     selected_file;
    quint64     transfer_file_size = 0;
    bool        flag               = true;

    BluetoothDbus               *session_dbus       = nullptr;
    BluetoothFileTransferWidget *transfer_widget    = nullptr;

    BluezQt::DevicePtr          m_device;
    BluezQt::Manager            *m_manager          = nullptr;
    BluezQt::Adapter            *m_adapter          = nullptr;
    BluezQt::ObexManager        *obex_manager       = nullptr;
    BluetoothAgent              *bluetoothAgent     = nullptr;
    BluetoothObexAgent          *bluetoothObexAgent = nullptr;
    BluezQt::ObexObjectPush     *opp                = nullptr;
    BluezQt::ObexTransferPtr    filePtr             = nullptr;

    QTimer *notify_timer;
    QDBusObjectPath pre_session;
    bool            dev_remove_flag = false;
    bool            sleep_flag      = false;
    bool            dev_connected_when_sleep = false;
    QString         pair_device_file;
    int             dev_callbak_flag = 0;
    bool            pair_flag = true;

    QTimer * callBackConnectTimer;
    void initAllTimer();
} ;
#endif // FEATURESWIDGET_H
