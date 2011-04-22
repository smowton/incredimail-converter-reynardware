#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QLineEdit>
class QDropEvent;

class myLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    myLineEdit(QWidget *parent = 0);

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
};

#endif // MYLINEEDIT_H
