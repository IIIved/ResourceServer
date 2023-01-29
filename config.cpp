#include "config.h"

#include <QSettings>
#include <QPointer>

Config LoadConfig(const QString& fileName) {
    QPointer<QSettings> settings = new QSettings(fileName, QSettings::IniFormat);
    return Config {
        .usersList = settings->value("SERVER_SETTING/user_white_list").toStringList(),
        .resourcesCount = settings->value("SERVER_SETTING/resources_count").toInt(),
        .port = settings->value("SERVER_SETTING/port").toInt(),
    };
}
