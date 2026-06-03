#pragma once
#include <QList>
#include <QTimer>
#include <QWidget>

class TRex;
class Obstaculo;
class Pajaro;
class QPushButton;

// Escena / campo de juego. Widget top-level que orquesta todo:
//  - timer principal: fisica del dino, movimiento de cactus, colisiones, puntaje y velocidad.
//  - timer de pajaros: cada 5 s aparece un pajaro (con su propio timer).
//  - cactus: aparecen con QTimer::singleShot a intervalos aleatorios.
class Juego : public QWidget {
    Q_OBJECT
public:
    explicit Juego(QWidget* parent = nullptr);

    enum class Estado { Inicio, Jugando, GameOver };

    static constexpr int kAncho       = 800;
    static constexpr int kAlto        = 300;
    static constexpr int kAlturaSuelo = 250;   // y del piso

protected:
    void paintEvent(QPaintEvent* ev)    override;
    void keyPressEvent(QKeyEvent* ev)   override;
    void keyReleaseEvent(QKeyEvent* ev) override;

private slots:
    void actualizar();        // timer principal (~16 ms)
    void aparecerPajaro();    // timer de pajaros (5 s)
    void aparecerCactus();    // disparado por singleShot
    void onPajaroSalio(Pajaro* quien);
    void onBotonStart();      // click del boton de inicio/reinicio

private:
    void mostrarInicio();     // pantalla inicial (con boton)
    void empezarPartida();    // arranca/reinicia el juego
    void pararTodo();
    void limpiarObstaculos();
    void programarCactus();   // reprograma el proximo grupo con singleShot
    void subirVelocidadSiCorresponde();

    QTimer m_principal;
    QTimer m_spawnPajaros;

    TRex*              m_trex   = nullptr;
    QPushButton*       m_boton  = nullptr;
    QList<Obstaculo*>  m_obstaculos;

    Estado m_estado     = Estado::Inicio;
    int    m_frames      = 0;
    int    m_puntaje     = 0;
    int    m_record      = 0;
    int    m_velocidad   = 6;
    bool   m_adelantando = false;   // flecha derecha mantenida
    bool   m_frenando    = false;   // flecha izquierda mantenida
};
