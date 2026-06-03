#include "BaseDatos.h"
#include "Logger.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

BaseDatos& BaseDatos::instancia() {
    static BaseDatos inst;
    return inst;
}

QString BaseDatos::hashPassword(const QString& password) {
    // SHA-256 en hexadecimal: lo que se guarda es el hash, nunca la clave.
    return QString::fromLatin1(
        QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
}

bool BaseDatos::abrir(const QString& ruta) {
    QSqlDatabase db = QSqlDatabase::contains("conexion_principal")
                          ? QSqlDatabase::database("conexion_principal")
                          : QSqlDatabase::addDatabase("QSQLITE", "conexion_principal");
    db.setDatabaseName(ruta);

    if (!db.open()) {
        m_ultimoError = db.lastError().text();
        Logger::instancia().registrar("[BaseDatos] No se pudo abrir " + ruta + ": " + m_ultimoError);
        return false;
    }

    if (!crearEsquema())
        return false;

    sembrarUsuarioPorDefecto();
    Logger::instancia().registrar("[BaseDatos] Base abierta: " + ruta);
    return true;
}

bool BaseDatos::crearEsquema() {
    QSqlDatabase db = QSqlDatabase::database("conexion_principal");
    QSqlQuery q(db);

    // Tablas seguras de crear con IF NOT EXISTS.
    const char* tablas[] = {
        "CREATE TABLE IF NOT EXISTS usuarios ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  usuario TEXT UNIQUE NOT NULL,"
        "  password_hash TEXT NOT NULL)",

        // Cada dibujo guardado lleva nombre y fecha/hora de creacion.
        "CREATE TABLE IF NOT EXISTS dibujos ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  nombre TEXT NOT NULL,"
        "  creado_en TEXT NOT NULL)",

        "CREATE TABLE IF NOT EXISTS puntos ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  trazo_id INTEGER NOT NULL,"
        "  orden INTEGER NOT NULL,"
        "  x REAL NOT NULL,"
        "  y REAL NOT NULL)"
    };

    for (const char* sql : tablas) {
        if (!q.exec(QString::fromLatin1(sql))) {
            m_ultimoError = q.lastError().text();
            Logger::instancia().registrar("[BaseDatos] Error creando esquema: " + m_ultimoError);
            return false;
        }
    }

    // Tabla trazos: ahora cada trazo pertenece a un dibujo (dibujo_id).
    // Detectamos si ya existia con el esquema viejo (sin dibujo_id) y migramos
    // preservando los datos para no perder dibujos previos.
    bool trazosExiste = false;
    bool tieneDibujoId = false;
    QSqlQuery info(db);
    info.exec("PRAGMA table_info(trazos)");
    while (info.next()) {
        trazosExiste = true;
        if (info.value(1).toString() == "dibujo_id")
            tieneDibujoId = true;
    }

    if (!trazosExiste) {
        if (!q.exec("CREATE TABLE trazos ("
                    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "  dibujo_id INTEGER NOT NULL,"
                    "  orden INTEGER NOT NULL,"
                    "  color TEXT NOT NULL,"
                    "  grosor INTEGER NOT NULL)")) {
            m_ultimoError = q.lastError().text();
            Logger::instancia().registrar("[BaseDatos] Error creando trazos: " + m_ultimoError);
            return false;
        }
    } else if (!tieneDibujoId) {
        // Migracion del esquema viejo: agregar la columna y reasignar los
        // trazos huerfanos a un dibujo "Dibujo importado".
        q.exec("ALTER TABLE trazos ADD COLUMN dibujo_id INTEGER");

        QSqlQuery cnt(db);
        cnt.exec("SELECT COUNT(*) FROM trazos");
        if (cnt.next() && cnt.value(0).toInt() > 0) {
            QSqlQuery ins(db);
            ins.prepare("INSERT INTO dibujos (nombre, creado_en) VALUES (?, ?)");
            ins.addBindValue("Dibujo importado");
            ins.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
            if (ins.exec()) {
                const qlonglong id = ins.lastInsertId().toLongLong();
                QSqlQuery upd(db);
                upd.prepare("UPDATE trazos SET dibujo_id = ? WHERE dibujo_id IS NULL");
                upd.addBindValue(id);
                upd.exec();
                Logger::instancia().registrar(
                    "[BaseDatos] Migracion: trazos previos movidos a 'Dibujo importado'");
            }
        }
    }
    return true;
}

void BaseDatos::sembrarUsuarioPorDefecto() {
    QSqlDatabase db = QSqlDatabase::database("conexion_principal");
    QSqlQuery q(db);
    q.exec("SELECT COUNT(*) FROM usuarios");
    if (q.next() && q.value(0).toInt() == 0) {
        QSqlQuery ins(db);
        ins.prepare("INSERT INTO usuarios (usuario, password_hash) VALUES (?, ?)");
        ins.addBindValue("admin");
        ins.addBindValue(hashPassword("1234"));
        if (ins.exec())
            Logger::instancia().registrar("[BaseDatos] Usuario por defecto creado: admin");
        else
            Logger::instancia().registrar("[BaseDatos] Error sembrando usuario: " + ins.lastError().text());
    }
}

bool BaseDatos::validarUsuario(const QString& usuario, const QString& password) {
    QSqlDatabase db = QSqlDatabase::database("conexion_principal");
    QSqlQuery q(db);
    q.prepare("SELECT password_hash FROM usuarios WHERE usuario = ?");
    q.addBindValue(usuario);
    if (!q.exec() || !q.next())
        return false;

    const QString hashGuardado = q.value(0).toString();
    return hashGuardado == hashPassword(password);
}

int BaseDatos::guardarDibujo(const QList<Trazo>& trazos, const QString& nombre) {
    QSqlDatabase db = QSqlDatabase::database("conexion_principal");

    if (!db.transaction()) {
        m_ultimoError = db.lastError().text();
        return -1;
    }

    // 1) Crear la fila del dibujo con su nombre y fecha/hora.
    const QString creado = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QSqlQuery insD(db);
    insD.prepare("INSERT INTO dibujos (nombre, creado_en) VALUES (?, ?)");
    insD.addBindValue(nombre);
    insD.addBindValue(creado);
    if (!insD.exec()) {
        m_ultimoError = insD.lastError().text();
        db.rollback();
        return -1;
    }
    const qlonglong dibujoId = insD.lastInsertId().toLongLong();

    // 2) Insertar los trazos del dibujo y, por cada uno, sus puntos.
    for (int i = 0; i < trazos.size(); ++i) {
        const Trazo& t = trazos.at(i);

        QSqlQuery insT(db);
        insT.prepare("INSERT INTO trazos (dibujo_id, orden, color, grosor) VALUES (?, ?, ?, ?)");
        insT.addBindValue(dibujoId);
        insT.addBindValue(i);
        insT.addBindValue(t.color);
        insT.addBindValue(t.grosor);
        if (!insT.exec()) {
            m_ultimoError = insT.lastError().text();
            db.rollback();
            return -1;
        }
        const qlonglong trazoId = insT.lastInsertId().toLongLong();

        for (int j = 0; j < t.puntos.size(); ++j) {
            const Punto& p = t.puntos.at(j);
            QSqlQuery insP(db);
            insP.prepare("INSERT INTO puntos (trazo_id, orden, x, y) VALUES (?, ?, ?, ?)");
            insP.addBindValue(trazoId);
            insP.addBindValue(j);
            insP.addBindValue(p.x);
            insP.addBindValue(p.y);
            if (!insP.exec()) {
                m_ultimoError = insP.lastError().text();
                db.rollback();
                return -1;
            }
        }
    }

    if (!db.commit()) {
        m_ultimoError = db.lastError().text();
        db.rollback();
        return -1;
    }

    Logger::instancia().registrar(
        QString("[BaseDatos] Dibujo guardado: '%1' (%2 trazos) id=%3")
            .arg(nombre).arg(trazos.size()).arg(dibujoId));
    return static_cast<int>(dibujoId);
}

QList<DibujoInfo> BaseDatos::listarDibujos() {
    QList<DibujoInfo> resultado;
    QSqlDatabase db = QSqlDatabase::database("conexion_principal");

    QSqlQuery q(db);
    if (!q.exec("SELECT id, nombre, creado_en FROM dibujos ORDER BY creado_en DESC, id DESC")) {
        m_ultimoError = q.lastError().text();
        return resultado;
    }
    while (q.next()) {
        DibujoInfo d;
        d.id     = q.value(0).toInt();
        d.nombre = q.value(1).toString();
        d.creado = q.value(2).toString();
        resultado.append(d);
    }
    return resultado;
}

QList<Trazo> BaseDatos::cargarDibujo(int dibujoId) {
    QList<Trazo> resultado;
    QSqlDatabase db = QSqlDatabase::database("conexion_principal");

    QSqlQuery qt(db);
    qt.prepare("SELECT id, color, grosor FROM trazos WHERE dibujo_id = ? ORDER BY orden ASC");
    qt.addBindValue(dibujoId);
    if (!qt.exec()) {
        m_ultimoError = qt.lastError().text();
        return resultado;
    }

    while (qt.next()) {
        const qlonglong trazoId = qt.value(0).toLongLong();
        Trazo t;
        t.color  = qt.value(1).toString();
        t.grosor = qt.value(2).toInt();

        QSqlQuery qp(db);
        qp.prepare("SELECT x, y FROM puntos WHERE trazo_id = ? ORDER BY orden ASC");
        qp.addBindValue(trazoId);
        if (qp.exec()) {
            while (qp.next()) {
                Punto p;
                p.x = qp.value(0).toDouble();
                p.y = qp.value(1).toDouble();
                t.puntos.append(p);
            }
        }
        resultado.append(t);
    }

    Logger::instancia().registrar(
        QString("[BaseDatos] Dibujo id=%1 cargado: %2 trazos").arg(dibujoId).arg(resultado.size()));
    return resultado;
}

bool BaseDatos::eliminarDibujo(int dibujoId) {
    QSqlDatabase db = QSqlDatabase::database("conexion_principal");

    if (!db.transaction()) {
        m_ultimoError = db.lastError().text();
        return false;
    }
    QSqlQuery del(db);
    // Borrar primero los puntos de los trazos del dibujo, luego trazos y el dibujo.
    del.prepare("DELETE FROM puntos WHERE trazo_id IN (SELECT id FROM trazos WHERE dibujo_id = ?)");
    del.addBindValue(dibujoId);
    if (!del.exec()) { m_ultimoError = del.lastError().text(); db.rollback(); return false; }

    QSqlQuery delT(db);
    delT.prepare("DELETE FROM trazos WHERE dibujo_id = ?");
    delT.addBindValue(dibujoId);
    if (!delT.exec()) { m_ultimoError = delT.lastError().text(); db.rollback(); return false; }

    QSqlQuery delD(db);
    delD.prepare("DELETE FROM dibujos WHERE id = ?");
    delD.addBindValue(dibujoId);
    if (!delD.exec()) { m_ultimoError = delD.lastError().text(); db.rollback(); return false; }

    if (!db.commit()) { m_ultimoError = db.lastError().text(); db.rollback(); return false; }
    Logger::instancia().registrar(QString("[BaseDatos] Dibujo id=%1 eliminado").arg(dibujoId));
    return true;
}
