#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QVariantList>
#include "user.h"
#include <iostream>
#include <QTcpSocket>
#include <QThread>
#include <QDataStream>
#include "SingleMessage.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    ~Client();

public slots:
    QVariantList getUserListVariant();
    int get_current_user_id();
    void signIn(QString username, QString password);
    void switchSingleChat(int id);
    QVariantList getChatVariant() const;

signals:
    void signInResponse(int statusCode);
    void messageReceived(QString message);
    void render();
    void switchSingleChatResponse();
    void renderChat();

private slots:
    void readData();

private:
    int currentUserId;
    QString username;
    QList<User> userList;
//    QList<SingleMessage> chat;
    QVariantList userListVariant;
    QVariantList chatVariant;

    QTcpSocket* socket;
    QThread receiveThread;
};

#endif // CLIENT_H
