#include "featureswidget.h"

FeaturesWidget::FeaturesWidget(QWidget *parent)
    : QWidget(parent)
{
    if(QGSettings::isSchemaInstalled("org.ukui.bluetooth")){
        settings = new QGSettings("org.ukui.bluetooth");

        paired_device_address = settings->get("paired-device-address").toStringList();
        finally_connect_the_device = settings->get("finally-connect-the-device").toString();
        paired_device = settings->get("paired-device").toStringList();
        Default_Adapter = settings->get("adapter-address").toString();

        qDebug() << "GSetting Value: " << Default_Adapter << finally_connect_the_device << paired_device;
        connect(settings, &QGSettings::changed,this,&FeaturesWidget::GSettings_value_chanage);
    }

    session_dbus = new BluetoothDbus(this);
    connect(session_dbus,&BluetoothDbus::sendDevAddress,this,&FeaturesWidget::Pair_device_by_address);

    m_manager = new BluezQt::Manager(this);
    bluetoothAgent = new BluetoothAgent(this);
    bluetoothObexAgent = new BluetoothObexAgent(this);
    BluezQt::InitManagerJob *job = m_manager->init();
    job->exec();
    qDebug() << m_manager->registerAgent(bluetoothAgent)->errorText();
    if(Default_Adapter.isEmpty()){
        m_adapter = m_manager->adapters().at(0).data();
        settings->set("adapter-address",QVariant::fromValue(m_adapter->address()));
    }else{
        m_adapter = m_manager->adapterForAddress(Default_Adapter).data();
    }
    qDebug() << m_adapter->isPowered();
    if(m_adapter->isPowered()){
        flag = true;
        settings->set("switch",QVariant::fromValue(flag));
    }else{
        flag = false;
        settings->set("switch",QVariant::fromValue(flag));
    }
    qDebug() << Q_FUNC_INFO << m_manager->isInitialized() << m_manager->isOperational();

    obex_manager = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *obex_job = obex_manager->init();
    obex_job->exec();
    qDebug() << Q_FUNC_INFO << obex_manager->registerAgent(bluetoothObexAgent);
    qDebug() << Q_FUNC_INFO << obex_manager->isInitialized() << obex_manager->isOperational() << obex_manager->sessions().size() << obex_manager->startService()->error();
//    qDebug() << m_adapter->devices().size();
    connect(obex_manager,&BluezQt::ObexManager::sessionAdded,this,&FeaturesWidget::file_transfer_session_add);

    tray_Menu = new QMenu();
    connect(tray_Menu,&QMenu::triggered,this,&FeaturesWidget::TraySignalProcessing);

    //Create taskbar tray icon and connect to signal slot
    //创建任务栏托盘图标，并连接信号槽
    bluetooth_tray_icon = new QSystemTrayIcon(QIcon::fromTheme("bluetooth-symbolic"),this);
    bluetooth_tray_icon->setContextMenu(tray_Menu);
    bluetooth_tray_icon->setToolTip(tr("Bluetooth"));
    bluetooth_tray_icon->show();
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
}

FeaturesWidget::~FeaturesWidget()
{
}

void FeaturesWidget::InitTrayMenu()
{
    tray_Menu->clear();
    if(flag)
        tray_Menu->addAction(tr("Turn off bluetooth"));
    else
        tray_Menu->addAction(tr("Turn on bluetooth"));

    QList<BluezQt::DevicePtr> device_list = m_adapter->devices();
    tray_Menu->addSeparator();
    QAction *head = new QAction(tr("Devices"));
    head->setDisabled(true);
    tray_Menu->addAction(head);
    for(int i=0; i < device_list.size(); i++){
        if(device_list.at(i)->isPaired()){
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
            device_menu->addAction(send);
            tray_Menu->addMenu(device_menu);

//            connect(device_menu,&QMenu::triggered,this,&FeaturesWidget::TrayItemSignalProcessing);
        }
    }
    tray_Menu->addSeparator();
    tray_Menu->addAction(tr("Bluetooth settings"));

}

void FeaturesWidget::Pair_device_by_address(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    qDebug() << Q_FUNC_INFO << device->name();
    device->pair();
    Connect_device_by_address(address);
}

void FeaturesWidget::Disconnect_device_by_address(QString address)
{
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    BluezQt::PendingCall *call = device->disconnectFromDevice();
    QProcess *process = new QProcess(this);
    QString cmd = "notify-send";
    QStringList arg;
    if(call->error() == 0){
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
}

void FeaturesWidget::Connect_device_by_address(QString address)
{
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    BluezQt::PendingCall *call = device->connectToDevice();
    device->setTrusted(true);
    qDebug() << Q_FUNC_INFO << call->error();
    QProcess *process = new QProcess(this);
    QString cmd = "notify-send";
    QStringList arg;
    if(call->error() == 0){
        qDebug() << Q_FUNC_INFO;
        QString text = tr("The connection with the Bluetooth device \"")+device->name()+tr("\" is successful!");
        arg << tr("BlueTooth") << text << "-t" << "5000" << "-i" << "blueman";
        process->start(cmd,arg);

        settings->set("finally-connect-the-device",QVariant::fromValue(device->address()));
    }else{
        qDebug() << Q_FUNC_INFO;
        QString text = tr("The connection with the Bluetooth device \"")+device->name()+tr("\" failed!");
        arg << tr("BlueTooth") << text << "-t" << "5000" << "-i" << "blueman";
        process->start(cmd,arg);
    }
}

void FeaturesWidget::Send_files_by_address(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    QString dev_name = m_adapter->deviceForAddress(address)->name();
    selected_file = QFileDialog::getOpenFileName(0,
        tr("Select the file to be sent"), getenv("HOME"), tr("All Files (*)"));

    qDebug() << "Select file:" << selected_file;
    if(!selected_file.isNull()){
        transfer_widget = new BluetoothFileTransferWidget(selected_file,dev_name,address);
        connect(transfer_widget,&BluetoothFileTransferWidget::sender_dev_name,this,&FeaturesWidget::file_transfer_creator);
        connect(transfer_widget,&BluetoothFileTransferWidget::close_the_pre_session,this,&FeaturesWidget::close_session);
        transfer_widget->exec();
    }
}

void FeaturesWidget::TraySignalProcessing(QAction *action)
{
    qDebug() << Q_FUNC_INFO << action->text() ;
    if(action->text() == tr("Turn on bluetooth")){

        BluezQt::PendingCall *call = m_adapter->setPowered(true);
        if(call->error() == 0){
            action->setText(tr("Turn off bluetooth"));
            flag = true;
            settings->set("switch",QVariant::fromValue(flag));
        }else
            qDebug() << "Failed to turn on Bluetooth:" << call->errorText();

    }else if(action->text() == tr("Turn off bluetooth")){

        BluezQt::PendingCall *call = m_adapter->setPowered(false);
        if(call->error() == 0){
            action->setText(tr("Turn on bluetooth"));
            flag = false;
            settings->set("switch",QVariant::fromValue(flag));
        }else
            qDebug() << "Failed to turn off Bluetooth:" << call->errorText();

    }else if(action->text() == tr("Bluetooth settings")){

    }else if(action->text() == tr("Disconnection")){
        Disconnect_device_by_address(action->statusTip());
    }else if(action->text() == tr("Connection")){
        Connect_device_by_address(action->statusTip());
    }else if(action->text() == tr("Send files")){
        Send_files_by_address(action->statusTip());
    }else if(action->text() == tr("Remove")){
        BluezQt::DevicePtr device = m_adapter->deviceForAddress(action->statusTip());
        qDebug() << m_adapter->removeDevice(device);
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
    qDebug() << Q_FUNC_INFO << dev << m_adapter->address();
    QMap<QString,QVariant> map;

    map["Source"] = m_adapter->address();
    map["Target"] = "OPP";

    BluezQt::PendingCall *target = obex_manager->createSession(dev,map);
    connect(target,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *call){
        QVariant v = call->value();
        QDBusObjectPath session = v.value<QDBusObjectPath>();

        pre_session = session;
        opp = new BluezQt::ObexObjectPush(session);
        BluezQt::PendingCall *transfer = opp->sendFile(selected_file);
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
}

