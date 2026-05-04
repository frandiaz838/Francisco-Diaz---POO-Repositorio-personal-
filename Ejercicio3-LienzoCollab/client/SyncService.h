#pragma once

#include "StrokeTypes.h"

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class QWebSocket;
class DrawingModel;

class SyncService : public QObject
{
    Q_OBJECT
public:
    explicit SyncService(QObject *parent = nullptr);
    ~SyncService() override;

    void setBaseHttpUrl(const QUrl &url);
    QUrl baseHttpUrl() const { return m_httpBase; }

    void attachModel(DrawingModel *model);
    void connectWebSocket();
    void fetchCanvas();
    void saveCanvas();

    void publishLocalStroke(const Stroke &stroke);
    bool isRealtimeConnected() const { return m_wsConnected; }

signals:
    void statusMessage(const QString &text);
    void saveCompleted(bool ok);

private slots:
    void onHttpFinished(QNetworkReply *reply);
    void onWsTextMessage(const QString &message);
    void onWsConnected();
    void onWsDisconnected();

private:
    QUrl wsUrlFromHttp(const QUrl &http) const;
    void handleCanvasGet(const QJsonObject &obj);
    void handleSaveResponse(const QByteArray &body, bool isRetry);
    void sendSaveRequest(bool isRetry);

    QNetworkAccessManager *m_nam = nullptr;
    QWebSocket *m_ws = nullptr;
    DrawingModel *m_model = nullptr;
    QUrl m_httpBase;
    QStringList m_lastSaveStrokeIds;
    bool m_wsConnected = false;
};