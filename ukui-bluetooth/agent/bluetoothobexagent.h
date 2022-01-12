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

private:
    FileReceivingPopupWidget *receiving_widget ;

};

#endif // BLUETOOTHOBEXAGENT_H
