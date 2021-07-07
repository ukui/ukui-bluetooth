#ifndef PINCODEWIDGET_H
#define PINCODEWIDGET_H

#include <QWidget>
#include <QGuiApplication>
#include <QList>
#include <QLabel>
#include <QIcon>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QString>
#include <QDialog>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPalette>
#include <QGSettings>
#include <QScreen>

class PinCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PinCodeWidget(QString name = "", QString pin = "", bool flag = true);
    ~PinCodeWidget();
    void Connection_timed_out();
    void pairFailureShow();
    void updateUIInfo(const QString &name,const QString &pin);
private slots:
    void onClick_close_btn(bool);
    void onClick_accept_btn(bool);
    void onClick_refuse_btn(bool);
    void GSettingsChanges(const QString &key);
signals:
    void accepted();
    void rejected();
private:
    QGSettings  *settings    = nullptr;

    QLabel      *PIN_label   = nullptr;
    QLabel      *tip_label   = nullptr;
    QLabel      *top_label   = nullptr;

    QVBoxLayout *main_layout = nullptr;

    QPushButton *close_btn   = nullptr;
    QPushButton *accept_btn  = nullptr;
    QPushButton *refuse_btn  = nullptr;

    QString dev_name;
    QString PINCode;
    bool    show_flag;
};

#endif // PINCODEWIDGET_H
