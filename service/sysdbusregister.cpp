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

#include "sysdbusregister.h"

SysDbusRegister::SysDbusRegister()
{

}

SysDbusRegister::~SysDbusRegister()
{

}

int SysDbusRegister::exitService()
{
    qApp->exit(0);
    return 0;
}

QString SysDbusRegister::writeKeyFile(QString devAddress, QString devName, qint32 type)
{
    if(devAddress.isNull())
        return QString("Address can not be empty ! ! !");
    if(devName.isNull())
        return QString("Name can not be empty ! ! !");
    if(devAddress.at(2) != ":" ||
       devAddress.at(5) != ":" ||
       devAddress.at(8) != ":" ||
       devAddress.at(11) != ":" ||
       devAddress.at(14) != ":")
        return QString("arg0 is not an address ! ! !");


    if(!QFile::exists(LIST_PATH)){
        QFile file(LIST_PATH);
        file.open(QIODevice::WriteOnly);
        file.close();
    }

    GKeyFile *key_file = nullptr;
    key_file = g_key_file_new();
    char *data;
    gsize length = 0;
    g_key_file_load_from_file(key_file,QString(LIST_PATH).toStdString().c_str(),G_KEY_FILE_NONE,NULL);
    g_key_file_set_string(key_file,devAddress.toStdString().c_str(),"Name",devName.toStdString().c_str());
    g_key_file_set_string(key_file,devAddress.toStdString().c_str(),"Type",QString("%1").arg(type).toStdString().c_str());
    g_key_file_set_string(key_file,devAddress.toStdString().c_str(),"ConnectTime",QString::number(QDateTime::currentMSecsSinceEpoch() / 1000).toStdString().c_str());


    data = g_key_file_to_data(key_file,&length,NULL);
    g_file_set_contents(QString(LIST_PATH).toStdString().c_str(),data,length,NULL);
    g_free(data);

    g_key_file_free(key_file);

    return QString("Key write ok!!!");
}

QString SysDbusRegister::removeKeyFile(QString devAddress)
{
    if(devAddress.isNull())
        return QString("Address can not be empty ! ! !");
    if(devAddress.at(2) != ":" ||
       devAddress.at(5) != ":" ||
       devAddress.at(8) != ":" ||
       devAddress.at(11) != ":" ||
       devAddress.at(14) != ":")
        return QString("arg0 is not an address ! ! !");

    if(!QFile::exists(LIST_PATH)){
        QFile file(LIST_PATH);
        file.open(QIODevice::WriteOnly);
        file.close();
    }

    GKeyFile *key_file = nullptr;
    char *data;
    gsize length = 0;
    GError *error;
    key_file = g_key_file_new();
    g_key_file_load_from_file(key_file,QString(LIST_PATH).toStdString().c_str(),G_KEY_FILE_NONE,NULL);
    if(g_key_file_has_group(key_file,devAddress.toStdString().c_str())){
        g_key_file_remove_group(key_file,devAddress.toStdString().c_str(),&error);
        data = g_key_file_to_data(key_file,&length,NULL);
        g_file_set_contents(QString(LIST_PATH).toStdString().c_str(),data,length,NULL);
        g_free(data);
    }
    g_key_file_free(key_file);

    return QString("Key remove ok!!!");
}

QString SysDbusRegister::getKeyFilePath()
{
    return QString(LIST_PATH);
}


