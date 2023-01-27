#ifndef RESOURCECONTROLLER_H
#define RESOURCECONTROLLER_H

#include "config.h"

#include <QObject>
#include <QString>
#include <QMutex>

#include <vector>

class TResourceController : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(TResourceController)

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
    TResourceController(const TConfig& config);

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
    const TConfig& m_config;
};

#endif // RESOURCECONTROLLER_H
