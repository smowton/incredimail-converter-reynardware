#include "clicklabel.h"
#include "about_dialog.h"

#include <QMessageBox>
#include <QMouseEvent>

ClickLabel::ClickLabel( QWidget *parent ): QLabel(parent)
{
// todo
}

void ClickLabel::mousePressEvent( QMouseEvent *e )
{
About_Dialog about;

   about.exec();
}
