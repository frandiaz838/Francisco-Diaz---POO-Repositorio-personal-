#include "DrawingView.h"
#include "DrawingModel.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTimer>
#include <QWheelEvent>
#include <QtMath>

namespace {

constexpr double kPreviewChainStepPx = 2.0;
constexpr double kPreviewResampleStepPx = 2.0;
constexpr double kPreviewTipJoinPx = 0.18;
constexpr int kPreviewMaxForChaikin = 480;

constexpr double kColorLerpStartR = 192;
constexpr double kColorLerpStartG = 19;
constexpr double kColorLerpStartB = 76;
constexpr double kColorLerpEndR = 24;
constexpr double kColorLerpEndG = 233;
constexpr double kColorLerpEndB = 199;

QPointF catmullRomPoint(const QPointF &p0, const QPointF &p1, const QPointF &p2, const QPointF &p3,
                       qreal t)
{
    const qreal t2 = t * t;
    const qreal t3 = t2 * t;
    return 0.5 * ((2.0 * p1) + (-p0 + p2) * t + (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t2
                  + (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t3);
}

QPointF mousePoint(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return event->localPos();
#endif
}

} // namespace

DrawingView::DrawingView(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

void DrawingView::setModel(DrawingModel *model)
{
    if (m_model) {
        disconnect(m_model, nullptr, this, nullptr);
    }
    m_model = model;
    if (m_model) {
        connect(m_model, &DrawingModel::modelReset, this, [this]() { rebuildLayerFromModel(); });
        connect(m_model, &DrawingModel::strokeAdded, this, [this](int idx) {
            const Stroke &s = m_model->strokes().at(idx);
            renderStrokeOntoLayer(s);
            const QRect dr = rectInflatedForStroke(s).intersected(rect());
            if (!dr.isEmpty())
                update(dr);
            else
                update();
        });
    }
    rebuildLayerFromModel();
}

void DrawingView::setClientId(const QString &clientId)
{
    m_clientId = clientId;
}

QColor DrawingView::colorForIndex(int idx1to9)
{
    const qreal t = qreal(qBound(1, idx1to9, 9) - 1) / 8.0;
    const int r = qRound(kColorLerpStartR + (kColorLerpEndR - kColorLerpStartR) * t);
    const int g = qRound(kColorLerpStartG + (kColorLerpEndG - kColorLerpStartG) * t);
    const int b = qRound(kColorLerpStartB + (kColorLerpEndB - kColorLerpStartB) * t);
    return QColor(r, g, b);
}

QVector<QPointF> DrawingView::resampleSpatial(const QVector<QPointF> &raw, double stepPx)
{
    if (raw.isEmpty())
        return {};
    if (raw.size() == 1)
        return raw;

    QVector<QPointF> out;
    out.append(raw.front());
    QPointF anchor = raw.front();

    for (int i = 1; i < raw.size(); ++i) {
        QPointF target = raw.at(i);
        QLineF seg(anchor, target);
        qreal len = seg.length();
        if (len < 1e-3)
            continue;
        seg.setLength(stepPx);
        while (len >= stepPx - 1e-3) {
            anchor = seg.p2();
            out.append(anchor);
            seg = QLineF(anchor, target);
            len = seg.length();
            if (len < 1e-3)
                break;
            seg.setLength(stepPx);
        }
        anchor = target;
    }
    if (QLineF(out.last(), raw.back()).length() > 1e-3)
        out.append(raw.back());
    return out;
}

QVector<QPointF> DrawingView::chaikinSmooth(const QVector<QPointF> &in, int iterations)
{
    QVector<QPointF> pts = in;
    for (int it = 0; it < iterations; ++it) {
        if (pts.size() < 2)
            break;
        QVector<QPointF> n;
        n.reserve(pts.size() * 2);
        n.append(pts.front());
        for (int i = 0; i < pts.size() - 1; ++i) {
            const QPointF &p0 = pts.at(i);
            const QPointF &p1 = pts.at(i + 1);
            n.append(0.75 * p0 + 0.25 * p1);
            n.append(0.25 * p0 + 0.75 * p1);
        }
        n.append(pts.back());
        pts = n;
    }
    return pts;
}

QVector<QPointF> DrawingView::catmullRomDense(const QVector<QPointF> &ctrl, int subdivisionsPerSegment)
{
    if (ctrl.isEmpty())
        return {};
    if (ctrl.size() == 1)
        return ctrl;

    QVector<QPointF> dense;
    QVector<QPointF> pts = ctrl;
    if (pts.size() == 2) {
        pts.prepend(pts.front() + (pts.front() - pts.back()));
        pts.append(pts.back() + (pts.back() - pts.front()));
    } else {
        pts.prepend(pts.front());
        pts.append(pts.back());
    }

    for (int i = 0; i < pts.size() - 3; ++i) {
        const QPointF &p0 = pts.at(i);
        const QPointF &p1 = pts.at(i + 1);
        const QPointF &p2 = pts.at(i + 2);
        const QPointF &p3 = pts.at(i + 3);
        if (i == 0)
            dense.append(p1);
        for (int s = 1; s <= subdivisionsPerSegment; ++s) {
            const qreal t = qreal(s) / qreal(subdivisionsPerSegment);
            dense.append(catmullRomPoint(p0, p1, p2, p3, t));
        }
    }
    return dense;
}

QVector<QPointF> DrawingView::processRawPolyline(const QVector<QPointF> &raw, bool lite)
{
    if (raw.isEmpty())
        return {};
    if (raw.size() == 1)
        return raw;

    const double step = lite ? 4.5 : 3.0;
    QVector<QPointF> spaced = resampleSpatial(raw, step);
    if (spaced.size() < 2)
        spaced = raw;
    const int chaikinIt = lite ? 1 : 2;
    QVector<QPointF> smoothed = chaikinSmooth(spaced, chaikinIt);
    const int subdiv = lite ? 4 : 8;
    QVector<QPointF> dense = catmullRomDense(smoothed, subdiv);
    if (dense.size() < 2)
        dense = smoothed;
    return dense;
}

QRect DrawingView::rectInflatedForStroke(const Stroke &stroke)
{
    if (stroke.points.isEmpty())
        return QRect();
    QRectF rf(stroke.points.first(), QSizeF(1, 1));
    for (const QPointF &p : stroke.points)
        rf |= QRectF(p, QSizeF(1, 1));
    const qreal m = qMax(4.0, stroke.widthPx * 0.5 + 6.0);
    return rf.adjusted(-m, -m, m, m).toAlignedRect();
}

QVector<QPointF> DrawingView::pencilPreviewPolyline() const
{
    if (m_rawPoints.isEmpty())
        return {};
    QVector<QPointF> pts = m_previewPoints;
    const QPointF tip = m_rawPoints.last();
    if (pts.isEmpty())
        pts.append(tip);
    else if (QLineF(pts.last(), tip).length() > kPreviewTipJoinPx)
        pts.append(tip);
    if (pts.size() < 2)
        return pts;

    QVector<QPointF> spaced = resampleSpatial(pts, kPreviewResampleStepPx);
    if (spaced.size() < 2)
        spaced = pts;

    if (spaced.size() <= kPreviewMaxForChaikin) {
        QVector<QPointF> smooth = chaikinSmooth(spaced, 1);
        if (smooth.size() >= 2)
            return smooth;
    }
    return spaced;
}

void DrawingView::schedulePaint(bool immediate)
{
    if (immediate) {
        m_deferredPaintQueued = false;
        m_paintThrottle.invalidate();
        update();
        return;
    }
    if (!m_paintThrottle.isValid())
        m_paintThrottle.start();
    if (m_paintThrottle.elapsed() >= 15) {
        m_paintThrottle.restart();
        update();
        return;
    }
    if (!m_deferredPaintQueued) {
        m_deferredPaintQueued = true;
        const int ms = qMax(1, 15 - int(m_paintThrottle.elapsed()));
        QTimer::singleShot(ms, this, [this]() {
            if (!m_deferredPaintQueued)
                return;
            m_deferredPaintQueued = false;
            m_paintThrottle.restart();
            update();
        });
    }
}

namespace {
void paintStroke(QPainter &p, const Stroke &stroke, const QColor &penColor)
{
    if (stroke.points.size() < 2)
        return;
    QPen pen;
    pen.setWidthF(stroke.widthPx);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    if (stroke.isEraser) {
        p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
        pen.setColor(QColor(0, 0, 0, 255));
    } else {
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        pen.setColor(penColor);
    }
    p.setPen(pen);
    QPainterPath path;
    path.moveTo(stroke.points.front());
    if (stroke.points.size() == 2) {
        path.lineTo(stroke.points.at(1));
    } else {
        QPointF prev = stroke.points.front();
        for (int i = 1; i < stroke.points.size(); ++i) {
            const QPointF curr = stroke.points.at(i);
            const QPointF mid = 0.5 * (prev + curr);
            path.quadTo(prev, mid);
            prev = curr;
        }
        path.lineTo(stroke.points.back());
    }
    p.strokePath(path, pen);
}
} // namespace

void DrawingView::renderStrokeOntoLayer(const Stroke &stroke)
{
    if (!m_layer.isNull() && stroke.points.size() >= 2) {
        QPainter p(&m_layer);
        p.setRenderHint(QPainter::Antialiasing, true);
        const QColor c = colorForIndex(stroke.colorIndex);
        paintStroke(p, stroke, c);
    }
}

void DrawingView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::white);
    if (!m_layer.isNull())
        painter.drawImage(0, 0, m_layer);

    if (m_drawing && !m_eraserMode) {
        const QVector<QPointF> previewPts = pencilPreviewPolyline();
        if (previewPts.size() >= 2) {
            Stroke tmp{};
            tmp.isEraser = false;
            tmp.colorIndex = m_colorIndex;
            tmp.widthPx = m_strokeWidth;
            tmp.points = previewPts;
            painter.setRenderHint(QPainter::Antialiasing, true);
            paintStroke(painter, tmp, colorForIndex(m_colorIndex));
        }
    }
}

void DrawingView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    rebuildLayerFromModel();
}

void DrawingView::rebuildLayerFromModel()
{
    m_layer = QImage(size(), QImage::Format_ARGB32_Premultiplied);
    m_layer.fill(QColor(Qt::white));
    if (!m_model)
        return;
    QPainter p(&m_layer);
    p.setRenderHint(QPainter::Antialiasing, true);
    for (const Stroke &s : m_model->strokes()) {
        const QColor c = colorForIndex(s.colorIndex);
        paintStroke(p, s, c);
    }
    schedulePaint(true);
}

void DrawingView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_eraserMode = false;
        m_activeButton = int(Qt::LeftButton);
        m_drawing = true;
        m_rawPoints.clear();
        m_previewPoints.clear();
        m_liveEraserCommitted = 0;
        const QPointF p0 = mousePoint(event);
        m_rawPoints.append(p0);
        m_previewPoints.append(p0);
        event->accept();
        schedulePaint(true);
        return;
    }
    if (event->button() == Qt::RightButton) {
        m_eraserMode = true;
        m_activeButton = int(Qt::RightButton);
        m_drawing = true;
        m_rawPoints.clear();
        m_previewPoints.clear();
        m_liveEraserCommitted = 0;
        m_rawPoints.append(mousePoint(event));
        event->accept();
        schedulePaint(true);
        return;
    }
    QWidget::mousePressEvent(event);
}

void DrawingView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_drawing) {
        QWidget::mouseMoveEvent(event);
        return;
    }
    const QPointF pos = mousePoint(event);
    const double minDist = m_eraserMode ? 1.0 : 1.2;
    if (!m_rawPoints.isEmpty() && QLineF(m_rawPoints.last(), pos).length() < minDist)
        return;
    m_rawPoints.append(pos);
    if (!m_eraserMode) {
        if (m_previewPoints.isEmpty())
            m_previewPoints.append(pos);
        else if (QLineF(m_previewPoints.last(), pos).length() >= kPreviewChainStepPx)
            m_previewPoints.append(pos);
    }
    if (m_eraserMode && !m_layer.isNull()) {
        const int pending = m_rawPoints.size() - 1 - m_liveEraserCommitted;
        if (pending >= 4) {
            const int chunkStart = qMax(0, m_liveEraserCommitted - 2);
            const QVector<QPointF> chunkRaw = m_rawPoints.mid(chunkStart);
            Stroke live = Stroke::createNew(m_clientId, true, m_colorIndex, m_strokeWidth);
            live.points = processRawPolyline(chunkRaw, true);
            renderStrokeOntoLayer(live);
            m_liveEraserCommitted = m_rawPoints.size() - 1;
            schedulePaint(true);
            event->accept();
            return;
        }
    }
    event->accept();
    if (!m_eraserMode)
        schedulePaint(false);
}

void DrawingView::mouseReleaseEvent(QMouseEvent *event)
{
    if (!m_drawing) {
        QWidget::mouseReleaseEvent(event);
        return;
    }
    if (int(event->button()) == m_activeButton) {
        finishCurrentStroke();
        m_activeButton = 0;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void DrawingView::finishCurrentStroke()
{
    m_drawing = false;
    m_activeButton = 0;
    if (m_rawPoints.isEmpty()) {
        schedulePaint(true);
        return;
    }
    if (m_rawPoints.size() == 1)
        m_rawPoints.append(m_rawPoints.last() + QPointF(0.5, 0.5));

    if (m_eraserMode && m_rawPoints.size() - 1 > m_liveEraserCommitted) {
        const int chunkStart = qMax(0, m_liveEraserCommitted - 2);
        const QVector<QPointF> chunkRaw = m_rawPoints.mid(chunkStart);
        Stroke live = Stroke::createNew(m_clientId, true, m_colorIndex, m_strokeWidth);
        live.points = processRawPolyline(chunkRaw, false);
        renderStrokeOntoLayer(live);
    }

    Stroke s = Stroke::createNew(m_clientId, m_eraserMode, m_colorIndex, m_strokeWidth);
    s.points = processRawPolyline(m_rawPoints, false);
    m_rawPoints.clear();
    m_previewPoints.clear();
    if (s.points.size() < 2) {
        schedulePaint(true);
        return;
    }
    emit localStrokeFinished(s);
    schedulePaint(true);
}

void DrawingView::wheelEvent(QWheelEvent *event)
{
    const double delta = event->angleDelta().y() / 120.0;
    m_strokeWidth = qBound(1.0, m_strokeWidth + delta * 0.75, 80.0);
    event->accept();
}

void DrawingView::keyPressEvent(QKeyEvent *event)
{
    const int k = event->key();
    if (k >= Qt::Key_1 && k <= Qt::Key_9) {
        m_colorIndex = k - Qt::Key_0;
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}
