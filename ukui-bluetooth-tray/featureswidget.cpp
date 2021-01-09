#include "featureswidget.h"

FeaturesWidget::FeaturesWidget(QWidget *parent)
    : QWidget(parent)
{
    //=============初始化蓝牙管理器，失败则退出进程===================
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *job = m_manager->init();
    job->exec();

    qDebug() << m_manager->isInitialized();
    if(!m_manager->isInitialized()){
        qDebug() << "BluezQt::Manager init failed !!!";
        qDebug() << "Program exit !!!";
        exit_flag = true;
        return;
    }

    if(!m_manager->isOperational()){
        qDebug() << "BluezQt::Manager  manager is not operational !!!";
        qDebug() << "BlueZ system daemon is not running";
        qDebug() << "Program exit !!!";
        exit_flag = true;
        return;
    }
    //========================End=================================


    //==============获取gsettings的配置信息==========================
    if(QGSettings::isSchemaInstalled("org.ukui.bluetooth")){
        settings = new QGSettings("org.ukui.bluetooth");

        paired_device_address = settings->get("paired-device-address").toStringList();
        finally_connect_the_device = settings->get("finally-connect-the-device").toString();
        paired_device = settings->get("paired-device").toStringList();
        Default_Adapter = settings->get("adapter-address").toString();
        File_save_path = settings->get("file-save-path").toString();

        qDebug() << "GSetting Value: " << Default_Adapter << finally_connect_the_device << paired_device;
        connect(settings, &QGSettings::changed,this,&FeaturesWidget::GSettings_value_chanage);
    }
    //========================End===================================

    session_dbus = new BluetoothDbus(this);
    connect(session_dbus,&BluetoothDbus::ConnectTheSendingDevice,this,&FeaturesWidget::Pair_device_by_address);
    connect(session_dbus,&BluetoothDbus::DisconnectTheSendingDevice,this,&FeaturesWidget::Disconnect_device_by_address);
    connect(session_dbus,&BluetoothDbus::RemoveTheSendingDevice,this,&FeaturesWidget::Remove_device_by_address);
    connect(session_dbus,&BluetoothDbus::sendTransferMesg,this,&FeaturesWidget::Dbus_file_transfer);
    connect(session_dbus,&BluetoothDbus::switch_signals,this,&FeaturesWidget::Dbus_bluetooth_switch);

    if(Default_Adapter.isEmpty()){
        m_adapter = m_manager->adapters().at(0).data();
        settings->set("adapter-address",QVariant::fromValue(m_adapter->address()));
    }else{
        m_adapter = m_manager->adapterForAddress(Default_Adapter).data();
    }

    if(!m_adapter->isDiscoverable()){
        m_adapter->setDiscoverable(true);
    }

    bluetoothAgent = new BluetoothAgent(this);
    bluetoothObexAgent = new BluetoothObexAgent(this);
    qDebug() << m_manager->registerAgent(bluetoothAgent)->errorText();
    qDebug() << m_adapter->isPowered();
    if(m_adapter->isPowered()){
        flag = true;
        settings->set("switch",QVariant::fromValue(flag));
    }else{
        flag = false;
        settings->set("switch",QVariant::fromValue(flag));
    }
    connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,[=](bool power){
        settings->set("switch",QVariant::fromValue(power));
    });

    if(File_save_path.isEmpty()){
        settings->set("file-save-path",QVariant::fromValue(QDir::homePath()));
    }
    qDebug() << Q_FUNC_INFO << m_manager->isInitialized() << m_manager->isOperational();

    obex_manager = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *obex_job = obex_manager->init();
    obex_job->exec();
    qDebug() << Q_FUNC_INFO << obex_manager->registerAgent(bluetoothObexAgent);
    qDebug() << Q_FUNC_INFO << obex_manager->isInitialized() << obex_manager->isOperational() << obex_manager->sessions().size() << obex_manager->startService()->error();
//    qDebug() << m_adapter->devices().size();
    connect(obex_manager,&BluezQt::ObexManager::sessionAdded,this,&FeaturesWidget::file_transfer_session_add);

    tray_Menu = new QMenu(this);
    connect(tray_Menu,&QMenu::triggered,this,&FeaturesWidget::TraySignalProcessing);

    //Create taskbar tray icon and connect to signal slot
    //创建任务栏托盘图标，并连接信号槽
    bluetooth_tray_icon = new QSystemTrayIcon(QIcon::fromTheme("bluetooth-active-symbolic"),this);
    bluetooth_tray_icon->setContextMenu(tray_Menu);
    bluetooth_tray_icon->setToolTip(tr("Bluetooth"));
    bluetooth_tray_icon->show();
    bluetooth_tray_icon->setVisible(settings->get("tray-show").toBool());

    connect(bluetooth_tray_icon,
            &QSystemTrayIcon::activated,
            [=](QSystemTrayIcon::ActivationReason reason){
                switch (reason)
                {
                    case QSystemTrayIcon::DoubleClick: /* 来自于双击激活。 */
                    case QSystemTrayIcon::Trigger: /* 来自于单击激活。 */
                        InitTrayMenu();
                        tray_Menu->move(bluetooth_tray_icon->geometry().x()+16,bluetooth_tray_icon->geometry().y()-100);
                        tray_Menu->exec();
                        break;
                }
            });

    Monitor_sleep_signal();
}

FeaturesWidget::~FeaturesWidget()
{
    delete settings;
}

void FeaturesWidget::InitTrayMenu()
{
    bool noDev = true;
    tray_Menu->clear();
    if(!flag){
        tray_Menu->addAction(tr("Turn on bluetooth"));
        tray_Menu->addSeparator();
    }else{
        tray_Menu->addAction(tr("Turn off bluetooth"));
        tray_Menu->addSeparator();
        QList<BluezQt::DevicePtr> device_list = m_adapter->devices();
        QAction *head = new QAction(tr("Devices"));
        head->setDisabled(true);
        tray_Menu->addAction(head);
        for(int i=0; i < device_list.size(); i++){
            if(device_list.at(i)->isPaired()){
                noDev = false;
                QMenu *device_menu = new QMenu(/*QIcon::fromTheme("pan-end-symbolic-rtl"),*/device_list.at(i)->name());
                QAction *status = new QAction();
                QAction *send   = new QAction();
                QAction *remove = new QAction();
                if(!flag){
                    device_menu->setDisabled(true);
                }
                status->setStatusTip(device_list.at(i)->address());
                send->setStatusTip(device_list.at(i)->address());
                remove->setStatusTip(device_list.at(i)->address());
                if(device_list.at(i)->isConnected()){
                    device_menu->setIcon(QIcon::fromTheme("software-installed-symbolic"));
                    status->setText(tr("Disconnection"));

                }else{
                    status->setText(tr("Connection"));
                    remove->setText(tr("Remove"));
                    device_menu->addAction(remove);
                }
                device_menu->addAction(status);
                send->setText(tr("Send files"));
                if(device_list.at(i)->type()==BluezQt::Device::Phone || device_list.at(i)->type()==BluezQt::Device::Computer)
                    device_menu->addAction(send);
                tray_Menu->addMenu(device_menu);

    //            connect(device_menu,&QMenu::triggered,this,&FeaturesWidget::TrayItemSignalProcessing);
            }
        }

        if(noDev){
            QAction *text = new QAction(tr("No paired devices"));
            text->setDisabled(true);
            tray_Menu->addAction(text);
        }
    }
    tray_Menu->addSeparator();
    tray_Menu->addAction(tr("Bluetooth settings"));

}

void FeaturesWidget::Pair_device_by_address(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);

    if(device->isPaired()){
        Connect_device_by_address(address);
    }else{
        if(device->type() == BluezQt::Device::Mouse || device->type() == BluezQt::Device::Keyboard){
            Connect_device_by_address(address);
        }else{
            qDebug() << Q_FUNC_INFO << device->name();
            BluezQt::PendingCall *call = device->pair();
            connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
                if(q->error() == 0){
                    Connect_device_by_address(address);
                }
            });
        }
    }
}

void FeaturesWidget::Disconnect_device_by_address(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    BluezQt::PendingCall *call = device->disconnectFromDevice();
    connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
        QProcess *process = new QProcess(this);
        QString cmd = "notify-send";
        QStringList arg;
        if(q->error() == 0){
            qDebug() << Q_FUNC_INFO;
            QString text = tr("Disconnect from the Bluetooth device \"")+device->name()+tr("\"");
            arg << tr("BlueTooth") << text << "-t" << "5000" << "-i" << "blueman";
            process->start(cmd,arg);
        }else{
            qDebug() << Q_FUNC_INFO;
            QString text = tr("Disconnect Error!!!");
            arg << tr("BlueTooth") << text << "-t" << "5000" << "-i" << "blueman";
            process->start(cmd,arg);
        }
        dev_disconnected_flag = false;
    });
}

void FeaturesWidget::Remove_device_by_address(QString address)
{
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    BluezQt::PendingCall *call = m_adapter->removeDevice(device);
    connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
        if(q->error() == 0){

        }else{
            qDebug() << Q_FUNC_INFO << "Device Remove failed!!!";
        }
    });
}

void FeaturesWidget::Connect_device_by_address(QString address)
{
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    connect(device.data(),&BluezQt::Device::connectedChanged,this,[=](bool connected){
        if(!connected){
            if(dev_disconnected_flag){
                QProcess *process = new QProcess(this);
                QString cmd = "notify-send";
                QStringList arg;
                qDebug() << Q_FUNC_INFO;
                QString text = tr("Bluetooth device")+" \""+device->name()+"\" "+tr("disconnected!");
                arg << tr("BlueTooth") << text << "-t" << "5000" << "-i" << "blueman";
                process->start(cmd,arg);
                dev_disconnected_flag = false;
            }
        }
    });

    BluezQt::DevicePtr finally_device = m_adapter->deviceForAddress(settings->get("finally-connect-the-device").toString());
    if(finally_device.isNull() || address == settings->get("finally-connect-the-device").toString()){
        Connect_device(device);
    }else{
        if(finally_device->isConnected()){
               BluezQt::PendingCall *call = device->disconnectFromDevice();
               connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
                    if(q->error() == 0){
                        Connect_device(device);
                    }
               });
        }else{
            Connect_device(device);
        }
    }
}

void FeaturesWidget::Connect_device(BluezQt::DevicePtr device)
{
    BluezQt::PendingCall *call = device->connectToDevice();
    device->setTrusted(true);
    qDebug() << Q_FUNC_INFO << call->error();
    connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
        QProcess *process = new QProcess(this);
        QString cmd = "notify-send";
        QStringList arg;
        if(q->error() == 0){
            qDebug() << Q_FUNC_INFO;
            QString text = tr("The connection with the Bluetooth device \"")+device->name()+tr("\" is successful!");
            arg << tr("BlueTooth") << text << "-t" << "5000" << "-i" << "blueman";
            process->start(cmd,arg);

            settings->set("finally-connect-the-device",QVariant::fromValue(device->address()));

            dev_disconnected_flag = true;
        }else{
            qDebug() << Q_FUNC_INFO;
            QString text = tr("The connection with the Bluetooth device \"")+device->name()+tr("\" failed!");
            arg << tr("BlueTooth") << text << "-t" << "5000" << "-i" << "blueman";
            process->start(cmd,arg);
        }
    });
}

void FeaturesWidget::Open_bluetooth_settings()
{
    QProcess *process = new QProcess(this);
    QString cmd = "ukui-control-center";
    QStringList arg;
    qDebug() << Q_FUNC_INFO;
    arg << "--bluetooth";
    process->start(cmd,arg);
}

void FeaturesWidget::Send_files_by_address(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    selected_file = QFileDialog::getOpenFileName(0,
        tr("Select the file to be sent"), getenv("HOME"), tr("All Files (*)"));

    qDebug() << "Select file:" << selected_file.unicode();
    if(!selected_file.isNull()){
        transfer_widget = new BluetoothFileTransferWidget(selected_file,address);
        connect(transfer_widget,&BluetoothFileTransferWidget::sender_dev_name,this,&FeaturesWidget::file_transfer_creator);
        connect(transfer_widget,&BluetoothFileTransferWidget::close_the_pre_session,this,&FeaturesWidget::close_session);
        transfer_widget->exec();
    }
}

void FeaturesWidget::Turn_on_or_off_bluetooth(bool f)
{
    if(f){
        BluezQt::PendingCall *call = m_adapter->setPowered(true);
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *p){
            if(p->error() == 0){
                flag = true;
    //            qDebug() << Q_FUNC_INFO << m_adapter->isPowered();
            }else
                qDebug() << "Failed to turn off Bluetooth:" << p->errorText();
        });
    }else{
        BluezQt::PendingCall *call = m_adapter->setPowered(false);
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *p){
            if(p->error() == 0){
                flag = false;
    //            qDebug() << Q_FUNC_INFO << m_adapter->isPowered();
            }else
                qDebug() << "Failed to turn off Bluetooth:" << p->errorText();
        });
    }
}

void FeaturesWidget::Dbus_file_transfer(QString file_path)
{
    qDebug() << "Select file:" << file_path.unicode();
    selected_file = file_path;
    if(!selected_file.isNull()){
        transfer_widget = new BluetoothFileTransferWidget(selected_file,"");
        connect(transfer_widget,&BluetoothFileTransferWidget::sender_dev_name,this,&FeaturesWidget::file_transfer_creator);
        connect(transfer_widget,&BluetoothFileTransferWidget::close_the_pre_session,this,&FeaturesWidget::close_session);
        transfer_widget->exec();
    }
}

void FeaturesWidget::Monitor_sleep_signal()
{
    if (QDBusConnection::systemBus().connect("org.freedesktop.login1", "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager", "PrepareForSleep", this,
            SLOT(Monitor_sleep_Slot(bool)))) {
        qDebug() << "PrepareForSleep signal connected successfully to slot";
    } else {
        qDebug() << "PrepareForSleep signal connection was not successful";
    }

    //如果进行了移除蓝牙适配器操作，则将dev_remove_flag标志位设为true
    connect(m_manager,&BluezQt::Manager::adapterRemoved,this,[=]{
        qDebug() << Q_FUNC_INFO << __LINE__;
        dev_remove_flag = true;
    });
}

void FeaturesWidget::Connect_the_last_connected_device()
{
    qDebug() << Q_FUNC_INFO;
    Connect_device_by_address(settings->get("finally-connect-the-device").toString());
}

void FeaturesWidget::Monitor_sleep_Slot(bool sleep)
{
    if(!sleep){
        qDebug() << "System wakes up from sleep !!!" << dev_remove_flag;
        if(dev_remove_flag){
            connect(m_manager,&BluezQt::Manager::adapterAdded,this,[=](BluezQt::AdapterPtr adapter){
                if(m_adapter != adapter.data()){
                    m_adapter = adapter.data();
                    connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,[=](bool power){
                        settings->set("switch",QVariant::fromValue(power));
                    });
                    Connect_the_last_connected_device();
                }
            });
        }else{

        }
        dev_remove_flag = false;
    }else{
        qDebug() << "System goes to sleep !!!" << dev_remove_flag;
    }
}

void FeaturesWidget::TraySignalProcessing(QAction *action)
{
    qDebug() << Q_FUNC_INFO << action->text() ;
    if(action->text() == tr("Turn on bluetooth")){
        Turn_on_or_off_bluetooth(true);

    }else if(action->text() == tr("Turn off bluetooth")){
        Turn_on_or_off_bluetooth(false);

    }else if(action->text() == tr("Bluetooth settings")){
        Open_bluetooth_settings();
    }else if(action->text() == tr("Disconnection")){
        Disconnect_device_by_address(action->statusTip());
    }else if(action->text() == tr("Connection")){
        Connect_device_by_address(action->statusTip());
    }else if(action->text() == tr("Send files")){
        Send_files_by_address(action->statusTip());
    }else if(action->text() == tr("Remove")){
        Remove_device_by_address(action->statusTip());
    }
}

void FeaturesWidget::file_transfer_session_add(BluezQt::ObexSessionPtr sessionPtr)
{
    qDebug() << Q_FUNC_INFO << sessionPtr->destination() << obex_manager->sessions().size();
    qDebug() << Q_FUNC_INFO << sessionPtr->source() << m_adapter->address();

    if(sessionPtr->source() == m_adapter->address()){

    }
}

void FeaturesWidget::file_transfer_creator(QString dev)
{
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(dev);
    if(!device->isConnected()){
        Connect_device_by_address(dev);
    }

    qDebug() << Q_FUNC_INFO << dev << m_adapter->address();
    QMap<QString,QVariant> map;

    map["Source"] = m_adapter->address();
    map["Target"] = "OPP";

    BluezQt::PendingCall *target = obex_manager->createSession(dev,map);
    qDebug() << Q_FUNC_INFO << target->error() << target->errorText();
    connect(target,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *call){
        QVariant v = call->value();
        QDBusObjectPath session = v.value<QDBusObjectPath>();

        pre_session = session;
        opp = new BluezQt::ObexObjectPush(session);
        BluezQt::PendingCall *transfer = opp->sendFile(selected_file);
        qDebug() << Q_FUNC_INFO << transfer->error() << transfer->errorText();
        connect(transfer,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *call){
            QVariant v = call->value();
            BluezQt::ObexTransferPtr ptr = v.value<BluezQt::ObexTransferPtr>();
            transfer_file_size = ptr->size();
//            connect(ptr.get(),&BluezQt::ObexTransfer::transferredChanged,this,[=](quint64 vl){
//                qDebug() << Q_FUNC_INFO << vl << __LINE__;
//            });
//            connect(ptr.get(),&BluezQt::ObexTransfer::statusChanged,this,[=](BluezQt::ObexTransfer::Status status){
//                qDebug() << Q_FUNC_INFO << status <<__LINE__;
//            });
            if (QDBusConnection::sessionBus().connect("org.bluez.obex", ptr->objectPath().path(),
                    "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                    SLOT(propertyChanged(QString, QVariantMap, QStringList)))) {
                qDebug() << "PropertiesChanged signal connected successfully to slot";
            } else {
                qDebug() << "PropertiesChanged signal connection was not successful";
            }
        });
    });
    qDebug() << Q_FUNC_INFO << obex_manager->sessions().size() << __LINE__;
}

void FeaturesWidget::close_session()
{
    if(pre_session.path() != ""){
        obex_manager->removeSession(pre_session);
    }
}

void FeaturesWidget::propertyChanged(QString name, QVariantMap map, QStringList list)
{
//    qDebug() << QString("properties of interface %1 changed").arg(name);
    for (QVariantMap::const_iterator it = map.cbegin(), end = map.cend(); it != end; ++it) {
        qDebug() << "property: " << it.key() << " value: " << it.value();
        if(it.key() == "Transferred"){
            QVariant value = it.value();
            transfer_widget->set_m_progressbar_value(value.value<quint64>());
        }else if(it.key() == "Status"){
            QVariant value = it.value();
            if(value.value<QString>() == "active"){
                transfer_widget->init_m_progressbar_value(transfer_file_size);
            }
            transfer_widget->get_transfer_status(value.value<QString>());
        }
    }
    for (const auto& element : list) {
        qDebug() << "list element: " << element;
    }
}

void FeaturesWidget::GSettings_value_chanage(const QString &key)
{
    qDebug() << Q_FUNC_INFO << key;
    if(key == "switch"){
        flag = settings->get(key).toBool();
    }else if(key == "trayShow"){
        if(bluetooth_tray_icon->isVisible() != settings->get(key).toBool()){
            bluetooth_tray_icon->setVisible(settings->get(key).toBool());
        }
    }else if(key == "file-save-path"){

    }
}

void FeaturesWidget::Dbus_bluetooth_switch(bool value)
{
    if(value != settings->get("switch").toBool()){
        Turn_on_or_off_bluetooth(value);
    }
}
