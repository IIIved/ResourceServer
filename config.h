#ifndef CONFIG_H
#define CONFIG_H

#include <QStringList>

struct TConfig {
    QStringList usersList;
    qint32 resourcesCount;
    qint32 port;
};

TConfig LoadConfig(const QString& fileName);


#endif // CONFIG_H
