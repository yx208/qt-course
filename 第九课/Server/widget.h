#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QTcpServer>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

public slots:
    void onNewConnection();

    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);

private slots:
    void on_sendButton_clicked();

    void on_imageButton_clicked();

private:
    Ui::Widget *ui;
    QTcpServer server;
    QList<QTcpSocket*> clients;

    int imageIndex;
    quint32 sizePackLast;
};
#endif // WIDGET_H
