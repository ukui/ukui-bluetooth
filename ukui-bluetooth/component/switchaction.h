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

#ifndef SWITCHACTION_H
#define SWITCHACTION_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

#include "switchbutton.h"

class SwitchAction : public QWidget
{
    Q_OBJECT
public:
    explicit SwitchAction(QWidget *parent = nullptr);
    ~SwitchAction();

    void setBtnStatus(bool value);
signals:
    void sendBtnStatus(bool value);
private:
    SwitchButton *switch_btn = nullptr;
};

#endif // SWITCHACTION_H
