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
#include <QMutex>

#include <iostream>
#include "Message.h"
#include "User.h"
#include "SignIn.h"

class ClientThread : public QThread
{
    Q_OBJECT

public:
    explicit ClientThread(qintptr socketDescriptor, QSqlDatabase& database, QMutex& db_mutex, QObject *parent = nullptr);
    void renderMessagesToClient(const QList<Message>& myStructArray, QTcpSocket* socket);

protected:
    void run();

signals:
    void error(QAbstractSocket::SocketError socketError);
    void switchChat(int type,int current_id, int target_id);
    void addUserToOnlineList(int id, QString name);
    void sendMessage(int current_id, QString name, QString message);
    void userDisconnected(int id);
    void renderUsersToClients();
    void renderGroupsToClients(int id);

private slots:
    void handleDisconnected();

private:
    qintptr socketDescriptor;
    QSqlDatabase& database;
    QMutex& db_mutex;
    int currentUserId = -1;
};

#endif // CLIENTTHREAD_H
