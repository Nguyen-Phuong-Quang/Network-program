#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QVariantList>
#include "user.h"
#include <iostream>
#include <QTcpSocket>
#include <QThread>
#include <QDataStream>
#include "Message.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    ~Client();

public slots:
    QVariantList getUserListVariant();
    QVariantList getGroupListVariant();
    QVariantList getChatVariant() const;
    QString getNameSelected() const;
    int getTypeSelected() const;

    int get_current_user_id();
    void signIn(QString username, QString password);
    void switchChat(int type, int target_id, QString chatName);
    void sendMessage(QString message);
    void createGroup(QString groupName);
    void requestJoinGroup(int groupId);

signals:
    void successConnection();
    void signInResponse(int statusCode);
    void createGroupResponse(int code);
    void messageReceived(QString message);
    void render();
    void renderChat();
    void switchSingleChatResponse();

private slots:
    void readData();

private:
    int currentUserId;
    QString name;

    QVariantList userListVariant;
    QVariantList groupListVariant;

    QVariantList chatVariant;

    QTcpSocket* socket;
    QThread receiveThread;
    QString nameSelected;
    int typeSelected;
};

#endif // CLIENT_H
