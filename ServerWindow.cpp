#include "ServerWindow.h"
#include "ui_server_window.h"
#include "resourceserver.h"

#include <QMessageBox>
#include <QSettings>
#include <QDateTime>

ServerWindow::ServerWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServerWindow)
    , m_server(new ResourceServer(this))
{
    ui->setupUi(this);
    connect(ui->startStopButton, &QPushButton::clicked, this, &ServerWindow::toggleStartServer);
    connect(m_server, &ResourceServer::logMessage, this, &ServerWindow::logMessage);
    connect(ui->regButton, &QPushButton::clicked, this, &ServerWindow::toggleReg);
    connect(ui->noResRequests, &QPushButton::clicked, this, &ServerWindow::toggleResRequests);
}

ServerWindow::~ServerWindow()
{
    delete ui;
}

void ServerWindow::toggleStartServer()
{
    if (m_server->isListening()) {
        m_server->stopServer();
        ui->startStopButton->setText(tr("Start Server"));
        logMessage(QStringLiteral("Server Stopped"));
    } else {
        if (!m_server->listen(QHostAddress::Any, m_server->port())) {
            QMessageBox::critical(this, tr("Error"), tr("Unable to start the server"));
            return;
        }
        m_server->startServer();
        logMessage(QStringLiteral("Server Started"));
        ui->startStopButton->setText(tr("Stop Server"));
    }
}

void ServerWindow::logMessage(const QString &msg)
{
    const auto time = QDateTime::currentDateTime();
    ui->logEditor->appendPlainText(time.toString("hh:mm:ss") + ": " + msg);
}

void ServerWindow::toggleReg()
{
  if (m_server->regOpened)
    ui->regButton->setText(tr("Открыть регистрацию"));
  else
    ui->regButton->setText(tr("Закрыть регистрацию"));

  m_server->regOpened = !m_server->regOpened;

}

void ServerWindow::toggleResRequests()
{
    if (m_server->acceptResRequest)
      ui->noResRequests->setText(tr("Принимать новые запросы на ресурсы"));
    else
      ui->noResRequests->setText(tr("Отколнять новые запросы на ресурсы"));

    m_server->acceptResRequest = !m_server->acceptResRequest;
}

void ServerWindow::on_clearResources_clicked()
{
  m_server->clearResources();
}

