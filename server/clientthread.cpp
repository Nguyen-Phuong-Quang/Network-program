#include "clientthread.h"

ClientThread::ClientThread(qintptr socketDescriptor, QSqlDatabase& database, QObject *parent) : QThread(parent), socketDescriptor(socketDescriptor), database(database) {}

void ClientThread::renderMessagesToClient(const QList<Message>& myStructArray, QTcpSocket* socket)
{
    QByteArray dataArray;
    QDataStream stream(&dataArray, QIODevice::WriteOnly);

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
        QDataStream stream(&socket);
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
            // Read the data from the socket
            QByteArray data = socket.readAll();

            // Deserialize the data into the struct
            QDataStream dataStream(data);
            SignIn myStruct;
            dataStream >> myStruct.username;
            dataStream >> myStruct.password;

            // Execute a query
            QSqlQuery query(database);
            query.prepare("SELECT * FROM users where username = :username");
            query.bindValue(":username", myStruct.username);

            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }

            // type
            stream << 1;

            if(!query.next()) {
                // No user found
                stream << 404;
                socket.flush();
                break;
            }

            QString password = query.value("password").toString();

            if(password != myStruct.password) {
                // Wrong creadential
                stream << 401;
                socket.flush();
                break;
            }

            // status
            stream << 200;

            currentUserId = query.value("user_id").toInt();

            // user log in
            stream << query.value("user_id").toInt();
            stream << query.value("name").toString();
            socket.flush();

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
                query.prepare("select M.user_id as sender_id, content, U.name from (select GP.user_id, content from (select * from groups where group_id = :targer_id) G inner join group_participants GP on GP.group_id = G.group_id inner join group_msg GM on GM.participant_id = GP.participant_id order by GM.created_time) M inner join users U on M.user_id = U.user_id");
                query.bindValue(":targetId", targetId);
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

            stream << 3;
            stream << 200;
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


            stream << 6;

            if(query.size() > 0) {
                stream << 409;
                socket.flush();
                break;
            }
            query.prepare("insert into groups (owner_id, group_name) values (:ownerId, :groupName)");
            query.bindValue(":ownerId", currentUserId);
            query.bindValue(":groupName", groupName);


            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }


            qDebug() << "User" << currentUserId << "group " << groupName;

            stream << 200;
            socket.flush();

            emit renderGroupsToClients(currentUserId);

            break;
        }
            // -----------------------------------------------------------------
            // Join group
        case 5: {
            int groupId;
            stream >> groupId;
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
