#include "bluetoothobexagent.h"

BluetoothObexAgent::BluetoothObexAgent(QObject *parent):ObexAgent(parent)
{
    if(daemonIsNotRunning()){
        QDBusConnection bus = QDBusConnection::sessionBus();
        // 在session bus上注册名为"com.kylin_user_guide.hotel"的service

        if (!bus.registerService("org.ukui.bluetooth")) {  //注意命名规则-和_
                qDebug() << bus.lastError().message();
                exit(1);
        }
        // "QDBusConnection::ExportAllSlots"表示把类Hotel的所有Slot都导出为这个Object的method
        qDebug() << Q_FUNC_INFO << bus.registerObject("/", "org.bluez.obex.Agent1", this, QDBusConnection::ExportAllContents);
    }
}

void BluetoothObexAgent::authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request)
{
    qDebug() << Q_FUNC_INFO << transfer->status() << transfer->fileName() << transfer->name() <<transfer->objectPath().path() << transfer->transferred();
    FileReceivingPopupWidget *receiving_widget = new FileReceivingPopupWidget(session->destination(),transfer->name());
    qDebug() << Q_FUNC_INFO << __LINE__;

    connect(receiving_widget,&FileReceivingPopupWidget::accepted,this,[=]{
        request.accept(transfer->name());
        receiving_widget->configuration_transfer_progress_bar(transfer->size());
        connect(transfer.data(),&BluezQt::ObexTransfer::transferredChanged,receiving_widget,&FileReceivingPopupWidget::update_transfer_progress_bar);
        connect(transfer.data(),&BluezQt::ObexTransfer::statusChanged,receiving_widget,&FileReceivingPopupWidget::file_transfer_completed);
        connect(receiving_widget,&FileReceivingPopupWidget::cancel,this,[=]{
            transfer->cancel();
        });
    });

    connect(receiving_widget,&FileReceivingPopupWidget::rejected,this,[=]{
        request.reject();
    });

    receiving_widget->exec();

    qDebug() << Q_FUNC_INFO << transfer->status() << transfer->type() << transfer->fileName() << transfer->transferred();
}

QDBusObjectPath BluetoothObexAgent::objectPath() const
{
    qDebug() << Q_FUNC_INFO;
    return QDBusObjectPath(QStringLiteral("/org/bluez/obex/Agent1"));
}

void BluetoothObexAgent::cancel()
{
    qDebug() << Q_FUNC_INFO;
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
