#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    QSqlDatabase database = QSqlDatabase::addDatabase("QPSQL");
    //            database.setHostName("chat-app.c6aoubm3unwy.us-east-1.rds.amazonaws.com");
    database.setHostName("127.0.0.1");
    database.setPort(5432);
    database.setDatabaseName("chat-app");
    database.setUserName("postgres");
    database.setPassword("Quang251209");
    if (!database.open()) {
        qDebug() << "Failed to connect to database:" << database.lastError().text();
    } else {
        qDebug() << "Database connected!";
    }

    ui->setupUi(this);

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, 5050)) {
        return;
    }

    connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::newConnection);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sendDataToClient(QByteArray data, QTcpSocket *socket)
{
    Client client = getCurrentClient(socket);
    mutex.lock();

    while(socket->state()!= QAbstractSocket::ConnectedState) {
    }

    socket->write(data);
    socket->waitForBytesWritten();
    mutex.unlock();
}

void MainWindow::renderUsersToClients()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    stream << 2;

    quint32 arraySize = static_cast<quint32>(clients.size());
    stream << arraySize;

    // Serialize each struct in the array
    for (const Client& client : clients) {
        stream << client.id;
        stream << client.name;
    }

    for(Client& client: clients) {
        if(client.id > 0) {
            sendDataToClient(data, client.socket);
        }
    }
}

void MainWindow::renderGroupToClient(int userId, QTcpSocket *socket)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QSqlQuery query(database);

    query.prepare("select P.group_id, group_name from groups G inner join group_participants P on G.group_id = P.group_id where user_id = :id and active = 1");
    query.bindValue(":id", userId);

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
    sendDataToClient(data, socket);
}

Client MainWindow::getCurrentClient(QTcpSocket *socket)
{
    Client cl;
    for(Client& client: clients) {
        if(client.socket == socket) {
            cl = client;
        }
    }

    return cl;
}

void MainWindow::newConnection()
{
    QTcpSocket *clientSocket = tcpServer->nextPendingConnection();
    clients.append({0, "", 0, 0, clientSocket, new QMutex});

    connect(clientSocket, &QTcpSocket::readyRead, [this, clientSocket]() {
        receiveData(clientSocket);
    });

    connect(clientSocket, &QTcpSocket::disconnected, [this, clientSocket]() {
        for (int i = 0; i < clients.size(); i++) {
            if (clients[i].socket == clientSocket) {
                for(const Client& client: clients) {
                    if(client.target_id == clients[i].id) {
                        QByteArray data;
                        QDataStream stream(&data, QIODevice::WriteOnly);
                        stream << 10;
                        sendDataToClient(data, client.socket);
                        break;
                    }
                }
                delete clients[i].mutex;
                clients.removeAt(i);
                break;
            }
        }
    });


}

void MainWindow::receiveData(QTcpSocket *socket)
{
    Client client = getCurrentClient(socket);
    QByteArray request = socket->readAll();
    int code;
    QDataStream stream(request);
    stream >> code;

    switch (code) {
    // Sign in
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
                        sendDataToClient(data, socket);

            break;
        }

        QString password = query.value("password").toString();

        if(password != myStruct.password) {
            // Wrong creadential
            out << 401;
                        sendDataToClient(data, socket);

            break;
        }

        // status
        out << 200;

        // user log in
        out << query.value("user_id").toInt();
        out << query.value("name").toString();
        for(Client& client: clients) {
            if(client.socket == socket) {
                client.id = query.value("user_id").toInt();
                client.name = query.value("name").toString();
            }
        }

        sendDataToClient(data, socket);
        renderGroupToClient(query.value("user_id").toInt(), socket);
        renderUsersToClients();
        break;
    }
        //Switch chat
    case 2: {
        QList<Message> messageList;
        int type;
        int targetId;

        stream >> type;
        stream >> targetId;

        QSqlQuery query(database);

        if(type == 0) {
            query.prepare("select DM.sender_id, DM.content, U.name from direct_msg DM inner join users U on DM.sender_id = U.user_id where (DM.sender_id = :currentId and DM.receiver_id = :targetId) or (DM.sender_id = :targetId and DM.receiver_id = :currentId) order by created_time");
            query.bindValue(":currentId", client.id);
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

        QByteArray data;
        QDataStream out(&data, QIODevice::WriteOnly);

        out << 3;
        // Serialize the array size
        quint32 arraySize = static_cast<quint32>(messageList.size());
        out << arraySize;

        // Serialize each struct in the array
        for (const Message& myStruct : messageList) {
            out << myStruct.sender_id;
            out << myStruct.name;
            out << myStruct.content;
        }

        sendDataToClient(data, socket);

        for(Client& client: clients) {
            if(client.socket == socket) {
                client.type = type;
                client.target_id = targetId;
                break;
            }
        }

        break;
    }
        //Send message
    case 3: {
        QString name, message;
        stream >> name;
        stream >> message;
        int type = client.type;
        int target_id = client.target_id;

        if (type == 0) {
            for(const Client& cl: clients) {
                if(cl.id == target_id) {
                    if(type == cl.type && cl.target_id == client.id) {

                        QByteArray data;
                        QDataStream stream(&data, QIODevice::WriteOnly);

                        stream << 4;

                        stream << client.id;
                        stream << name;
                        stream << message;

                        sendDataToClient(data, cl.socket);

                        break;

                    }
                }
            }
        } else if (type == 1) {
            for(const Client& cl: clients) {
                if(cl.id != client.id) {
                    if(type == cl.type && cl.target_id == target_id) {

                        QByteArray data;
                        QDataStream stream(&data, QIODevice::WriteOnly);

                        stream << 4;
                        stream << client.id;
                        stream << name;
                        stream << message;

                        sendDataToClient(data, cl.socket);
                    }
                }
            }
        }

        if(type == 0) {
            QSqlQuery query(database);

            query.prepare("insert into direct_msg (sender_id, receiver_id, content) values (:senderId, :receiverId, :message)");
            query.bindValue(":senderId", client.id);
            query.bindValue(":receiverId", target_id);
            query.bindValue(":message", message);

            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }

        } else if (type == 1) {
            QSqlQuery query(database);

            query.prepare("INSERT INTO group_msg (participant_id, content) SELECT gp.participant_id, :content FROM group_participants gp WHERE gp.group_id = :groupId AND gp.user_id = :userid");
            query.bindValue(":userid", client.id);
            query.bindValue(":groupId", target_id);
            query.bindValue(":content", message);

            if (!query.exec()) {
                qDebug() << "Query execution failed!";
            }

        }
        break;
    }
        //Create group
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
            sendDataToClient(data, socket);
            break;
        }
        query.prepare("insert into groups (group_name, owner_id) values (:groupName, :ownerId)");
        query.bindValue(":groupName", groupName);
        query.bindValue(":ownerId", client.id);


        if (!query.exec()) {
            qDebug() << "Query execution failed!";
        }


        out << 200;
        sendDataToClient(data, socket);
        renderGroupToClient(client.id, socket);
        break;
    }
        //Request join group
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
            sendDataToClient(data, socket);
            break;
        }

        query.prepare("select P.group_id, group_name from groups G inner join group_participants P on G.group_id = P.group_id where P.group_id = :groupId and user_id = :id and active = 1");
        query.bindValue(":id", client.id);
        query.bindValue(":groupId", groupId);

        if (!query.exec()) {
            qDebug() << "Query execution failed!";
        }

        if(query.size() > 0) {
            out << 409;
            sendDataToClient(data, socket);
            break;
        }

        query.prepare("insert into join_group_requests (user_id, group_id) values (:userId, :groupId)");
        query.bindValue(":userId", client.id);
        query.bindValue(":groupId", groupId);


        if (!query.exec()) {
            qDebug() << "Query insert execution failed!";
        }
        out << 200;
        sendDataToClient(data, socket);
        break;
    }
        //Get pending request
    case 6: {
        int groupId;
        stream >> groupId;

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

        sendDataToClient(data, socket);
        break;
    }
        //Accept or reject request
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

            for(Client& cl: clients) {
                if(cl.id  == userId) {
                    renderGroupToClient(cl.id, cl.socket);
                }
            }
        }

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

        sendDataToClient(data, socket);
        break;
    }
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

        sendDataToClient(data, socket);
        renderGroupToClient(userId, socket);
        break;

    }
    }
}
