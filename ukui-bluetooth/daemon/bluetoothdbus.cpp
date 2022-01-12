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
        qDebug() << Q_FUNC_INFO << __LINE__ << bus.registerObject("/org/ukui/bluetooth", "org.ukui.bluetooth", this,QDBusConnection::ExportAllSlots/*|QDBusConnection::ExportAllSignals*/);
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
    emit this->ConnectTheSendingDevice(address);
}

void BluetoothDbus::disConnectToDevice(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    emit this->DisconnectTheSendingDevice(address);
}

void BluetoothDbus::removeDevice(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    emit this->RemoveTheSendingDevice(address);
}

QStringList BluetoothDbus::getPairedDevice()
{

}

QString BluetoothDbus::getDevcieByAddress(QString)
{

}

void BluetoothDbus::file_transfer(QStringList file)
{
    qDebug() << Q_FUNC_INFO << file;
    emit this->sendTransferMesg(file);
}

void BluetoothDbus::Bluetooth_switch(bool value)
{
    qDebug() << Q_FUNC_INFO ;
    emit this->switch_signals(value);
}
