#include "pincodewidget.h"
#include "xatom-helper.h"

PinCodeWidget::PinCodeWidget(QString name, QString pin)
   : dev_name(name),
     PINCode(pin)
{
    if(QGSettings::isSchemaInstalled("org.ukui.style")){
        settings = new QGSettings("org.ukui.style");
        connect(settings,&QGSettings::changed,this,&PinCodeWidget::GSettingsChanges);
    }

    // 添加窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

//    this->setWindowFlags(Qt::Dialog/*|Qt::FramelessWindowHint*/);
    this->setFixedSize(420,330);
    this->setWindowIcon(QIcon::fromTheme("bluetooth"));
    this->setWindowTitle(tr("Bluetooth pairing"));
    this->setAttribute(Qt::WA_DeleteOnClose);

    QPalette palette;
    if(settings->get("style-name").toString() == "ukui-default"){
        palette.setColor(QPalette::Background,QColor(Qt::white));
    }else{
        palette.setColor(QPalette::Background,QColor(Qt::black));
    }
    this->setPalette(palette);

    QString top_text = tr("Is it paired with:");
    QString tip_text = tr("Please make sure the number displayed on \"")+dev_name+tr("\" matches the number below. Please do not enter this code on any other accessories.");
    QLabel *top_label = new QLabel(top_text,this);
    top_label->setStyleSheet("QLabel{\
                             font-size: 18px;\
                             font-family: PingFangSC-Medium, PingFang SC;\
                             font-weight: 500;\
                             line-height: 25px;}");
    top_label->setGeometry(32,48,334,25);

    QLabel *tip_label = new QLabel(tip_text,this);
    tip_label->setStyleSheet("QLabel{\
                             font-size: 14px;\
                             font-family: PingFangSC-Regular, PingFang SC;\
                             font-weight: 400;\
                             line-height: 20px;}");
    tip_label->setGeometry(32,89,359,60);
    tip_label->setWordWrap(true);

    QLabel *PIN_label = new QLabel(PINCode,this);
    PIN_label->setStyleSheet("QLabel{\
                             font-size: 36px;\
                             font-family: ArialMT;\
                             line-height: 42px;}");
    PIN_label->setGeometry(151,166,160,40);

    accept_btn = new QPushButton(tr("Accept"),this);
//    accept_btn->setStyleSheet("QPushButton{\
//                              background: #E7E7E7;\
//                              border-radius: 6px;\
//                              border: 0px solid #979797;}\
//                              QPushButton:hover{background:rgba(67,127,240,1);\
//                              border-radius:6px;color:white;border: 0px solid #979797;}");
    accept_btn->setGeometry(160,255,120,36);
    connect(accept_btn,&QPushButton::clicked,this,&PinCodeWidget::onClick_accept_btn);

    refuse_btn = new QPushButton(tr("Refush"),this);
//    refuse_btn->setStyleSheet("QPushButton{\
//                              background: #E7E7E7;\
//                              border-radius: 6px;\
//                              border: 0px solid #979797;}\
//                              QPushButton:hover{background:rgba(67,127,240,1);\
//                              border-radius:6px;color:white;border: 0px solid #979797;}");
    refuse_btn->setGeometry(290,255,120,36);
    connect(refuse_btn,&QPushButton::clicked,this,&PinCodeWidget::onClick_refuse_btn);

    close_btn = new QPushButton(this);
    QIcon icon = QIcon::fromTheme("window-close-symbolic");
    close_btn->setIcon(icon);
    close_btn->setProperty("isWindowButton", 0x2);
    close_btn->setProperty("useIconHighlightEffect", 0x8);
//    close_btn->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    close_btn->setFlat(true);
    close_btn->setGeometry(386,4,30,30);
    connect(close_btn,&QPushButton::clicked,this,&PinCodeWidget::onClick_close_btn);
}

PinCodeWidget::~PinCodeWidget()
{

}

void PinCodeWidget::Connection_timed_out()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(tr("Pair"));
    msgBox.setText(tr("Connection timed out !!!"));
    int ret = msgBox.exec();
    if(ret){
        this->close();
    }
}

void PinCodeWidget::onClick_close_btn(bool)
{
    emit this->rejected();
    this->close();
}

void PinCodeWidget::onClick_accept_btn(bool)
{
    emit this->accepted();
    this->close();
}

void PinCodeWidget::onClick_refuse_btn(bool)
{
    emit this->rejected();
    this->close();
}

void PinCodeWidget::GSettingsChanges(const QString &key)
{
    QPalette palette;
    qDebug() << Q_FUNC_INFO << key;
    if(key == "styleName"){
        if(settings->get("style-name").toString() == "ukui-default"){
            palette.setColor(QPalette::Background,QColor(Qt::white));
        }else{
            palette.setColor(QPalette::Background,QColor(Qt::black));
        }
    }
    this->setPalette(palette);
}
