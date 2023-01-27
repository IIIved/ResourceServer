#include "resource_controller.h"

#include <QDateTime>

#include <algorithm>

TResourceController::TResourceController(const TConfig& config)
    : m_config(config)
{
    m_resourcesData.resize(m_config.resourcesCount);
}

bool TResourceController::checkUserAccess(const QString& userName) {
    const auto& usersList = m_config.usersList;

    const auto it = std::find_if(usersList.begin(), usersList.end(), [userName](const auto& value){
        return value == userName;
    });

    return it !=  usersList.end();
}

std::vector<qint32> TResourceController::resourceListByMask(qint32 mask) const
{
    std::vector<qint32> list;

    for(qint32 i = 0; i < 31; i++) {

       if( mask & 1 << i) {
          list.push_back(i);
       }
    }

    return list;
}

void TResourceController::updateResources()
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


TResourceController::EResourceResponses TResourceController::reserveResource(const QString& userName, const quint32 idx, const qint64 lifeTimeMSecs) {
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

void TResourceController::freeClientResources(const QString& userName)
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

void TResourceController::freeAllResources()
{
    for(quint32 idx = 0; idx < m_resourcesData.size(); idx++) {
            auto& data = m_resourcesData[idx];
            data = TResourceData();
        }
}
