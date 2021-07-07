#ifndef DEVICESELETERWIDGET_H
#define DEVICESELETERWIDGET_H

#include <KF5/BluezQt/bluezqt/adapter.h>
#include <KF5/BluezQt/bluezqt/manager.h>
#include <KF5/BluezQt/bluezqt/initmanagerjob.h>
#include <KF5/BluezQt/bluezqt/device.h>
#include <KF5/BluezQt/bluezqt/agent.h>
#include <KF5/BluezQt/bluezqt/pendingcall.h>
#include <KF5/BluezQt/bluezqt/obexmanager.h>
#include <KF5/BluezQt/bluezqt/initobexmanagerjob.h>

#include <QWidget>
#include <QString>
#include <QScrollArea>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QToolButton>
#include <QDebug>
#include <QList>
#include <QPalette>
#include <QColor>
#include <QGSettings>

class DeviceSeleterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceSeleterWidget(QWidget *parent = nullptr, QString dev = "", bool f = true);
    ~DeviceSeleterWidget();
    void InitUI();
    QString get_seleter_device();
    QString get_seleter_dev_name();
signals:
    void sign_select();

private slots:
    void itemToolbuttonClicked();
    void GSettingsChanges(const QString &key);
private:
    QGSettings *GSettings = nullptr;
    QGSettings *settings  = nullptr;
private:
    BluezQt::Manager *m_manager = nullptr;
    QString select_dev;
    QString select_name = "";
    bool    flag = false;

    QList<QToolButton*> toolbutton_list;
    QToolButton *btn                = nullptr;
    QLabel      *Tiptop             = nullptr;
    QScrollArea *m_scroll           = nullptr;
    QWidget     *dev_widget         = nullptr;
    QVBoxLayout *device_list_layout = nullptr;
};

#endif // DEVICESELETERWIDGET_H
