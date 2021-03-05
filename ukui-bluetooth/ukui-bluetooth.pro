TARGET = ukui-bluetooth
DESTDIR = .
TEMPLATE = app

QT       += core gui dbus KWindowSystem x11extras

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 link_pkgconfig

PKGCONFIG += gsettings-qt gio-2.0
# 适配窗口管理器圆角阴影
LIBS +=-lpthread
LIBS +=-lX11 -lXrandr -lXinerama -lXi -lXcursor

LIBS += -L /usr/lib/x86_64-linux-gnu -l KF5BluezQt -lgio-2.0 -lglib-2.0

inst1.files += ../data/org.bluez.Agent1.conf
inst1.path = /etc/dbus-1/system.d/
inst2.files += ../data/org.ukui.bluetooth.gschema.xml
inst2.path = /usr/share/glib-2.0/schemas/
#inst3.files += data/ukui-bluetooth.desktop
#inst3.files = /etc/xdg/autostart/
target.source += $$TARGET
target.path = /usr/bin
INSTALLS += inst1 \
    inst2 \
#    inst3 \
    target

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
#           QT_NO_WARNING_OUTPUT \
#           QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix {
    UI_DIR = .ui
    MOC_DIR = .moc
    OBJECTS_DIR = .obj
}

SOURCES += \
    fileSend/bluetoothfiletransferwidget.cpp \
    fileReceive/filereceivingpopupwidget.cpp \
    main/main.cpp \
    main/featureswidget.cpp \
    pin/pincodewidget.cpp \
    fileSend/deviceseleterwidget.cpp \
    config/xatom-helper.cpp \
    daemon/bluetoothdbus.cpp \
    agent/bluetoothobexagent.cpp \
    agent/bluetoothagent.cpp

HEADERS += \
    fileSend/bluetoothfiletransferwidget.h \
    main/featureswidget.h \
    fileReceive/filereceivingpopupwidget.h \
    pin/pincodewidget.h \
    fileSend/deviceseleterwidget.h \
    config/xatom-helper.h \
    daemon/bluetoothdbus.h \
    agent/bluetoothobexagent.h \
    agent/bluetoothagent.h

#TRANSLATIONS += \
#    ../translations/ukui-bluetooth_zh_CN.ts

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ukui-bluetooth.qrc
