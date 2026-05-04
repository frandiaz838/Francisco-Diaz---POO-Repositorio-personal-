#pragma once
#include <QByteArray>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QString>

// Capa de red: se comunica con el backend FastAPI del VPS.
// Todos los metodos son async: cuando terminan emiten boardActualizado
// u operacionOk (o errorApi si falla).
//
// Autenticacion: Basic Auth nginx (poo:clavepoo) en cada request.
// SSL:           Certificado autofirmado — se ignoran errores SSL.
class ApiKanban : public QObject {
    Q_OBJECT
public:
    explicit ApiKanban(const QString& baseUrl,
                       const QString& basicUser,
                       const QString& basicPass,
                       QObject* parent = nullptr);

    void fetchBoard();

    void crearColumna(const QString& nombre);
    void editarColumna(int id, const QString& nombre);
    void eliminarColumna(int id);

    void crearTarjeta(int colId, const QString& titulo, const QString& desc);
    void editarTarjeta(int id, const QString& titulo, const QString& desc);
    void eliminarTarjeta(int id);
    void moverTarjeta(int cardId, int colId, int pos = 9999);

signals:
    void boardActualizado(QJsonArray board);
    void operacionOk(const QString& msg);
    void errorApi(const QString& msg);

private:
    QNetworkRequest prepararRequest(const QString& path) const;
    void            hacerDelete(const QString& path);

    QString                m_baseUrl;
    QByteArray             m_basicHeader;
    QNetworkAccessManager  m_nam;
};
