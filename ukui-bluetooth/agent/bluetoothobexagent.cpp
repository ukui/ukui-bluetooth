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
#include "bluetoothobexagent.h"

BluetoothObexAgent::BluetoothObexAgent(QObject *parent):ObexAgent(parent)
{
    if(daemonIsNotRunning()){
        qDebug() << Q_FUNC_INFO << __LINE__;
        QDBusConnection bus = QDBusConnection::sessionBus();

        if (!bus.registerService("org.ukui.bluetooth")) {  //注意命名规则-和_
                qDebug() << bus.lastError().message();
                exit(1);
        }
        // "QDBusConnection::ExportAllSlots"表示把类Hotel的所有Slot都导出为这个Object的method
        //qDebug() << Q_FUNC_INFO <<
        bus.registerObject("/", "org.bluez.obex.Agent1", this, QDBusConnection::ExportAllContents);
    }
}

void BluetoothObexAgent::authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request)
{
//    qDebug() << Q_FUNC_INFO << transfer->status() << transfer->fileName() << transfer->name() <<transfer->objectPath().path() << transfer->transferred();
    if(transfer->status() == BluezQt::ObexTransfer::Queued ){//|| transfer->status() == BluezQt::ObexTransfer::Complete
        receiving_widget = new FileReceivingPopupWidget(session->destination(),transfer->name(),session->source());
        qDebug() << Q_FUNC_INFO << __LINE__;

        connect(receiving_widget,&FileReceivingPopupWidget::accepted,this,[=]{
            request.accept(transfer->name());
            receiving_widget->configuration_transfer_progress_bar(transfer->size());
            connect(transfer.data(),&BluezQt::ObexTransfer::transferredChanged,receiving_widget,&FileReceivingPopupWidget::update_transfer_progress_bar);
            connect(transfer.data(),&BluezQt::ObexTransfer::statusChanged,receiving_widget,&FileReceivingPopupWidget::file_transfer_completed);
            connect(receiving_widget,&FileReceivingPopupWidget::cancel,this,[=]{
                transfer->cancel();
                qDebug() << Q_FUNC_INFO << "cancel" <<  __LINE__;
                //receiveDisConnectSignal(session->destination());

            });
        });

        connect(receiving_widget,&FileReceivingPopupWidget::rejected,this,[=]{
            request.reject();
        });

        receiving_widget->show();
    }
//    qDebug() << Q_FUNC_INFO << transfer->status() << transfer->type() << transfer->fileName() << transfer->transferred();
}

QDBusObjectPath BluetoothObexAgent::objectPath() const
{
    qDebug() << Q_FUNC_INFO;
    return QDBusObjectPath(QStringLiteral("/org/bluez/obex/Agent1"));
}

void BluetoothObexAgent::cancel()
{
    qDebug() << Q_FUNC_INFO;
    if (nullptr == receiving_widget)
    {
        qDebug() << Q_FUNC_INFO << "receiving_widget is null" ;
        return ;
    }

    receiving_widget->file_transfer_completed(BluezQt::ObexTransfer::Error);
}

void BluetoothObexAgent::release()
{
    qDebug() << Q_FUNC_INFO;
}

int BluetoothObexAgent::daemonIsNotRunning()
{
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.isConnected())
        return 0;

    QDBusReply<QString> reply = conn.interface()->call("GetNameOwner", "org.bluez.obex.Agent1");
    return reply.value() == "";
}

void BluetoothObexAgent::receiveDisConnectSignal(QString address)
{
    qDebug() << Q_FUNC_INFO << address << __LINE__;

    QDBusMessage m = QDBusMessage::createMethodCall("org.ukui.bluetooth","/org/ukui/bluetooth","org.ukui.bluetooth","disConnectToDevice");
    m << address;
    qDebug() << Q_FUNC_INFO << m.arguments().at(0).value<QString>() <<__LINE__;
    // 发送Message
    QDBusMessage response = QDBusConnection::sessionBus().call(m);
}
