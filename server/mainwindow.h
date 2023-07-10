#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDataStream>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMutex>
#include "Client.h"
#include "Message.h"
#include "SignIn.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void sendDataToClient(QByteArray data, QTcpSocket* socket);
    void renderUsersToClients();
    void renderGroupToClient(int userId, QTcpSocket* socket);
    Client getCurrentClient(QTcpSocket* socket);

private slots:
    void newConnection();
    void receiveData(QTcpSocket *socket);

private:
    Ui::MainWindow *ui;
    QTcpServer *tcpServer;
    QList<Client> clients;
    QSqlDatabase database;
    QMutex mutex;
};

#endif // MAINWINDOW_H
