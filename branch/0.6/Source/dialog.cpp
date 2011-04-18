#include "dialog.h"
#include "about_dialog.h"
#include "ui_dialog.h"
#include "ui_about_dialog.h"

#include <incredimail_convert.h>

#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    QDir hd;
    QString HomeDir;
    QStringList dirs_listing;
    QMessageBox msgbox;
    QFileInfo fileinfo;

    ui->radioButton_3->setChecked(true);

    // Find the home directory
    HomeDir = hd.homePath();
    HomeDir.append("/Local Settings/Application Data/IM/Identities/");
    hd.setPath(HomeDir);
    if( hd.exists(HomeDir) ) {
        dirs_listing = hd.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst);
        HomeDir.append(dirs_listing.at(0));
        HomeDir.append("/container.db");
        fileinfo.setFile( HomeDir );
        if( !fileinfo.exists() ) {
           msgbox.setText("Could not find the default database file");
           msgbox.setInformativeText(HomeDir);
        } else {
           ui->lineEdit->setText(HomeDir);
        }
    } else {
        msgbox.setText("Could not find the default database directory");
        msgbox.setInformativeText(HomeDir);
        msgbox.exec();
    }
}


Dialog::~Dialog()
{
    delete ui;
}


void Dialog::on_toolButton_pressed()
{
QFileDialog FileDialog;

    if( ui->radioButton_3->isChecked() ) {
       ui->lineEdit->setText(FileDialog.getOpenFileName(this, tr("IMM Database"), ".", tr("IMM Database (*.imm)") ));
    } else {
       ui->lineEdit->setText(FileDialog.getExistingDirectory(this, tr("Incredimail Database"), "." ) );
    }

    ui->toolButton->setDown(false);
}

void Dialog::on_Convert_pressed()
{
Incredimail_Convert ic;
QString sql, imm_db, attachment, eml;
QFileInfo db;
int email, deleted = 0;
int file_offset, size, deleted_email = 0;
int i = 0;

   db.setFile(ui->lineEdit->text());
   imm_db = ui->lineEdit->text();
   sql = db.path();
   sql.append("/Containers.db");
   attachment = db.path();
   attachment.append("/Attachments");

   ic.Set_Database_File(imm_db);
   ic.Set_SQLite_File(sql);
   ic.Incredimail_2_Email_Count(email, deleted);

   qDebug() << "Email:" << email << "Deleted:" << deleted;


   for( i = 0; i < email; i++ ) {
      ic.Incredimail_2_Get_Email_Offset_and_Size( file_offset, size, i, deleted_email );
      qDebug() << "File Offset" << file_offset << "Size" << size << "Deleted email" << deleted_email;
      eml = QString("EML_File_%1.eml").arg(i);
      ic.Extract_EML_File(eml, file_offset, size);
      ic.Insert_Attachments(eml, attachment );
   }
}
