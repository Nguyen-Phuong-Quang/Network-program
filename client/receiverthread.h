#ifndef RECEIVERTHREAD_H
#define RECEIVERTHREAD_H

#include <QTcpSocket>
#include <QThread>

class ReceiverThread: public QThread
{
public:
    QTcpSocket* socket;
    explicit ReceiverThread();

protected:
    void run() override;

private:

};

#endif // RECEIVERTHREAD_H
