#pragma once
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QPixmap>
#include "Modelos.h"

class ApiClima : public QObject {
    Q_OBJECT
public:
    explicit ApiClima(const ConfigApp& config, QObject* parent = nullptr);

    void obtenerClima();
    void descargarImagen(const QString& url, const QString& rutaCache);

signals:
    void climaActualizado(DatosClima datos);
    void imagenDescargada(QPixmap pixmap, QString rutaCache);
    void errorRed(QString mensaje);

private slots:
    // Slot unico que recibe todas las respuestas del manager (patron de clase)
    void slot_respuesta(QNetworkReply* reply);

private:
    void procesarClima(QNetworkReply* reply);
    void procesarImagen(QNetworkReply* reply, const QString& rutaCache);

    ConfigApp              m_config;
    QNetworkAccessManager* m_manager = nullptr;
};
