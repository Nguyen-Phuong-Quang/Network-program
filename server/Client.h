#ifndef CLIENT_H
#define CLIENT_H

#include<QString>
#include<QTcpSocket>
#include<QMutex>

struct Client {
    int id;
    QString name;
    int type;
    int target_id;
    QTcpSocket* socket;
    QMutex* mutex;
};

#endif // CLIENT_H
