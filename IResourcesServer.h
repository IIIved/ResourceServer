#ifndef IRESOURCESSERVER_H
#define IRESOURCESSERVER_H

#include <QTcpServer>
#include <QVector>
#include <QVariant>
#include <QMetaEnum>
#include <QTimer>
#include <QMessageBox>

class QThread;
class ServerWorker;
class IResourcesServer : public QTcpServer
{
public:
    explicit IResourcesServer(QObject *parent = nullptr)
        : QTcpServer(parent) {}

    virtual qint32 port() const = 0;

    virtual void clearResources() = 0;
protected:
    virtual void incomingConnection(qintptr socketDescriptor) = 0;

private:
    virtual void authorization(ServerWorker *sender, const QJsonObject &doc) = 0;
    virtual bool tryProcessResourceStatus(ServerWorker *sender, const QJsonObject &doc) = 0;
};

#endif // IRESOURCESSERVER_H
