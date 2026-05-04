#include "mainwidget.h"

#include "actionlog.h"
#include "edittrabajodialog.h"
#include "loginwidget.h"
#include "notasdialog.h"
#include "sessionmanager.h"

#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDir>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QTextCursor>
#include <QTimer>
#include <QUuid>
#include <QVBoxLayout>

MainWidget::MainWidget(const QString &usuario, QWidget *parent)
    : QWidget(parent)
    , m_usuario(usuario)
    , m_store(QDir(SessionManager::dataDirPath()).filePath(QStringLiteral("trabajos.json")))
{
    m_store.load();
    construirUi();
    refreshHistorialUi();
    rebuildGrid();
    scheduleSessionExpiry();

    log(QStringLiteral("APERTURA_PANEL"));
}

MainWidget::~MainWidget() = default;

void MainWidget::closeEvent(QCloseEvent *event)
{
    if (m_cierrePorExpiracion) {
        QWidget::closeEvent(event);
        return;
    }
    QWidget::closeEvent(event);
    QApplication::quit();
}

void MainWidget::construirUi()
{
    setWindowTitle(QStringLiteral("Planificador de trabajos prácticos"));

    auto *titulo = new QLabel(QStringLiteral("Usuario: %1").arg(m_usuario));
    QFont f = titulo->font();
    f.setBold(true);
    titulo->setFont(f);

    m_btnAgregar = new QPushButton(QStringLiteral("➕ Nuevo trabajo práctico"));

    m_filtroEstado = new QComboBox;
    m_filtroEstado->blockSignals(true);
    m_filtroEstado->addItem(QStringLiteral("Estado: (todos)"), QString());
    m_filtroEstado->addItem(QStringLiteral("Pendiente"), QStringLiteral("Pendiente"));
    m_filtroEstado->addItem(QStringLiteral("En curso"), QStringLiteral("En curso"));
    m_filtroEstado->addItem(QStringLiteral("Entregado"), QStringLiteral("Entregado"));
    m_filtroEstado->blockSignals(false);

    m_filtroPrioridad = new QComboBox;
    m_filtroPrioridad->blockSignals(true);
    m_filtroPrioridad->addItem(QStringLiteral("Prioridad: (todas)"), QString());
    m_filtroPrioridad->addItem(QStringLiteral("Alta"), QStringLiteral("Alta"));
    m_filtroPrioridad->addItem(QStringLiteral("Media"), QStringLiteral("Media"));
    m_filtroPrioridad->addItem(QStringLiteral("Baja"), QStringLiteral("Baja"));
    m_filtroPrioridad->blockSignals(false);

    auto *filtros = new QHBoxLayout;
    filtros->addWidget(new QLabel(QStringLiteral("Filtros:")));
    filtros->addWidget(m_filtroEstado, 1);
    filtros->addWidget(m_filtroPrioridad, 1);

    m_grid = new QGridLayout;
    m_grid->setColumnStretch(0, 2);
    m_grid->setColumnStretch(1, 1);
    m_grid->setColumnStretch(2, 1);

    auto *gridHost = new QWidget;
    gridHost->setLayout(m_grid);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(gridHost);

    m_historialUi = new QPlainTextEdit;
    m_historialUi->setReadOnly(true);
    m_historialUi->setMaximumBlockCount(5000);
    m_historialUi->setPlaceholderText(QStringLiteral("Historial de acciones…"));

    auto *historialTitulo = new QLabel(QStringLiteral("Historial de acciones"));
    historialTitulo->setFont(f);

    auto *root = new QVBoxLayout(this);
    root->addWidget(titulo);
    root->addWidget(m_btnAgregar);
    root->addLayout(filtros);
    root->addWidget(scroll, 3);
    root->addWidget(historialTitulo);
    root->addWidget(m_historialUi, 1);

    connect(m_btnAgregar, &QPushButton::clicked, this, &MainWidget::agregarTrabajo);
    connect(m_filtroEstado, &QComboBox::currentIndexChanged, this, &MainWidget::filtrosCambiaron);
    connect(m_filtroPrioridad, &QComboBox::currentIndexChanged, this, &MainWidget::filtrosCambiaron);
}

void MainWidget::scheduleSessionExpiry()
{
    const QDateTime inicio = SessionManager::sessionStartedAt();
    if (!inicio.isValid()) {
        onSessionExpired();
        return;
    }

    const qint64 transcurrido = inicio.msecsTo(QDateTime::currentDateTime());
    int restante = SessionManager::timeoutMs - int(transcurrido);
    if (restante <= 0) {
        QTimer::singleShot(0, this, &MainWidget::onSessionExpired);
        return;
    }

    QTimer::singleShot(restante, this, &MainWidget::onSessionExpired);
}

void MainWidget::onSessionExpired()
{
    if (m_cierrePorExpiracion)
        return;

    m_cierrePorExpiracion = true;
    log(QStringLiteral("SESION_EXPIRADA"));
    SessionManager::clearSession();

    QMessageBox::information(this,
                             QStringLiteral("Sesión"),
                             QStringLiteral("La sesión expiró (5 minutos). Volvé a iniciar sesión."));

    close();

    auto *login = new LoginWidget;
    login->setAttribute(Qt::WA_DeleteOnClose);
    login->show();
}

void MainWidget::refreshHistorialUi()
{
    m_historialUi->setPlainText(ActionLog::readAllText());
    QTextCursor c = m_historialUi->textCursor();
    c.movePosition(QTextCursor::End);
    m_historialUi->setTextCursor(c);
}

void MainWidget::log(const QString &mensaje)
{
    ActionLog::append(m_usuario, mensaje);
    refreshHistorialUi();
}

QVector<TrabajoPractico> MainWidget::trabajosFiltrados() const
{
    const QString est = m_filtroEstado->currentData().toString();
    const QString pri = m_filtroPrioridad->currentData().toString();

    QVector<TrabajoPractico> out;
    for (const TrabajoPractico &t : m_store.items()) {
        if (!est.isEmpty() && t.estado != est)
            continue;
        if (!pri.isEmpty() && t.prioridad != pri)
            continue;
        out.append(t);
    }
    return out;
}

void MainWidget::rebuildGrid()
{
    while (m_grid->count() > 0) {
        QLayoutItem *it = m_grid->takeAt(0);
        if (QWidget *w = it->widget())
            w->deleteLater();
        delete it;
    }

    int r = 0;

    auto marcarCabecera = [](QLabel *lab) {
        QFont f = lab->font();
        f.setBold(true);
        lab->setFont(f);
    };

    auto *h0 = new QLabel(QStringLiteral("Título"));
    auto *h1 = new QLabel(QStringLiteral("Estado"));
    auto *h2 = new QLabel(QStringLiteral("Prioridad"));
    auto *h3 = new QLabel(QStringLiteral("Acciones"));
    marcarCabecera(h0);
    marcarCabecera(h1);
    marcarCabecera(h2);
    marcarCabecera(h3);
    m_grid->addWidget(h0, r, 0);
    m_grid->addWidget(h1, r, 1);
    m_grid->addWidget(h2, r, 2);
    m_grid->addWidget(h3, r, 3);
    ++r;

    const QVector<TrabajoPractico> items = trabajosFiltrados();

    if (items.isEmpty()) {
        auto *empty = new QLabel(QStringLiteral("No hay trabajos para mostrar con el filtro actual."));
        empty->setWordWrap(true);
        m_grid->addWidget(empty, r, 0, 1, 4);
        return;
    }

    for (const TrabajoPractico &t : items) {
        m_grid->addWidget(new QLabel(t.titulo), r, 0);
        m_grid->addWidget(new QLabel(t.estado), r, 1);
        m_grid->addWidget(new QLabel(t.prioridad), r, 2);

        auto *acciones = new QWidget;
        auto *hl = new QHBoxLayout(acciones);
        hl->setContentsMargins(0, 0, 0, 0);

        auto *btnEditar = new QPushButton(QStringLiteral("Editar"));
        auto *btnEliminar = new QPushButton(QStringLiteral("Eliminar"));
        auto *btnNotas = new QPushButton(QStringLiteral("Notas"));

        const QString id = t.id;
        connect(btnEditar, &QPushButton::clicked, this, [this, id]() { editarTrabajo(id); });
        connect(btnEliminar, &QPushButton::clicked, this, [this, id]() { eliminarTrabajo(id); });
        connect(btnNotas, &QPushButton::clicked, this, [this, id]() { abrirNotas(id); });

        hl->addWidget(btnEditar);
        hl->addWidget(btnEliminar);
        hl->addWidget(btnNotas);
        hl->addStretch(1);

        m_grid->addWidget(acciones, r, 3);
        ++r;
    }
}

void MainWidget::filtrosCambiaron()
{
    rebuildGrid();
}

void MainWidget::agregarTrabajo()
{
    bool ok = false;
    const QString titulo = QInputDialog::getText(this,
                                                 QStringLiteral("Nuevo trabajo práctico"),
                                                 QStringLiteral("Título:"),
                                                 QLineEdit::Normal,
                                                 {},
                                                 &ok);
    if (!ok || titulo.trimmed().isEmpty())
        return;

    TrabajoPractico t;
    t.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    t.titulo = titulo.trimmed();
    t.estado = QStringLiteral("Pendiente");
    t.prioridad = QStringLiteral("Media");
    t.notas.clear();

    m_store.add(t);
    m_store.save();

    log(QStringLiteral("TP_CREADO: %1").arg(t.titulo));
    rebuildGrid();
}

void MainWidget::editarTrabajo(const QString &id)
{
    const TrabajoPractico actual = m_store.findById(id);
    if (actual.id.isEmpty())
        return;

    EditarTrabajoDialog dlg(actual, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    TrabajoPractico t = dlg.trabajo();
    if (t.titulo.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("Datos inválidos"), QStringLiteral("El título no puede quedar vacío."));
        return;
    }

    m_store.update(t);
    m_store.save();

    log(QStringLiteral("TP_EDITADO: %1").arg(t.titulo));
    rebuildGrid();
}

void MainWidget::eliminarTrabajo(const QString &id)
{
    const TrabajoPractico actual = m_store.findById(id);
    if (actual.id.isEmpty())
        return;

    const auto res = QMessageBox::question(this,
                                           QStringLiteral("Confirmar"),
                                           QStringLiteral("¿Eliminar «%1»?").arg(actual.titulo));
    if (res != QMessageBox::Yes)
        return;

    m_store.removeById(id);
    m_store.save();

    log(QStringLiteral("TP_ELIMINADO: %1").arg(actual.titulo));
    rebuildGrid();
}

void MainWidget::abrirNotas(const QString &id)
{
    TrabajoPractico t = m_store.findById(id);
    if (t.id.isEmpty())
        return;

    NotasDialog dlg(t.titulo, t.notas, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    t.notas = dlg.notas();
    m_store.update(t);
    m_store.save();

    log(QStringLiteral("NOTA_GUARDADA: %1").arg(t.titulo));
    rebuildGrid();
}
