#include "MainWindow.h"
#include "DrawingModel.h"
#include "DrawingView.h"
#include "SyncService.h"

#include <QLabel>
#include <QPushButton>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QUuid>
#include <QUrl>

void MainWindow::startNetworking(const QUrl &baseHttpUrl)
{
    m_sync->setBaseHttpUrl(baseHttpUrl);
    if (m_statusLabel)
        m_statusLabel->setText(tr("Servidor: %1").arg(baseHttpUrl.toString(QUrl::RemovePassword)));

    m_sync->fetchCanvas();
    m_sync->connectWebSocket();
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle(tr("Lienzo colaborativo"));
    resize(1024, 720);

    m_clientId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_model = new DrawingModel(this);
    m_view = new DrawingView(this);
    m_view->setModel(m_model);
    m_view->setClientId(m_clientId);

    m_sync = new SyncService(this);
    m_sync->attachModel(m_model);

    auto *central = new QWidget(this);
    auto *lay = new QVBoxLayout(central);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    m_statusLabel = new QLabel(tr("Listo"), central);
    m_statusLabel->setStyleSheet(QStringLiteral("padding:4px 8px;background:#f0f0f0;"));
    lay->addWidget(m_view);
    lay->addWidget(m_statusLabel);
    setCentralWidget(central);

    auto *tb = addToolBar(tr("Principal"));
    tb->setMovable(false);
    tb->setFloatable(false);
    tb->setIconSize(QSize(24, 24));

    auto *saveBtn = new QPushButton(tr("Guardar"), this);
    saveBtn->setCursor(Qt::PointingHandCursor);
    saveBtn->setMinimumHeight(36);
    saveBtn->setMinimumWidth(120);
    saveBtn->setStyleSheet(QStringLiteral(
        "QPushButton {"
        "  background-color: #0078D4;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 2px;"
        "  padding: 8px 20px;"
        "  font-family: 'Segoe UI', sans-serif;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #106EBE; }"
        "QPushButton:pressed { background-color: #005A9E; }"
        "QPushButton:disabled { background-color: #a6a6a6; color: #f0f0f0; }"));
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveClicked);
    tb->addWidget(saveBtn);

    connect(m_view, &DrawingView::localStrokeFinished, this, &MainWindow::onLocalStrokeFinished);
    connect(m_sync, &SyncService::statusMessage, this, &MainWindow::onStatusMessage);
}

void MainWindow::onLocalStrokeFinished(const Stroke &stroke)
{
    m_model->addStrokeFromLocal(stroke);
    m_sync->publishLocalStroke(stroke);

    // En modo WebSocket puro, el trazo se considera sincronizado al publicarlo.
    m_model->markStrokesSynced(QStringList{stroke.id});
}

void MainWindow::onSaveClicked()
{
    m_sync->saveCanvas();
}

void MainWindow::onStatusMessage(const QString &text)
{
    if (m_statusLabel)
        m_statusLabel->setText(text);
}