#include "widget.h"
#include "ui_widget.h"

#include <QDebug>
#include <QHostAddress>
#include <QFileDialog>
#include <QDataStream>

Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget)
{
    this->imageIndex = 0;
    this->sizePackLast = 0;

    this->ui->setupUi(this);
    connect(&this->server, &QTcpServer::newConnection, this, &Widget::onNewConnection);

    bool ok = this->server.listen(QHostAddress::AnyIPv4, 8888);
    qDebug() << "listen: " << ok;
}

Widget::~Widget()
{
    delete this->ui;
}

void Widget::onNewConnection()
{
    QTcpSocket *socket = this->server.nextPendingConnection();
    this->clients.append(socket);

    connect(socket, &QIODevice::readyRead, this, &Widget::onReadyRead);
    connect(socket, &QAbstractSocket::connected, this, &Widget::onConnected);
    connect(socket, &QAbstractSocket::disconnected, this, &Widget::onDisconnected);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
}

void Widget::onReadyRead()
{
    QObject *obj = this->sender();
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(obj);

    qint64 sizeNow = 0;

    do {
        sizeNow = socket->bytesAvailable();
        QDataStream stream(socket);

        // 第一次读取
        if (this->sizePackLast == 0)
        {
            // 主要防止: 当前收到 2 个字节，但是使用标识包大小的数字为 4 字节（quint32）
            // 所以这里需要保证拿到 4 个字节，这样才有意义
            if (sizeNow < sizeof(quint32))
                return;

            // 流出一个 quint32 大小的字节给 sizePack
            stream >> this->sizePackLast;
        }

        // 收到的字节大小 >= 应该收到的包大小
        // 说明包已经完整
        // 这里 -4 的原因是因为上面已经流了 4 字节（quint32）
        if (sizeNow < this->sizePackLast - 4)
            return;

        QByteArray dataFull;
        stream >> dataFull;

        this->sizePackLast = 0;

        // 粘包
        sizeNow = socket->bytesAvailable();

        // 根据不同消息类型做处理
        QString prefix = dataFull.mid(0, 4);
        if (prefix == "TXT:")
        {
            this->ui->messageView->append(dataFull.mid(4));
        }
        else if (prefix == "IMG:")
        {
            QString imageTag = QString("<img width=\"100\" src=\"%1\">");
            QString stringifyIndex = QString::number(this->imageIndex++);
            imageTag = imageTag.arg(stringifyIndex + ".png");

            QFile file(stringifyIndex + ".png");
            file.open(QIODevice::WriteOnly);
            file.write(dataFull.mid(4));
            file.close();

            this->ui->messageView->insertHtml(imageTag);
        }

    } while(sizeNow > 0);
}

void Widget::onConnected()
{
    qDebug() << "connected";
}

void Widget::onDisconnected()
{
    QObject *obj = this->sender();
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(obj);

    this->clients.removeAll(socket);
    socket->deleteLater();

    qDebug() << "disconnected";
}

void Widget::onError(QAbstractSocket::SocketError socketError)
{
    qDebug() << "error: " << socketError;
}

void Widget::on_sendButton_clicked()
{

    QString message = "TXT:" + this->ui->messageInput->toPlainText();

    QByteArray sendData;
    QDataStream dataStream(&sendData, QIODevice::WriteOnly);
    dataStream << (quint32)0 << message.toUtf8();
    dataStream.device()->seek(0);
    dataStream << sendData.size();

    for (QList<QTcpSocket*>::iterator iter = clients.begin(); iter != clients.end(); ++iter)
    {
        QTcpSocket *client = *iter;
        client->write(sendData);
    }

}

void Widget::on_imageButton_clicked()
{
    QString imageFile = QFileDialog::getOpenFileName(this, "title", ".", "Image Files (*.png *.jpg)");
    if (imageFile.isEmpty())
        return;

    QFile file(imageFile);
    file.open(QIODevice::ReadOnly);
    QByteArray data = "IMG:" + file.readAll();
    file.close();

    QByteArray sendData;
    QDataStream dataStream(&sendData, QIODevice::WriteOnly);
    dataStream << (quint32)0 << data;
    dataStream.device()->seek(0);
    dataStream << sendData.size();

    for (QList<QTcpSocket*>::iterator iter = clients.begin(); iter != clients.end(); ++iter)
    {
        QTcpSocket *client = *iter;
        client->write(sendData);
    }
}










