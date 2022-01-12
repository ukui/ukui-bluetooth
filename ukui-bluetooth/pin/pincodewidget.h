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
#include <QLineEdit>

class PinCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PinCodeWidget(QString name = "", QString pin = "", bool flag = true);
    PinCodeWidget(QString name = "", bool flag = true);
    ~PinCodeWidget();
    void Connection_timed_out();
    void pairFailureShow();
    QString getEnteredPINCode();
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
    QLineEdit   *PINCode_edit = nullptr;
    QString dev_name;
    QString PINCode;
    bool    show_flag;
    QString dev_name_beyond_display_length(QString devName);
    QString Insert_dev_name_newline(QString devName,int count);
};

#endif // PINCODEWIDGET_H
