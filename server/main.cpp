#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QDebug>
#include <QObject>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QReadWriteLock>

#include "User.h"
#include "clientthread.h"

//QReadWriteLock readWriteLock;
//QReadLocker readLock{&readWriteLock}; // Auto read-locker
//QWriteLocker writeLock{&readWriteLock}; // Auto write-locker

int main()
{

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

            ClientThread* thread = new ClientThread(clientSocket->socketDescriptor(), database);

            QObject::connect(thread, &ClientThread::addUserToOnlineList, [&](int id, QString name) {

                userList.append({id, name, clientSocket});


                qDebug() << "User signed in: ID" << id << "Name:" << name;

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
            });

            // Connect to the userDisconnected signal of the ClientThread
            QObject::connect(thread, &ClientThread::userDisconnected, [&](int userId) {
                // Remove the user from the userList based on the user ID
                for (auto it = userList.begin(); it != userList.end(); ++it) {
                    if (it->id == userId) {
                        userList.erase(it);
                        break;
                    }
                }

                qDebug() << "User id: " << userId << "Disconnect";

                for(const User& user: userList) {
                    QDataStream coedStream(user.socket);
                    coedStream << 2;

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
