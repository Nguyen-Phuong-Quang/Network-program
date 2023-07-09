#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QDebug>
#include <QObject>
#include <QList>
#include <QMutex>
#include "User.h"
#include "Group.h"
#include "clientthread.h"

void sendDataToClient(const QByteArray& data, QTcpSocket* socket)
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

int main()
{
    int connection = 0;
    QMutex mutex;

    QList<User> userList;

    QTcpServer server;
    server.listen(QHostAddress::Any, 12345); // Listen on any IP and port
    qDebug() << "Server start...";

    while (true) {
        if (server.waitForNewConnection()) {
            QTcpSocket* clientSocket = server.nextPendingConnection();
            connection++;
            QString connectionName = QString("connection_%1").arg(connection);
            QSqlDatabase database = QSqlDatabase::addDatabase("QPSQL", connectionName);
            database.setHostName("chat-app.c6aoubm3unwy.us-east-1.rds.amazonaws.com");
            database.setPort(5432);
            database.setDatabaseName("chat-app");
            database.setUserName("postgres");
            database.setPassword("Quang251209");

            QByteArray data;
            QDataStream stream(&data, QIODevice::WriteOnly);
            if (!database.open()) {
                qDebug() << "Failed to connect to database:" << database.lastError().text();
                stream << 400;
            } else {
                stream << 200;
                //                qDebug() << "Database connected!";
            }
            QDataStream out(clientSocket);
            out.writeRawData(data.data(), data.size());
            clientSocket->flush();

            ClientThread* thread = new ClientThread(clientSocket->socketDescriptor(), database);

            // Handle online users
            QObject::connect(thread, &ClientThread::addUserToOnlineList, [&](int id, QString name) {
                mutex.lock();
                userList.append({id, name, clientSocket, 0, 0});
                mutex.unlock();
            });

            // Send online user list to clients
            QObject::connect(thread, &ClientThread::renderUsersToClients, [&]() {
                mutex.lock();

                for(const User& user: userList) {
                    QByteArray data;
                    QDataStream stream(&data, QIODevice::WriteOnly);

                    stream << 2;

                    quint32 arraySize = static_cast<quint32>(userList.size());
                    stream << arraySize;

                    // Serialize each struct in the array
                    for (const User& u : userList) {
                        stream << u.id;
                        stream << u.name;
                    }

                    sendDataToClient(data, user.socket);
                    qDebug() << "Send to " << user.name;
                }

                mutex.unlock();
            });

            // Send group list to clients
            QObject::connect(thread, &ClientThread::renderGroupsToClients, [&](int id) {
                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);

                QSqlQuery query(database);

                query.prepare("select P.group_id, group_name from groups G inner join group_participants P on G.group_id = P.group_id where user_id = :id and active = 1");
                query.bindValue(":id", id);

                if (!query.exec()) {
                    qDebug() << "Query execution failed!";
                }

                // Code for client
                stream << 5;

                // Size of records
                stream << query.size();
                while (query.next()) {
                    stream << query.value("group_id").toInt();
                    stream << query.value("group_name").toString();
                }

                sendDataToClient(data, clientSocket);
            });

            // Handle switch chat
            QObject::connect(thread, &ClientThread::switchChat, [&](int type, int current_id, int target_id) {
                mutex.lock();
                for(User& user: userList) {
                    if(user.id  == current_id) {
                        user.type = type;
                        user.target_id = target_id;
                        break;
                    }
                }
                mutex.unlock();
            });

            // Handle send message
            QObject::connect(thread, &ClientThread::sendMessage, [&](int current_id, QString name, QString message) {
                int type = -1;
                int target_id = -1;

                mutex.lock();
                for(const User& user: userList) {
                    if(user.id  == current_id) {
                        type = user.type;
                        target_id = user.target_id;
                        break;
                    }
                }

                if (type == 0) {
                    for(const User& user: userList) {
                        if(user.id == target_id) {
                            if(type == user.type && user.target_id == current_id) {
                                QByteArray data;
                                QDataStream stream(&data, QIODevice::WriteOnly);

                                stream << 4;

                                stream << current_id;
                                stream << name;
                                stream << message;

                                sendDataToClient(data, user.socket);

                                break;

                            }
                        }
                    }
                } else if (type == 1) {
                    for(const User& user: userList) {
                        if(user.id != current_id) {
                            if(type == user.type && user.target_id == target_id) {
                                QByteArray data;
                                QDataStream stream(&data, QIODevice::WriteOnly);

                                stream << 4;
                                stream << current_id;
                                stream << name;
                                stream << message;

                                sendDataToClient(data, user.socket);

                            }
                        }
                    }
                }

                mutex.unlock();

                if(type == 0) {
                    QSqlQuery query(database);

                    query.prepare("insert into direct_msg (sender_id, receiver_id, content) values (:senderId, :receiverId, :message)");
                    query.bindValue(":senderId", current_id);
                    query.bindValue(":receiverId", target_id);
                    query.bindValue(":message", message);

                    if (!query.exec()) {
                        qDebug() << "Query execution failed!";
                    }

                } else if (type == 1) {
                    QSqlQuery query(database);

                    query.prepare("INSERT INTO group_msg (participant_id, content) SELECT gp.participant_id, :content FROM group_participants gp WHERE gp.group_id = :groupId AND gp.user_id = :userid");
                    query.bindValue(":userid", current_id);
                    query.bindValue(":groupId", target_id);
                    query.bindValue(":content", message);

                    if (!query.exec()) {
                        qDebug() << "Query execution failed!";
                    }

                }


            });

            // Connect to the userDisconnected signal of the ClientThread
            QObject::connect(thread, &ClientThread::userDisconnected, [&](int userId) {
                if(userId > 0) {
                    mutex.lock();
                    userList.erase(std::remove_if(userList.begin(), userList.end(),[userId](const User& user) {return user.id == userId;}), userList.end());

                    qDebug() << "User id: " << userId << "Disconnect";
                    mutex.unlock();
                }
            });

            // Render pending requests
            QObject::connect(thread, &ClientThread::renderPendingRequests, [&](int groupId) {
                QSqlQuery query(database);
                QByteArray data;
                QDataStream stream(&data, QIODevice::WriteOnly);
                query.prepare("select U.user_id, name from join_group_requests J inner join users U on J.user_id = U.user_id where group_id = :groupId");
                query.bindValue(":groupId", groupId);

                if (!query.exec()) {
                    qDebug() << "Query execution failed!";
                }

                stream << 8;

                stream << query.size();

                while (query.next()) {
                    stream << query.value("user_id").toInt();
                    stream << query.value("name").toString();
                }

                sendDataToClient(data, clientSocket);
            });

            // Render new group to user accepted
            QObject::connect(thread, &ClientThread::renderGroupToUserAccepted, [&](int userId) {

                mutex.lock();
                for(User& user: userList) {
                    if(user.id  == userId) {
                        QByteArray subData;
                        QDataStream subStream(&data, QIODevice::WriteOnly);
                        QSqlQuery query(database);

                        query.prepare("select P.group_id, group_name from groups G inner join group_participants P on G.group_id = P.group_id where user_id = :id and active = 1");
                        query.bindValue(":id", userId);

                        if (!query.exec()) {
                            qDebug() << "Query execution failed!";
                        }

                        // Code for client
                        subStream << 5;

                        // Size of records
                        subStream << query.size();
                        while (query.next()) {
                            subStream << query.value("group_id").toInt();
                            subStream << query.value("group_name").toString();
                        }

                        sendDataToClient(data, user.socket);
                        break;
                    }
                }
                mutex.unlock();


            });

            QObject::connect(thread, &ClientThread::finished, thread, &ClientThread::deleteLater);
            QObject::connect(thread, &ClientThread::error, [clientSocket](QAbstractSocket::SocketError socketError) {
                qDebug() << "Socket error:" << socketError;
                clientSocket->disconnectFromHost();
                clientSocket->deleteLater();
            });

            thread->start();
        }
    }

    return 0;
}
