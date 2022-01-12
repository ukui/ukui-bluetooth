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
#include "featureswidget.h"
#include "../daemon/bluetoothdbus.h"

#include <QObject>
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStyleFactory>
#include <QTextCodec>
#include <QDebug>
#include <ukui-log4qt.h>

int main(int argc, char *argv[])
{
    initUkuiLog4qt("ukui-bluetooth");

    QApplication app(argc, argv);
//    QApplication::setStyle(QStyleFactory::create("ukui-default"));
    QApplication::setQuitOnLastWindowClosed(false);

    QString locale = QLocale::system().name();
//        QString locale = "es";
    QTranslator translator;
    if(locale == "zh_CN" || locale == "es" || locale == "fr" || locale == "de" || locale == "ru") {//中文 西班牙语 法语 德语 俄语
        if(!translator.load("ukui-bluetooth_" + locale + ".qm",
                            ":/translations/"))
            qDebug() << "Load translation file："<< "ukui-bluetooth_" + locale + ".qm" << " failed!";
        else
            app.installTranslator(&translator);
    }

    //加载Qt对话框默认的国际化
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale,
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    FeaturesWidget *w = new FeaturesWidget();
    if( w->exit_flag ){
        return 0;
    }
    return app.exec();
}
