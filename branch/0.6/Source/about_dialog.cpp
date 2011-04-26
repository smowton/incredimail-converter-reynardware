#include "about_dialog.h"
#include "ui_about_dialog.h"

About_Dialog::About_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About_Dialog)
{
    ui->setupUi(this);

    ui->label->setOpenExternalLinks(true);
    ui->label_2->setOpenExternalLinks(true);

}

About_Dialog::~About_Dialog()
{
    delete ui;
}

void About_Dialog::on_pushButton_pressed()
{
    close();
}
