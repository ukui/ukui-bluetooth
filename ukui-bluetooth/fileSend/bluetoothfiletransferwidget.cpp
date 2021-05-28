#include "bluetoothfiletransferwidget.h"
#include "../config/xatom-helper.h"

bool BluetoothFileTransferWidget::isShow = false;
BluetoothFileTransferWidget::BluetoothFileTransferWidget(QUrl name, QString dev_address):
//    QWidget(parent),
    file_name(name)
{
    isShow = true;

    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        GSettings = new QGSettings("org.ukui.style");
        connect(GSettings,&QGSettings::changed,this,&BluetoothFileTransferWidget::GSettingsChanges);
    }

    // =================添加窗管协议================================
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    // ===========================================================

    qDebug() << Q_FUNC_INFO << __LINE__;
    this->setFixedSize(440,498);
//    this->setWindowFlags(Qt::Dialog/*|Qt::FramelessWindowHint*/);
    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    this->setWindowTitle(tr("Bluetooth file transfer"));
    this->setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    if(GSettings->get("style-name").toString() == "ukui-default"){
        palette.setColor(QPalette::Background,QColor(Qt::white));
    }else{
        palette.setColor(QPalette::Background,QColor(Qt::black));
    }
    this->setPalette(palette);


    main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(8,4,4,0);

    QHBoxLayout *title_layout = new QHBoxLayout();
    title_layout->setSpacing(8);
    title_layout->setContentsMargins(0,0,0,0);

    title_icon = new QLabel(this);
    title_icon->setPixmap(QIcon::fromTheme("preferences-system-bluetooth").pixmap(20,20));
    title_icon->resize(20,20);
    title_layout->addWidget(title_icon);

    title_text = new QLabel(tr("Bluetooth file transfer"),this);
    title_text->resize(200,30);
    title_layout->addWidget(title_text);
    title_layout->addStretch(1);

    close_btn = new QPushButton(this);
    close_btn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    close_btn->setProperty("isWindowButton", 0x2);
    close_btn->setProperty("useIconHighlightEffect", 0x8);
    close_btn->setFlat(true);
    close_btn->setFixedSize(30,30);
    connect(close_btn,&QPushButton::clicked,this,[=]{
        emit this->close_the_pre_session();
        this->close();
    });
    title_layout->addWidget(close_btn,1,Qt::AlignRight);
    main_layout->addLayout(title_layout);
    main_layout->addStretch(10);

    tip_text = new QLabel(tr("Transferring to \"")+dev_name+"\"",this);
    tip_text->setStyleSheet("QLabel{\
                            font-size: 18px;\
                            font-family: PingFangSC-Medium, PingFang SC;\
                            font-weight: 500;\
                            color: rgba(0, 0, 0, 0.85);\
                            line-height: 25px;}");
    tip_text->setGeometry(32,58,400,25);
    tip_text->setVisible(false);

    Get_fie_type();

    target_frame = new QFrame(this);
    target_frame->setGeometry(32,42,376,70);
//    target_frame->setStyleSheet("background:red;");
    QHBoxLayout *target_layout = new QHBoxLayout(target_frame);
    target_layout->setSpacing(10);
    target_layout->setContentsMargins(0,0,0,0);
    QVBoxLayout *info_layout = new QVBoxLayout();
    info_layout->setSpacing(0);
    info_layout->setContentsMargins(0,0,0,0);

    target_icon = new QLabel(target_frame);
    target_icon->setPixmap(file_icon.pixmap(40,40));
    target_icon->resize(60,40);
    target_icon->setAlignment(Qt::AlignCenter);
    target_layout->addWidget(target_icon);

    target_size = new QLabel(file_size,target_frame);
    target_size->setAlignment(Qt::AlignTop);
    target_size->resize(200,20);

    QFontMetrics fontMetrics(target_size->font());
    QString fileName = fontMetrics.elidedText(file_name.path().split("/").at(file_name.path().split("/").length()-1), Qt::ElideMiddle, 280);
    target_name = new QLabel(fileName,target_frame);
    target_name->setAlignment(Qt::AlignBottom);
    target_name->setToolTip(file_name.path());
    target_name->resize(290,35);

    info_layout->addStretch();
    info_layout->addWidget(target_name,Qt::AlignCenter);
    info_layout->addWidget(target_size,Qt::AlignCenter);
    info_layout->addStretch();
    target_layout->addLayout(info_layout);
    target_layout->addStretch(1);

    m_progressbar = new QProgressBar(this);
    m_progressbar->setOrientation(Qt::Horizontal);
    m_progressbar->setGeometry(32,175,376,10);
    m_progressbar->setVisible(false);

    dev_widget = new DeviceSeleterWidget(this,dev_address);
    dev_widget->resize(376,270);
    dev_widget->setGeometry(32,125,376,270);

    ok_btn = new QPushButton(tr("OK"),this);
    ok_btn->setFixedSize(120,36);
    ok_btn->setGeometry(288,435,120,36);
    if(dev_widget->get_seleter_device() == ""){
        ok_btn->setEnabled(false);
    }
    connect(dev_widget, &DeviceSeleterWidget::sign_select, this, [=] {
        ok_btn->setEnabled(true);
    });
    connect(ok_btn,&QPushButton::clicked,this,&BluetoothFileTransferWidget::onClicked_OK_Btn);

    cancel_btn = new QPushButton(tr("Cancel"),this);
    cancel_btn->setFixedSize(120,36);
    cancel_btn->setGeometry(288,221,120,36);
    cancel_btn->setVisible(false);
    connect(cancel_btn,&QPushButton::clicked,this,[=]{
        emit this->close_the_pre_session();
        this->close();
    });

    tranfer_status_icon = new QLabel(this);
    tranfer_status_icon->setFixedSize(64,64);
    tranfer_status_icon->setAlignment(Qt::AlignHCenter);
    tranfer_status_icon->setGeometry(188,77,64,64);
    tranfer_status_icon->setVisible(false);

    tranfer_status_text = new QLabel(this);
    tranfer_status_text->setGeometry(32,149,376,20);
    tranfer_status_text->setAlignment(Qt::AlignCenter);
    tranfer_status_text->setVisible(false);
    tranfer_status_text->setStyleSheet("QLabel{\
                                       font-size: 14px;\
                                       font-family: PingFangSC-Regular, PingFang SC;\
                                       font-weight: 400;}");
}

BluetoothFileTransferWidget::~BluetoothFileTransferWidget()
{
    isShow = false;
}

void BluetoothFileTransferWidget::Get_fie_type()
{
    GError *error;
    qDebug() << Q_FUNC_INFO << file_name.path();
    GFile *file = g_file_new_for_path(file_name.path().toStdString().c_str());
    GFileInfo *file_info = g_file_query_info(file,"*",G_FILE_QUERY_INFO_NONE,NULL,&error);
    qDebug() << Q_FUNC_INFO  << g_file_info_get_size(file_info) << g_file_info_get_content_type(file_info);

    file_size = g_format_size_full(g_file_info_get_size(file_info),G_FORMAT_SIZE_IEC_UNITS);

    QString str = g_file_info_get_content_type(file_info);
    if (str.split("/").at(0) == "image"){
        file_icon = QIcon(file_name.path());
        if (file_icon.isNull()) {
            file_icon = QIcon::fromTheme("folder-documents-symbolic");
        }
    }else if (str.split("/").at(0) == "video")
        file_icon = QIcon::fromTheme("video-x-generic-symbolic");
    else if (str.split("/").at(0) == "audio" ||
             str.split("/").at(0) == "application" ||
             str.split("/").at(0) == "text"){
        if (QIcon::hasThemeIcon(str.split("/").join("-"))) {
            file_icon = QIcon::fromTheme(str.split("/").join("-"));
        } else {
            file_icon = QIcon::fromTheme("folder-documents-symbolic");
        }
    }else
        file_icon = QIcon::fromTheme("folder-documents-symbolic");
}

void BluetoothFileTransferWidget::Initialize_and_start_animation()
{
    this->resize(440,510);
    tip_text->setText(tr("Transferring to \"")+dev_widget->get_seleter_dev_name()+"\"");
    tip_text->update();

    ok_btn->setVisible(false);

    main_animation_group = new QParallelAnimationGroup(this);

    QPropertyAnimation *this_action = new QPropertyAnimation(this,"geometry");
    this_action->setDuration(300);
    this_action->setStartValue(this->geometry());
    QRect this_rect = this->geometry();
    this_rect.setHeight(300);
    this_action->setEndValue(this_rect);

    QPropertyAnimation *dev_widget_action = new QPropertyAnimation(dev_widget,"geometry");
    dev_widget_action->setDuration(100);
    dev_widget_action->setStartValue(dev_widget->geometry());
    QRect dev_widget_rect = dev_widget->geometry();
    dev_widget_rect.setHeight(0);
    qDebug() << Q_FUNC_INFO << dev_widget_rect;
    dev_widget_action->setEndValue(dev_widget_rect);
    connect(dev_widget_action,&QPropertyAnimation::finished,this,[=]{
        cancel_btn->setVisible(true);
        m_progressbar->setVisible(true);
    });

    QPropertyAnimation *target_frame_action = new QPropertyAnimation(target_frame,"pos");
    target_frame_action->setDuration(300);
    target_frame_action->setStartValue(target_frame->geometry().topLeft());
    target_frame_action->setEndValue(QPoint(32,102));
    connect(target_frame_action,&QPropertyAnimation::finished,this,[=]{
        tip_text->setVisible(true);
    });

    main_animation_group->addAnimation(this_action);
    main_animation_group->addAnimation(dev_widget_action);
    main_animation_group->addAnimation(target_frame_action);
    main_animation_group->start();
    connect(main_animation_group,&QParallelAnimationGroup::finished,this,[=]{
        this->setFixedSize(440,300);
    });
}

void BluetoothFileTransferWidget::init_m_progressbar_value(quint64 value)
{
    qDebug() << Q_FUNC_INFO << value;
    m_progressbar->setMinimum(0);
    m_progressbar->setMaximum(value);
    m_progressbar->setValue(0);
}

void BluetoothFileTransferWidget::get_transfer_status(QString status)
{
    if(status == "complete"){
        if(main_animation_group->state() == QAbstractAnimation::Running){
            connect(main_animation_group,&QParallelAnimationGroup::finished,this,[=]{
                tip_text->setVisible(false);
                target_frame->setVisible(false);
                m_progressbar->setVisible(false);
            });
        }
        tip_text->setVisible(false);
        target_frame->setVisible(false);
        m_progressbar->setVisible(false);

        tranfer_status_icon->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(64,64));
        tranfer_status_icon->setVisible(true);
        tranfer_status_text->setText(tr("Successfully transmitted!"));
        tranfer_status_text->setVisible(true);

        cancel_btn->setText(tr("Close"));
    }else if(status == "active"){

    }else if(status == "error"){
        tranfer_error();
    }
}

void BluetoothFileTransferWidget::tranfer_error()
{
    if(main_animation_group->state() == QAbstractAnimation::Running){
        connect(main_animation_group,&QParallelAnimationGroup::finished,this,[=]{
            tip_text->setVisible(false);
            target_frame->setVisible(false);
            m_progressbar->setVisible(false);
        });
    }
    tip_text->setVisible(false);
    target_frame->setVisible(false);
    m_progressbar->setVisible(false);

    tranfer_status_icon->setPixmap(QIcon::fromTheme("dialog-error").pixmap(64,64));
    tranfer_status_icon->setVisible(true);
    tranfer_status_text->setText(tr("Transmission failed!"));
    tranfer_status_text->setVisible(true);

    cancel_btn->setText(tr("Close"));
    emit this->close_the_pre_session();
}

void BluetoothFileTransferWidget::set_m_progressbar_value(quint64 value)
{
    if(--active_flag <= 0)
        m_progressbar->setValue(value);
}

void BluetoothFileTransferWidget::GSettingsChanges(const QString &key)
{
    qDebug() << Q_FUNC_INFO << key;
    if(key == "styleName"){
        QPalette palette;
        if(GSettings->get("style-name").toString() == "ukui-default"){
            palette.setColor(QPalette::Background,QColor(Qt::white));
            target_icon->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::black));
            target_icon->setProperty("useIconHighlightEffect", 0x10);
        }else{
            palette.setColor(QPalette::Background,QColor(Qt::black));
            target_icon->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
            target_icon->setProperty("useIconHighlightEffect", 0x10);
        }
        this->setPalette(palette);
    }
}

void BluetoothFileTransferWidget::onClicked_OK_Btn()
{
//    qDebug() << Q_FUNC_INFO << this->rect() << this->geometry() << dev_widget->geometry() << ok_btn->geometry() << target_frame->geometry();

    qDebug() << Q_FUNC_INFO;
    qDebug() << Q_FUNC_INFO << dev_widget->get_seleter_device();
    if(dev_widget->get_seleter_device() == ""){

    }else{
        Initialize_and_start_animation();
        emit this->sender_dev_name(dev_widget->get_seleter_device());
    }
}
