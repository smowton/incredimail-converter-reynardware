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

    qDebug() << "Start of new run";

    ui->radioButton_2->setChecked(true);
    ui->toolButton_2->setHidden(true);
    ui->toolButton_3->setHidden(true);
    ui->label_2->setHidden(true);
    ui->label_3->setHidden(true);

    // Find the home directory
    HomeDir = hd.homePath();
    HomeDir.append("/Local Settings/Application Data/IM/Identities/");
    hd.setPath(HomeDir);
    if( hd.exists(HomeDir) ) {
        dirs_listing = hd.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::DirsFirst);
        HomeDir.append(dirs_listing.at(0));
        qDebug() << "Home Directory Path" << HomeDir;
        ui->lineEdit->setText(HomeDir);
    } else {
        msgbox.setText("Could not find the default database directory");
        qWarning() << "Could not find the default database directory";
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


void Dialog::on_Convert_pressed() {
Incredimail_2 ic;
QString sql, eml, temp_str;
QStringList dir_listing, file_listing;
QMessageBox msgbox;
QFileInfo check_attachment;
int email, deleted, file_offset, size, deleted_email = 0;

   qDebug() << "User Pressed the Convert Button";
   qDebug() << "Text from LineEdit:" << ui->lineEdit->text();
   ic.Setup_Internal_Class_Directories(ui->lineEdit->text());

   sql = ic.root_path;
   sql.append("Containers.db");

   if( !ic.Set_SQLite_File(sql) ) {
      qWarning() << "SQL Database does not exist, current sql file:" << sql;
      qWarning() << "Looking if user selected any " << ui->label_3->text();
      sql = ui->label_3->text();
      if( sql.isNull() || !ic.Set_SQLite_File(sql) ) {
          msgbox.setText("Bad SQL File Exiting!");
          msgbox.exec();
          qFatal("Bad SQL File Exiting!");
      }
   }

   check_attachment.setFile(ic.attachment_path);
   if(!check_attachment.exists()) {
       qWarning() << "Bad attachment path, checking if user selected one" << ic.attachment_path;
       check_attachment.setFile(ui->label_2->text());
       if( !check_attachment.exists() ) {
          qCritical() << "Attachment Directory does not exit, continuing on without attachments";
       }
   }

   if(ui->radioButton_3->isChecked()) {
       temp_str = ui->lineEdit->text();
       temp_str = temp_str.right( temp_str.size() - temp_str.lastIndexOf("/") );
       file_listing << temp_str;
       qDebug() << "Single Database Selected";
   } else if(ui->radioButton_2->isChecked()) {
       file_listing = ic.Setup_IM_Directory_Processing();
       qDebug() << "Directory Selected";
   }

   // convert to directories
   for( int i = 0; i < file_listing.size(); i++ ) {
       QString temp = ic.root_path;
       temp = temp.append(file_listing.value(i));
       dir_listing << temp;
       qDebug() << "Directory Listing:" << dir_listing.at(i);
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
          eml = ic.root_path;
          eml.append(file_listing.value(i));
          eml.chop(4); // this is a quick hack to get rid of .imm

          QDir emldir;
          emldir.mkdir(eml);
          eml.append( QString("/EML_File_%1.eml").arg(j) );

          if( !deleted_email && ui->checkBox->isChecked() ) {
             ic.Extract_EML_File(eml, file_offset, size);
             ic.Insert_Attachments(eml);
          }

          progress_bar.setValue(j);
          if(progress_bar.wasCanceled())
             break;
      }
      ic.Close_Database_File();
   }
   qDebug() << "Converion Completed";
   msgbox.setText("Completed");
   msgbox.exec();
}

void Dialog::on_toolButton_2_pressed()
{
QFileDialog FileDialog;

   ui->label_2->setText( FileDialog.getExistingDirectory(this, tr("Attachment Directory"), "." ) );

   ui->toolButton_2->setDown(false);
}

void Dialog::on_toolButton_3_pressed()
{
QFileDialog FileDialog;

   ui->label_3->setText( FileDialog.getOpenFileName(this, tr("Containers File"), ".", tr("SQLite DB (*.db)") ) );

   ui->toolButton_3->setDown(false);
}


void Dialog::on_lineEdit_textChanged(QString )
{
    QFileInfo check_sql, check_attachment;
    QString sql_file;
    Incredimail_2 temp_ic;

        temp_ic.Setup_Internal_Class_Directories(ui->lineEdit->text());

        sql_file = temp_ic.root_path;
        sql_file.append("Containers.db");

        check_sql.setFile(sql_file);

        if( !check_sql.exists() ) {
           ui->toolButton_3->show();
           ui->label_3->show();
           ui->label_3->setText("Please Select Containers.db file");
       } else {
           ui->toolButton_3->setHidden(true);
           ui->label_3->setHidden(true);
           ui->label_3->clear();
       }

        check_attachment.setFile(temp_ic.attachment_path);

        if(!check_attachment.exists()) {
            ui->toolButton_2->show();
            ui->label_2->show();
            ui->label_2->setText("Please Select Attachment Directory");
        } else {
           ui->toolButton_2->setHidden(true);
           ui->label_2->setHidden(true);
           ui->label_2->clear();
        }

        if( ui->label_2->isHidden() && ui->label_3->isHidden() ) {
           ui->Convert->setEnabled(true);
        } else {
           ui->Convert->setEnabled(false);
        }
}
