#include "config.h"

#include <QSettings>
#include <QPointer>

TConfig LoadConfig(const QString& fileName) {
    QPointer<QSettings> settings = new QSettings(fileName, QSettings::IniFormat);
    return TConfig {
        .usersList = settings->value("SERVER_SETTING/user_white_list").toStringList(),
        .resourcesCount = settings->value("SERVER_SETTING/resources_count").toInt(),
        .port = settings->value("SERVER_SETTING/port").toInt(),
    };
}
