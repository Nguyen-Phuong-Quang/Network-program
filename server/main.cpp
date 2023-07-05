#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QDebug>
#include <QObject>
#include "clientthread.h"
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

    QTcpServer server;
    server.listen(QHostAddress::Any, 12345); // Listen on any IP and port
    qDebug() << "Server start...";
    while (true) {
        if (server.waitForNewConnection()) {
            QTcpSocket* clientSocket = server.nextPendingConnection();

            ClientThread* thread = new ClientThread(clientSocket->socketDescriptor(), database);
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
