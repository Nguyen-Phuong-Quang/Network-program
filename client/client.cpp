#include "client.h"
#include <QDebug>
#include <QDataStream>


Client::Client(QObject *parent) : QObject(parent)
{
    // Create the socket
    socket = new QTcpSocket(this);

    // Move socket to the receive thread
    socket->moveToThread(&receiveThread);

    // Connect the readyRead signal to the readData slot
    connect(socket, &QTcpSocket::readyRead, this, &Client::readData);

    // Start the receive thread
    receiveThread.start();

    // Connect to the TCP server upon initialization
    socket->connectToHost("127.0.0.1", 12345); // Connect to server IP and port
    if (socket->waitForConnected())
    {
        qDebug() << "Connected to server";
    }
    else
    {
        qDebug() << "Failed to connect to server";
    }
}

Client::~Client()
{
    QDataStream stream(socket);

    stream << 0;
    socket->flush();

    receiveThread.quit();
    receiveThread.wait();
    socket->disconnectFromHost();
    socket->deleteLater(); // Cleanup the socket
}

void Client::readData()
{
    while (socket->bytesAvailable() > 0)
    {

        //Check type of response
        int type;
        QDataStream in(socket);
        in >> type;

        switch(type) {
        // Sign in
        case 1: {
            int code;
            in >> code;

            // If success
            if(code == 200) {
                in >> currentUserId;
                in >> username;
                qDebug() << currentUserId << " " << username << "Sign in";

            }
            emit signInResponse(code);
            break;
        }
            // Users online
        case 2: {
            userListVariant.clear();
            QByteArray dataArray = socket->readAll();
            QDataStream stream(dataArray);

            // Deserialize the array size
            quint32 arraySize;
            stream >> arraySize;

            // Deserialize each struct in the array
            for (quint32 i = 0; i < arraySize; ++i) {
                User user;
                stream >> user.id;
                stream >> user.name;
                if(user.id != currentUserId) {
                    QVariantMap userMap;
                    userMap["id"] = user.id;
                    userMap["name"] = user.name;
                    userListVariant.append(userMap);
                }

            }
            break;
        }

            // Switch message
        case 3: {
            int code;
            in >> code;
            if(code == 200) {
                // Receive new chat data
                chatVariant.clear();
                QByteArray dataArray = socket->readAll();
                QDataStream stream(dataArray);

                // Deserialize the array size
                quint32 arraySize;
                stream >> arraySize;

                // Deserialize each struct in the array
                for (quint32 i = 0; i < arraySize; ++i) {
                    SingleMessage message;
                    stream >> message.sender_id;
                    stream >> message.content;
                    QVariantMap messageMap;
                    messageMap["sender_id"] = message.sender_id;
                    messageMap["content"] = message.content;
                    chatVariant.append(messageMap);
                }
            }
            emit renderChat();
            break;
        }
        }

        emit render();
    }
}

QVariantList Client::getChatVariant() const
{
    return chatVariant;
}

int Client::get_current_user_id() {
    return currentUserId;
}

QVariantList Client::getUserListVariant()
{
    return userListVariant;
}

void Client::signIn(QString username, QString password) {

    // Send data to the server
    QDataStream out(socket);

    out << 1;
    out << username;
    out << password;
    socket->flush();

}

void Client::switchSingleChat(int targetUserId) {
    // Send data to the server
    QDataStream out(socket);

    out << 2;
    out << currentUserId;
    out << targetUserId;
    socket->flush();
}


