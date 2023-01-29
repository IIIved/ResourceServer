#ifndef RESOURCESERVER_H
#define RESOURCESERVER_H

#include "IResourcesServer.h"

#include "ResourceController.h"
#include "config.h"

#include <QTcpServer>
#include <QVector>
#include <QVariant>
#include <QMetaEnum>
#include <QTimer>
#include <QMessageBox>


class QThread;
class ServerWorker;
class IResourcesServer;
class ResourceServer : public IResourcesServer
{
    Q_OBJECT
    Q_DISABLE_COPY(ResourceServer)
public:
    explicit ResourceServer(QObject *parent = nullptr);

    qint32 port() const override;
    bool regOpened = true;
    bool acceptResRequest = true;
    void clearResources() override;

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void logMessage(const QString &msg);

public slots:
    void stopServer();
    void startServer();

private slots:
    void jsonReceived(ServerWorker *sender, const QJsonObject &doc);

    void sendMessageToAllUsers(const QJsonObject &message, ServerWorker *exclude);

    void userDisconnected(ServerWorker *sender, const QString &message);
    void userError(ServerWorker *sender);

    void sendJson(ServerWorker *destination, const QJsonObject &message);

    void resourceIsReleasedByServer(const QString& userName, quint32 idx);

private:
    void authorization(ServerWorker *sender, const QJsonObject &doc) override;
    bool tryProcessResourceStatus(ServerWorker *sender, const QJsonObject &doc) override;

private:
    Config m_config;
    ResourceController m_resource_controller;
    QTcpServer tcpServer;
    QVector<ServerWorker *> m_clients;
    QTimer m_update_timer;
};
#endif // RESOURCESERVER_H
