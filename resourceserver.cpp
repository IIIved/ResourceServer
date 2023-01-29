#include "resourceserver.h"
#include "ServerWorker.h"
#include "IResourcesServer.h"

#include <QThread>
#include <functional>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QSettings>

namespace
{
    template<typename QEnum>
    QString enumToString (const QEnum value)
    {
        return QString(QMetaEnum::fromType<QEnum>().valueToKey(value));
    }

    QString toStringJsonValue(const QJsonObject &doc, const QString& key) {
        const QJsonValue keyVal = doc.value(key);
        if (   keyVal.isNull()
            || !keyVal.isString()) {
            throw 0;
        }
        return keyVal.toString();
    }

    template<class T>
    T toNumJsonValue(const QJsonObject &doc, const QString& key) {
        const QJsonValue keyVal = doc.value(key);
        T result = 0;
        bool success = false;

        double tmp = keyVal.toString().toLong(&success,10);

        if (success &&
            tmp >= static_cast<double>(std::numeric_limits<T>::min()) &&
            tmp <= static_cast<double>(std::numeric_limits<T>::max()) &&
            std::floor(tmp) == tmp
        ) {
            result = std::floor(tmp);
        } else {
            throw 0;
        }
        return result;
    }
}

QJsonObject SendingMessage::toJsonObject() const {
    QJsonObject jsonObject;
    const auto setValue = [&jsonObject](const QString& key, const QVariant& value ){
        if(value.isValid()) {
            jsonObject[key] = value.toString();
        }
    };

    setValue(QStringLiteral("type"), type);
    setValue(QStringLiteral("username"), username);
    setValue(QStringLiteral("message"), message);

    setValue(QStringLiteral("status"), status);
    setValue(QStringLiteral("request"), request);
    setValue(QStringLiteral("resource"), resource);

    return jsonObject;
}

ResourceServer::ResourceServer(QObject *parent)
    : m_config(LoadConfig(":/Config/config.ini"))
    , m_resource_controller(m_config)
    , m_update_timer(parent)
{

    connect(&m_update_timer, SIGNAL(timeout()), &m_resource_controller, SLOT(updateResources()));
}

qint32 ResourceServer::port() const {
    return m_config.port;
}

void ResourceServer::incomingConnection(qintptr socketDescriptor)
{
    ServerWorker *worker = new ServerWorker(this);
    if (!worker->setSocketDescriptor(socketDescriptor)) {
        worker->deleteLater();
        return;
    }

    connect(worker, &ServerWorker::disconnectedFromServer, this, std::bind(&ResourceServer::userDisconnected, this, worker,""));
    connect(worker, &ServerWorker::error, this, std::bind(&ResourceServer::userError, this, worker));
    connect(worker, &ServerWorker::jsonReceived, this, std::bind(&ResourceServer::jsonReceived, this, worker, std::placeholders::_1));
    connect(worker, &ServerWorker::logMessage, this, &ResourceServer::logMessage);

    connect(&m_resource_controller, &ResourceController::resourceIsReleasedByServer, this, &ResourceServer::resourceIsReleasedByServer);

    m_clients.append(worker);
    emit logMessage(QStringLiteral("New client Connected"));
}

void ResourceServer::sendJson(ServerWorker *destination, const QJsonObject &message)
{
    Q_ASSERT(destination);
    destination->sendJson(message);
}

void ResourceServer::sendMessageToAllUsers(const QJsonObject &message, ServerWorker *exclude)
{
    for (ServerWorker *worker : m_clients) {
        Q_ASSERT(worker);
        if (worker == exclude)
            continue;
        sendJson(worker, message);
    }
}

void ResourceServer::resourceIsReleasedByServer(const QString& userName,quint32 idx) {

    for (ServerWorker *worker : m_clients) {
        if(   worker->userName().isEmpty()
           || worker->userName() != userName
           || userName.isEmpty())
            continue;

        emit logMessage("resource (" + QString::number(idx) + ") is released for the " + userName );

        sendJson(worker, SendingMessage{
                     .type = QStringLiteral("resource_status"),
                     .message = enumToString(ResourceController::ResourceIsReleasedByServer),
                     .status = 0,
                     .resource = 1 << idx
                 }.toJsonObject());
    }
}


void ResourceServer::userDisconnected(ServerWorker *sender, const QString &msg)
{
    if(m_clients.removeAll(sender)) {
        const QString userName = sender->userName();

        sendJson(sender, SendingMessage{
                     .type = QStringLiteral("userdisconnected"),
                     .username = userName.isEmpty() ? "unknown" : userName,
                     .message = msg.isEmpty() ? "unknown" : msg
                 }.toJsonObject());

        if(!userName.isEmpty()) {
            m_resource_controller.freeClientResources(userName);
            emit logMessage(userName + " disconnected");
        }

        sender->deleteLater();
    }
}

void ResourceServer::userError(ServerWorker *sender)
{
    Q_UNUSED(sender)
}

void ResourceServer::startServer() {
    m_update_timer.start(1000);
}

void ResourceServer::stopServer()
{
    m_update_timer.stop();
    for (ServerWorker *worker : m_clients) {
        worker->disconnectFromServer();
    }
    close();
}

void ResourceServer::jsonReceived(ServerWorker *sender, const QJsonObject &doc)
{
    Q_ASSERT(sender);
    emit logMessage("JSON received " + QString::fromUtf8(QJsonDocument(doc).toJson(QJsonDocument::Compact)));

    if (sender->userName().isEmpty()) {
        authorization(sender, doc);
    } else {
        tryProcessResourceStatus(sender, doc);
    }
}

bool ResourceServer::tryProcessResourceStatus(ServerWorker *sender, const QJsonObject &doc)
{
    try{
        const auto userName = sender->userName();
        const QString type = toStringJsonValue(doc,"type");
        const qint64 lifeTimeSecondsMSecs = toNumJsonValue<qint64>(doc,"time") * 1000;
        const quint32 request = toNumJsonValue<quint32>(doc,"request");
        if(type == "request") {

            const auto list = m_resource_controller.resourceListByMask(request);

            if(list.empty() || !acceptResRequest) {
                sendJson(sender, SendingMessage{
                             .type = QStringLiteral("resource_status"),
                             .message = enumToString(ResourceController::InvalidResourceIndex),
                             .status = 0
                         }.toJsonObject());
                emit logMessage(userName + " couldn't reserve a resource by " + enumToString(ResourceController::InvalidResourceIndex));
            } else {

                QString idxString;

                for(const auto idx:list) {
                    const auto response = m_resource_controller.reserveResource(userName, idx, lifeTimeSecondsMSecs);
                    sendJson(sender, SendingMessage{
                                 .type = QStringLiteral("resource_status"),
                                 .message = enumToString(response),
                                 .status = response == ResourceController::Success,
                                 .resource = 1 << idx
                             }.toJsonObject());

                    if(response == ResourceController::Success){
                        idxString += QString::number(idx+1);
                    } else {
                        emit logMessage(userName + " couldn't reserve a resource by " + enumToString(response));
                    }
                }
                if(!idxString.isEmpty()) {
                    emit logMessage(userName + " has reserved resources: " +  idxString);
                }
            }
        }
        return true;

    } catch(...){
        return false;
    };
}


void ResourceServer::authorization(ServerWorker *sender, const QJsonObject &doc)
{
    Q_ASSERT(sender);


    const QJsonValue typeVal = doc.value("type");
    const QJsonValue usernameVal = doc.value("username");

    const auto disconect = [&](auto sender, const QString& msg){
        emit logMessage(usernameVal.toString() + ":" + msg );
        userDisconnected(sender, msg);
    };

    if(regOpened == true){

    if (   typeVal.isNull()
        || !typeVal.isString()
        || typeVal.toString() != "login"
        || usernameVal.isNull()
        || !usernameVal.isString()) {
        disconect(sender,"invalid authorization format");
        return;
    }

    const QString newUserName = usernameVal.toString().simplified();
    if (   newUserName.isEmpty()
        || !m_resource_controller.checkUserAccess(newUserName)) {
        disconect(sender,"the user does not have access");
        return;
    }

    for (const ServerWorker *worker : m_clients) {
        if (worker == sender)
            continue;
        if (worker->userName().compare(newUserName, Qt::CaseInsensitive) == 0) {
            disconect(sender,"duplicate username");
            return;
        }
    }

    sender->setUserName(newUserName);

    emit logMessage(newUserName + " conected" );

    sendJson(sender, SendingMessage{
                 .type = QStringLiteral("login"),
                 .status = 1
             }.toJsonObject());
    } else {
        disconect(sender, " authorization close");
    }
}

void ResourceServer::clearResources(){
    m_resource_controller.freeAllResources();
    emit logMessage("all resources are released");
}
