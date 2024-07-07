#include "widget.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QDesktopWidget>

static HWND g_worker = 0;
static BOOL CALLBACK EnumCallBack(HWND handle, LPARAM lparam)
{
    HWND p = FindWindowEx(handle, 0, L"SHELLDLL_DefView", 0);

    if (p != 0)
    {
        qDebug() << p << endl;
        g_worker = FindWindowEx(0, handle, L"WorkerW", 0);
    }

    return true;
}

Widget::Widget(QWindow *parent) : QWindow(parent), store(this)
{
    HWND hwndProgram = FindWindow(L"Progman", L"Program Manager");
    SendMessageTimeout(hwndProgram, 0x052C, 0, 0, SMTO_NORMAL, 1000, 0);
    EnumWindows(EnumCallBack, 0);

    if (g_worker == 0)
    {
        qDebug() << "没有找到" << endl;
        abort();
    }

    QRect fullRect = QApplication::desktop()->availableGeometry();
    this->setGeometry(fullRect);

    this->image = QImage("Your picture path.");
    this->setParent(QWindow::fromWinId((WId)g_worker));
}

bool Widget::event(QEvent *event)
{
    if (event->type() == QEvent::Expose || event->type() == QEvent::Resize)
    {
        QRect rect(QPoint(0, 0), this->size());
        this->store.resize(this->size());

        this->store.beginPaint(rect);
        QPainter painter(this->store.paintDevice());
        painter.drawImage(0, 0, this->image);
        this->store.endPaint();

        store.flush(rect);
    }

    return QWindow::event(event);
}

Widget::~Widget()
{

}

