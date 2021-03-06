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
#include "featureswidget.h"
#include <fcntl.h>

enum rfkill_type {
    RFKILL_TYPE_ALL = 0,
    RFKILL_TYPE_WLAN,
    RFKILL_TYPE_BLUETOOTH,
    RFKILL_TYPE_UWB,
    RFKILL_TYPE_WIMAX,
    RFKILL_TYPE_WWAN,
};

enum rfkill_operation {
    RFKILL_OP_ADD = 0,
    RFKILL_OP_DEL,
    RFKILL_OP_CHANGE,
    RFKILL_OP_CHANGE_ALL,
};

struct rfkill_event {
    uint32_t idx;
    uint8_t  type;
    uint8_t  op;
    uint8_t  soft;
    uint8_t  hard;
};

enum {
    OPT_b = (1 << 0), /* must be = 1 */
    OPT_u = (1 << 1),
    OPT_l = (1 << 2),
};

static guint watch = 0;
bool spe_bt_node = false;
bool not_hci_node = false;
bool M_adapter_flag = false;
bool M_power_on = false;


static gboolean rfkill_event(GIOChannel *chan,
                GIOCondition cond, gpointer data)
{
    unsigned char buf[32];
    struct rfkill_event *event = (struct rfkill_event *)buf;
    char sysname[PATH_MAX];
    ssize_t len;
    int fd, id;

    if (cond & (G_IO_NVAL | G_IO_HUP | G_IO_ERR))
        return FALSE;

    fd = g_io_channel_unix_get_fd(chan);

    memset(buf, 0, sizeof(buf));
    len = read(fd, buf, sizeof(buf));
    if (len < 0) {
        if (errno == EAGAIN)
            return TRUE;
        return FALSE;
    }

    if (len != sizeof(struct rfkill_event))
        return TRUE;

    qDebug("RFKILL event idx %u type %u op %u soft %u hard %u",
                    event->idx, event->type, event->op,
                        event->soft, event->hard);

    if (event->type != RFKILL_TYPE_BLUETOOTH &&
                    event->type != RFKILL_TYPE_ALL)
    {
        qDebug() << Q_FUNC_INFO << "Not bt====" ;
        return TRUE;
    }

    memset(sysname, 0, sizeof(sysname));
    snprintf(sysname, sizeof(sysname) - 1,
            "/sys/class/rfkill/rfkill%u/name", event->idx);

    fd = open(sysname, O_RDONLY);
    if (fd < 0)
    {
        qDebug () << Q_FUNC_INFO  << __LINE__;

        return TRUE;
    }

    if (read(fd, sysname, sizeof(sysname) - 1) < 4) {
        close(fd);
        qDebug () << Q_FUNC_INFO  << __LINE__;

        return TRUE;
    }

    close(fd);

    if (g_str_has_prefix(sysname, "tpacpi_bluetooth_sw") == TRUE)
    {
        spe_bt_node = true;
        qDebug () << Q_FUNC_INFO <<  "spe_bt_node" << spe_bt_node  << __LINE__;
        if (event->soft)
        {
            not_hci_node = true ;
            qDebug () << Q_FUNC_INFO <<  "event->soft:" << event->soft  << __LINE__;
        }
        else
            not_hci_node = false ;
    }
    else if (g_str_has_prefix(sysname, "hci") == TRUE)
    {
        qDebug () << Q_FUNC_INFO <<  "not_hci_node:FALSE"  << __LINE__;
        not_hci_node = false;
    }
    else
    {
        qDebug () << Q_FUNC_INFO  << "not_hci_node:TRUE" << __LINE__;
        not_hci_node = true;
    }


    return TRUE;
}
void rfkill_init(void)
{
    qDebug () << Q_FUNC_INFO << __LINE__;

    int fd;
    GIOChannel *channel;

    fd = open("/dev/rfkill", O_RDWR);
    if (fd < 0) {
        return;
    }

    channel = g_io_channel_unix_new(fd);
    g_io_channel_set_close_on_unref(channel, TRUE);

    watch = g_io_add_watch(channel,
                GIOCondition(G_IO_IN | G_IO_NVAL | G_IO_HUP | G_IO_ERR),
                rfkill_event, NULL);

    g_io_channel_unref(channel);
}

void rfkill_set_idx(void)
{
    qDebug () << Q_FUNC_INFO  << __LINE__;
    struct rfkill_event event;

    int rf_fd;
    int mode;
    int rf_type;
    int rf_idx;
    unsigned rf_opt = 0;

    /* Must have one or two params */
    mode = O_RDWR | O_NONBLOCK;

    rf_type = RFKILL_TYPE_BLUETOOTH;
    rf_idx = -1;

    rf_fd = open("/dev/rfkill", mode);

    memset(&event, 0, sizeof(event));
    if (rf_type >= 0) {
        event.type = rf_type;
        event.op = RFKILL_OP_CHANGE_ALL;
    }

    if (rf_idx >= 0) {
        event.idx = rf_idx;
        event.op = RFKILL_OP_CHANGE;
    }

    /* Note: OPT_b == 1 */
    event.soft = (rf_opt & OPT_b);

    write(rf_fd, &event, sizeof(event));

}

void rfkill_exit(void)
{
    if (watch == 0)
        return;

    g_source_remove(watch);
    watch = 0;
}


static qint64 getFileSize(QString filename)
{
    QFileInfo info(filename);
    if (info.exists())
        return info.size();
    else
        return 0;
}

FeaturesWidget::FeaturesWidget(QWidget *parent)
    : QWidget(parent)
{
    rfkill_init();
    rfkill_set_idx();

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

    obex_manager = new BluezQt::ObexManager(this);
    BluezQt::InitObexManagerJob *obex_job = obex_manager->init();
    obex_job->exec();
    qDebug() << Q_FUNC_INFO << obex_manager->isInitialized() << obex_manager->isOperational() << obex_manager->sessions().size();
    connect(obex_manager,&BluezQt::ObexManager::sessionAdded,this,&FeaturesWidget::file_transfer_session_add);

    InitAgentAndDbus();
    M_registerAgent();

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
        not_hci_node = true;
        M_adapter_flag = false;
    }else if(m_manager->adapters().size() == 1){
        m_adapter = m_manager->adapters().at(0).data();
        settings->set("adapter-address",QVariant::fromValue(m_adapter->address()));
        M_adapter_flag = true;

    }else{
        if(adapter_list.indexOf(Default_Adapter) != -1){
            m_adapter = m_manager->adapterForAddress(Default_Adapter).data();
        }else{
            m_adapter = m_manager->adapterForAddress(adapter_list.at(0)).data();
        }
        M_adapter_flag = true;

    }
    initAllTimer();

    showBluetoothTrayIcon();

    adapterChangeFUN();
    if (!not_hci_node)
        cur_adapter_address = m_adapter->address();

    qDebug() << "===========" << m_manager->isBluetoothBlocked();

    if (!not_hci_node)
        Turn_on_or_off_bluetooth(settings->get("switch").toBool());

    if(File_save_path.isEmpty()){
        settings->set("file-save-path",QVariant::fromValue(QDir::homePath()));
    }

    qDebug() << Q_FUNC_INFO << m_manager->isInitialized() << m_manager->isOperational();

    if (!not_hci_node)
        NotifyOnOff();


    if (!not_hci_node)
        Monitor_sleep_signal();

    if(!not_hci_node && !m_manager->isBluetoothBlocked() && m_adapter->isPowered()){
        QTimer::singleShot(500,this,[=]{
            Connect_the_last_connected_device();
        });
    }
    qDebug() << Q_FUNC_INFO << "end" << __LINE__;

}

FeaturesWidget::~FeaturesWidget()
{
    delete settings;
    rfkill_exit();
}

void FeaturesWidget::initAllTimer()
{
    notify_timer = new QTimer();
    connect(notify_timer, &QTimer::timeout, this, [=]()
    {
        if (nullptr == m_device)
        {
            QString text = QString(tr("The connection with the Bluetooth device is successful!"));
            SendNotifyMessage(text);
        }
        else
        {
            if (m_device->isPaired() && m_device->isConnected())
            {
                is_connected.push_back(m_device->name());
                QString text = QString(tr("The connection with the Bluetooth device “%1” is successful!").arg(m_device->name()));
                SendNotifyMessage(text);
            }
        }
    });

    callBackConnectTimer = new QTimer();
    callBackConnectTimer->setInterval(1000);//2000 ms 最好
    connect(callBackConnectTimer, &QTimer::timeout, this, [=]()
    {
        callBackConnectTimer->stop();
        Connect_the_last_connected_device();
    });

}

void FeaturesWidget::showBluetoothTrayIcon()
{
    qDebug () <<Q_FUNC_INFO << __LINE__;
    QPalette palette;
    palette.setBrush(QPalette::Base,QColor(Qt::black));
    palette.setBrush(QPalette::Active, QPalette::Text,QColor(Qt::white));
    palette.setBrush(QPalette::Disabled, QPalette::Text,QColor(Qt::darkGray));
    palette.setBrush(QPalette::WindowText, QColor(Qt::white));
    palette.setBrush(QPalette::ButtonText, QColor(Qt::white));

    tray_Menu = new QMenu(this);
//    tray_Menu = new NewMenu();
    tray_Menu->setProperty("setIconHighlightEffectDefaultColor", tray_Menu->palette().color(QPalette::Active, QPalette::Base));
    tray_Menu->setPalette(palette);
    tray_Menu->setMinimumWidth(240);
    connect(tray_Menu,&QMenu::triggered,this,&FeaturesWidget::TraySignalProcessing);

    //Create taskbar tray icon and connect to signal slot
    //创建任务栏托盘图标，并连接信号槽
    bluetooth_tray_icon = new QSystemTrayIcon(this);
    bluetooth_tray_icon->setContextMenu(tray_Menu);
    bluetooth_tray_icon->setToolTip(tr("Bluetooth"));
    bluetooth_tray_icon->setVisible(settings->get("tray-show").toBool());

    if (spe_bt_node)
    {
        setTrayIcon(not_hci_node);
    }
    else
    {
        if (m_manager->adapters().size())
        {
            setTrayIcon((nullptr != m_adapter) && (m_adapter->isPowered()));
        }
        else
        {
            bluetooth_tray_icon->setVisible(false);
        }
    }

    connect(bluetooth_tray_icon,
            &QSystemTrayIcon::activated,
            [=](QSystemTrayIcon::ActivationReason reason){
                switch (reason)
                {
                    case QSystemTrayIcon::DoubleClick: /* 来自于双击激活。 */
                    case QSystemTrayIcon::Trigger: /* 来自于单击激活。 */
                       InitTrayMenu();
                       break;
                    case QSystemTrayIcon::Context:
                        //InitTrayMenu();
                        break;
                }
            });

}


void FeaturesWidget::InitTrayMenu()
{
    qDebug() << Q_FUNC_INFO << spe_bt_node << not_hci_node << M_adapter_flag;
    tray_Menu->clear();

    bool btPower = false ;
    if (spe_bt_node)
    {
        if (not_hci_node)
            btPower = false;
        else
        {
            if (!M_adapter_flag)
                return;
            else
                btPower = m_adapter->isPowered();
        }
    }
    else
    {
        if (not_hci_node)
            return;
        else
        {
            btPower = m_adapter->isPowered();
        }
    }

    m_action = new SwitchAction(this);
    //m_action->setBtnStatus(m_adapter->isPowered());
    m_action->setBtnStatus(btPower);
    connect(m_action,&SwitchAction::sendBtnStatus,this,[=](bool value){
        Turn_on_or_off_bluetooth(value);
    });
    QWidgetAction *switch_txt = new QWidgetAction(tray_Menu);
    switch_txt->setDefaultWidget(m_action);
    tray_Menu->addAction(switch_txt);
    tray_Menu->addSeparator();

    qDebug() << Q_FUNC_INFO << flag << btPower;//m_adapter->isPowered();
    //if(m_adapter->isPowered()){
    if(btPower){
        QList<BluezQt::DevicePtr> device_list = m_adapter->devices();
        qDebug() << Q_FUNC_INFO << device_list.size();
        QAction *head = new QAction(tr("My Devices"));
        head->setDisabled(true);
        tray_Menu->addAction(head);
        bool head_rm = true;
        for(int i=0; i < device_list.size(); i++){
            if(device_list.at(i)->isPaired()){
                QPalette palette;
                head_rm = false;
                palette.setBrush(QPalette::Base,QColor(Qt::black));
                palette.setBrush(QPalette::Text,QColor(Qt::white));
                QString devname = device_list.at(i)->name();
                QFontMetrics fontMetrics(head->font());
                devname = fontMetrics.elidedText(devname, Qt::ElideRight, 150);
                QAction *device_action = new QAction(devname,tray_Menu);
                device_action->setCheckable(true);
                QMenu *device_menu = new QMenu();
                device_action->setToolTip(device_list.at(i)->name());
                device_menu->setPalette(palette);
                device_action->setObjectName(device_list.at(i)->address());

                QAction *status = new QAction(tray_Menu);
                QAction *send   = new QAction(tray_Menu);
                QAction *remove = new QAction(tray_Menu);
                if(!flag){
                    device_action->setChecked(true);
                }
                status->setStatusTip(device_list.at(i)->address());
                send->setStatusTip(device_list.at(i)->address());
                remove->setStatusTip(device_list.at(i)->address());
                if(device_list.at(i)->isConnected()){
                    device_action->setChecked(true);

                    if (device_list.at(i)->type() == BluezQt::Device::Mouse || device_list.at(i)->type() == BluezQt::Device::Keyboard) {
                        remove->setText(tr("Remove"));
                    } else {
                        status->setText(tr("Disconnection"));
                    }

#ifdef BATTERY
                    BluezQt::BatteryPtr dev_battery = device_list.at(i)->battery();
                    qDebug() << Q_FUNC_INFO << __LINE__ << dev_battery.isNull();
                    if(!dev_battery.isNull()){
                       QAction *battery = new QAction();
                       battery->setDisabled(true);
                       battery->setIcon(QIcon::fromTheme("battery-level-100-symbolic"));
                       battery->setText(tr("Power ")+QString::number(dev_battery->percentage(),10)+"%");
                       device_menu->addAction(battery);
                    }
#endif
                }else{
                    device_action->setChecked(false);

                    if(device_list.at(i)->type()==BluezQt::Device::Headset || device_list.at(i)->type()==BluezQt::Device::Headphones || device_list.at(i)->type()==BluezQt::Device::AudioVideo)
                        status->setText(tr("Connect audio"));
                    else
                        status->setText(tr("Connection"));

                    remove->setText(tr("Remove"));
                    device_menu->addAction(remove);
                }

                if (device_list.at(i)->type() == BluezQt::Device::Mouse || device_list.at(i)->type() == BluezQt::Device::Keyboard) {
                    device_menu->addAction(remove);
                } else {
                    device_menu->addAction(status);
                }

                send->setText(tr("Send files"));
                if(device_list.at(i)->type()==BluezQt::Device::Phone || device_list.at(i)->type()==BluezQt::Device::Computer)
                {
                    if (qgetenv("DESKTOP_SESSION") == "ukui")
                        device_menu->addAction(send);

                }

                qDebug() << Q_FUNC_INFO << device_menu->size();
                disconnect(device_list.at(i).data(), &BluezQt::Device::connectedChanged, device_action, nullptr);
                connect(device_list.at(i).data(),&BluezQt::Device::connectedChanged,device_action,[=](bool value){
                    qDebug() << Q_FUNC_INFO << value << __LINE__;
                    if(value) {
//                        device_menu->setIcon(QIcon::fromTheme("ukui-dialog-success"));
                        device_action->setChecked(true);


                        if (device_list.at(i)->type() == BluezQt::Device::Mouse || device_list.at(i)->type() == BluezQt::Device::Keyboard) {
                            device_menu->removeAction(status);
                        } else {
                            status->setText(tr("Disconnection"));
                            device_menu->addAction(status);
                            device_menu->removeAction(remove);
                        }

#ifdef BATTERY
                        BluezQt::BatteryPtr dev_battery = device_list.at(i)->battery();
                        if(!dev_battery.isNull()){
                           QAction *battery = new QAction();
                           battery->setDisabled(true);
                           battery->setIcon(QIcon::fromTheme("battery-level-100-symbolic"));
                           battery->setText(tr("Power ")+QString::number(dev_battery->percentage(),10)+"%");
                           device_menu->addAction(battery);
                        }
#endif
                        send->setText(tr("Send files"));
                        if(device_list.at(i)->type()==BluezQt::Device::Phone || device_list.at(i)->type()==BluezQt::Device::Computer)
                        {
                            if (qgetenv("DESKTOP_SESSION") == "ukui")
                                device_menu->addAction(send);

                        }
                    }
                    else {
                        device_action->setChecked(false);

                        if(device_list.at(i)->type()==BluezQt::Device::Headset || device_list.at(i)->type()==BluezQt::Device::Headphones || device_list.at(i)->type()==BluezQt::Device::AudioVideo)
                            status->setText(tr("Connect audio"));
                        else
                            status->setText(tr("Connection"));

                        remove->setText(tr("Remove"));
                        device_menu->clear();
                        device_menu->addAction(remove);
                        device_menu->addAction(status);

                        send->setText(tr("Send files"));
                        if(device_list.at(i)->type()==BluezQt::Device::Phone || device_list.at(i)->type()==BluezQt::Device::Computer)
                        {
                            if (qgetenv("DESKTOP_SESSION") == "ukui")
                                device_menu->addAction(send);

                        }

                        if (device_list.at(i)->type() == BluezQt::Device::Mouse || device_list.at(i)->type() == BluezQt::Device::Keyboard){
                            device_menu->removeAction(status);
                        }
                    }
                });

                tray_Menu->addAction(device_action);
                device_action->setMenu(device_menu);


    //            connect(device_menu,&QMenu::triggered,this,&FeaturesWidget::TrayItemSignalProcessing);
                }
        }
        if (head_rm) {
            tray_Menu->removeAction(head);
        }
    }
    tray_Menu->addSeparator();
    QAction *settins_action = new QAction(tr("Bluetooth settings"),tray_Menu);
    settins_action->setCheckable(true);
    tray_Menu->addAction(settins_action);

    if (!QFile("/usr/bin/ukui-panel").exists()) {
        tray_Menu->move(bluetooth_tray_icon->geometry().x()+16,bluetooth_tray_icon->geometry().y()-50);
        tray_Menu->exec();
    } else {
        setWidgetPosition();
    }
}

void FeaturesWidget::setWidgetPosition()
{
    #define MARGIN 4

    //menu->exec();
    int width = tray_Menu->sizeHint().width();
    int height= tray_Menu->sizeHint().height();
    qDebug()<<"menu 's  width  height"<<width<<height;

    QDBusInterface iface("org.ukui.panel",
                         "/panel/position",
                         "org.ukui.panel",
                         QDBusConnection::sessionBus());
    QDBusReply<QVariantList> reply=iface.call("GetPrimaryScreenGeometry");
    //reply获取的参数共5个，分别是 主屏可用区域的起点x坐标，主屏可用区域的起点y坐标，主屏可用区域的宽度，主屏可用区域高度，任务栏位置
//    reply.value();
if (!iface.isValid() || !reply.isValid() || reply.value().size()<5) {
        qCritical() << QDBusConnection::sessionBus().lastError().message();
        this->setGeometry(0,0,width,height);
    }


//    qDebug()<<reply.value().at(4).toInt();
    QVariantList position_list=reply.value();

    switch(reply.value().at(4).toInt()){
    case 1:
        //任务栏位于上方
        tray_Menu->setGeometry(position_list.at(0).toInt()+position_list.at(2).toInt()-width-MARGIN,
                          position_list.at(1).toInt()+MARGIN,
                          width,height);
        break;
        //任务栏位于左边
    case 2:
        tray_Menu->setGeometry(position_list.at(0).toInt()+MARGIN,
                          position_list.at(1).toInt()+reply.value().at(3).toInt()-height-MARGIN,
                          width,height);
        break;
        //任务栏位于右边
    case 3:
        tray_Menu->setGeometry(position_list.at(0).toInt()+position_list.at(2).toInt()-width-MARGIN,
                          position_list.at(1).toInt()+reply.value().at(3).toInt()-height-MARGIN,
                          width,height);
        break;
        //任务栏位于下方
    default:
        tray_Menu->setGeometry(position_list.at(0).toInt()+position_list.at(2).toInt()-width-MARGIN,
                          position_list.at(1).toInt()+reply.value().at(3).toInt()-height-MARGIN,
                          width,height);
        break;
    }

    tray_Menu->show();
}

void FeaturesWidget::InitAgentAndDbus()
{
    session_dbus = new BluetoothDbus(this);
    connect(session_dbus,&BluetoothDbus::ConnectTheSendingDevice,this,&FeaturesWidget::Pair_device_by_address);
    connect(session_dbus,&BluetoothDbus::DisconnectTheSendingDevice,this,&FeaturesWidget::Disconnect_device_by_address);
    connect(session_dbus,&BluetoothDbus::RemoveTheSendingDevice,this,&FeaturesWidget::Remove_device_by_address);
    connect(session_dbus,&BluetoothDbus::sendTransferMesg,this,&FeaturesWidget::Dbus_file_transfer);
    connect(session_dbus,&BluetoothDbus::switch_signals,this,&FeaturesWidget::Dbus_bluetooth_switch);

    bluetoothAgent = new BluetoothAgent(this);
    connect(bluetoothAgent, &BluetoothAgent::agentRemoveDevice, this, &FeaturesWidget::Remove_device_by_devicePtr);    connect(bluetoothAgent, &BluetoothAgent::startNotifyTimer, this, &FeaturesWidget::Start_notify_timer);
    connect(bluetoothAgent, &BluetoothAgent::stopNotifyTimer, this, &FeaturesWidget::Stop_notify_timer);

    bluetoothObexAgent = new BluetoothObexAgent(this);
}

void FeaturesWidget::Start_notify_timer(BluezQt::DevicePtr) {
    notify_timer->setSingleShot(true);
    notify_timer->start(2000);
}

void FeaturesWidget::Stop_notify_timer(BluezQt::DevicePtr) {
    notify_timer->stop();
}

void FeaturesWidget::showWarnningMesgBox()
{
    QMessageBox *mbox = new QMessageBox(QMessageBox::Warning,tr("Warning"),tr("The selected file is empty, please select the file again !"),QMessageBox::Ok);
    QDesktopWidget *desktop = QApplication::desktop();
    mbox->move((desktop->width()-mbox->width())/2,(desktop->height()-mbox->height())/2);
    mbox->exec();
}

void FeaturesWidget::Pair_device_by_address(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    if(device.isNull())
        return;

    if(device->isPaired()){
        if(device->type()==BluezQt::Device::Headset || device->type()==BluezQt::Device::Headphones || device->type()==BluezQt::Device::AudioVideo){
            Connect_device_audio(address);
        }else{
            Connect_device_by_address(address);
        }
    }else{
        qDebug() << Q_FUNC_INFO << device->name();
        BluezQt::PendingCall *call = device->pair();
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
            qDebug() << Q_FUNC_INFO << __LINE__ <<q->error() << q->errorText();
            if(q->error() == 0){
                //emit device.data()->pairedChanged(true);
                if(device->type()==BluezQt::Device::Headset || device->type()==BluezQt::Device::Headphones || device->type()==BluezQt::Device::AudioVideo){
                    Connect_device_audio(address);
                }else{
                    Connect_device_by_address(address);
                }
            }
            else {
                qDebug() << Q_FUNC_INFO << q->error();
                emit device.data()->pairedChanged(false);
            }
        });
    }
}

void FeaturesWidget::Disconnect_device_by_address(QString address)
{
    qDebug() << Q_FUNC_INFO << address;
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    if(device.isNull())
        return;

    BluezQt::PendingCall *call = device->disconnectFromDevice();
    connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
        if(q->error() == 0){
            qDebug() << Q_FUNC_INFO;
//            QString text = QString(tr("Bluetooth device “%1” disconnected successfully").arg(device->name()));
//            SendNotifyMessage(text);
        }else{
            qDebug() << Q_FUNC_INFO;
//            QString text = tr("Disconnect Error!!!");
//            SendNotifyMessage(text);
        }
    });
}

void FeaturesWidget::Remove_device_by_address(QString address)
{
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    Remove_device_by_devicePtr(device);
}

void FeaturesWidget::Connect_device_by_address(QString address)
{
    qDebug() << Q_FUNC_INFO << (m_adapter == nullptr);
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    if(device.isNull()){
        qDebug() << Q_FUNC_INFO << "The connected device does not exist !";
        return;
    }
    qDebug() << Q_FUNC_INFO << device->uuids();

    BluezQt::DevicePtr finally_device = m_adapter->deviceForAddress(settings->get("finally-connect-the-device").toString());
    if(finally_device.isNull() || address == settings->get("finally-connect-the-device").toString()){
        qDebug() << Q_FUNC_INFO << finally_device.isNull() << address << settings->get("finally-connect-the-device").toString();
        Connect_device(device);
    }else
    {
        if (device->type()==BluezQt::Device::Mouse)
        {
            Connect_device(device);
        }
        else
        {
            if(finally_device->isConnected()){
                   BluezQt::PendingCall *call = device->disconnectFromDevice();
                   connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
                        if(q->error() == 0){
                            Connect_device(device);
                        }
                   });
            }
            else
            {
                Connect_device(device);
            }
        }
    }
}

void FeaturesWidget::Connect_device_audio(QString address)
{
    qDebug() << Q_FUNC_INFO << (m_adapter == nullptr);
    BluezQt::DevicePtr device = m_adapter->deviceForAddress(address);
    if(device.isNull()){
        qDebug() << Q_FUNC_INFO << "The connected device does not exist !";
        return;
    }
    qDebug() << Q_FUNC_INFO << device->uuids();

    QList<BluezQt::DevicePtr> devlist = m_adapter->devices();
    foreach (BluezQt::DevicePtr dev, devlist) {
        if (dev->address() == address)
            continue;

        if (dev->isConnected()) {
            if (dev->type()==BluezQt::Device::Headset || dev->type()==BluezQt::Device::Headphones || dev->type()==BluezQt::Device::AudioVideo) {
                BluezQt::PendingCall *call = dev->disconnectFromDevice();
                connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
                     if(q->error() == 0){
                         Connect_device(device);
                     }
                });
                return;
            }
        }
    }
    Connect_device(device);
}

void FeaturesWidget::Connect_device(BluezQt::DevicePtr device)
{
    if (!device->isPaired())
    {
        BluezQt::PendingCall *pair_call = device->pair();
        connect(pair_call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *pair_q){

            if(pair_q->error() == 0)
            {
                qDebug() << Q_FUNC_INFO << "pair successful" << __LINE__;
                writeDeviceInfoToFile(device->address(),device->name(),device->type());
            }
            else
                qDebug() << Q_FUNC_INFO << "pair failure" << __LINE__;

            device->setTrusted(true);

            BluezQt::PendingCall *conn_call = device->connectToDevice();
            connect(conn_call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *conn_q){
                if(conn_q->error() == 0)
                {
                    qDebug() << Q_FUNC_INFO << "connect successful" << __LINE__;
                    settings->set("finally-connect-the-device",QVariant::fromValue(device->address()));
                    writeDeviceInfoToFile(device->address(),device->name(),device->type());
                }
                else
                {
                    qDebug() << Q_FUNC_INFO << "connect failure" << conn_q->errorText() << __LINE__;
                }
            });
        });
    }
    else
    {
        BluezQt::PendingCall *call = device->connectToDevice();
        qDebug() << Q_FUNC_INFO << device->adapter()->name();
        device->setTrusted(true);
        qDebug() << Q_FUNC_INFO << call->error() << __LINE__;
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
            if(q->error() == 0){
                qDebug() << Q_FUNC_INFO << __LINE__;
                 QTimer::singleShot(500, [=]{
                     emit device->connectedChanged(true);
                 });
        //            QString text = QString(tr("The connection with the Bluetooth device “%1” is successful!").arg(device->name()));
        //            SendNotifyMessage(text);
                settings->set("finally-connect-the-device",QVariant::fromValue(device->address()));

                writeDeviceInfoToFile(device->address(),device->name(),device->type());
            }else{
                qDebug() << Q_FUNC_INFO << q->errorText();
        //            QString text = QString(tr("The connection with the Bluetooth device “%1” is failed!").arg(device->name()));
        //            SendNotifyMessage(text);
            }
        });
    }
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
    QDBusMessage msg = iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
    qDebug() << Q_FUNC_INFO << msg.errorMessage();
}

void FeaturesWidget::NotifyOnOff()
{
    QList<BluezQt::DevicePtr> device_list = m_adapter->devices();
    qDebug() << Q_FUNC_INFO << device_list.size();

    for(int i=0; i < device_list.size(); i++){
        if(device_list.at(i)->isPaired()){
            connect(device_list.at(i).data(),&BluezQt::Device::connectedChanged,this,[=](bool value){
                qDebug() << Q_FUNC_INFO << "connectedChanged";
                if(value) {
                    m_device = device_list.at(i);
                    notify_timer->setSingleShot(true);
                    notify_timer->start(2000);
                }
                else {
                    if (!is_connected.contains(device_list.at(i)->name()))
                        return;
                    QString text = QString(tr("Bluetooth device “%1” disconnected!").arg(device_list.at(i)->name()));
                    SendNotifyMessage(text);
                    is_connected.removeOne(device_list.at(i)->name());
                }
            });
        }
    }

    connect(m_adapter, &BluezQt::Adapter::deviceAdded, this, [=](BluezQt::DevicePtr dev) {
        qDebug() << Q_FUNC_INFO << dev.data()->name();
        connect(dev.data(),&BluezQt::Device::pairedChanged,this,[=](bool value){
            qDebug() << Q_FUNC_INFO << "pairedChanged" << value;
            if(value && pair_flag) {
                pair_flag = false;
                QTimer::singleShot(500,this,[=]{
                    pair_flag = true;
                });
                m_device = dev;
                notify_timer->setSingleShot(true);
                notify_timer->start(2000);

            }
        });

        connect(dev.data(),&BluezQt::Device::connectedChanged,this,[=](bool value){
            qDebug() << Q_FUNC_INFO << "connectedChanged" << value;
            if (dev.data()->isPaired()) {
                if(value) {
                    m_device = dev;
                    notify_timer->setSingleShot(true);
                    notify_timer->start(2000);

                }
                else {
                    if (!is_connected.contains(dev->name()))
                        return;
                    QString text = QString(tr("Bluetooth device “%1” disconnected!").arg(dev->name()));
                    SendNotifyMessage(text);
                    is_connected.removeOne(dev->name());
                }
            }
        });
    });
}

void FeaturesWidget::Send_files_by_address(QString address)
{
    selected_file = nullptr;
    if(!obex_manager->isOperational()){
        BluezQt::PendingCall *call = obex_manager->startService();
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
            QTimer::singleShot(1000,this,[=]{
                obex_manager->registerAgent(bluetoothObexAgent)->errorText();
            });
        });
    }

    QFileDialog fd;

    QList<QUrl> list = fd.sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        //华为990、9a0需要屏蔽最小系统挂载的目录
        if (fi.fileName() == "2691-6AB8")
             continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(&fd);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, &fd,
    [=, &sidebarNum, &mntUrlList, &list, &fd](const QString path) {
        QDir wmntDir(path);
        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList wfilist = wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
                       QFileInfo fi = wfilist.at(i);
                 //华为990、9a0需要屏蔽最小系统挂载的目录
                  if (fi.fileName() == "2691-6AB8")
                       continue;
                 mntUrlList << QUrl("file://" + fi.filePath());
             }
             fd.setSidebarUrls(list + mntUrlList);
             fd.update();
        });

        connect(&fd, &QFileDialog::finished, &fd, [=, &list, &fd]() {
            fd.setSidebarUrls(list);
        });

    //自己QFileDialog的用法，这里只是列子
    fd.setNameFilter(QLatin1String("All Files (*)"));

    fd.setDirectory(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0));

    //这行要添加，设置左侧导航
    fd.setSidebarUrls(list + mntUrlList);
    if (fd.exec() == QDialog::Accepted) {
        qDebug()<<fd.selectedFiles()<<fd.filter()<<fd.selectedNameFilter() << fd.selectedUrls();
        selected_file = fd.selectedUrls().first().path();
    }

    if(!selected_file.isNull()){

        if (getFileSize(selected_file) == 0) {
            showWarnningMesgBox();
            return;
        }

        if (BluetoothFileTransferWidget::isShow == false) {
            transfer_widget = new BluetoothFileTransferWidget(selected_file,address);
            connect(transfer_widget,&BluetoothFileTransferWidget::sender_dev_name,this,&FeaturesWidget::file_transfer_creator);
            connect(transfer_widget,&BluetoothFileTransferWidget::close_the_pre_session,this,&FeaturesWidget::close_session);
            transfer_widget->show();
        }
        else
        {
            qDebug() << Q_FUNC_INFO << transfer_widget->get_send_data_state() << __LINE__;

            if (BluetoothFileTransferWidget::_SEND_FAILURE == transfer_widget->get_send_data_state()   ||
                BluetoothFileTransferWidget::_SEND_COMPLETE == transfer_widget->get_send_data_state()  )
            {
                emit transfer_widget->close_the_pre_session();
                transfer_widget->close();
                delete transfer_widget;
                transfer_widget = new BluetoothFileTransferWidget(selected_file,address);
                connect(transfer_widget,&BluetoothFileTransferWidget::sender_dev_name,this,&FeaturesWidget::file_transfer_creator);
                connect(transfer_widget,&BluetoothFileTransferWidget::close_the_pre_session,this,&FeaturesWidget::close_session);
                transfer_widget->show();
            }
//            QMessageBox::warning(NULL, tr("bluetooth"), tr("A transfer is in progress..."),
//                                     QMessageBox::Ok,QMessageBox::Ok);
        }
    }
}

void FeaturesWidget::Turn_on_or_off_bluetooth(bool f)
{
    qDebug() << Q_FUNC_INFO << f;

    if(f)
    {
        if (spe_bt_node)
        {
            M_power_on = true;
            if (not_hci_node)
            {
                rfkill_set_idx();
            }
        }


        qDebug() << Q_FUNC_INFO
                 << "spe_bt_node:"  << spe_bt_node
                 << " not_hci_node" << not_hci_node;


        if (!not_hci_node)
        {
            if (m_manager->isBluetoothBlocked())
                m_manager->setBluetoothBlocked(false);
            BluezQt::PendingCall *call = m_adapter->setPowered(true);
            connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *p){
                if(p->error() == 0){
                    flag = true;
                    qDebug() << Q_FUNC_INFO<< "Set turn on Bluetooth: " << m_adapter->isPowered();
                }else
                    qDebug() << "Failed to turn on Bluetooth:" << p->errorText();
            });
        }
    }else{
        if (spe_bt_node)
        {
            M_power_on = false;
        }
        BluezQt::PendingCall *call = m_adapter->setPowered(false);
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *p){
            if(p->error() == 0){
                flag = false;
                qDebug() << Q_FUNC_INFO <<"Set turn off Bluetooth: " << m_adapter->isPowered();
                m_manager->setBluetoothBlocked(true);
            }else
                qDebug() << "Failed to turn off Bluetooth:" << p->errorText();
        });
    }
}

void FeaturesWidget::Dbus_file_transfer(QStringList file_path)
{
    if(!obex_manager->isOperational()){
        BluezQt::PendingCall *call = obex_manager->startService();
        connect(call,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
            QTimer::singleShot(1000,this,[=]{
                obex_manager->registerAgent(bluetoothObexAgent)->errorText();
            });
        });
    }

    qDebug() << "Select file:" << file_path;
    selected_file = QUrl(file_path.first()).path();
    if(!selected_file.isNull()){

        if (getFileSize(selected_file) == 0) {
            showWarnningMesgBox();
            return;
        }

        if (BluetoothFileTransferWidget::isShow == false) {
            transfer_widget = new BluetoothFileTransferWidget(selected_file,"");
            connect(transfer_widget,&BluetoothFileTransferWidget::sender_dev_name,this,&FeaturesWidget::file_transfer_creator);
            connect(transfer_widget,&BluetoothFileTransferWidget::close_the_pre_session,this,&FeaturesWidget::close_session);
            transfer_widget->show();
        }
        else
        {

            if (BluetoothFileTransferWidget::_SEND_FAILURE == transfer_widget->get_send_data_state()   ||
                BluetoothFileTransferWidget::_SEND_COMPLETE == transfer_widget->get_send_data_state()  )
            {
                emit transfer_widget->close_the_pre_session();
                transfer_widget->close();
                delete transfer_widget;
                transfer_widget = new BluetoothFileTransferWidget(selected_file,"");
                connect(transfer_widget,&BluetoothFileTransferWidget::sender_dev_name,this,&FeaturesWidget::file_transfer_creator);
                connect(transfer_widget,&BluetoothFileTransferWidget::close_the_pre_session,this,&FeaturesWidget::close_session);
                transfer_widget->show();
            }
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

void FeaturesWidget::M_registerAgent()
{
    qDebug() << m_manager->registerAgent(bluetoothAgent)->errorText();

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
}

bool FeaturesWidget::Connect_device_name_white_list(QString dev_name)
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    if (("Mi Air 2S" == dev_name)

       )
        return true;
    return false;
}

void FeaturesWidget::Connect_the_last_connected_device()
{
    qDebug() << Q_FUNC_INFO << "Connect Back to start";
    QList<BluezQt::DevicePtr> devlist = m_adapter->devices();
    foreach (BluezQt::DevicePtr devptr, devlist) {
        BluezQt::Device* dev = devptr.data();
        if (dev->isPaired() && !dev->isConnected()
                && (dev->type() == BluezQt::Device::Keyboard || dev->type() == BluezQt::Device::Mouse)) {
            connect(dev, &BluezQt::Device::rssiChanged, this, [=] (qint16 value) {
                qDebug() << Q_FUNC_INFO << "rssiChanged" << dev->name() << value;

                BluezQt::PendingCall *pp = dev->connectToDevice();
                connect(pp,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *call){
                    if(call->error() == 0)
                    {
                        emit dev->connectedChanged(true);
                        writeDeviceInfoToFile(dev->address(),dev->name(),dev->type());
                    }
                    else
                    {
                        qDebug() << Q_FUNC_INFO << "rssiChanged" << dev->name() << call->errorText();
                    }
                });

                disconnect(dev, &BluezQt::Device::rssiChanged, nullptr, nullptr);
            });
        }
    }
    QStringList target_list = getDeviceConnectTimeList();
    qDebug() << Q_FUNC_INFO << "getDeviceConnectTimeList" << target_list.size();
    foreach (QString dev_address, target_list) {
        BluezQt::Device* dev = m_adapter->deviceForAddress(dev_address).data();
        if (dev != nullptr) {
            qDebug() << Q_FUNC_INFO << dev->type() << dev->name();
            if (dev->isPaired() && !dev->isConnected() &&
                    (dev->type() == BluezQt::Device::Headset ||
                     dev->type() == BluezQt::Device::Headphones ||
                     dev->type() == BluezQt::Device::AudioVideo ||
                     Connect_device_name_white_list(dev->name()))) {
                qDebug() << Q_FUNC_INFO << dev->type() << dev->name();

                BluezQt::PendingCall *pp = dev->connectToDevice();
                connect(pp,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *call){
                    if(call->error() == 0){
                        writeDeviceInfoToFile(dev->address(),dev->name(),dev->type());
                        QTimer::singleShot(500, [=]{
                            emit dev->connectedChanged(true);
                        });
                    }else{
                        qDebug() << Q_FUNC_INFO << "AudioVideo" << dev->name() << call->errorText();
                    }
                });

                break;
            }
        }
    }

    QTimer::singleShot(10000, [=] {
        foreach (BluezQt::DevicePtr devptr, devlist) {
            BluezQt::Device* dev = devptr.data();
            if (dev->isPaired() && !dev->isConnected()
                    && (dev->type() == BluezQt::Device::Keyboard || dev->type() == BluezQt::Device::Mouse)) {
                disconnect(dev, &BluezQt::Device::rssiChanged, nullptr, nullptr);
            }
        }
        qDebug() << Q_FUNC_INFO << "rssiChanged Connecting Back to the end";
    });
    qDebug() << Q_FUNC_INFO << "Connecting Back to the end";

}

// ===================蓝牙管理器设备操作监听======（主要针对系统的休眠和增加新的蓝牙硬件）=======================
void FeaturesWidget::adapterChangeFUN()
{
    qDebug() << Q_FUNC_INFO << __LINE__;

    if (!not_hci_node)
    {
        connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);
        connect(m_adapter,&BluezQt::Adapter::deviceRemoved,this,&FeaturesWidget::adapterDeviceRemove);
    }

    connect(m_manager,&BluezQt::Manager::adapterRemoved,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO << adapter->address() << adapter->name() <<__LINE__;
//        if(adapter_list.size() == 1)
//        {
//            QMessageBox::warning(NULL,tr("bluetooth"),tr("Bluetooth Adapter Abnormal!!!"),QMessageBox::Yes,QMessageBox::Yes);
//        }

        adapter_list.removeAll(adapter->address());
        settings->set("adapter-address-list",QVariant::fromValue(adapter_list));

        if(adapter_list.size() == 0){
            not_hci_node = true;
            M_adapter_flag = false;
            qDebug() << Q_FUNC_INFO << spe_bt_node << not_hci_node << __LINE__;
            if (!spe_bt_node && not_hci_node)
            {
                //非休眠状态下提示蓝牙适配器
                if (!sleep_flag)
                {
                    QString text = QString(tr("no bluetooth adapter!"));
                    SendNotifyMessage(text);
                }

                if(bluetooth_tray_icon->isVisible())
                {
                    bluetooth_tray_icon->setVisible(false);
                }
            }
        }else{
            QTimer::singleShot(200,this,[=]{
                if(!sleep_flag){
                    m_adapter = m_manager->adapterForAddress(adapter_list.at(0)).data();
//                    if(!m_adapter->isDiscoverable()){
//                        m_adapter->setDiscoverable(true);
//                        m_adapter->setDiscoverableTimeout(0);
//                    }
                    connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);
                }
            });
        }
    });

    connect(m_manager,&BluezQt::Manager::adapterAdded,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO << adapter->address() << adapter->name() << bluetooth_tray_icon->isVisible() << adapter.data()->isPowered() <<__LINE__;
        adapter_list.append(adapter->address());
        not_hci_node = false;

        if (!bluetooth_tray_icon->isVisible())
            bluetooth_tray_icon->setVisible(true);

        settings->set("adapter-address-list",QVariant::fromValue(adapter_list));
        m_adapter = adapter.data();
        connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);
        M_adapter_flag = true ;
        if (spe_bt_node && M_power_on)
        {
            Turn_on_or_off_bluetooth(true);
        }
        NotifyOnOff();

    });

    connect(m_manager,&BluezQt::Manager::adapterChanged,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO << "adapterChanged" <<__LINE__;

        if (m_adapter->address() != adapter.data()->address())
        {
            m_adapter = adapter.data();
            connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);
        }
    });

    connect(m_manager,&BluezQt::Manager::usableAdapterChanged,this,[=](BluezQt::AdapterPtr adapter){
        qDebug() << Q_FUNC_INFO << "usableAdapterChanged" <<__LINE__;
    });

    connect(m_manager,&BluezQt::Manager::operationalChanged,this,[=](bool status){
        qDebug() <<  Q_FUNC_INFO << "operationalChanged " << status << __LINE__;
        setTrayIcon(status);
        if (status) {
            NotifyOnOff();
            M_registerAgent();
        }
    });
}
// ===========================================END========================================================

void FeaturesWidget::writeDeviceInfoToFile(const QString &devAddress, const QString &devName, const BluezQt::Device::Type type)
{
    QDBusMessage m = QDBusMessage::createMethodCall("com.bluetooth.systemdbus", "/", "com.bluetooth.interface", "writeKeyFile");
    m << devAddress << devName << type;
    QDBusMessage response = QDBusConnection::systemBus().call(m);
    qDebug() << Q_FUNC_INFO << devAddress << devName << type << response.errorMessage();
}

void FeaturesWidget::removeDeviceInfoToFile(const QString &devAddress)
{
    QDBusMessage m = QDBusMessage::createMethodCall("com.bluetooth.systemdbus", "/", "com.bluetooth.interface", "removeKeyFile");
    m << devAddress;
    QDBusMessage response = QDBusConnection::systemBus().call(m);
    qDebug() << Q_FUNC_INFO << devAddress << response.errorMessage();
}

void FeaturesWidget::setTrayIcon(bool v)
{
    if (v) {
        bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-active-symbolic"));
    } else {
        if(QIcon::hasThemeIcon("bluetooth-error"))
            bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-error"));
        else
            bluetooth_tray_icon->setIcon(QIcon::fromTheme("bluetooth-active-symbolic"));
    }
}

QStringList FeaturesWidget::getDeviceConnectTimeList()
{
    QStringList pair_device_list;
    pair_device_list.clear();
    if(!QFile::exists(LIST_PATH))
        return pair_device_list;

    QVector<int> pair_device_time_list;
    pair_device_time_list.clear();
    GKeyFile *key_file = nullptr;
    key_file = g_key_file_new();
    g_key_file_load_from_file(key_file,QString(LIST_PATH).toStdString().c_str(),G_KEY_FILE_NONE,NULL);
    gchar **list;
    bool ok;
    list  = g_key_file_get_groups(key_file,NULL);
    for(int i = 0; list[i] != NULL; i++){
        pair_device_list.append(QString::fromUtf8(list[i]));
        pair_device_time_list.append(QString::fromUtf8(g_key_file_get_string(key_file,list[i],"ConnectTime",NULL)).toInt(&ok,10));
    }

    //*****************给从配置文件中得到的设备列表和设备对应的最后连接时间列表排序***********START**************
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
    //************************************  END  ******************************************************

    g_key_file_free(key_file);

    return pair_device_list;
}

void FeaturesWidget::Monitor_sleep_Slot(bool sleep)
{
    qDebug () << __FUNCTION__ << "app is sleep: " << sleep << __LINE__;

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
//                if(!m_adapter->isDiscoverable()){
//                    m_adapter->setDiscoverable(true);
//                    m_adapter->setDiscoverableTimeout(0);
//                }
            }else{

            }

            if (m_manager->adapters().size())
            {
                connect(m_adapter,&BluezQt::Adapter::poweredChanged,this,&FeaturesWidget::adapterPoweredChanged);
                dev_callbak_flag = 0;
                callBackConnectTimer->start(1000);
                //Connect_the_last_connected_device();
            }
        });

    }else{
        sleep_flag = true;
        qDebug() << "System goes to sleep !!!" << dev_remove_flag;
    }
    qDebug() << Q_FUNC_INFO << "end";

}

void FeaturesWidget::adapterPoweredChanged(bool value)
{
    qDebug() << Q_FUNC_INFO << value;
    settings->set("switch",QVariant::fromValue(value));
    flag = value;
    setTrayIcon(value);
    if (value == true) {//开启
        callBackConnectTimer->start(1000);
        //Connect_the_last_connected_device();
    }
    qDebug() << Q_FUNC_INFO << "end";
}

void FeaturesWidget::Remove_device_by_devicePtr(BluezQt::DevicePtr ptr)
{
    qDebug() << Q_FUNC_INFO;
    BluezQt::PendingCall *call = m_adapter->removeDevice(ptr);
    connect(call, &BluezQt::PendingCall::finished, this, [=](BluezQt::PendingCall *q) {
        if (q->error() == 0)
            removeDeviceInfoToFile(ptr.data()->address());
        else
            qDebug() << Q_FUNC_INFO << "Agent Device Remove Failed!!!";
    });
    qDebug() << Q_FUNC_INFO << "end";
}

void FeaturesWidget::adapterDeviceRemove(BluezQt::DevicePtr ptr)
{
    qDebug() << Q_FUNC_INFO << ptr.data()->address() << ptr.data()->name();
    removeDeviceInfoToFile(ptr.data()->address());
    qDebug() << Q_FUNC_INFO << "end";
}

void FeaturesWidget::TraySignalProcessing(QAction *action)
{
    qDebug() << Q_FUNC_INFO << action->text() ;
    if(action->text() == tr("Bluetooth settings")){
        Open_bluetooth_settings();
    }else if(action->text() == tr("Disconnection")){
        Disconnect_device_by_address(action->statusTip());
    }else if(action->text() == tr("Connection")){
        Connect_device_by_address(action->statusTip());
    }else if(action->text() == tr("Connect audio")){
        Connect_device_audio(action->statusTip());
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
        BluezQt::PendingCall *transfer = opp->sendFile(selected_file);
        qDebug() << Q_FUNC_INFO << transfer->error() << transfer->errorText();
        connect(transfer,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *call){
            qDebug() << Q_FUNC_INFO << __LINE__;
            QVariant v = call->value();
            filePtr = v.value<BluezQt::ObexTransferPtr>();
            if(filePtr){
                transfer_file_size = filePtr->size();
                qDebug() << Q_FUNC_INFO << "Sending file size:" << transfer_file_size << __LINE__;

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
        if (nullptr != filePtr)
        {
            qDebug() << Q_FUNC_INFO << "filePtr" <<__LINE__;
            BluezQt::PendingCall * stopFileSend = filePtr->cancel();
            connect(stopFileSend,&BluezQt::PendingCall::finished,this,[=](BluezQt::PendingCall *q){
                qDebug() << Q_FUNC_INFO  << q->errorText() << __LINE__;
                obex_manager->removeSession(pre_session);
            });
        }
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

//        if(!m_adapter->isDiscoverable()){
//            m_adapter->setDiscoverable(true);
//            m_adapter->setDiscoverableTimeout(0);
//        }
        if(m_adapter->isPowered())
            flag = true;
        else
            flag = false;
    }
}

//org.ukui.bluetooth dbus蓝牙开关接口
void FeaturesWidget::Dbus_bluetooth_switch(bool value)
{
    qDebug() << Q_FUNC_INFO << value << settings->get("switch").toBool();

    if(value != settings->get("switch").toBool()){
        Turn_on_or_off_bluetooth(value);
    }
}

