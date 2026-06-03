#include "ExportadorJpg.h"
#include <QDateTime>
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>

bool ExportadorJpg::exportar(const QString& codigo,
                             const QString& lenguaje,
                             const QString& rutaSalida) {
    const QStringList lineas = codigo.split('\n');

    QFont fuente("Consolas");
    fuente.setStyleHint(QFont::Monospace);
    fuente.setPointSize(11);

    QFontMetrics metrics(fuente);
    const int altoLinea = metrics.height() + 4;
    const int margen    = 30;
    const int anchoNum  = metrics.horizontalAdvance("0000  ");

    // Calcular ancho minimo necesario
    int anchoMax = 0;
    for (const QString& l : lineas)
        anchoMax = qMax(anchoMax, metrics.horizontalAdvance(l));

    const int ancho  = qMax(800, anchoMax + anchoNum + margen * 2);
    const int alto   = margen * 2 + 60 /*titulo*/ + altoLinea * qMax(1, lineas.size());

    QImage img(ancho, alto, QImage::Format_RGB32);
    img.fill(QColor("#1e1e2e"));

    QPainter p(&img);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setFont(fuente);

    // Cabecera con titulo y fecha
    QFont titulo = fuente;
    titulo.setPointSize(14);
    titulo.setBold(true);
    p.setFont(titulo);
    p.setPen(QColor("#4A90E2"));
    p.drawText(margen, margen + 20,
               QString("Editor Multilenguaje - %1").arg(lenguaje));

    QFont sub = fuente;
    sub.setPointSize(9);
    p.setFont(sub);
    p.setPen(QColor("#888888"));
    p.drawText(margen, margen + 40,
               QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));

    // Linea separadora
    p.setPen(QColor("#4A90E2"));
    p.drawLine(margen, margen + 50, ancho - margen, margen + 50);

    // Codigo: numero de linea + texto
    p.setFont(fuente);
    int y = margen + 70;
    for (int i = 0; i < lineas.size(); ++i) {
        p.setPen(QColor("#666666"));
        p.drawText(margen, y, QString("%1").arg(i + 1, 4).leftJustified(4));
        p.setPen(QColor("#e4e4e7"));
        p.drawText(margen + anchoNum, y, lineas[i]);
        y += altoLinea;
    }

    return img.save(rutaSalida, "JPG", 92);
}
