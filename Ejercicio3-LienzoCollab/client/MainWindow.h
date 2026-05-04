#pragma once

#include "StrokeTypes.h"

#include <QMainWindow>
#include <QUrl>

class DrawingModel;
class DrawingView;
class SyncService;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void startNetworking(const QUrl &baseHttpUrl);

private slots:
    void onLocalStrokeFinished(const Stroke &stroke);
    void onSaveClicked();
    void onStatusMessage(const QString &text);

private:
    DrawingModel *m_model = nullptr;
    DrawingView *m_view = nullptr;
    SyncService *m_sync = nullptr;
    QLabel *m_statusLabel = nullptr;
    QString m_clientId;
};