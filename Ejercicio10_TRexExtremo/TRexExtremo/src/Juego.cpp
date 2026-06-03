#include "Juego.h"

#include "Cactus.h"
#include "Obstaculo.h"
#include "Pajaro.h"
#include "TRex.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPushButton>
#include <QRandomGenerator>

Juego::Juego(QWidget* parent) : QWidget(parent) {
    setWindowTitle("T-Rex Extremo");
    setFixedSize(kAncho, kAlto);
    setFocusPolicy(Qt::StrongFocus);

    m_trex = new TRex(kAlturaSuelo, this);

    // Boton de Inicio / Reinicio.
    m_boton = new QPushButton("▶  Iniciar", this);
    m_boton->setCursor(Qt::PointingHandCursor);
    m_boton->setStyleSheet(
        "QPushButton { background:#535353; color:white; border:none; border-radius:8px;"
        " padding:10px 28px; font-size:16px; font-weight:bold; }"
        " QPushButton:hover { background:#6b6b6b; }");
    m_boton->resize(180, 46);
    m_boton->move((kAncho - m_boton->width()) / 2, kAlto / 2 + 26);
    connect(m_boton, SIGNAL(clicked()), this, SLOT(onBotonStart()));

    // Conexiones de los QTimer en estilo viejo SIGNAL/SLOT (pedido de la consigna).
    connect(&m_principal,    SIGNAL(timeout()), this, SLOT(actualizar()));
    connect(&m_spawnPajaros, SIGNAL(timeout()), this, SLOT(aparecerPajaro()));

    mostrarInicio();
}

// ── Estados ──────────────────────────────────────────────────────────────
void Juego::mostrarInicio() {
    m_estado = Estado::Inicio;
    m_principal.stop();
    m_spawnPajaros.stop();
    limpiarObstaculos();
    m_puntaje    = 0;
    m_frames     = 0;
    m_velocidad  = 6;
    m_adelantando = false;
    m_frenando    = false;
    m_trex->reiniciar();
    m_trex->show();
    m_boton->setText("▶  Iniciar");
    m_boton->show();
    m_boton->raise();
    update();
}

void Juego::empezarPartida() {
    limpiarObstaculos();
    m_estado     = Estado::Jugando;
    m_frames     = 0;
    m_puntaje    = 0;
    m_velocidad  = 6;
    m_adelantando = false;
    m_frenando    = false;
    m_trex->reiniciar();
    m_trex->show();
    m_boton->hide();

    m_principal.start(16);        // ~60 fps
    m_spawnPajaros.start(5000);   // un pajaro cada 5 segundos
    programarCactus();            // primer grupo de cactus
    setFocus();
    update();
}

void Juego::pararTodo() {
    m_principal.stop();
    m_spawnPajaros.stop();
    for (Obstaculo* o : m_obstaculos) {
        if (auto* p = qobject_cast<Pajaro*>(o))
            p->detenerTimer();
    }
}

void Juego::limpiarObstaculos() {
    for (Obstaculo* o : m_obstaculos)
        o->deleteLater();
    m_obstaculos.clear();
}

void Juego::onBotonStart() {
    empezarPartida();
}

// ── Spawns ─────────────────────────────────────────────────────────────
void Juego::programarCactus() {
    if (m_estado != Estado::Jugando)
        return;
    // Separacion EN TIEMPO entre grupos de cactus. El minimo (700 ms) es mayor
    // que la duracion de un salto (~530 ms), asi siempre se alcanza a aterrizar
    // y volver a saltar: nunca quedan dos grupos a distancia imposible.
    // QTimer::singleShot reprograma el proximo grupo (evento unico).
    const int intervalo = QRandomGenerator::global()->bounded(700, 1500);
    QTimer::singleShot(intervalo, this, SLOT(aparecerCactus()));
}

void Juego::aparecerCactus() {
    if (m_estado != Estado::Jugando)
        return;
    // Un "grupo" de 1 a 3 cactus PEGADOS entre si: se ven como un obstaculo
    // ancho pero se pasa de un solo salto (limitamos el ancho total a ~80 px).
    const int cantidad = QRandomGenerator::global()->bounded(1, 4);   // 1..3
    int xActual    = kAncho;
    int anchoGrupo = 0;
    for (int i = 0; i < cantidad; ++i) {
        Cactus* c = new Cactus(m_velocidad, this);
        if (i > 0 && anchoGrupo + c->width() > 80) {   // no agrandar de mas
            c->deleteLater();
            break;
        }
        c->move(xActual, kAlturaSuelo - c->height());
        c->show();
        m_obstaculos.append(c);
        xActual    += c->width();   // pegado al anterior
        anchoGrupo += c->width();
    }
    programarCactus();   // agendar el proximo grupo
}

void Juego::aparecerPajaro() {
    if (m_estado != Estado::Jugando)
        return;
    // Cada pajaro nace con su PROPIO timer, a la misma velocidad que los cactus
    // (se mueve parejo con ellos).
    Pajaro* p = new Pajaro(m_velocidad, kAlturaSuelo, this);
    p->show();
    connect(p, SIGNAL(salioDePantalla(Pajaro*)), this, SLOT(onPajaroSalio(Pajaro*)));
    m_obstaculos.append(p);
}

void Juego::onPajaroSalio(Pajaro* quien) {
    m_obstaculos.removeOne(quien);
    quien->deleteLater();
}

// ── Timer principal ─────────────────────────────────────────────────────
void Juego::actualizar() {
    m_trex->aplicarFisica();

    // Avance/freno continuos mientras se mantiene la flecha apretada.
    if (m_adelantando) m_trex->adelantarse();
    if (m_frenando)    m_trex->frenarse();

    // Mover los cactus (los pajaros se mueven con su propio timer).
    QList<Obstaculo*> aBorrar;
    for (Obstaculo* o : m_obstaculos) {
        if (o->tipo() == "cactus") {
            o->avanzar();
            if (o->fueraDePantalla())
                aBorrar.append(o);
        }
    }
    for (Obstaculo* o : aBorrar) {
        m_obstaculos.removeOne(o);
        o->deleteLater();
    }

    // Deteccion de colisiones (polimorfica sobre Obstaculo*).
    const QRect cajaTrex = m_trex->cajaColision();
    for (Obstaculo* o : m_obstaculos) {
        if (cajaTrex.intersects(o->cajaColision())) {
            m_estado = Estado::GameOver;
            m_record = qMax(m_record, m_puntaje);
            pararTodo();
            m_boton->setText("↻  Reiniciar");
            m_boton->show();
            m_boton->raise();
            update();
            return;
        }
    }

    // Puntaje y velocidad.
    ++m_frames;
    m_puntaje = m_frames / 4;
    subirVelocidadSiCorresponde();
    update();
}

void Juego::subirVelocidadSiCorresponde() {
    // Sube de a 1 cada 200 puntos, hasta un maximo de 25.
    const int objetivo = qMin(6 + m_puntaje / 200, 25);
    if (objetivo != m_velocidad) {
        m_velocidad = objetivo;
        for (Obstaculo* o : m_obstaculos)
            o->setVelocidad(m_velocidad);   // cactus y pajaros, misma velocidad
    }
}

// ── Teclado ──────────────────────────────────────────────────────────────
void Juego::keyPressEvent(QKeyEvent* ev) {
    if (m_estado != Estado::Jugando) {
        // En Inicio o Game Over: Espacio / ↑ / R arrancan la partida.
        if (ev->key() == Qt::Key_Space || ev->key() == Qt::Key_Up || ev->key() == Qt::Key_R)
            empezarPartida();
        return;
    }

    switch (ev->key()) {
    case Qt::Key_Space:
    case Qt::Key_Up:     m_trex->saltar();            break;   // saltar (Espacio o ↑)
    case Qt::Key_Down:   m_trex->agacharse(true);     break;
    case Qt::Key_Right:  m_adelantando = true;        break;   // mantener apretado
    case Qt::Key_Left:   m_frenando    = true;        break;
    default:             QWidget::keyPressEvent(ev);  return;
    }
}

void Juego::keyReleaseEvent(QKeyEvent* ev) {
    if (ev->isAutoRepeat()) {
        QWidget::keyReleaseEvent(ev);
        return;
    }
    switch (ev->key()) {
    case Qt::Key_Down:   m_trex->agacharse(false);    break;
    case Qt::Key_Right:  m_adelantando = false;       break;
    case Qt::Key_Left:   m_frenando    = false;       break;
    default:             QWidget::keyReleaseEvent(ev); break;
    }
}

// ── Dibujo de la escena ───────────────────────────────────────────────────
void Juego::paintEvent(QPaintEvent* /*ev*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // Cielo.
    p.fillRect(rect(), QColor("#f7f7f7"));

    // Suelo.
    p.setPen(QPen(QColor("#535353"), 2));
    p.drawLine(0, kAlturaSuelo, kAncho, kAlturaSuelo);
    // Textura del piso (puntitos).
    p.setPen(QColor("#bdbdbd"));
    for (int x = (m_frames * 4) % 24; x < kAncho; x += 24)
        p.drawPoint(x, kAlturaSuelo + 8);

    // Puntaje y record arriba a la derecha.
    p.setPen(QColor("#535353"));
    p.setFont(QFont("Consolas", 13, QFont::Bold));
    const QString txt = QString("HI %1   %2")
                            .arg(m_record, 5, 10, QChar('0'))
                            .arg(m_puntaje, 5, 10, QChar('0'));
    p.drawText(QRect(0, 12, kAncho - 16, 24), Qt::AlignRight, txt);

    // Ayuda de controles abajo.
    p.setFont(QFont("Segoe UI", 8));
    p.setPen(QColor("#9e9e9e"));
    p.drawText(QRect(10, kAlto - 22, kAncho, 18), Qt::AlignLeft,
               "Espacio/↑: saltar   ↓: agacharse   →: adelantar   ←: frenar");

    // Pantalla de Inicio.
    if (m_estado == Estado::Inicio) {
        p.setPen(QColor("#535353"));
        p.setFont(QFont("Consolas", 30, QFont::Bold));
        p.drawText(QRect(0, kAlto / 2 - 70, kAncho, 44), Qt::AlignCenter, "T-REX  EXTREMO");
        p.setFont(QFont("Segoe UI", 11));
        p.drawText(QRect(0, kAlto / 2 - 20, kAncho, 24), Qt::AlignCenter,
                   "Esquivá cactus y pájaros");
    }

    // Overlay de Game Over.
    if (m_estado == Estado::GameOver) {
        p.fillRect(rect(), QColor(255, 255, 255, 180));
        p.setPen(QColor("#535353"));
        p.setFont(QFont("Consolas", 28, QFont::Bold));
        p.drawText(QRect(0, kAlto / 2 - 70, kAncho, 40), Qt::AlignCenter, "G A M E   O V E R");
        p.setFont(QFont("Segoe UI", 12));
        p.drawText(QRect(0, kAlto / 2 - 24, kAncho, 30), Qt::AlignCenter,
                   QString("Puntaje: %1     Récord: %2").arg(m_puntaje).arg(m_record));
    }
}
