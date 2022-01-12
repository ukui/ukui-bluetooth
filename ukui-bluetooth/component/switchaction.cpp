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
#include "switchaction.h"

SwitchAction::SwitchAction(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(240,40);

    QHBoxLayout *main_layout = new QHBoxLayout(this);
    main_layout->setSpacing(0);
    main_layout->setContentsMargins(32,0,16,0);

    QLabel *tip_label = new QLabel(tr("Bluetooth"),this);
    main_layout->addWidget(tip_label,1,Qt::AlignLeft|Qt::AlignVCenter);

    switch_btn = new SwitchButton(this);
    connect(switch_btn,&SwitchButton::checkedChanged,this,[=](bool value){
        sendBtnStatus(value);
    });

    main_layout->addWidget(switch_btn);
}

SwitchAction::~SwitchAction()
{

}

void SwitchAction::setBtnStatus(bool value)
{
    switch_btn->setChecked(value);
}
