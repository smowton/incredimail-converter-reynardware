#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QDialog>

namespace Ui {
    class About_Dialog;
}

class About_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit About_Dialog(QWidget *parent = 0);
    ~About_Dialog();

private:
    Ui::About_Dialog *ui;
};

#endif // ABOUT_DIALOG_H
