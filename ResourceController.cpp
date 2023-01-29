#include "ResourceController.h"

#include <QDateTime>

#include <algorithm>

ResourceController::ResourceController(const Config& config)
    : m_config(config)
{
    m_resourcesData.resize(m_config.resourcesCount);
}

bool ResourceController::checkUserAccess(const QString& userName) {
    const auto& usersList = m_config.usersList;

    const auto it = std::find_if(usersList.begin(), usersList.end(), [userName](const auto& value){
        return value == userName;
    });

    return it !=  usersList.end();
}

std::vector<qint32> ResourceController::resourceListByMask(qint32 mask) const
{
    std::vector<qint32> list;
    QByteArray bar;
    bar.resize(4);
    memcpy(bar.data(), &mask, sizeof(qint32));

    for(qint32 i = 0; i < 31; i++) {
       if(bar[i] > 0) {
          list.push_back(i);
       }
    }

    return list;
}

void ResourceController::updateResources()
{
    QMutexLocker ml(&m_mutex);
    const auto currentTimestamp = QDateTime::currentMSecsSinceEpoch();

    for(quint32 idx = 0; idx < m_resourcesData.size(); idx++) {
        auto& data = m_resourcesData[idx];
        if(data.satus != EResourceStatus::Free) {
            if(data.startTime + data.lifeTimeMSecs >= currentTimestamp) {
                continue;
            }

            if(!data.reservedByUser.isEmpty()) {
                emit resourceIsReleasedByServer(data.reservedByUser,idx);
            }

            data = TResourceData();
        }
    }
}


ResourceController::EResourceResponses ResourceController::reserveResource(const QString& userName, const quint32 idx, const qint64 lifeTimeMSecs) {
    QMutexLocker ml(&m_mutex);

    if(idx >= m_resourcesData.size()) {
        return EResourceResponses::InvalidResourceIndex;
    }

    if( !checkUserAccess(userName) ) {
        return EResourceResponses::UserNoAccess;
    }

    const auto currentTimestamp = QDateTime::currentMSecsSinceEpoch();
    auto& data = m_resourcesData[idx];


    if(    data.satus == EResourceStatus::Busy
        && data.reservedByUser == userName ) {
        data.startTime = currentTimestamp;
        return EResourceResponses::Success;
    }

    if(data.satus != EResourceStatus::Free) {
        if(data.startTime + data.lifeTimeMSecs >= currentTimestamp) {
            return EResourceResponses::ResourceIsBusyByAnotherUserPleaseWait;
        }
    }

    data.startTime = currentTimestamp;
    data.lifeTimeMSecs = lifeTimeMSecs;
    data.reservedByUser = userName;
    data.satus = EResourceStatus::Busy;

    return EResourceResponses::Success;
}

void ResourceController::freeClientResources(const QString& userName)
{
    if(userName.isEmpty()) {
        return;
    }

    QMutexLocker ml(&m_mutex);
    for(quint32 idx = 0; idx < m_resourcesData.size(); idx++) {
        auto& data = m_resourcesData[idx];
        if(data.reservedByUser == userName) {
            emit resourceIsReleasedByServer(data.reservedByUser,idx);
        }
        data = TResourceData();
    }
}

void ResourceController::freeAllResources()
{
    for(quint32 idx = 0; idx < m_resourcesData.size(); idx++) {
            auto& data = m_resourcesData[idx];
            data = TResourceData();
        }
}
