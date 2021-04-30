#ifndef BLUETOOTHFILETRANSFERWIDGET_H
#define BLUETOOTHFILETRANSFERWIDGET_H

#include <gio/gio.h>
#include <gio/gfile.h>
#include <gio/gfileinfo.h>
#include <gio/gioerror.h>

#include <string>

#include <KF5/BluezQt/bluezqt/obexmanager.h>
#include <KF5/BluezQt/bluezqt/initobexmanagerjob.h>
#include <KF5/BluezQt/bluezqt/obexobjectpush.h>
#include <KF5/BluezQt/bluezqt/obexfiletransfer.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include "deviceseleterwidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QByteArray>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QScrollArea>
#include <QString>
#include <QFont>
#include <QDebug>
#include <QUrl>
#include <QDialog>
#include <QFileInfo>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QProgressBar>
#include <QGSettings>
#include <QFontMetrics>

class DeviceSeleterWidget;

enum FILE_TYPE{
    Normal,
    mp3,
    mp4,
    image,
};

class BluetoothFileTransferWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BluetoothFileTransferWidget(QUrl name,QString dev_address = "");
    ~BluetoothFileTransferWidget();

    static bool isShow;

    void Get_fie_type();
    void Get_file_size(float);
    void Initialize_and_start_animation();
    void init_m_progressbar_value(quint64);
    void get_transfer_status(QString);
    void tranfer_error();
signals:
    void sender_dev_name(QString);
    void close_the_pre_session();
public slots:
    void onClicked_OK_Btn();
    void set_m_progressbar_value(quint64);
    void GSettingsChanges(const QString &key);
private:
    QGSettings *GSettings;

    QVBoxLayout *main_layout = nullptr;

    QLabel *tip_text;

    QLabel *title_icon;
    QLabel *title_text;

    QFrame *target_frame;
    QLabel *target_icon;
    QLabel *target_name;
    QLabel *target_size;

    QLabel *tranfer_status_icon;
    QLabel *tranfer_status_text;

    QIcon file_icon;

    int active_flag = 2;

    DeviceSeleterWidget *dev_widget;
    QParallelAnimationGroup *main_animation_group;
    QProgressBar *m_progressbar;

    QPushButton *close_btn;
    QPushButton *cancel_btn;
    QPushButton *ok_btn;

    QUrl file_name;
    QString file_size;
    QString dev_name;
};

#endif // BLUETOOTHFILETRANSFERWIDGET_H
