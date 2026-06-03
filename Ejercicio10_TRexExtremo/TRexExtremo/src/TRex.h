#pragma once
#include <QWidget>

// El dinosaurio que controla el jugador. Hereda de QWidget.
// Maneja la fisica del salto, el agacharse y el desplazamiento
// horizontal (adelantarse / frenarse). El juego le aplica la fisica
// en cada frame del timer principal con aplicarFisica().
class TRex : public QWidget {
    Q_OBJECT
public:
    enum class Estado { Corriendo, Saltando, Agachado };

    explicit TRex(int alturaSuelo, QWidget* parent = nullptr);

    // Acciones del jugador.
    void saltar();
    void agacharse(bool activado);
    void adelantarse();
    void frenarse();

    // Avanza la fisica un paso (gravedad/salto). La llama el timer principal.
    void aplicarFisica();

    // Rectangulo de colision (en coordenadas de la escena).
    QRect cajaColision() const;

    // Vuelve al estado inicial (para reiniciar la partida).
    void reiniciar();

    Estado estado() const { return m_estado; }

protected:
    void paintEvent(QPaintEvent* ev) override;

private:
    void apoyarEnSuelo();        // recoloca el dino sobre el piso segun su alto

    int    m_alturaSuelo;        // y del piso de la escena
    int    m_xInicial = 60;
    double m_velY      = 0.0;    // velocidad vertical (salto)
    Estado m_estado    = Estado::Corriendo;
    int    m_pasoCorrida = 0;    // contador para animar las patas

    static constexpr int kAnchoNormal   = 44;
    static constexpr int kAltoNormal     = 48;
    static constexpr int kAnchoAgachado  = 62;
    static constexpr int kAltoAgachado   = 30;
    static constexpr double kGravedad    = 0.9;
    static constexpr double kImpulso     = -15.0;
    static constexpr int kPasoHorizontal = 3;   // por frame (mantener apretado)
};
