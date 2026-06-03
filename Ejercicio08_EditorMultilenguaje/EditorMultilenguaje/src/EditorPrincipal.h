#pragma once
#include <QMainWindow>
#include <memory>
#include "Modelos.h"
#include "Pantalla.h"
#include "ValidadorSintaxis.h"

class QComboBox;
class QLabel;
class QPushButton;
class EditorTexto;

// Pantalla principal del editor. Hereda de QMainWindow y de Pantalla.
// Centraliza el editor, el selector de lenguaje y el panel lateral del CV.
class EditorPrincipal : public QMainWindow, public Pantalla {
    Q_OBJECT
public:
    explicit EditorPrincipal(const ConfigApp& config, QWidget* parent = nullptr);

    void    inicializarUI()                              override;
    void    conectarEventos()                            override;
    void    cargarDatos()                                override;
    bool    validarEstado()                              override;
    void    registrarEvento(const QString& descripcion)  override;
    QString nombrePantalla() const                       override { return "EditorPrincipal"; }

protected:
    void keyPressEvent(QKeyEvent* ev) override;
    void mousePressEvent(QMouseEvent* ev) override;
    void resizeEvent(QResizeEvent* ev) override;
    void closeEvent(QCloseEvent* ev) override;
    void focusInEvent(QFocusEvent* ev) override;
    void focusOutEvent(QFocusEvent* ev) override;

private slots:
    void onLenguajeCambiado(const QString& lenguaje);
    void onExportarJpg();
    void onSalir();
    void onValidacionLinea(int numeroLinea, bool valida, const QString& mensaje);

private:
    QWidget* construirPanelCv();
    void     aplicarValidador(const QString& lenguaje);

    ConfigApp                          m_config;
    EditorTexto*                       m_editor       = nullptr;
    QComboBox*                         m_cmbLenguaje  = nullptr;
    QPushButton*                       m_btnExportar  = nullptr;
    QPushButton*                       m_btnSalir     = nullptr;
    QLabel*                            m_lblDiagnostico = nullptr;

    // Validador polimorfico: la clase no sabe cual valida concretamente.
    std::unique_ptr<ValidadorSintaxis> m_validador;
};
