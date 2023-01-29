#ifndef CONFIG_H
#define CONFIG_H

#include <QStringList>

struct Config {
    QStringList usersList;
    qint32 resourcesCount;
    qint32 port;
};

Config LoadConfig(const QString& fileName);

#endif // CONFIG_H
