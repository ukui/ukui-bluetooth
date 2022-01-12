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
#ifndef BLUETOOTHAGENT_H
#define BLUETOOTHAGENT_H

#include "pin/pincodewidget.h"

#include <unistd.h>
#include <QObject>
#include <KF5/BluezQt/bluezqt/agent.h>
#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/request.h>

#include <QDBusObjectPath>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusAbstractAdaptor>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnectionInterface>
#include <QMessageBox>
#include <QTimer>

class PinCodeWidget;

class BluetoothAgent : public BluezQt::Agent
{
    Q_OBJECT

public:
    explicit BluetoothAgent(QObject *parent = nullptr);

    QDBusObjectPath objectPath() const override;

    void requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request) override;
    void displayPinCode(BluezQt::DevicePtr device, const QString &pinCode) override;
    void requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request) override;
    void displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered) override;
    void requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request) override;
    void requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request) override;
    void authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request) override;

    void cancel() override;
    void release() override;
    int daemonIsNotRunning();
    void emitRemoveSignal(BluezQt::DevicePtr device);

signals:
    void requestAccept();
    void requestReject();
    void stopNotifyTimer(BluezQt::DevicePtr);
    void startNotifyTimer(BluezQt::DevicePtr);
    void agentRemoveDevice(BluezQt::DevicePtr);
private:

    int64_t _pinCodeDisplay;

    BluezQt::DevicePtr m_device;
    // requestPinCode
    bool m_pinRequested;
    // displayPinCode
    QString m_displayedPinCode;
    // requestPasskey
    bool m_passkeyRequested;
    // displayPasskey
    QString m_displayedPasskey;
    QString m_enteredPasskey;
    // requestConfirmation
    QString m_requestedPasskey;
    // requestAuthorization
    bool m_authorizationRequested;
    // authorizeService
    QString m_authorizedUuid;
    // cancel
    bool m_cancelCalled;
    // release
    bool m_releaseCalled;

    bool m_hasClosePinCode;

    PinCodeWidget *pincodewidget = nullptr;
    PinCodeWidget *Keypincodewidget = nullptr;
};

#endif // BLUETOOTHAGENT_H
