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

private slots:
    void itemToolbuttonClicked();

private:
    BluezQt::Manager *m_manager;
    QString select_dev;
    QString select_name = "";

    QList<QToolButton*> toolbutton_list;
    QToolButton *btn = nullptr;
    bool flag = false;

    QLabel *Tiptop;
    QScrollArea *m_scroll;
    QWidget *dev_widget;
    QVBoxLayout *device_list_layout;
};

#endif // DEVICESELETERWIDGET_H
