#include "featureswidget.h"
#include "../daemon/bluetoothdbus.h"

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
//    QApplication::setStyle(QStyleFactory::create("ukui-default"));
    QApplication::setQuitOnLastWindowClosed(false);


    QString locale = QLocale::system().name();
//        QString locale = "es";
    QTranslator translator;
    if(locale == "zh_CN" || locale == "es" || locale == "fr" || locale == "de" || locale == "ru") {//中文 西班牙语 法语 德语 俄语
        if(!translator.load("ukui-bluetooth_" + locale + ".qm",
                            ":/qmfile/translations/"))
            qDebug() << "Load translation file："<< "ukui-bluetooth_" + locale + ".qm" << " failed!";
        else
            app.installTranslator(&translator);
    }

    //加载Qt对话框默认的国际化
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale,
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    FeaturesWidget w;
    return app.exec();
}
