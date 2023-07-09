#include "clientthread.h"

ClientThread::ClientThread(qintptr socketDescriptor, QSqlDatabase& database, QObject *parent) : QThread(parent), socketDescriptor(socketDescriptor), database(database) {}

void ClientThread::renderMessagesToClient(const QList<Message>& myStructArray, QTcpSocket* socket)
{
    QByteArray dataArray;
    QDataStream stream(&dataArray, QIODevice::WriteOnly);

    stream << 3;
    // Serialize the array size
    quint32 arraySize = static_cast<quint32>(myStructArray.size());
    stream << arraySize;

    // Serialize each struct in the array
    for (const Message& myStruct : myStructArray) {
        stream << myStruct.sender_id;
        stream << myStruct.name;
        stream << myStruct.content;
    }

    // Send the serialized array to the client
    sendDataToClient(dataArray, socket);
    //    return;
}

void ClientThread::run()
{
    // Get the thread ID
    Qt::HANDLE threadId = this->currentThreadId();
    qDebug() << "Thread ID:" << threadId;


    QTcpSocket socket;
    if (!socket.setSocketDescriptor(socketDescriptor)) {
        emit error(socket.error());
        return;
    }

    connect(&socket, &QTcpSocket::disconnected, this, &ClientThread::handleDisconnected);

    while (socket.waitForReadyRead()) {
        QByteArray data = socket.readAll();
        int code;
        QDataStream stream(data);
        stream >> code;

        switch (code) {
        // Client disconnect
        case 0: {
            database.close();
            emit userDisconnected(currentUserId);
            emit renderUsersToClients();
            break;
        }
            // -----------------------------------------------------------------
            //Sign in
        case 1: {
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
            QByteArray data;
            QDataStream out(&data, QIODevice::WriteOnly);

            out << 1;

            if(!query.next()) {
                // No user found
                out << 404;
                sendDataToClient(data, &socket);

                break;
            }

            QString password = query.value("password").toString();

            if(password != myStruct.password) {
                // Wrong creadential
                out << 401;
                sendDataToClient(data, &socket);

                break;
            }

            // status
            out << 200;

            currentUserId = query.value("user_id").toInt();

            // user log in
            out << query.value("user_id").toInt();
            out << query.value("name").toString();
            sendDataToClient(data, &socket);

            emit addUserToOnlineList(query.value("user_id").toInt(), query.value("name").toString());

            emit renderUsersToClients();

            emit renderGroupsToClients(query.value("user_id").toInt());
            break;
        }
            // -----------------------------------------------------------------
            // Switch chat
        case 2: {
            QList<Message> messageList;
            int type;
            int targetId;

            stream >> type;
            stream >> targetId;


            QSqlQuery query(database);

            if(type == 0) {
                query.prepare("select DM.sender_id, DM.content, U.name from direct_msg DM inner join users U on DM.sender_id = U.user_id where (DM.sender_id = :currentId and DM.receiver_id = :targetId) or (DM.sender_id = :targetId and DM.receiver_id = :currentId) order by created_time");
                query.bindValue(":currentId", currentUserId);
                query.bindValue(":targetId", targetId);
            } else if(type == 1) {
                query.prepare(" select U.user_id sender_id, U.name, content from group_msg GM inner join group_participants GP on GM.participant_id = GP.participant_id inner join users U on U.user_id = GP.user_id where GP.group_id = :groupId order by created_time");
                query.bindValue(":groupId", targetId);
            }

            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }

            while(query.next()) {
                Message message;
                message.sender_id = query.value("sender_id").toInt();
                message.name = query.value("name").toString();
                message.content = query.value("content").toString();
                messageList.append(message);
            }

            renderMessagesToClient(messageList, &socket);
            emit switchChat(type, currentUserId, targetId);
            break;
        }
            // -----------------------------------------------------------------
            // Send message
        case 3: {
            QString name, message;
            stream >> name;
            stream >> message;
            emit sendMessage(currentUserId, name, message);
            break;
        }
            // -----------------------------------------------------------------
            // Create group
        case 4: {
            QString groupName;
            stream >> groupName;

            // Execute a query
            QSqlQuery query(database);
            query.prepare("SELECT * FROM groups where group_name = :groupName");
            query.bindValue(":groupName", groupName);


            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }

            QByteArray data;
            QDataStream out(&data, QIODevice::WriteOnly);
            out << 6;


            if(query.size() > 0) {
                out << 409;
                sendDataToClient(data, &socket);
                break;
            }
            query.prepare("insert into groups (group_name) values (:ownerId, :groupName)");
            query.bindValue(":groupName", groupName);


            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }


            qDebug() << "User" << currentUserId << "group " << groupName;
            out << 200;

            sendDataToClient(data, &socket);

            emit renderGroupsToClients(currentUserId);

            break;
        }
            // -----------------------------------------------------------------
            // Join group
        case 5: {
            int groupId;
            stream >> groupId;

            QByteArray data;
            QDataStream out(&data, QIODevice::WriteOnly);
            out << 7;

            // Execute a query
            QSqlQuery query(database);
            query.prepare("SELECT * FROM groups where group_id = :groupId");
            query.bindValue(":groupId", groupId);

            if (!query.exec()) {
                qDebug() << "Query select execution failed!";
            }

            if(query.size() < 1) {
                out << 404;
                sendDataToClient(data, &socket);
                break;
            }

            query.prepare("insert into join_group_requests (user_id, group_id) values (:userId, :groupId)");
            query.bindValue(":userId", currentUserId);
            query.bindValue(":groupId", groupId);


            if (!query.exec()) {
                qDebug() << "Query insert execution failed!";
            }
            out << 200;
            sendDataToClient(data, &socket);
            break;
        }
            // -----------------------------------------------------------------
            // Get pending requests
        case 6: {
            int groupId;
            stream >> groupId;
            emit renderPendingRequests(groupId);
            break;
        }
            // -----------------------------------------------------------------
            // Handle accept or reject user
        case 7: {
            int type; // 1: Accept, 0: Reject
            int groupId;
            int userId;
            stream >> type;
            stream >> groupId;
            stream >> userId;

            QSqlQuery query(database);
            query.prepare("delete from join_group_requests where group_id = :groupId and user_id = :userId");
            query.bindValue(":groupId", groupId);
            query.bindValue(":userId", userId);

            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }

            if(type == 1) {
                query.prepare("select participant_id where group_id = :groupId and user_id = :userId");
                query.bindValue(":groupId", groupId);
                query.bindValue(":userId", userId);

                if (!query.exec()) {
                    qDebug() << "Query execution failed!";
                }

                if(query.next()) {
                    query.prepare("update group_participants set avtive = 1 where group_id = :groupId and user_id = :userId");
                    query.bindValue(":groupId", groupId);
                    query.bindValue(":userId", userId);
                } else {
                    query.prepare("insert into group_participants (group_id, user_id, active) values (:groupId, :userId, 1)");
                    query.bindValue(":groupId", groupId);
                    query.bindValue(":userId", userId);
                }

                if (!query.exec()) {
                    qDebug() << "Query execution failed!";
                }

                emit renderGroupToUserAccepted(userId);
            }

            emit renderPendingRequests(groupId);
            break;
        }
            // -----------------------------------------------------------------
            // Left group
        case 8: {
            int userId;
            int groupId;
            stream >> userId;
            stream >> groupId;

            QSqlQuery query(database);
            query.prepare("update group_participants set active = 0 where group_id = :groupId and user_id = :userId");
            query.bindValue(":groupId", groupId);
            query.bindValue(":userId", userId);

            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }

            QByteArray data;
            QDataStream out(&data, QIODevice::WriteOnly);

            out << 9;

            sendDataToClient(data, &socket);
            emit renderGroupsToClients(userId);
            break;

        }
        }
    }

    // Clean up
    socket.disconnectFromHost();
}

void ClientThread::sendDataToClient(const QByteArray& data, QTcpSocket* socket)
{
    while (socket->bytesToWrite() > 0) {
        socket->waitForBytesWritten();
    }

    QDataStream out(socket);
    out.writeRawData(data.data(), data.size());
    socket->flush();

    while (!socket->waitForReadyRead()) {
        // Optionally add a timeout mechanism or handle other tasks while waiting
    }

    // Read the data sent by the client
    QByteArray clientData = socket->readAll();
    QDataStream clientStream(clientData);

    // Process the received data from the client
    int clientResponseCode;
    clientStream >> clientResponseCode;

    while (clientResponseCode != 200) {

    }
}

void ClientThread::handleDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socket->deleteLater();
        quit();  // Terminate the thread event loop
    }
}

