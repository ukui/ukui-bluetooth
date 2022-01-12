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
