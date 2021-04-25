#include "filereceivingpopupwidget.h"
#include "../config/xatom-helper.h"

FileReceivingPopupWidget::FileReceivingPopupWidget(QString address, QString source, QString root):
    target_address(address),
    target_source(source),
    root_address(root)
{
    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        StyleSettings = new QGSettings("org.ukui.style");
        connect(StyleSettings,&QGSettings::changed,this,&FileReceivingPopupWidget::GSettingsChanges);
    }

    // 添加窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

//    this->setWindowFlags(Qt::Dialog/*|Qt::FramelessWindowHint*/);
    this->setFixedSize(440,250);
    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    this->setWindowTitle(tr("Bluetooth file transfer"));
    this->setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    if(StyleSettings->get("style-name").toString() == "ukui-default"){
        palette.setColor(QPalette::Background,QColor(Qt::white));
    }else{
        palette.setColor(QPalette::Background,QColor(Qt::black));
    }
    this->setPalette(palette);

    if(QGSettings::isSchemaInstalled("org.ukui.bluetooth")){
        settings = new QGSettings("org.ukui.bluetooth");

        file_path = settings->get("file-save-path").toString();

        connect(settings, &QGSettings::changed,this,&FileReceivingPopupWidget::GSettings_value_chanage);
    }else{
        file_path = QDir::homePath();
    }

    QDesktopWidget *desktop_widget = QApplication::desktop();
    desktop = desktop_widget->availableGeometry();
    qDebug() << Q_FUNC_INFO << this->width() << this->height();
    qDebug() << Q_FUNC_INFO <<desktop <<desktop.right() << desktop.bottom() << desktop.right()-this->width() << desktop.bottom()-this->height();
    this->move(QPoint(desktop.right()-this->width(),desktop.bottom()-this->height()));

    window_pop_up_animation();

    close_btn = new QPushButton(this);
    close_btn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    close_btn->setGeometry(406,4,30,30);
    close_btn->setProperty("isWindowButton", 0x2);
    close_btn->setProperty("useIconHighlightEffect", 0x8);
    close_btn->setFlat(true);
    connect(close_btn,&QPushButton::clicked,this,[=]{
        emit this->rejected();
        this->close();
    });

    icon_label = new QLabel(this);
    icon_label->setPixmap(QIcon::fromTheme("preferences-system-bluetooth").pixmap(20,20));
    icon_label->setGeometry(28,51,20,20);

    target_name = getDeviceNameByAddress(target_address);
    file_source = new QLabel(tr("Bluetooth file transfer from \"")+target_name+"\"",this);
    file_source->setGeometry(56,35,350,50);
    file_source->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    file_source->setWordWrap(true);
    file_source->setStyleSheet("QLabel{\
                               font-size: 18px;\
                               font-family: PingFangSC-Medium, PingFang SC;\
                               font-weight: 500;\
                               line-height: 25px;}");

    file_icon = new QLabel(this);
    file_icon->setPixmap(QIcon::fromTheme("ukui-folder-documents-symbolic").pixmap(42,42));
    file_icon->setAlignment(Qt::AlignCenter);
    file_icon->setGeometry(28,95,65,42);

    QFontMetrics fontMetrics(file_source->font());
    QString fileName = fontMetrics.elidedText(target_source, Qt::ElideMiddle, 280);
    file_name = new QLabel(fileName,this);
    file_name->setToolTip(target_source);
    file_name->setGeometry(101,87,293,60);
    file_name->setAlignment(Qt::AlignVCenter|Qt::AlignLeft);
    file_name->setWordWrap(true);

    transfer_progress = new QProgressBar(this);
    transfer_progress->setGeometry(29,147,376,6);
    transfer_progress->setVisible(false);

    cancel_btn = new QPushButton(tr("Cancel"),this);
    cancel_btn->setGeometry(152,177,120,36);
    connect(cancel_btn,&QPushButton::clicked,this,[=]{
        emit this->rejected();
        this->close();
    });

    accept_btn = new QPushButton(tr("Accept"),this);
    accept_btn->setGeometry(288,177,120,36);
    connect(accept_btn,&QPushButton::clicked,this,[=]{
        OnClickedAcceptBtn();
    });

    view_btn = new QPushButton(tr("View"),this);
    view_btn->setGeometry(288,177,120,36);
    view_btn->setVisible(false);
}

FileReceivingPopupWidget::~FileReceivingPopupWidget()
{
    delete settings;
}

QString FileReceivingPopupWidget::getDeviceNameByAddress(QString address)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    BluezQt::Manager *manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *job = manager->init();
    job->exec();
    BluezQt::AdapterPtr adapter = manager->adapterForAddress(root_address);
//    BluezQt::AdapterPtr adapter = manager->usableAdapter();
    QString name = adapter->deviceForAddress(address)->name();
    qDebug() << Q_FUNC_INFO << __LINE__;
    return name;
}

void FileReceivingPopupWidget::configuration_transfer_progress_bar(quint64 value)
{
    transfer_progress->setMinimum(0);
    transfer_progress->setMaximum(value);
    transfer_progress->setValue(0);
}

void FileReceivingPopupWidget::window_pop_up_animation()
{
    qDebug() << Q_FUNC_INFO << desktop << desktop.right() << desktop.bottom() <<this->geometry();

//    QPropertyAnimation *window_action = new QPropertyAnimation(this,"geometry");
//    window_action->setDuration(100);
//    QRect this_rect = this->rect();
//    this_rect.setHeight(0);
//    this_rect.setWidth(0);
//    window_action->setStartValue(QRect(desktop.right()-1,desktop.bottom()-this->height(),desktop.right(),desktop.bottom()));
//    window_action->setEndValue(QRect(desktop.right()-this->width(),desktop.bottom()-this->height(),desktop.right(),desktop.bottom()));
//    window_action->setStartValue(QPoint(desktop.right()-1,desktop.bottom()-this->height()));
//    window_action->setEndValue(QPoint(desktop.right()-this->width(),desktop.bottom()-this->height()));
//    qDebug() << Q_FUNC_INFO << this_rect << this->geometry() << QRect(desktop.right()-1,desktop.bottom()-this->height(),desktop.right(),desktop.bottom()) <<QRect(desktop.right()-this->width(),desktop.bottom()-this->height(),desktop.right(),desktop.bottom());
    //    window_action->start();
}

bool FileReceivingPopupWidget::move_file()
{
    QString s = QDir::homePath()+"/.cache/obexd/"+target_source;
    QString d = file_path+"/"+target_source;
    if(!QFile::exists(file_path)){
        int status;
        status = mkdir(file_path.toStdString().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        qDebug() << Q_FUNC_INFO << status;
        if(status == -1)
            return -1;
    }

    GError *error;
    GFile *source = g_file_new_for_path(s.toStdString().c_str());
    GFile *destination = g_file_new_for_path(d.toStdString().c_str());
    bool flag = g_file_move(source,destination,G_FILE_COPY_OVERWRITE,NULL,NULL,NULL,&error);
    qDebug() << Q_FUNC_INFO << "move file" << "target_path =" << s << " source_path =" << d << "flag =" << flag;
    return flag;
}

void FileReceivingPopupWidget::OnClickedAcceptBtn()
{
    transfer_progress->setVisible(true);
    accept_btn->setVisible(false);

    QParallelAnimationGroup *actions = new QParallelAnimationGroup(this);

    QPropertyAnimation *progress_action = new QPropertyAnimation(transfer_progress,"geometry");
    QRect transfer_progress_rect = transfer_progress->geometry();
    transfer_progress_rect.setWidth(0);
    progress_action->setStartValue(transfer_progress_rect);
    progress_action->setEndValue(transfer_progress->geometry());
    progress_action->setDuration(200);

    QPropertyAnimation *cancel_btn_action = new QPropertyAnimation(cancel_btn,"pos");
    cancel_btn_action->setStartValue(cancel_btn->geometry().topLeft());
    cancel_btn_action->setEndValue(QPoint(288,177));
    cancel_btn_action->setDuration(100);

    actions->addAnimation(progress_action);
    actions->addAnimation(cancel_btn_action);
    actions->start();
    connect(actions,&QParallelAnimationGroup::finished,this,[=]{
        this->accepted();
    });
}

void FileReceivingPopupWidget::update_transfer_progress_bar(quint64 value)
{
    transfer_progress->setValue(value);
    transfer_progress->repaint();
}

void FileReceivingPopupWidget::file_transfer_completed(BluezQt::ObexTransfer::Status status)
{
    qDebug() << Q_FUNC_INFO <<status ;
    if(status == BluezQt::ObexTransfer::Active){
        cancel_btn->disconnect();
        cancel_btn->connect(cancel_btn,&QPushButton::clicked,this,[=]{
            emit this->cancel();
            this->close();
        });
        close_btn->disconnect();
        close_btn->connect(close_btn,&QPushButton::clicked,this,[=]{
            emit this->cancel();
            this->close();
        });
    }else if(status == BluezQt::ObexTransfer::Complete){
        close_btn->disconnect();
        close_btn->connect(close_btn,&QPushButton::clicked,this,[=]{
            this->close();
        });
        cancel_btn->setVisible(false);

//        bool x= QFile::rename(QDir::homePath()+"/.cache/obexd/"+target_source,file_path+"/"+target_source);
        bool x = move_file();
//        view_btn->setVisible(true);
        qDebug() << Q_FUNC_INFO << __LINE__ << x;
        if(x){
//            connect(view_btn,&QPushButton::clicked,this,[=]{
                QProcess *process = new QProcess(this);
                QString cmd = "peony";
                QStringList arg;
                qDebug() << Q_FUNC_INFO;
                arg <<  "--show-items" <<file_path+"/"+target_source;
                process->startDetached(cmd,arg);
                this->close();
//            });
        }

    }else if(status == BluezQt::ObexTransfer::Error){
        close_btn->disconnect();
        close_btn->connect(close_btn,&QPushButton::clicked,this,[=]{
            this->close();
        });
        cancel_btn->setVisible(false);

        QFrame *warn_frame = new QFrame(this);
        warn_frame->setGeometry(0,176,440,30);
        QHBoxLayout *warn_layout = new QHBoxLayout(warn_frame);
        warn_layout->setSpacing(10);
        warn_layout->setContentsMargins(0,0,0,0);

        QLabel *warn_icon = new QLabel(this);
        warn_icon->setPixmap(QIcon::fromTheme("emblem-important-symbolic").pixmap(30,30));
        warn_icon->setProperty("setIconHighlightEffectDefaultColor", QColor(248, 206, 83));
        warn_icon->setProperty("useIconHighlightEffect", 0x10);

        QLabel *warn_text = new QLabel(tr("Sender canceled or transmission error"),this);
        warn_text->setAlignment(Qt::AlignVCenter);

        warn_layout->addStretch();
        warn_layout->addWidget(warn_icon);
        warn_layout->addWidget(warn_text);
        warn_layout->addStretch();
        warn_frame->show();
    }
}

void FileReceivingPopupWidget::GSettings_value_chanage(const QString &key)
{
    if(key == "file-save-path"){
        file_path = settings->get("file-save-path").toString();
    }
}

void FileReceivingPopupWidget::GSettingsChanges(const QString &key)
{
    qDebug() << Q_FUNC_INFO << key;
    if(key == "styleName"){
        QPalette palette;
        if(StyleSettings->get("style-name").toString() == "ukui-default"){
            palette.setColor(QPalette::Background,QColor(Qt::white));
        }else{
            palette.setColor(QPalette::Background,QColor(Qt::black));
        }
        this->setPalette(palette);
    }
}
