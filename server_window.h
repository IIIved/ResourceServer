#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QWidget>

namespace Ui {
class ServerWindow;
}
class IResourcesServer;
class ServerWindow : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerWindow)
public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();
private:
    Ui::ServerWindow *ui;
    IResourcesServer *m_server;
private slots:
    void toggleStartServer();
    void logMessage(const QString &msg);
    void toggleReg();
    void toggleResRequests();
    void on_clearResources_clicked();
};

#endif // SERVERWINDOW_H
