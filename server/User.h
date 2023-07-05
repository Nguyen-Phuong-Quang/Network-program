#ifndef USER_H
#define USER_H

#include<QString>
#include<QTcpSocket>

struct User {
    int id;
    QString name;
    QTcpSocket* socket;
};

#endif // USER_H
