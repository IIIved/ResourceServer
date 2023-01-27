#ifndef IRESOURCESSERVER_H
#define IRESOURCESSERVER_H

#include "resource_controller.h"
#include "config.h"

#include <QTcpServer>
#include <QVector>
#include <QVariant>
#include <QMetaEnum>
#include <QTimer>
#include <QMessageBox>

struct SendingMessage{
    QVariant type;
    QVariant username;
    QVariant message;
    QVariant status;
    QVariant resource;
    QVariant request;

    QJsonObject toJsonObject() const;
};

class QThread;
class ServerWorker;
class IResourcesServer : public QTcpServer
{
    Q_OBJECT
    Q_DISABLE_COPY(IResourcesServer)
public:
    explicit IResourcesServer(QObject *parent = nullptr);

    qint32 port() const;
    bool regOpened = true;
    bool acceptResRequest = true;
    void clearResources();
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
    void authorization(ServerWorker *sender, const QJsonObject &doc);
    bool tryProcessResourceStatus(ServerWorker *sender, const QJsonObject &doc);

private:
    TConfig m_config;
    TResourceController m_resource_controller;

    QVector<ServerWorker *> m_clients;
    QTimer m_update_timer;
};

#endif // IRESOURCESSERVER_H
