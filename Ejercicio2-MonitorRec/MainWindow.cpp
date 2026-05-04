#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCloseEvent>
#include <QDateTime>
#include <QJsonValue>
#include <QFont>
#include <cmath>
#include <limits>

namespace {

double jsonNumero(const QJsonObject &o, const QStringList &claves, bool *ok = nullptr) {
    if (ok)
        *ok = false;
    for (const QString &c : claves) {
        if (!o.contains(c))
            continue;
        const QJsonValue v = o.value(c);
        if (v.isDouble()) {
            if (ok)
                *ok = true;
            return v.toDouble();
        }
        if (v.isString()) {
            bool conv = false;
            const double x = v.toString().toDouble(&conv);
            if (conv) {
                if (ok)
                    *ok = true;
                return x;
            }
        }
    }
    return std::numeric_limits<double>::quiet_NaN();
}

QString jsonTexto(const QJsonObject &o, const QStringList &claves) {
    for (const QString &c : claves) {
        if (!o.contains(c))
            continue;
        const QJsonValue v = o.value(c);
        if (v.isString() && !v.toString().isEmpty())
            return v.toString();
        if (v.isDouble())
            return QString::number(v.toDouble(), 'f', 1);
    }
    return QString();
}

bool statusRemotoIndicaAlerta(const QString &s) {
    const QString t = s.trimmed().toLower();
    return t.contains("warn") || t.contains("alert") || t.contains("crit") ||
           t.contains("degrad") || t.contains("error") || t == "bad" ||
           t == "down";
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Panel de monitoreo VPS");
    setMinimumSize(520, 640);

    controller = new MonitorController(this);

    txtUrl = new QLineEdit("http://167.86.77.220/salud.php");
    spinIntervalo = new QSpinBox();
    spinIntervalo->setRange(1, 300);
    spinIntervalo->setValue(5);
    spinIntervalo->setSuffix(" s");

    spinUmbralCpu = new QSpinBox();
    spinUmbralCpu->setRange(1, 99);
    spinUmbralCpu->setValue(80);
    spinUmbralCpu->setSuffix(" %");
    spinUmbralCpu->setToolTip(
        "Si la CPU supera este valor, el panel pasa a estado ALERTA (estilo monitor).");

    btnConectar = new QPushButton("Refrescar ahora / aplicar intervalo");
    btnConectar->setMinimumHeight(36);

    btnDetener = new QPushButton("Detener sondeo");
    btnDetener->setToolTip("Detiene el temporizador; no cancela una descarga ya en curso.");
    btnLimpiarHistorial = new QPushButton("Limpiar historial");

    lblEstado = new QLabel("Estado: sin datos");
    lblEstado->setObjectName("lblEstado");
    QFont fEstado = lblEstado->font();
    fEstado.setPointSize(11);
    fEstado.setBold(true);
    lblEstado->setFont(fEstado);

    lblUltimoCheck = new QLabel("Último check: —");
    lblUltimoCheck->setStyleSheet("color: #555;");

    auto estiloMetrica = [](QLabel *l) {
        l->setMinimumHeight(28);
        l->setStyleSheet(
            "QLabel { background: #f4f6f8; border: 1px solid #d0d7de; "
            "border-radius: 6px; padding: 6px 10px; font-size: 13px; }");
    };

    lblCpu = new QLabel("CPU: —");
    lblCarga = new QLabel("Carga (load): —");
    lblMemoria = new QLabel("Memoria: —");
    lblDisco = new QLabel("Disco: —");
    lblUptime = new QLabel("Uptime: —");
    for (QLabel *m : {lblCpu, lblCarga, lblMemoria, lblDisco, lblUptime})
        estiloMetrica(m);

    lblCpu->setStyleSheet(
        "QLabel { background: #f4f6f8; border: 1px solid #d0d7de; "
        "border-radius: 6px; padding: 8px 10px; font-size: 16px; font-weight: 600; }");

    txtHistorial = new QTextEdit();
    txtHistorial->setReadOnly(true);
    txtHistorial->setPlaceholderText("Aquí verás el historial corto de chequeos y fallos…");
    txtHistorial->document()->setMaximumBlockCount(250);

    QGroupBox *grpCfg = new QGroupBox("Configuración");
    QFormLayout *formCfg = new QFormLayout(grpCfg);
    formCfg->addRow("URL del endpoint:", txtUrl);
    formCfg->addRow("Intervalo de sondeo:", spinIntervalo);
    formCfg->addRow("Umbral de alerta (CPU):", spinUmbralCpu);

    QGroupBox *grpMet = new QGroupBox("Métricas");
    QGridLayout *grid = new QGridLayout(grpMet);
    grid->addWidget(lblCpu, 0, 0, 1, 2);
    grid->addWidget(lblCarga, 1, 0);
    grid->addWidget(lblMemoria, 1, 1);
    grid->addWidget(lblDisco, 2, 0);
    grid->addWidget(lblUptime, 2, 1);

    QGroupBox *grpHist = new QGroupBox("Historial de eventos");
    QVBoxLayout *layHist = new QVBoxLayout(grpHist);
    layHist->addWidget(txtHistorial);

    QWidget *central = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(grpCfg);

    QHBoxLayout *layAcciones = new QHBoxLayout();
    layAcciones->addWidget(btnConectar, 1);
    layAcciones->addWidget(btnDetener);
    layAcciones->addWidget(btnLimpiarHistorial);
    mainLayout->addLayout(layAcciones);

    mainLayout->addWidget(lblEstado);
    mainLayout->addWidget(lblUltimoCheck);
    mainLayout->addWidget(grpMet);
    mainLayout->addWidget(grpHist, 1);

    setCentralWidget(central);

    connect(btnConectar, &QPushButton::clicked, this, [this]() {
        const QString url = txtUrl->text().trimmed();
        if (url.isEmpty()) {
            txtHistorial->append(QString("[%1] Aviso: URL vacía; no se inicia el sondeo.")
                                     .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
            controller->detener();
            return;
        }
        controller->configurar(url, spinIntervalo->value());
        controller->solicitarDatos();
    });

    connect(btnDetener, &QPushButton::clicked, this, [this]() {
        controller->detener();
        txtHistorial->append(QString("[%1] Sondeo detenido.")
                                 .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    });

    connect(btnLimpiarHistorial, &QPushButton::clicked, txtHistorial, &QTextEdit::clear);

    connect(controller, &MonitorController::datosRecibidos, this,
            &MainWindow::actualizarInterfaz);
    connect(controller, &MonitorController::errorOcurrido, this, &MainWindow::mostrarError);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    controller->detener();
    QMainWindow::closeEvent(event);
}

QString MainWindow::formatearPorcentaje(double v) {
    if (std::isnan(v))
        return "—";
    return QString("%1%").arg(v, 0, 'f', 1);
}

QString MainWindow::formatearTextoMetrica(const QJsonObject &json,
                                          const QStringList &claves) {
    bool ok = false;
    const double n = jsonNumero(json, claves, &ok);
    if (ok)
        return formatearPorcentaje(n);
    const QString t = jsonTexto(json, claves);
    return t.isEmpty() ? QString("—") : t;
}

void MainWindow::aplicarEstadoPanel(const QString &clave, const QString &texto) {
    lblEstado->setText(texto);
    QString color;
    QString borde;
    if (clave == "ok") {
        color = "#0f5132";
        borde = "#badbcc";
    } else if (clave == "alerta") {
        color = "#664d03";
        borde = "#ffecb5";
    } else if (clave == "caido") {
        color = "#842029";
        borde = "#f5c2c7";
    } else {
        color = "#41464b";
        borde = "#dee2e6";
    }
    lblEstado->setStyleSheet(QString(
        "QLabel#lblEstado { color: %1; background: %2; border: 1px solid %3; "
        "border-radius: 8px; padding: 10px 12px; }")
                                 .arg(color, borde + "55", borde));
}

void MainWindow::actualizarInterfaz(const QJsonObject &json) {
    const QDateTime ahora = QDateTime::currentDateTime();
    const QString ts = ahora.toString("yyyy-MM-dd hh:mm:ss");
    lblUltimoCheck->setText("Último check: " + ts + " (hora local)");

    bool cpuOk = false;
    const double cpu = jsonNumero(json, {"cpu", "cpu_usage", "cpu_percent"}, &cpuOk);
    const int umbral = spinUmbralCpu->value();

    const QString statusRaw =
        jsonTexto(json, {"status", "estado", "health", "state"}).trimmed();
    const QString statusLower = statusRaw.toLower();

    const bool alertaPorCpu = cpuOk && cpu >= umbral;
    const bool alertaPorServidor =
        statusRemotoIndicaAlerta(statusRaw) ||
        (statusLower == "degraded") || statusLower.contains("high");

    QString claveEstado;
    QString textoEstado;
    if (alertaPorCpu || alertaPorServidor) {
        claveEstado = "alerta";
        textoEstado =
            alertaPorCpu
                ? QString("Estado: ALERTA (CPU ≥ %1%)").arg(umbral)
                : QString("Estado: ALERTA (%1)").arg(statusRaw.isEmpty() ? "servidor" : statusRaw);
    } else {
        claveEstado = "ok";
        textoEstado = statusRaw.isEmpty()
                          ? QString("Estado: OK")
                          : QString("Estado: OK — %1").arg(statusRaw);
    }
    aplicarEstadoPanel(claveEstado, textoEstado);

    lblCpu->setText(QString("CPU: %1").arg(
        formatearPorcentaje(cpuOk ? cpu : std::numeric_limits<double>::quiet_NaN())));
    {
        QString ss =
            "QLabel { border-radius: 6px; padding: 8px 10px; font-size: 16px; font-weight: 600; ";
        if (!cpuOk)
            ss += "background: #f4f6f8; border: 1px solid #d0d7de; color: #57606a; }";
        else if (cpu >= umbral)
            ss += "background: #fff5f5; border: 1px solid #f85149; color: #cf222e; }";
        else if (cpu >= umbral * 0.75)
            ss += "background: #fff8c5; border: 1px solid #d4a72c; color: #7d4e00; }";
        else
            ss += "background: #dafbe1; border: 1px solid #2da44e; color: #116329; }";
        lblCpu->setStyleSheet(ss);
    }

    {
        bool loadOk = false;
        const double load =
            jsonNumero(json, {"load", "load1", "load_1", "loadavg", "load_average"}, &loadOk);
        if (loadOk && !std::isnan(load))
            lblCarga->setText(QString("Carga (load): %1").arg(load, 0, 'f', 2));
        else {
            const QString t = jsonTexto(json, {"load", "load1", "loadavg"}).trimmed();
            lblCarga->setText(t.isEmpty() ? QString("Carga (load): —")
                                          : QString("Carga (load): %1").arg(t));
        }
    }
    lblMemoria->setText(
        "Memoria: " + formatearTextoMetrica(json, {"mem", "memory", "ram", "mem_used_pct"}));
    lblDisco->setText(
        "Disco: " + formatearTextoMetrica(json, {"disk", "disk_usage", "disco", "df"}));

    const QString upHuman =
        jsonTexto(json, {"uptime", "up", "tiempo", "uptime_human"}).trimmed();
    if (!upHuman.isEmpty()) {
        lblUptime->setText("Uptime: " + upHuman);
    } else {
        bool upOk = false;
        const double sec =
            jsonNumero(json, {"uptime_sec", "uptime_seconds"}, &upOk);
        if (upOk && !std::isnan(sec))
            lblUptime->setText(QString("Uptime: %1 s").arg(sec, 0, 'f', 0));
        else
            lblUptime->setText("Uptime: —");
    }

    txtHistorial->append(
        QString("[%1] OK — CPU %2 | status: %3")
            .arg(ahora.toString("hh:mm:ss"),
                 cpuOk ? QString::number(cpu, 'f', 1) + "%" : "n/d",
                 statusRaw.isEmpty() ? "n/d" : statusRaw));
}

void MainWindow::mostrarError(const QString &msg) {
    aplicarEstadoPanel("caido", "Estado: CAÍDO / sin respuesta válida");
    lblUltimoCheck->setText("Último check: falló a las " +
                            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    lblCpu->setText("CPU: —");
    lblCpu->setStyleSheet(
        "QLabel { background: #f4f6f8; border: 1px solid #d0d7de; "
        "border-radius: 6px; padding: 8px 10px; font-size: 16px; font-weight: 600; "
        "color: #57606a; }");
    lblCarga->setText("Carga (load): —");
    lblMemoria->setText("Memoria: —");
    lblDisco->setText("Disco: —");
    lblUptime->setText("Uptime: —");

    txtHistorial->append(QString("[%1] ERROR — %2")
                             .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), msg));
}
