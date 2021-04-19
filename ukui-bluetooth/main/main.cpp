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
        return -1;
    }
    return app.exec();
}
