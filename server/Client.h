#ifndef CLIENT_H
#define CLIENT_H

#include<QString>
#include<QTcpSocket>

struct Client {
    int id;
    QString name;
    int type;
    int target_id;
    QTcpSocket* socket;
};

#endif // CLIENT_H
