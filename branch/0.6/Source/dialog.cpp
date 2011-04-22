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
#include <QProgressDialog>

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
QString sql, attachment, eml, root_dir;
QStringList dir_listing, file_listing;
int email, deleted, file_offset, size, deleted_email = 0;

   ic.IM_Database_Info.setFile(ui->lineEdit->text());
   if( ic.IM_Database_Info.isFile() ) {
       sql = attachment = root_dir = ic.IM_Database_Info.absolutePath();
   } else if( ic.IM_Database_Info.isDir() ) {
       sql = attachment = root_dir = ic.IM_Database_Info.absoluteFilePath();
   }

   sql.append("/Containers.db");
   attachment.append("/Attachments/");
   root_dir.append("/");

   ic.IM_Attachment.setFile(attachment);
   ic.Set_SQLite_File(sql);

   if(ui->radioButton_3->isChecked()) {
       file_listing << ic.IM_Database_Info.fileName();
   } else if(ui->radioButton_2->isChecked()) {
       file_listing = ic.Setup_IM_Directory_Processing();
   }

   // convert to directories
   for( int i = 0; i < file_listing.size(); i++ ) {
       QString temp = root_dir;
       root_dir.append(file_listing.value(i));
       dir_listing << root_dir;
       root_dir = temp;
       qDebug() << "File Listing:" << file_listing.at(i);
   }


   for( int i = 0; i < dir_listing.size(); i++ ) {
      ic.Set_Database_File( dir_listing.at(i) );
      ic.Email_Count(email, deleted);
      QProgressDialog progress_bar("Converting...", "Abort", 0, email, this);
      progress_bar.setWindowModality(Qt::WindowModal);
      progress_bar.setMinimumDuration(0);
      for( int j = 0; j < email; j++ ) {
          progress_bar.setLabelText(QString("Converting %1 of %2").arg(j).arg(email));
          ic.Get_Email_Offset_and_Size( file_offset, size, j, deleted_email );
          eml = root_dir;
          eml.append(file_listing.value(i));
          eml.chop(4); // this is a quick hack to get rid of .imm
          QDir emldir;
          emldir.mkdir(eml);
          eml.append( QString("/EML_File_%1.eml").arg(j) );
          // insert deleted check?
          ic.Extract_EML_File(eml, file_offset, size);
          ic.Insert_Attachments(eml);
          progress_bar.setValue(j);
          if(progress_bar.wasCanceled())
             break;
      }
      ic.Close_Database_File();
   }

}
