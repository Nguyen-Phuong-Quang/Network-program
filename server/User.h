#ifndef USER_H
#define USER_H

#include<QString>
#include<QTcpSocket>

struct User {
    int id;
    QString name;
    QTcpSocket* socket;
    int type;
    int target_id;
//    bool sign_in;
};

#endif // USER_H
