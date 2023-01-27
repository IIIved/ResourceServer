#include "server_worker.h"

#include <QDataStream>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

ServerWorker::ServerWorker(QObject *parent)
    : QObject(parent)
    , m_serverSocket(this)
{
    connect(&m_serverSocket, &QTcpSocket::readyRead, this, &ServerWorker::receiveJson);
    connect(&m_serverSocket, &QTcpSocket::disconnected, this, &ServerWorker::disconnectedFromServer);

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    connect(&m_serverSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &ServerWorker::error);
#else
    connect(&m_serverSocket, &QAbstractSocket::errorOccurred, this, &ServerWorker::error);
#endif
}


ServerWorker::~ServerWorker() {
}


bool ServerWorker::setSocketDescriptor(qintptr socketDescriptor)
{
    return m_serverSocket.setSocketDescriptor(socketDescriptor);
}

void ServerWorker::sendJson(const QJsonObject &json)
{
    const QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    //emit logMessage("Sending to " + userName() + " - " + QString::fromUtf8(jsonData));

    QDataStream socketStream(&m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_5_7);
    socketStream << jsonData;
}

void ServerWorker::disconnectFromServer()
{
    m_serverSocket.disconnectFromHost();
}

QString ServerWorker::userName() const
{
    return m_userName;
}

void ServerWorker::setUserName(const QString &userName)
{
    m_userName = userName;
}

void ServerWorker::receiveJson()
{
    QByteArray jsonData;
    QDataStream socketStream(&m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_5_7);

    for (;;) {
        socketStream.startTransaction();
        socketStream >> jsonData;
        if (socketStream.commitTransaction()) {
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject())
                    emit jsonReceived(jsonDoc.object());
                else
                    emit logMessage("Invalid message: " + QString::fromUtf8(jsonData));
            } else {
                emit logMessage("Invalid message: " + QString::fromUtf8(jsonData));
            }
        } else {
            break;
        }
    }
}
