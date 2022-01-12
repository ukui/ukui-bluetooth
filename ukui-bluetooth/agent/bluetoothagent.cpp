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
#include "bluetoothagent.h"


BluetoothAgent::BluetoothAgent(QObject *parent)
    : Agent(parent)
    , m_pinRequested(false)
    , m_passkeyRequested(false)
    , m_authorizationRequested(false)
    , m_cancelCalled(false)
    , m_releaseCalled(false)
    , m_hasClosePinCode(false)
{
    qDebug() << Q_FUNC_INFO;
    if(daemonIsNotRunning()){
        QDBusConnection bus = QDBusConnection::systemBus();
        // 在session bus上注册名为"org.bluez.Agent1"的service

        if (!bus.registerService("org.bluez.Agent1")) {  //注意命名规则-和_
                qDebug() << bus.lastError().message();
                exit(1);
        }
        // "QDBusConnection::ExportAllSlots"表示把类Hotel的所有Slot都导出为这个Object的method
        bus.registerObject("/", this ,QDBusConnection::ExportAllContents);
    }
}

void BluetoothAgent::emitRemoveSignal(BluezQt::DevicePtr device) {
    if (device.data()->isPaired()) {
        QTimer::singleShot(100,this,[=]{
            emit agentRemoveDevice(device);
        });

    }
}

QDBusObjectPath BluetoothAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/BluetoothAgent"));
}

void BluetoothAgent::requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request)
{
    qDebug() << Q_FUNC_INFO;
    m_device = device;
    m_pinRequested = true;
    m_hasClosePinCode = true;

    emit stopNotifyTimer(device);

    Keypincodewidget = new PinCodeWidget(device->name(), true);

    connect(Keypincodewidget,&PinCodeWidget::accepted,this,[=]{
        request.accept(Keypincodewidget->getEnteredPINCode());
        emit startNotifyTimer(device);
        m_hasClosePinCode = false;
        return;
    });

    connect(Keypincodewidget,&PinCodeWidget::rejected,this,[=]{
        request.reject();
        m_hasClosePinCode = false;
        return;
    });

    connect(device.data(), &BluezQt::Device::pairedChanged, this, [=](bool st){
        if (st == false) {
            request.reject();
            if (pincodewidget != nullptr && m_hasClosePinCode != false) {
                m_hasClosePinCode = false;
                pincodewidget->close();
            }
        }
        return;
    });

    connect(Keypincodewidget, &PinCodeWidget::destroyed, this, [=] {
        Keypincodewidget = nullptr;
        m_hasClosePinCode = false;
    });
    //保持在最前
    Keypincodewidget->show();
    Keypincodewidget->activateWindow();
}

void BluetoothAgent::displayPinCode(BluezQt::DevicePtr device, const QString &pinCode)
{
    m_device = device;
    m_displayedPinCode = pinCode;
    printf("\n\nCCCCCCCCCCCCCCCCCC - %s\n\n", pinCode.toStdString().data());
    qDebug() << Q_FUNC_INFO;
}

void BluetoothAgent::requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request)
{
    qDebug() << Q_FUNC_INFO;
    m_device = device;
    m_passkeyRequested = true;

    request.accept(0);
}

void BluetoothAgent::displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered)
{
    qDebug() << Q_FUNC_INFO << ( m_device.isNull() ? "NULL" : m_device.data()->address() ) << entered << passkey;
    if(m_displayedPasskey == passkey)
        return;

    emit stopNotifyTimer(device);
    m_device = device;
    m_displayedPasskey = passkey;
    m_enteredPasskey = entered;

    connect(device.data(), &BluezQt::Device::pairedChanged, this, [=](bool st){
        qDebug() << Q_FUNC_INFO << st << __LINE__;
        if (st) {
            if (Keypincodewidget) {
                Keypincodewidget->setHidden(true);
                emit startNotifyTimer(device);
            }
        }else{
            if(Keypincodewidget){}
//                Keypincodewidget->close();
                QTimer::singleShot(1000,this,[=]{
                    qDebug() << Q_FUNC_INFO << device.data()->isConnected() << __LINE__;
                    if (!device.data()->isConnected())
                        Keypincodewidget->pairFailureShow();
                });
            disconnect(device.data(), &BluezQt::Device::pairedChanged, nullptr, nullptr);
        }
    });

    if(Keypincodewidget != nullptr){
        if (Keypincodewidget) {
            Keypincodewidget->updateUIInfo(device.data()->name(),passkey);
            return;
        }
    }

    Keypincodewidget = new PinCodeWidget(device->name(),passkey,false);

    //保持在最前
    Keypincodewidget->show();
    Keypincodewidget->activateWindow();

}

void BluetoothAgent::requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request)
{
    qDebug() << Q_FUNC_INFO << device->name() << passkey;

    if(pincodewidget != nullptr){
        return;
    }
    emit stopNotifyTimer(device);
    m_device = device;
    m_requestedPasskey = passkey;

    pincodewidget = new PinCodeWidget(device->name(),passkey);
    m_hasClosePinCode = true;

    connect(pincodewidget,&PinCodeWidget::accepted,this,[=]{
        request.accept();
        emit startNotifyTimer(device);
        return;
    });

    connect(pincodewidget,&PinCodeWidget::rejected,this,[=]{
        request.reject();
        emitRemoveSignal(device);
        return;
    });

    connect(m_device.data(), &BluezQt::Device::pairedChanged, this, [=](bool st){
        if (st == false) {
            request.reject();
            if (pincodewidget != nullptr && m_hasClosePinCode != false) {
                m_hasClosePinCode = false;
                pincodewidget->close();
                emitRemoveSignal(device);
            }
        }
        return;
    });
    connect(pincodewidget, &PinCodeWidget::destroyed, this, [=] {
        pincodewidget = nullptr;
        m_cancelCalled = false;
    });

    //保持在最前
    pincodewidget->show();
    pincodewidget->activateWindow();

//    request.accept();
}

void BluetoothAgent::requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request)
{
    qDebug() << Q_FUNC_INFO << device->name();
    m_device = device;
    m_authorizationRequested = true;

    request.accept();
}

void BluetoothAgent::authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request)
{
    qDebug() << Q_FUNC_INFO << device->name();
    m_device = device;
    m_authorizedUuid = uuid;

    request.accept();
}

void BluetoothAgent::cancel()
{
    qDebug() << Q_FUNC_INFO;
    if (m_cancelCalled)
        return;
    if (pincodewidget != nullptr && m_hasClosePinCode != false) {
        m_cancelCalled = true;
        m_hasClosePinCode = false;
        pincodewidget->Connection_timed_out();
        emitRemoveSignal(m_device);
    }
}

void BluetoothAgent::release()
{
    qDebug() << Q_FUNC_INFO;
    m_releaseCalled = true;
}

int BluetoothAgent::daemonIsNotRunning()
{
    QDBusConnection conn = QDBusConnection::systemBus();
    if (!conn.isConnected())
        return 0;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", "org.bluez.Agent1");
    return reply.value() == "";
}
