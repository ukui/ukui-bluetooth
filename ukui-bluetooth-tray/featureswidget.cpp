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
    //========================END=================================


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
    //========================END===================================

    session_dbus = new BluetoothDbus(this);
    connect(session_dbus,&BluetoothDbus::ConnectTheSendingDevice,this,&FeaturesWidget::Pair_device_by_address);
    connect(session_dbus,&BluetoothDbus::DisconnectTheSendingDevice,this,&FeaturesWidget::Disconnect_device_by_address);
    connect(session_dbus,&BluetoothDbus::RemoveTheSendingDevice,this,&FeaturesWidget::Remove_device_by_address);
    connect(session_dbus,&BluetoothDbus::sendTransferMesg,this,&FeaturesWidget::Dbus_file_transfer);
    connect(session_dbus,&BluetoothDbus::switch_signals,this,&FeaturesWidget::Dbus_bluetooth_switch);

    if(m_manager->adapters().size()){
        adapter_list.clear();
        for(int i = 0;i < m_manager->adapters().size(); i++){
            adapter_list << m_manager->adapters().at(i).data()->address();
        }
        settings->set("adapter-address-list",QVariant::fromValue(adapter_list));
        qDebug() << Q_FUNC_INFO << adapter_list <<__LINE__;
    }

    if(m_manager->adapters().size() == 0){
        qDebug() << "no bluetooth adapter !!!";
        qDebug() << "Program exit !!!";
        exit_flag = true;
        return;
    }else if(m_manager->adapters().size() == 1){
        m_adapter = m_manager->adapters().at(0).data();
        settings->set("adapter-address",QVariant::fromValue(m_adapter->address()));
    }else{
        if(adapter_list.indexOf(Default_Adapter) != -1){
            m_adapter = m_manager->adapterForAddress(Default_Adapter).data();
        }else{
            m_adapter = m_manager->adapterForAddress(adapter_list.at(0)).data();
        }
    }

    if(!m_adapter->isDiscoverable()){
        m_adapter->setDiscoverable(true);
        m_adapter->setDiscoverableTimeout(0);
    }

    adapterChangeFUN();
    cur_adapter_address = m_adapter->address();

    bluetoothAgent = new BluetoothAgent(this);
    bluetoothObexAgent = new BluetoothObexAgent(this);
    qDebug() << m_manager->registerAgent(bluetoothAgent)->errorText();
    qDebug() << m_adapter->isPowered();

    if(settings->get("switch").toString() == "false"){
        m_adapter->setPowered(false);
    }else{
        if(!m_adapter->isPowered()){
            m_adapter->setPowered(true);
        }
    }

    if(File_save_path.isEmpty()){
        settings->set("file-save-path",QVariant::fromValue(QDir::homePath()));
    }
    qDebug() << Q_FUNC_INFO << m_manager->isInitialized() << m_manager->isOperational();

    obex_manager = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *obex_job = obex_manager->init();
    obex_job->exec();
    qDebug() << Q_FUNC_INFO << obex_manager->isInitialized() << obex_manager->isOperational() << obex_manager->sessions().size();

    if(!obex_manager->isOperational()){
        BluezQt::PendingCall *call = obex_manager->startService();
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
            QTimer::singleShot(1000,this,[=]{
                obex_manager->registerAgent(bluetoothObexAgent)->errorText();
            });
        });
    }else{
        obex_manager->registerAgent(bluetoothObexAgent);
    }
    connect(obex_manager,&BluezQt::ObexManager::sessionAdded,this,&FeaturesWidget::file_transfer_session_add);

    QPalette palette;
    palette.setBrush(QPalette::Base,QColor(Qt::black));
    palette.setBrush(QPalette::Text,QColor(Qt::white));
    tray_Menu = new QMenu(this);
    tray_Menu->setPalette(palette);
    tray_Menu->setMinimumWidth(160);
    connect(tray_Menu,&QMenu::triggered,this,&FeaturesWidget::TraySignalProcessing);

    //Create taskbar tray icon and connect to signal slot
    //创建任务栏托盘图标，并连接信号槽
    bluetooth_tray_icon = new QSystemTrayIcon(QIcon::fromTheme("bluetooth-active-symbolic"),this);
    bluetooth_tray_icon->setContextMenu(tray_Menu);
    bluetooth_tray_icon->setToolTip(tr("Bluetooth"));
    bluetooth_tray_icon->show();
    bluetooth_tray_icon->setVisible(settings->get("tray-show").toBool());

    if(settings->get("switch").toString() == "false"){
        bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-disabled-symbolic"));
    }else{
        bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-active-symbolic"));
    }

    connect(bluetooth_tray_icon,
            &QSystemTrayIcon::activated,
            [=](QSystemTrayIcon::ActivationReason reason){
                switch (reason)
                {
                    case QSystemTrayIcon::DoubleClick: /* 来自于双击激活。 */
                    case QSystemTrayIcon::Trigger: /* 来自于单击激活。 */
                    case QSystemTrayIcon::Context:
                        InitTrayMenu();
                        tray_Menu->move(bluetooth_tray_icon->geometry().x()+16,bluetooth_tray_icon->geometry().y()-50);
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
    QAction *switch_txt = new QAction(tray_Menu);
    if(!flag){
        switch_txt->setText(tr("Turn on bluetooth"));
        if(m_manager->isBluetoothBlocked()){
            switch_txt->setDisabled(true);
        }
        tray_Menu->addAction(switch_txt);
        tray_Menu->addSeparator();
    }else{
        switch_txt->setText(tr("Turn off bluetooth"));
        tray_Menu->addAction(switch_txt);
        tray_Menu->addSeparator();
        QList<BluezQt::DevicePtr> device_list = m_adapter->devices();
        QAction *head = new QAction(tr("Devices"));
        head->setDisabled(true);
        tray_Menu->addAction(head);
        for(int i=0; i < device_list.size(); i++){
            if(device_list.at(i)->isPaired()){
                noDev = false;
                QPalette palette;
                palette.setBrush(QPalette::Base,QColor(Qt::black));
                palette.setBrush(QPalette::Text,QColor(Qt::white));
                QMenu *device_menu = new QMenu(/*QIcon::fromTheme("pan-end-symbolic-rtl"),*/device_list.at(i)->name());
//                qDebug() << Q_FUNC_INFO << device_menu->size();
//                connect(device_list.at(i).data(),&BluezQt::Device::connectedChanged,device_menu,[=](bool value){
//                    qDebug() << Q_FUNC_INFO;
//                    if(value)
//                        device_menu->setIcon(QIcon::fromTheme("software-installed-symbolic"));
//                    else
//                        device_menu->;
//                });
                device_menu->setPalette(palette);
                device_menu->setObjectName(device_list.at(i)->address());
                device_menu->setMinimumWidth(160);
                QAction *status = new QAction(tray_Menu);
                QAction *send   = new QAction(tray_Menu);
                QAction *remove = new QAction(tray_Menu);
                if(!flag){
                    device_menu->setDisabled(true);
                }
                status->setStatusTip(device_list.at(i)->address());
                send->setStatusTip(device_list.at(i)->address());
                remove->setStatusTip(device_list.at(i)->address());
                if(device_list.at(i)->isConnected()){
                    device_menu->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
                    device_menu->setProperty("useIconHighlightEffect", 0x10);
                    device_menu->setIcon(QIcon::fromTheme("software-installed-symbolic"));                    
                    status->setText(tr("Disconnection"));

                    BluezQt::BatteryPtr dev_battery = device_list.at(i)->battery();
                    qDebug() << Q_FUNC_INFO << __LINE__ << dev_battery.isNull();
                    if(!dev_battery.isNull()){
                       QAction *battery = new QAction();
                       battery->setDisabled(true);
                       battery->setIcon(QIcon::fromTheme("battery-level-100-symbolic"));
                       battery->setText(tr("Power ")+QString::number(dev_battery->percentage(),10)+"%");
                       device_menu->addAction(battery);
                    }
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
                else {
                    qDebug() << Q_FUNC_INFO << q->error();
                    emit device.data()->pairedChanged(false);
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
        if(q->error() == 0){
            qDebug() << Q_FUNC_INFO;
            QString text = QString(tr("Bluetooth device “%1” disconnected successfully").arg(device->name()));
            SendNotifyMessage(text);
        }else{
            qDebug() << Q_FUNC_INFO;
            QString text = tr("Disconnect Error!!!");
            SendNotifyMessage(text);
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
            GKeyFile *key_file = nullptr;
            char *data;
            gsize length = 0;
            GError *error;
            key_file = g_key_file_new();
            g_key_file_load_from_file(key_file,pair_device_file.toStdString().c_str(),G_KEY_FILE_NONE,NULL);
            if(g_key_file_has_group(key_file,address.toStdString().c_str())){
                g_key_file_remove_group(key_file,address.toStdString().c_str(),&error);
                data = g_key_file_to_data(key_file,&length,NULL);
                g_file_set_contents(pair_device_file.toStdString().c_str(),data,length,NULL);
                g_free(data);
            }
            g_key_file_free(key_file);
        }else{
            qDebug() << Q_FUNC_INFO << "Device Remove failed!!!";
        }
    });
}

bool FeaturesWidget::Connect_device_by_address(QString address)
{
    qDebug() << Q_FUNC_INFO << (m_adapter == nullptr);
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    if(!device){
        qDebug() << Q_FUNC_INFO << "The connected device does not exist !";
        return false;
    }
    qDebug() << Q_FUNC_INFO << device->uuids();
    connect(device.data(),&BluezQt::Device::connectedChanged,this,[=](bool connected){
        if(!connected){
            if(dev_disconnected_flag){
                qDebug() << Q_FUNC_INFO;
                QString text = QString(tr("Bluetooth device “%1” disconnected!").arg(device->name()));
                SendNotifyMessage(text);
                dev_disconnected_flag = false;
            }
        }
    });

    BluezQt::DevicePtr finally_device = m_adapter->deviceForAddress(settings->get("finally-connect-the-device").toString());
    if(finally_device.isNull() || address == settings->get("finally-connect-the-device").toString()){
        qDebug() << Q_FUNC_INFO << finally_device.isNull() << address << settings->get("finally-connect-the-device").toString();
        return Connect_device(device);
    }else{
        if(finally_device->isConnected()){
               BluezQt::PendingCall *call = device->disconnectFromDevice();
               connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
                    if(q->error() == 0){
                        Connect_device(device);
                    }
               });
        }else{
            return Connect_device(device);
        }
    }
}

bool FeaturesWidget::Connect_device(BluezQt::DevicePtr device)
{
    BluezQt::PendingCall *call = device->connectToDevice();
    device->setTrusted(true);
    qDebug() << Q_FUNC_INFO << call->error();
    connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
        if(q->error() == 0){
            qDebug() << Q_FUNC_INFO;
            QString text = QString(tr("The connection with the Bluetooth device “%1” is successful!").arg(device->name()));
            SendNotifyMessage(text);

            settings->set("finally-connect-the-device",QVariant::fromValue(device->address()));
            dev_disconnected_flag = true;

            writeDeviceInfoToFile(device->address(),device->name());

            return true;
        }else{
            qDebug() << Q_FUNC_INFO << q->errorText();
            QString text = QString(tr("The connection with the Bluetooth device “%1” is failed!").arg(device->name()));
            SendNotifyMessage(text);

            return false;
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

void FeaturesWidget::SendNotifyMessage(QString message)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                             "/org/freedesktop/Notifications",
                             "org.freedesktop.Notifications",
                             QDBusConnection::sessionBus());
    QList<QVariant> args;
    args<<tr("ukui-bluetooth")
        <<((unsigned int) 0)
        <<"bluetooth"
        <<tr("Bluetooth message") //显示的是什么类型的信息
        <<message //显示的具体信息
        <<QStringList()
        <<QVariantMap()
        <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}

void FeaturesWidget::Send_files_by_address(QString address)
{
    if(!obex_manager->isOperational()){
        BluezQt::PendingCall *call = obex_manager->startService();
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
            QTimer::singleShot(1000,this,[=]{
                obex_manager->registerAgent(bluetoothObexAgent)->errorText();
            });
        });
    }

    qDebug() << Q_FUNC_INFO << address;
    QTextCodec* codec =QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForLocale(codec);
    selected_file = QFileDialog::getOpenFileName(0,
        tr("Select the file to be sent"), getenv("HOME"), tr("All Files (*)"));
    qDebug() << "Select file:" << selected_file.path();
    if(!selected_file.path().isNull()){
        if (BluetoothFileTransferWidget::isShow == false) {
            transfer_widget = new BluetoothFileTransferWidget(selected_file,address);
            connect(transfer_widget,&BluetoothFileTransferWidget::sender_dev_name,this,&FeaturesWidget::file_transfer_creator);
            connect(transfer_widget,&BluetoothFileTransferWidget::close_the_pre_session,this,&FeaturesWidget::close_session);
            transfer_widget->show();
        }
        else {
//            QMessageBox::warning(NULL, tr("bluetooth"), tr("A transfer is in progress..."),
//                                     QMessageBox::Ok,QMessageBox::Ok);
        }
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

void FeaturesWidget::Dbus_file_transfer(QUrl file_path)
{
    if(!obex_manager->isOperational()){
        BluezQt::PendingCall *call = obex_manager->startService();
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
            QTimer::singleShot(1000,this,[=]{
                obex_manager->registerAgent(bluetoothObexAgent)->errorText();
            });
        });
    }

    qDebug() << "Select file:" << file_path.path();
    selected_file = file_path;
    if(!selected_file.path().isNull()){
        if (BluetoothFileTransferWidget::isShow == false) {
            transfer_widget = new BluetoothFileTransferWidget(selected_file,"");
            connect(transfer_widget,&BluetoothFileTransferWidget::sender_dev_name,this,&FeaturesWidget::file_transfer_creator);
            connect(transfer_widget,&BluetoothFileTransferWidget::close_the_pre_session,this,&FeaturesWidget::close_session);
            transfer_widget->show();
        }
        else {
//            QMessageBox::warning(NULL, tr("bluetooth"), tr("A transfer is in progress..."),
//                                     QMessageBox::Ok,QMessageBox::Ok);
        }
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
}

void FeaturesWidget::Connect_the_last_connected_device()
{
    qDebug() << Q_FUNC_INFO;
    QStringList target_list;
    target_list.clear();
    target_list = getDeviceConnectTimeList();
    qDebug() << Q_FUNC_INFO << target_list.at(dev_callbak_flag);

    BluezQt::DevicePtr dev = m_adapter->deviceForAddress(target_list.at(dev_callbak_flag));
    BluezQt::PendingCall *pp = dev->connectToDevice();
    connect(pp,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *call){
        if(call->error() == 0){
            writeDeviceInfoToFile(dev->address(),dev->name());
        }else{
            if(dev_callbak_flag < target_list.length()-1){
                dev_callbak_flag++;
                Connect_the_last_connected_device();
            }else{
                return;
            }
        }
    });
}

// ===================蓝牙管理器设备操作监听======（主要针对系统的休眠和增加新的蓝牙硬件）=======================
void FeaturesWidget::adapterChangeFUN()
{
    connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);
    connect(m_adapter,&BluezQt::Adapter::deviceRemoved,this,&FeaturesWidget::adapterDeviceRemove);

    connect(m_manager,&BluezQt::Manager::adapterRemoved,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO << adapter->address() << adapter->name() <<__LINE__;

        adapter_list.removeAll(adapter->address());
        settings->set("adapter-address-list",QVariant::fromValue(adapter_list));

        if(adapter_list.size() == 0){
            if(bluetooth_tray_icon->isVisible()){
                bluetooth_tray_icon->setVisible(false);
            }
        }else{
            QTimer::singleShot(200,this,[=]{
                if(!sleep_flag){
                    m_adapter = m_manager->adapterForAddress(adapter_list.at(0)).data();
                    if(!m_adapter->isDiscoverable()){
                        m_adapter->setDiscoverable(true);
                        m_adapter->setDiscoverableTimeout(0);
                    }
                    connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);
                }
            });
        }
    });

    connect(m_manager,&BluezQt::Manager::adapterAdded,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO << adapter->address() << adapter->name() <<__LINE__;
        adapter_list.append(adapter->address());
        if(adapter_list.size() != 0){
            if(!bluetooth_tray_icon->isVisible()){
                bluetooth_tray_icon->setVisible(true);
            }
        }
        settings->set("adapter-address-list",QVariant::fromValue(adapter_list));
    });

//    connect(m_manager,&BluezQt::Manager::adapterChanged,this,[=](BluezQt::AdapterPtr adapter){
//        qDebug() << Q_FUNC_INFO <<__LINE__;
//        m_adapter = adapter.data();
//        connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);
//        if(m_adapter->address() == cur_adapter_address)
//            QTimer::singleShot(1000,this,[=]{
//                Connect_the_last_connected_device();
//            });
//        else{
//            QTimer::singleShot(1000,this,[=]{
//                cur_adapter_address = m_adapter->address();
//            });
//        }
//    });

    connect(m_manager,&BluezQt::Manager::usableAdapterChanged,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO <<__LINE__;
    });
}
// ===========================================END========================================================

void FeaturesWidget::createPairDeviceFile()
{
    if(pair_device_file.isNull())
        pair_device_file = QDir::homePath() + "/.config/pairDevice.list";

    if(!QFile::exists(pair_device_file)){
        QFile file(pair_device_file);
        file.open(QIODevice::WriteOnly);
        file.close();
    }else{
        return;
    }
}

void FeaturesWidget::writeDeviceInfoToFile(const QString &devAddress, const QString &devName)
{
    createPairDeviceFile();
    GKeyFile *key_file = nullptr;
    key_file = g_key_file_new();
    char *data;
    gsize length = 0;
    g_key_file_load_from_file(key_file,pair_device_file.toStdString().c_str(),G_KEY_FILE_NONE,NULL);
    g_key_file_set_string(key_file,devAddress.toStdString().c_str(),"Name",devName.toStdString().c_str());
    g_key_file_set_string(key_file,devAddress.toStdString().c_str(),"ConnectTime",QString::number(QDateTime::currentMSecsSinceEpoch() / 1000).toStdString().c_str());


    data = g_key_file_to_data(key_file,&length,NULL);
    g_file_set_contents(pair_device_file.toStdString().c_str(),data,length,NULL);
    g_free(data);

    g_key_file_free(key_file);
}

QStringList FeaturesWidget::getDeviceConnectTimeList()
{
    QStringList pair_device_list;
    QVector<int> pair_device_time_list;
    pair_device_list.clear();
    pair_device_time_list.clear();
    GKeyFile *key_file = nullptr;
    key_file = g_key_file_new();
    g_key_file_load_from_file(key_file,pair_device_file.toStdString().c_str(),G_KEY_FILE_NONE,NULL);
    gchar **list;
    bool ok;
    list  = g_key_file_get_groups(key_file,NULL);
    for(int i = 0; list[i] != NULL; i++){
        pair_device_list.append(QString::fromUtf8(list[i]));
        pair_device_time_list.append(QString::fromUtf8(g_key_file_get_string(key_file,list[i],"ConnectTime",NULL)).toInt(&ok,10));
    }

    for(int i = 0; i < pair_device_time_list.length(); i++){
        for(int j = 0; j < pair_device_time_list.length()-i-1; j++){
            if(pair_device_time_list.at(j) < pair_device_time_list.at(j+1)){
                int max = pair_device_time_list.at(j);
                pair_device_time_list.replace(j,pair_device_time_list.at(j+1));
                pair_device_time_list.replace(j+1,max);
                QString target = pair_device_list.at(j);
                pair_device_list.removeAt(j);
                pair_device_list.insert(j+1,target);
            }
        }
    }

    g_key_file_free(key_file);

    return pair_device_list;
}

void FeaturesWidget::Monitor_sleep_Slot(bool sleep)
{
    if(!sleep){
        qDebug() << "System wakes up from sleep !!!" << dev_remove_flag;
        sleep_flag = false;

        QTimer::singleShot(200,this,[=]{
            if(m_manager->adapters().size()){
                adapter_list.clear();
                for(int i = 0;i < m_manager->adapters().size(); i++){
                    adapter_list << m_manager->adapters().at(i).data()->address();
                }
                settings->set("adapter-address-list",QVariant::fromValue(adapter_list));
            }

            if(m_manager->adapters().size()){
                if(adapter_list.indexOf(cur_adapter_address) != -1){
                    m_adapter = m_manager->adapterForAddress(cur_adapter_address).data();
                }else{
                    m_adapter = m_manager->adapterForAddress(adapter_list.at(0)).data();
                }
                if(!m_adapter->isDiscoverable()){
                    m_adapter->setDiscoverable(true);
                    m_adapter->setDiscoverableTimeout(0);
                }
            }else{

            }
            connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);
            dev_callbak_flag = 0;
            Connect_the_last_connected_device();
        });

    }else{
        sleep_flag = true;
        qDebug() << "System goes to sleep !!!" << dev_remove_flag;
    }
}

void FeaturesWidget::adapterPoweredChanged(bool value)
{
    qDebug() << Q_FUNC_INFO;
    settings->set("switch",QVariant::fromValue(value));
    flag = value;
    if (value == true) {//开启
        bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-active-symbolic"));
    }else {//关闭
        bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-disabled-symbolic"));
    }
}

void FeaturesWidget::adapterDeviceRemove(BluezQt::DevicePtr ptr)
{
    qDebug() << Q_FUNC_INFO << ptr.data()->address() << ptr.data()->name();
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
//    BluezQt::DevicePtr device = m_adapter->deviceForAddress(dev);
//    if(!device->isConnected()){
//        Connect_device_by_address(dev);
//    }

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
        BluezQt::PendingCall *transfer = opp->sendFile(selected_file.path());
        qDebug() << Q_FUNC_INFO << transfer->error() << transfer->errorText();
        connect(transfer,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *call){
            qDebug() << Q_FUNC_INFO << __LINE__;
            QVariant v = call->value();
            filePtr = v.value<BluezQt::ObexTransferPtr>();
            if(filePtr){
                transfer_file_size = filePtr->size();
//                connect(ptr.get(),&BluezQt::ObexTransfer::transferredChanged,this,[=](quint64 vl){
//                    qDebug() << Q_FUNC_INFO << vl << __LINE__;
//                });
//                connect(ptr.get(),&BluezQt::ObexTransfer::statusChanged,this,[=](BluezQt::ObexTransfer::Status status){
//                    qDebug() << Q_FUNC_INFO << status <<__LINE__;
//                });
                if (QDBusConnection::sessionBus().connect("org.bluez.obex", filePtr->objectPath().path(),
                        "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                        SLOT(propertyChanged(QString, QVariantMap, QStringList)))) {
                    qDebug() << "PropertiesChanged signal connected successfully to slot";
                } else {
                    qDebug() << "PropertiesChanged signal connection was not successful";
                }
            }else{
                transfer_widget->tranfer_error();
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
            }else if(value.value<QString>() == "error"){
                qDebug() << Q_FUNC_INFO << __LINE__;
                filePtr->cancel();
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
    if(key == "trayShow"){
        if(bluetooth_tray_icon->isVisible() != settings->get(key).toBool()){
            bluetooth_tray_icon->setVisible(settings->get(key).toBool());
        }
    }else if(key == "file-save-path"){

    }else if(key == "adapterAddress"){
        m_adapter = m_manager->adapterForAddress(settings->get("adapterAddress").toString()).data();
        cur_adapter_address = m_adapter->address();
        connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);

        if(!m_adapter->isDiscoverable()){
            m_adapter->setDiscoverable(true);
            m_adapter->setDiscoverableTimeout(0);
        }
        if(m_adapter->isPowered())
            flag = true;
        else
            flag = false;
    }
}

//org.ukui.bluetooth dbus蓝牙开关接口
void FeaturesWidget::Dbus_bluetooth_switch(bool value)
{
    if(value != settings->get("switch").toBool()){
        Turn_on_or_off_bluetooth(value);
    }
}
