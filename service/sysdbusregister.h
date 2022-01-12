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

#ifndef SYSDBUSREGISTER_H
#define SYSDBUSREGISTER_H

#include <string>
#include <glib.h>
#include <glib/gprintf.h>

#include <QtDBus/QDBusContext>
#include <QObject>
#include <QCoreApplication>
#include <QFile>
#include <QDateTime>
#include <QDebug>

#define LIST_PATH "/etc/pairDevice.list"

class SysDbusRegister : public QObject, protected QDBusContext
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.bluetooth.interface")
public:
    SysDbusRegister();
    ~SysDbusRegister();
private:

public slots:
    Q_SCRIPTABLE int exitService();
    Q_SCRIPTABLE QString writeKeyFile(QString,QString,qint32);
    Q_SCRIPTABLE QString removeKeyFile(QString);
    Q_SCRIPTABLE QString getKeyFilePath();
};

#endif // SYSDBUSREGISTER_H
