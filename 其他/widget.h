#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QWindow>
#include <QBackingStore>
#include <windows.h>

class Widget : public QWindow
{
    Q_OBJECT

public:
    Widget(QWindow *parent = nullptr);
    ~Widget();

    bool event(QEvent *event);

private:
    QBackingStore store;
    QImage image;

};

#endif // WIDGET_H
