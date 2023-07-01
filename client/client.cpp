#include "client.h"
#include<QDebug>

Client::Client(QObject *parent) : QObject(parent)
{
    user a;
    a.id = 1;
    a.name = "Quang";
    userList << a;
    user b;
    b.id = 2;
    b.name = "Giang";
    userList << b;

    currentUser = new user;
    currentUser->id = 1;
    currentUser->name = "Quang";
}

Client::~Client(){
    delete currentUser;
    currentUser = nullptr;
}

int Client::get_current_user_id() {
    return currentUser->id;
}

QVariantList Client::getUserListVariant()
{
    userListVariant.clear();
    for (const user& u : userList) {
         QVariantMap userMap;
            userMap["id"] = u.id;
            userMap["name"] = u.name;
            userListVariant.append(userMap);
     }
    return userListVariant;
}

