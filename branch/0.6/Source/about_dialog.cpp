#include "about_dialog.h"
#include "ui_about_dialog.h"

About_Dialog::About_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About_Dialog)
{
    ui->setupUi(this);
}

About_Dialog::~About_Dialog()
{
    delete ui;
}
