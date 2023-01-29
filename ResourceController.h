#ifndef RESOURCECONTROLLER_H
#define RESOURCECONTROLLER_H

#include "config.h"

#include <QObject>
#include <QString>
#include <QMutex>
#include <QtPlugin>
#include <vector>

struct SendingMessage{
    QVariant type;
    QVariant username;
    QVariant message;
    QVariant status;
    QVariant resource;
    QVariant request;

    QJsonObject toJsonObject() const;
};

class IResourcesServer;

class ResourceController : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(ResourceController)

public:
    enum EResourceResponses {
        Success,
        InvalidResourceIndex,
        UserNoAccess,
        ResourceIsBusyByAnotherUserPleaseWait,
        ResourceIsReleasedByServer
    };

    enum EResourceStatus {
        Free,
        Busy
    };

    struct TResourceData {
        EResourceStatus satus = EResourceStatus::Free;
        qint64 startTime = 0;
        qint64 lifeTimeMSecs = 0;
        QString reservedByUser;
    };

    Q_ENUM(EResourceStatus)
    Q_ENUM(EResourceResponses)

public:
    ResourceController(const Config& config);

    bool checkUserAccess(const QString& userName);

    std::vector<qint32> resourceListByMask(qint32 mask) const;

    EResourceResponses reserveResource(const QString& userName, const quint32 idx, const qint64 lifeTimeMSecs);

    void freeClientResources(const QString& userName);
    void freeAllResources();

signals:
    void resourceIsReleasedByServer(const QString& userName, quint32 idx);

public slots:
    void updateResources();

private:
    QMutex m_mutex;

    std::vector<TResourceData> m_resourcesData;
    const Config& m_config;
};

#endif // RESOURCECONTROLLER_H
