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
    explicit BluetoothFileTransferWidget(QString name,QString dev_address = "");
    ~BluetoothFileTransferWidget();

    typedef enum _SEND_DATA_STATE
    {
        _SEND_NONE = 0,
        _SEND_COMPLETE,
        _SENDING,
        _SEND_FAILURE

    }SEND_DATA_STATE;

    static bool isShow;

    void Get_fie_type();
    void Get_file_size(float);
    void Initialize_and_start_animation();
    void init_m_progressbar_value(quint64);
    void get_transfer_status(QString);
    void tranfer_error();

    int  get_send_data_state();

signals:
    void sender_dev_name(QString);
    void close_the_pre_session();
public slots:
    void onClicked_OK_Btn();
    void set_m_progressbar_value(quint64);
    void GSettingsChanges(const QString &key);
private:
    QGSettings *GSettings     = nullptr;
    QVBoxLayout *main_layout  = nullptr;
    QLabel      *tip_text     = nullptr;
    QLabel      *title_icon   = nullptr;
    QLabel      *title_text   = nullptr;
    QFrame      *target_frame = nullptr;
    QLabel      *target_icon  = nullptr;
    QLabel      *target_name  = nullptr;
    QLabel      *target_size  = nullptr ;

    QLabel                  *tranfer_status_icon  = nullptr;
    QLabel                  *tranfer_status_text  = nullptr;
    DeviceSeleterWidget     *dev_widget           = nullptr;
    QParallelAnimationGroup *main_animation_group = nullptr;
    QProgressBar            *m_progressbar        = nullptr;
    QPushButton             *close_btn            = nullptr;
    QPushButton             *cancel_btn           = nullptr;
    QPushButton             *ok_btn               = nullptr;

    QIcon   file_icon;
    int     active_flag = 2;
    QString file_name;
    QString file_size;
    QString dev_name;
    SEND_DATA_STATE send_state = _SEND_NONE;

};

#endif // BLUETOOTHFILETRANSFERWIDGET_H
