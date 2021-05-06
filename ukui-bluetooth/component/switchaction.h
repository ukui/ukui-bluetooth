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
