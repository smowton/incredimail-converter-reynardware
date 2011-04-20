#include "dialog.h"
#include "about_dialog.h"
#include "ui_dialog.h"
#include "ui_about_dialog.h"

#include <incredimail_2.h>

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

    ui->radioButton_3->setChecked(true);

    // Find the home directory
    HomeDir = hd.homePath();
    HomeDir.append("/Local Settings/Application Data/IM/Identities/");
    hd.setPath(HomeDir);
    if( hd.exists(HomeDir) ) {
        dirs_listing = hd.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst);
        HomeDir.append(dirs_listing.at(0));
        ui->lineEdit->setText(HomeDir);
    } else {
        msgbox.setText("Could not find the default database directory");
        qDebug() << "Could not find the default database directory";
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
Incredimail_2 ic;
QString sql, imm_db, attachment, eml;
QFileInfo db;
QStringList dir_listing;
int email, deleted, file_offset, size, deleted_email = 0;

   imm_db = ui->lineEdit->text();
   db.setFile(imm_db);
   ic.Set_Database_Path(db.path());
   sql = attachment = db.path();
   sql.append("/Containers.db");
   attachment.append("/Attachments");

   ic.Set_Attachments_Directory(attachment);
   ic.Set_SQLite_File(sql);

   if(ui->radioButton_3->isChecked()) {
       dir_listing << imm_db;
   } else if(ui->radioButton_2->isChecked()) {
       dir_listing = ic.Setup_IM_Directory_Processing();
   }

   for( int i = 0; i < dir_listing.size(); i++ ) {
      ic.Set_Database_File( dir_listing.at(0) );
      ic.Email_Count(email, deleted);
      for( int j = 0; j < email; j++ ) {
          ic.Get_Email_Offset_and_Size( file_offset, size, j, deleted_email );
          // **** mkdir here ****
          eml = QString("EML_File_%1.eml").arg(j);
          ic.Extract_EML_File(eml, file_offset, size);
          ic.Insert_Attachments(eml);
      }
      ic.Close_Database_File();
   }
}
