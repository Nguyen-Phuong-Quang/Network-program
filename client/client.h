#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QVariantList>
#include "user.h"
#include <iostream>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    ~Client();

public slots:
    QVariantList getUserListVariant();
    int get_current_user_id();
signals:

private:
    user * currentUser {nullptr};
    QList<user> userList;
    QVariantList userListVariant;
};

#endif // CLIENT_H
