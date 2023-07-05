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

            // user log in
            codeStream << 1;
            codeStream << "Quang";
            socket.flush();

            QList<User> myStructList;

            // Add 6 elements to the list
            myStructList.append({7, "John"});
            myStructList.append({2, "Alice"});
            myStructList.append({3, "Bob"});
            myStructList.append({4, "Emma"});
            myStructList.append({5, "Michael"});
            myStructList.append({6, "Sophia"});

            codeStream << 2;

            sendArrayToClient(myStructList, &socket);
        }

            // Switch chat
        case 2: {
            QList<SingleMessage> messageList;
            int currentId;
            int targetId;
            codeStream >> currentId;
            codeStream >> targetId;

            if (targetId == 2) {
                // Add 20 elements to the list
                for (int i = 0; i < 40; ++i) {
                    SingleMessage message;
                    if (i % 2 == 0) {
                        message.sender_id = 1;
                    } else {
                        message.sender_id = 2;
                    }
                    message.content = "This is message number " + QString::number(i + 1);
                    messageList.append(message);
                }
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
