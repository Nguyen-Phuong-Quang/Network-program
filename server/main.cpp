#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QDebug>
#include <QObject>
#include <QList>
#include <QMutex>
#include "User.h"
#include "clientthread.h"

int main()
{
    QMutex mutex;
    QMutex db_mutex;

    // Create and open the database connection
    QSqlDatabase database = QSqlDatabase::addDatabase("QPSQL");
    database.setHostName("chat-app.c6aoubm3unwy.us-east-1.rds.amazonaws.com");
    database.setPort(5432);
    database.setDatabaseName("chat-app");
    database.setUserName("postgres");
    database.setPassword("Quang251209");

    if (!database.open()) {
        qDebug() << "Failed to connect to database:" << database.lastError().text();
        return 1;
    } else {
        qDebug() << "Database connected!";
    }

    QList<User> userList;

    QTcpServer server;
    server.listen(QHostAddress::Any, 12345); // Listen on any IP and port
    qDebug() << "Server start...";

    while (true) {
        if (server.waitForNewConnection()) {
            QTcpSocket* clientSocket = server.nextPendingConnection();

            ClientThread* thread = new ClientThread(clientSocket->socketDescriptor(), database, db_mutex);

            QObject::connect(thread, &ClientThread::addUserToOnlineList, [&](int id, QString name) {
                mutex.lock();
                userList.append({id, name, clientSocket, 0, 0});

                for(const User& user: userList) {
                    QDataStream codeStream(user.socket);
                    codeStream << 2;

                    QByteArray dataArray;
                    QDataStream stream(&dataArray, QIODevice::WriteOnly);
                    // Serialize the array size

                    quint32 arraySize = static_cast<quint32>(userList.size());
                    stream << arraySize;

                    // Serialize each struct in the array
                    for (const User& u : userList) {
                        stream << u.id;
                        stream << u.name;
                    }

                    // Send the serialized array to the client
                    user.socket->write(dataArray);
                    user.socket->flush();

                }
                mutex.unlock();
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
            QObject::connect(thread, &ClientThread::sendMessage, [&](int current_id, QString message) {
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

                for(const User& user: userList) {
                    if(user.id == target_id) {
                        if(type == user.type && user.target_id == current_id) {
                            QDataStream stream(user.socket);

                            stream << 4;

                            stream << current_id;
                            stream << message;

                            user.socket->flush();

                            break;

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
                    db_mutex.lock();
                    if (!query.exec()) {
                        qDebug() << "Query execution failed!";
                    }
                    db_mutex.unlock();
                }


            });

            // Connect to the userDisconnected signal of the ClientThread
            QObject::connect(thread, &ClientThread::userDisconnected, [&](int userId) {
                if(userId > 0) {
                    mutex.lock();
                    // Remove the user from the userList based on the user ID
                    for (auto it = userList.begin(); it != userList.end(); ++it) {
                        if (it->id == userId) {
                            userList.erase(it);
                            break;
                        }
                    }

                    qDebug() << "User id: " << userId << "Disconnect";

                    for(const User& user: userList) {
                        QDataStream codeStream(user.socket);
                        codeStream << 2;

                        QByteArray dataArray;
                        QDataStream stream(&dataArray, QIODevice::WriteOnly);
                        // Serialize the array size


                        quint32 arraySize = static_cast<quint32>(userList.size());
                        stream << arraySize;

                        // Serialize each struct in the array
                        for (const User& u : userList) {
                            stream << u.id;
                            stream << u.name;
                        }

                        // Send the serialized array to the client
                        user.socket->write(dataArray);
                        user.socket->flush();

                        mutex.unlock();
                    }
                }

                clientSocket->disconnectFromHost();
                clientSocket->deleteLater();
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
