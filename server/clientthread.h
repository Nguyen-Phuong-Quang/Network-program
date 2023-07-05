#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QDataStream>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <iostream>
#include "SingleMessage.h"
#include "User.h"
#include "SignIn.h"

class ClientThread : public QThread
{
    Q_OBJECT

public:
    explicit ClientThread(qintptr socketDescriptor, QSqlDatabase& database, QObject *parent = nullptr);
    void sendArrayToClient(const QList<User>& myStructArray, QTcpSocket* socket);
    void sendMessage(const QList<SingleMessage>& myStructArray, QTcpSocket* socket);

protected:
    void run();

signals:
    void error(QAbstractSocket::SocketError socketError);

private slots:
    void handleDisconnected();

private:
    qintptr socketDescriptor;
    QSqlDatabase& database;
};

#endif // CLIENTTHREAD_H
