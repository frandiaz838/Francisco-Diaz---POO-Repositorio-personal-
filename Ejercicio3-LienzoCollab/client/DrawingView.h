#pragma once

#include "StrokeTypes.h"

#include <QColor>
#include <QElapsedTimer>
#include <QRect>
#include <QImage>
#include <QString>
#include <QVector>
#include <QWidget>

class DrawingModel;
class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

class DrawingView : public QWidget
{
    Q_OBJECT
public:
    explicit DrawingView(QWidget *parent = nullptr);

    void setModel(DrawingModel *model);
    void setClientId(const QString &clientId);

    int colorIndex() const { return m_colorIndex; }
    double strokeWidth() const { return m_strokeWidth; }
    bool isDrawingActive() const { return m_drawing; }

signals:
    void localStrokeFinished(const Stroke &stroke);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    void rebuildLayerFromModel();
    void renderStrokeOntoLayer(const Stroke &stroke);
    void finishCurrentStroke();
    void schedulePaint(bool immediate);
    QVector<QPointF> pencilPreviewPolyline() const;

    static QColor colorForIndex(int idx1to9);
    static QRect rectInflatedForStroke(const Stroke &stroke);
    static QVector<QPointF> resampleSpatial(const QVector<QPointF> &raw, double stepPx);
    static QVector<QPointF> chaikinSmooth(const QVector<QPointF> &in, int iterations);
    static QVector<QPointF> catmullRomDense(const QVector<QPointF> &ctrl, int subdivisionsPerSegment);
    static QVector<QPointF> processRawPolyline(const QVector<QPointF> &raw, bool lite);

    DrawingModel *m_model = nullptr;
    QString m_clientId;
    QImage m_layer;
    QVector<QPointF> m_rawPoints;
    QVector<QPointF> m_previewPoints;
    bool m_drawing = false;
    bool m_eraserMode = false;
    int m_activeButton = 0;
    int m_liveEraserCommitted = 0;
    int m_colorIndex = 5;
    double m_strokeWidth = 4.0;
    QElapsedTimer m_paintThrottle;
    bool m_deferredPaintQueued = false;
};
