#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include <QEvent>

class ClickLabel : public QLabel
{
    Q_OBJECT
    public:
            ClickLabel(QWidget *parent = 0);

    protected:
            virtual void mousePressEvent( QMouseEvent * );
};

#endif // CLICKLABEL_H
