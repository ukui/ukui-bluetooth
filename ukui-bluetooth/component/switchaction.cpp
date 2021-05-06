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
