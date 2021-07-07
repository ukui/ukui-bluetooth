#ifndef FILERECEIVINGPOPUPWIDGET_H
#define FILERECEIVINGPOPUPWIDGET_H

#include <gio/gio.h>
#include <gio/gfile.h>
#include <gio/gioerror.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <string>

#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/manager.h>
#include <KF5/BluezQt/bluezqt/initmanagerjob.h>
#include <KF5/BluezQt/bluezqt/obextransfer.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>
#include <QDialog>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QPalette>
#include <QString>
#include <QRect>
#include <QPoint>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QFile>
#include <QProcess>
#include <QGSettings>
#include <QHBoxLayout>

class FileReceivingPopupWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FileReceivingPopupWidget(QString address = "", QString source = "", QString root = "");
    ~FileReceivingPopupWidget();
    QString getDeviceNameByAddress(QString);
    void configuration_transfer_progress_bar(quint64);
    void window_pop_up_animation();
    bool move_file();
public slots:
    void OnClickedAcceptBtn();
    void update_transfer_progress_bar(quint64);
    void file_transfer_completed(BluezQt::ObexTransfer::Status status);
    void GSettings_value_chanage(const QString &key);
    void GSettingsChanges(const QString &key);
signals:
    void cancel();
    void rejected();
    void accepted();
private:
    QGSettings   *StyleSettings = nullptr;
    QGSettings   *settings      = nullptr;

    QPushButton  *close_btn     = nullptr;
    QPushButton  *cancel_btn    = nullptr;
    QPushButton  *accept_btn    = nullptr;
    QPushButton  *view_btn      = nullptr;

    QLabel       *icon_label    = nullptr;
    QLabel       *file_source   = nullptr;
    QLabel       *file_name     = nullptr;
    QLabel       *file_icon     = nullptr;
    QProgressBar *transfer_progress = nullptr;

    QRect   desktop;
    QString target_address;
    QString target_name;
    QString target_source;
    QString root_address;
    QString file_path;
};

#endif // FILERECEIVINGPOPUPWIDGET_H
