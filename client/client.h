#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QVariantList>
#include <QIODevice>
#include <QMutex>
#include "user.h"
#include <iostream>
#include <QTcpSocket>
#include <QThread>
#include <QDataStream>
#include<QAbstractSocket>
#include "Message.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);
    ~Client();
    void sendDataToServer(const QByteArray& data);
public slots:
    QVariantList getUserListVariant();
    QVariantList getGroupListVariant();
    QVariantList getRequestListVariant() const;
    QVariantList getChatVariant() const;
    QString getNameSelected() const;
    int getTypeSelected() const;
    QString getName() const;

    int get_current_user_id();
    void signIn(QString username, QString password);
    void switchChat(int type, int target_id, QString chatName);
    void sendMessage(QString message);
    void createGroup(QString groupName);
    void requestJoinGroup(int groupId);
    void getPendingRequests();
    void acceptOrRejectUser(int type, int userId);
    void leftGroup();

signals:
    void successConnection();
    void signInResponse(int statusCode);
    void createGroupResponse(int code);
    void joinGroupResponse(int code);
    void messageReceived(QString message);
    void render();
    void renderChat();
    void renderRequestList();
    void switchSingleChatResponse();
    void hideChatView();

private slots:
    void readData();

private:
    QByteArray receivedDataBuffer;

    int currentUserId;
    QString name;

    QVariantList userListVariant;
    QVariantList groupListVariant;
    QVariantList requestListVariant;
    QVariantList chatVariant;

    QTcpSocket* socket;

    QString nameSelected;
    int typeSelected;
    int targetId;

    void responseToServerSuccess();

    QMutex mutex;

};

#endif // CLIENT_H
