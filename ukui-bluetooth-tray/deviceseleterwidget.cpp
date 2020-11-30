#include "deviceseleterwidget.h"

DeviceSeleterWidget::DeviceSeleterWidget(QWidget *parent, QString dev, bool f):
    QWidget(parent),
    select_dev(dev)
{
    this->resize(372,270);
    QPalette palette;
    palette.setColor(QPalette::Background,QColor(235,235,235));
    this->setPalette(palette);

    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *job = m_manager->init();
    job->exec();

    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(5);
    main_layout->setContentsMargins(0,0,0,0);

    if(f){
        QLabel *Tiptop = new QLabel(tr("Select equipment"),this);
        Tiptop->setFixedSize(200,25);
        Tiptop->setStyleSheet("QLabel{\
                             font-size: 18px;\
                             font-family: PingFangSC-Medium, PingFang SC;\
                             font-weight: 500;\
                             color: rgba(0, 0, 0, 0.85);\
                             line-height: 25px;}");
        main_layout->addWidget(Tiptop);
    }

    m_scroll = new QScrollArea(this);
    m_scroll->setWidgetResizable(true);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    m_scroll->setFixedSize(376,240);
    main_layout->addWidget(m_scroll);

    dev_widget = new QWidget();
    m_scroll->setWidget(dev_widget);

    device_list_layout = new QVBoxLayout(dev_widget);
    device_list_layout->setSpacing(5);
    device_list_layout->setContentsMargins(8,8,8,8);
    dev_widget->setLayout(device_list_layout);

    InitUI();
}

DeviceSeleterWidget::~DeviceSeleterWidget()
{

}

void DeviceSeleterWidget::InitUI()
{
    QList<BluezQt::DevicePtr> device_list = m_manager->usableAdapter()->devices();
    qDebug() << Q_FUNC_INFO << __LINE__ << device_list.size();
    for(int i=0; i < device_list.size(); i++){
//        qDebug() << Q_FUNC_INFO << device_list.at(i)->type() << device_list.at(i)->name();
        if((device_list.at(i)->type() == BluezQt::Device::Phone)||(device_list.at(i)->type() == BluezQt::Device::Computer)){
            if(device_list.at(i)->isPaired()){
                QIcon icon;
                switch (device_list.at(i)->type()){
                case BluezQt::Device::Type::Phone:
                    icon = QIcon::fromTheme("phone-apple-iphone-symbolic");
                    break;
                case BluezQt::Device::Type::Computer:
                    icon = QIcon::fromTheme("video-display-symbolic");
                    break;
                case BluezQt::Device::Type::Uncategorized:
                default:
                    icon = QIcon::fromTheme("bluetooth-symbolic");
                    break;
                }

                QToolButton *item = new QToolButton(dev_widget);
                item->setFixedSize(this->width()-16,40);
                item->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
                item->setStatusTip(device_list.at(i)->address());
                item->setIcon(icon);
                item->setProperty("useIconHighlightEffect", 0x8);
                item->setText("  "+device_list.at(i)->name());

                if(select_dev != ""){
                    if(device_list.at(i)->address() == select_dev){
                        item->setStyleSheet("QToolButton{background:white;}");
                        item->setChecked(true);
                        btn = item;
                        select_name = device_list.at(i)->name();
                    }
                }

                connect(item,&QToolButton::clicked,this,&DeviceSeleterWidget::itemToolbuttonClicked);
                toolbutton_list.append(item);
                device_list_layout->addWidget(item,Qt::AlignTop);
    //            qDebug() << Q_FUNC_INFO << dev_widget->width() <<device_list.at(i)->name();
            }
        }
    }
    device_list_layout->addStretch();
}

QString DeviceSeleterWidget::get_seleter_device()
{
    qDebug() << Q_FUNC_INFO;
//    for(int i = 0; i < toolbutton_list.size(); i++){
//        toolbutton_list.at(i)->setChecked(false);
//        qDebug() << Q_FUNC_INFO << __LINE__ << toolbutton_list.at(i)->isChecked();
//        if(toolbutton_list.at(i)->isChecked()){
//            qDebug() << Q_FUNC_INFO <<toolbutton_list.at(i)->statusTip();
//            return toolbutton_list.at(i)->statusTip();
//        }
//    }
    return select_dev;
}

QString DeviceSeleterWidget::get_seleter_dev_name()
{
    return select_name;
}

void DeviceSeleterWidget::itemToolbuttonClicked()
{

    if(btn != nullptr || flag){
        btn->setStyleSheet("QToolButton{background:#D9D9D9;}");
        btn->setChecked(false);
    }

    QToolButton *p = qobject_cast<QToolButton *>(sender());
    btn = p;
    p->setStyleSheet("QToolButton{background:white;}");
    p->setChecked(true);
    select_dev = p->statusTip();
    select_name = p->text();

    if(flag == false)
        flag = true;
}
