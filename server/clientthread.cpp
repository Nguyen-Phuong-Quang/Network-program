#include "clientthread.h"

ClientThread::ClientThread(qintptr socketDescriptor, QSqlDatabase& database, QObject *parent) : QThread(parent), socketDescriptor(socketDescriptor), database(database) {}

void ClientThread::sendArrayToClient(const QList<User>& myStructArray, QTcpSocket* socket)
{
    QByteArray dataArray;
    QDataStream stream(&dataArray, QIODevice::WriteOnly);
    // Serialize the array size
    quint32 arraySize = static_cast<quint32>(myStructArray.size());
    stream << arraySize;

    // Serialize each struct in the array
    for (const User& myStruct : myStructArray) {
        stream << myStruct.id;
        stream << myStruct.name;
    }

    // Send the serialized array to the client
    socket->write(dataArray);
    socket->flush();
}

void ClientThread::sendMessage(const QList<SingleMessage>& myStructArray, QTcpSocket* socket)
{
    QByteArray dataArray;
    QDataStream stream(&dataArray, QIODevice::WriteOnly);

    // Serialize the array size
    quint32 arraySize = static_cast<quint32>(myStructArray.size());
    stream << arraySize;

    // Serialize each struct in the array
    for (const SingleMessage& myStruct : myStructArray) {
        stream << myStruct.sender_id;
        stream << myStruct.content;
    }

    // Send the serialized array to the client
    socket->write(dataArray);
    socket->flush();
    //    return;
}

void ClientThread::run()
{
    QTcpSocket socket;
    if (!socket.setSocketDescriptor(socketDescriptor)) {
        emit error(socket.error());
        return;
    }

    connect(&socket, &QTcpSocket::disconnected, this, &ClientThread::handleDisconnected);

    while (socket.waitForReadyRead()) {
        int code;
        QDataStream codeStream(&socket);
        codeStream >> code;

        switch (code) {
        // Client disconnect
        case 0: {
            emit userDisconnected(currentUserId);
            break;
        }
            // -----------------------------------------------------------------
            //Sign in
        case 1: {
            // Read the data from the socket
            QByteArray data = socket.readAll();

            // Deserialize the data into the struct
            QDataStream stream(data);
            SignIn myStruct;
            stream >> myStruct.username;
            stream >> myStruct.password;

            // Execute a query
            QSqlQuery query(database);
            query.prepare("SELECT * FROM users where username = :username");
            query.bindValue(":username", myStruct.username);

            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }

            // type
            codeStream << 1;

            if(!query.next()) {
                // No user found
                codeStream << 404;
                break;
            }

            QString password = query.value("password").toString();

            if(password != myStruct.password) {
                // Wrong creadential
                codeStream << 401;
                break;
            }

            // status
            codeStream << 200;

            currentUserId = query.value("user_id").toInt();

            // user log in
            codeStream << query.value("user_id").toInt();
            codeStream << query.value("name").toString();
            socket.flush();

            emit addUserToOnlineList(query.value("user_id").toInt(), query.value("name").toString());
        }

            // -----------------------------------------------------------------
            // Switch chat
        case 2: {
            QList<SingleMessage> messageList;
            int currentId;
            int targetId;
            codeStream >> currentId;
            codeStream >> targetId;

            QSqlQuery query(database);

            query.prepare("select * from direct_msg where (sender_id = :currentId and receiver_id = :targetId) or (sender_id = :targetId and receiver_id = :currentId) order by created_time");
            query.bindValue(":currentId", currentId);
            query.bindValue(":targetId", targetId);

            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }

            while(query.next()) {
                SingleMessage message;
                message.sender_id = query.value("sender_id").toInt();
                message.content = query.value("content").toString();
                qDebug() << message.sender_id << ": " << message.content ;
                messageList.append(message);
            }

            codeStream << 3;
            codeStream << 200;
            sendMessage(messageList, &socket);
        }
        }

    }

    // Clean up
    socket.disconnectFromHost();
}

void ClientThread::handleDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {

        socket->deleteLater();
        quit();  // Terminate the thread event loop
    }
}
