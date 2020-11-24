#include "filereceivingpopupwidget.h"

FileReceivingPopupWidget::FileReceivingPopupWidget(QString address, QString source):
    target_address(address),
    target_source(source)
{
    this->setWindowFlags(Qt::Dialog|Qt::FramelessWindowHint);
    this->resize(440,250);

    QDesktopWidget *desktop_widget = QApplication::desktop();
    desktop = desktop_widget->availableGeometry();
    qDebug() << Q_FUNC_INFO << this->width() << this->height();
    qDebug() << Q_FUNC_INFO <<desktop <<desktop.right() << desktop.bottom() << desktop.right()-this->width() << desktop.bottom()-this->height();
    this->move(QPoint(desktop.right()-this->width(),desktop.bottom()-this->height()));

    window_pop_up_animation();

    close_btn = new QPushButton(this);
    close_btn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    close_btn->setIconSize(QSize(15,15));
    close_btn->setGeometry(413,14,14,14);
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
                               color: rgba(0, 0, 0, 0.85);\
                               line-height: 25px;}");

    file_icon = new QLabel(this);
    file_icon->setPixmap(QIcon::fromTheme("ukui-folder-documents-symbolic").pixmap(42,42));
    file_icon->setAlignment(Qt::AlignCenter);
    file_icon->setGeometry(28,95,65,42);

    file_name = new QLabel(target_source,this);
    file_name->setToolTip(target_source);
    file_name->setGeometry(101,97,293,40);
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

}

QString FileReceivingPopupWidget::getDeviceNameByAddress(QString address)
{
    qDebug() << Q_FUNC_INFO << __LINE__;
    BluezQt::Manager *manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *job = manager->init();
    job->exec();
    BluezQt::AdapterPtr adapter = manager->usableAdapter();
    QString name = adapter->deviceForAddress(address)->name();
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
        view_btn->setVisible(true);
    }else if(status == BluezQt::ObexTransfer::Error){
        close_btn->disconnect();
        close_btn->connect(close_btn,&QPushButton::clicked,this,[=]{
            this->close();
        });
        cancel_btn->setVisible(false);

        QLabel *warn_icon = new QLabel(this);
        warn_icon->setPixmap(QIcon::fromTheme("emblem-important-symbolic").pixmap(30,30));
        warn_icon->setGeometry(60,176,30,30);
        warn_icon->setProperty("setIconHighlightEffectDefaultColor", QColor(248, 206, 83));
        warn_icon->setProperty("useIconHighlightEffect", 0x10);
        warn_icon->show();

        QLabel *warn_text = new QLabel(tr("Sender canceled or transmission error"),this);
        warn_text->setGeometry(100,176,300,30);
        warn_text->show();
    }
}
