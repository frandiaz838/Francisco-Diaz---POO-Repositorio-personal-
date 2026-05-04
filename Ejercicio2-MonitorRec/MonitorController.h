#ifndef MONITORCONTROLLER_H
#define MONITORCONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

class MonitorController : public QObject {
    Q_OBJECT
public:
    explicit MonitorController(QObject *parent = nullptr);
    void configurar(QString url, int intervaloSegundos);
    void solicitarDatos();
    void detener();

signals:
    void datosRecibidos(const QJsonObject &json);
    void errorOcurrido(const QString &msg);

private slots:
    void alFinalizarRespuesta(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    QTimer *timer;
    QString urlServidor;
    bool solicitudEnCurso;
};

#endif