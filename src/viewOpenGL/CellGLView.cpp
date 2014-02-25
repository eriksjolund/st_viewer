/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "CellGLView.h"

#include "math/Common.h"

#include <QGLPainter>
#include <QArray>
#include <QWheelEvent>
#include <QtOpenGL>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QGuiApplication>
#include <QVector2DArray>

static const qreal DEFAULT_ZOOM_MIN = 1.0f;
static const qreal DEFAULT_ZOOM_MAX = 20.0f;
static const qreal DEFAULT_ZOOM_IN  = qreal(1.1 / 1.0);
static const qreal DEFAULT_ZOOM_OUT = qreal(1.0 / 1.1);
static const qreal DELTA_PANNING = 3.0f;
static const qreal DELTA_MOUSE_PANNING = 1.0f;
static const bool DEBUG_MODE = true;

CellGLView::CellGLView(QScreen *parent) :
    QWindow(parent)
{
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setDepthBufferSize(0);
    format.setAlphaBufferSize(24);
    format.setBlueBufferSize(24);
    format.setGreenBufferSize(24);
    format.setRedBufferSize(24);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setStereo(false);
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(format);
}

CellGLView::~CellGLView()
{
    //NOTE View does not own rendering nodes nor texture
    // draw rendering nodes
    foreach(GraphicItemGL *node, m_nodes) {
        delete node;
        node = 0;
    }
    if ( m_context ) {
        delete m_context;
    }
    m_context = 0;
}

void CellGLView::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
}

void CellGLView::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
}

void CellGLView::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);
    ensureContext();
    if ( !m_initialized ) {
        initializeGL();
    }
    if ( isExposed() ) {
        paintGL();
    }
    m_context->swapBuffers(this); // this is important
}

void CellGLView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    const QRect rect = geometry();
    ensureContext();
    if ( !m_initialized ) {
        initializeGL();
    }
    resizeGL(rect.width(), rect.height());
}

void CellGLView::ensureContext()
{
    if ( !m_context ) {
        m_context = new QOpenGLContext();
        m_context->setFormat(format);
        const bool success = m_context->create();
        Q_UNUSED(success);
    }
    m_context->makeCurrent(this);
}

void CellGLView::initializeGL()
{
    QGLPainter painter;
    painter.begin();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_CULL_FACE);

    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_initialized = true;
}

void CellGLView::paintGL()
{
    // textures are being deallocated properly
    //QGLTexture2D::processPendingResourceDeallocations();

    QGLPainter painter;
    painter.begin();

    painter.setClearColor(Qt::black);
    glClear(GL_COLOR_BUFFER_BIT);

    QRectF viewport(m_viewport);
    QMatrix4x4 projm;
    projm.ortho(viewport);
    painter.projectionMatrix() = projm;

    //emit signalSceneUpdated(sceneTransformations().mapRect(m_scene));
    //emit signalViewPortUpdated(m_viewport);

    // draw rendering nodes
    foreach(GraphicItemGL *node, m_nodes) {
        if ( node->visible() ) {
            QTransform local_transform = nodeTransformations(node);
            if ( node->transformable() ) {
                local_transform *= sceneTransformations();
            }
            painter.modelViewMatrix().push();
            painter.modelViewMatrix() *= local_transform;
            node->draw(&painter);
            painter.modelViewMatrix().pop();
            if (DEBUG_MODE) {
                drawRect(local_transform.mapRect(node->boundingRect()), &painter);
            }
        }
    }

    if (DEBUG_MODE) {
        //draw viewport rect panned
        drawRect(QRectF(10.0, 10.0, width() - 20.0, height() -20.0), &painter);
    }

   //glFlush(); // forces to send the data to the GPU saving time (no need for this when only 1 context)
}

void CellGLView::drawRect(const QRectF rect, QGLPainter *painter)
{
    const QPointF stl = rect.topLeft();
    const QPointF str = rect.topRight();
    const QPointF sbr = rect.bottomRight();
    const QPointF sbl = rect.bottomLeft();
    QVector2DArray vertices;
    vertices.append(stl.x(), stl.y());
    vertices.append(str.x(), str.y());
    vertices.append(sbr.x(), sbr.y());
    vertices.append(sbl.x(), sbl.y());
    painter->modelViewMatrix().push();
    painter->clearAttributes();
    painter->setStandardEffect(QGL::FlatColor);
    painter->setColor(Qt::red);
    painter->setVertexAttribute(QGL::Position, vertices );
    painter->draw(QGL::LineLoop, vertices.size());
    painter->modelViewMatrix().pop();
}

void CellGLView::resizeGL(int width, int height)
{
    //devicePixelRatio() fixes the problem with MAC retina
    const qreal pixelRatio = devicePixelRatio();
    glViewport(0.0, 0.0, width * pixelRatio, height * pixelRatio);
    m_viewport = QRectF(0.0, 0.0, width * pixelRatio, height * pixelRatio);
}

void CellGLView::wheelEvent(QWheelEvent* event)
{
    qreal zoomFactor = qPow(4.0 / 3.0, (event->delta() / 240.0));
    setZoom(zoomFactor * m_zoom);
    event->ignore();
}

void CellGLView::addRenderingNode(GraphicItemGL *node)
{
    Q_ASSERT(node != nullptr);
    m_nodes.append(node);
    connect(node, SIGNAL(updated()), this, SLOT(update()));
}

void CellGLView::removeRenderingNode(GraphicItemGL *node)
{
    Q_ASSERT(node != nullptr);
    m_nodes.removeOne(node);
    disconnect(node, SIGNAL(updated()), this, SLOT(update()));
}

void CellGLView::update()
{
    QGuiApplication::postEvent(this, new QExposeEvent(geometry()));
}

void CellGLView::setZoom(qreal delta)
{
    //TODO check for min and max
    if ( m_zoom != delta ) {
        m_zoom = delta;
        update();
    }
}

void CellGLView::centerOn(const QPointF& point)
{
    //TODO check and validate this
    qDebug() << "CellGLView : center on " << point;
    //m_panx += point.x();
    //m_pany -= -point.y();
    update();
}

void CellGLView::rotate(qreal angle)
{
    if (angle >= -180.0 && angle <= 180.0 && m_rotate != angle ) {
        m_rotate += angle;
        STMath::clamp(m_rotate, -360.0, 360.0);
        update();
    }
}

void CellGLView::setViewPort(QRectF viewport)
{
    if ( m_viewport != viewport && viewport.isValid() ) {
        m_viewport = viewport;
        update();
    }
}

void CellGLView::setScene(QRectF scene)
{
    if ( m_scene != scene && scene.isValid() ) {
        m_scene = scene;
        update();
    }
}

const QImage CellGLView::grabPixmapGL() const
{
    const int w = width();
    const int h = height();
    QImage res(w, h, QImage::Format_RGB32);
    glReadBuffer(GL_FRONT_LEFT);
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, res.bits());
    res = res.rgbSwapped();
    const QVector<QColor> pal = QColormap::instance().colormap();
    if (pal.size()) {
        res.setColorCount(pal.size());
        for (int i = 0; i < pal.size(); i++) {
            res.setColor(i, pal.at(i).rgb());
        }
    }
    return res.mirrored();
}

void CellGLView::zoomIn()
{
    setZoom(m_zoom * DEFAULT_ZOOM_IN);
}

void CellGLView::zoomOut()
{
    setZoom(m_zoom * DEFAULT_ZOOM_OUT);
}

void CellGLView::mousePressEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton ) {
        setCursor(Qt::ClosedHandCursor);
        m_panning = true;
        m_originPanning = event->globalPos(); //moving needs globalPos
        const QPointF point = event->localPos();

        foreach(GraphicItemGL *node, m_nodes) {
            const QPointF localPoint = nodeTransformations(node).inverted().map(point);
            QMouseEvent newEvent(
                event->type(),
                localPoint,
                event->windowPos(),
                event->screenPos(),
                event->button(),
                event->buttons(),
                event->modifiers()
            );
            if ( node->selectable() && node->contains(localPoint) ){
                node->mousePressEvent(&newEvent);
            }
        }
    }
    else if ( event->button() == Qt::RightButton && !m_rubberBanding ) {
        // Rubberbanding changes cursor to pointing hand
        setCursor(Qt::PointingHandCursor);
        m_rubberBanding = true;
        m_originRubberBand = event->pos(); //rubberband does not need global
    }
    event->ignore();
}

void CellGLView::mouseReleaseEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton ) {
        unsetCursor();
        m_panning = false;
        const QPointF point = event->localPos();

        foreach(GraphicItemGL *node, m_nodes) {
            const QPointF localPoint = nodeTransformations(node).inverted().map(point);
            QMouseEvent newEvent(
                event->type(),
                localPoint,
                event->windowPos(),
                event->screenPos(),
                event->button(),
                event->buttons(),
                event->modifiers()
            );
            if ( node->selectable() && node->contains(localPoint) ) {
                node->mouseReleaseEvent(&newEvent);
            }
        }
    }  else if ( event->button() == Qt::RightButton && m_rubberBanding ) {
        unsetCursor();
        const QPointF origin = m_originRubberBand;
        const QPointF destiny = event->localPos();

        foreach(GraphicItemGL *node, m_nodes) {
            if (node->rubberBandable() ) {
                QTransform node_trans = nodeTransformations(node);
                if ( node->transformable() ) {
                    node_trans *= sceneTransformations();
                }

                QRect rect(qMin(origin.x(), destiny.x()), qMin(origin.y(), destiny.y()),
                           qAbs(origin.x() - destiny.x()) + 1, qAbs(origin.y() - destiny.y()) + 1);

                QRect transformed = node_trans.inverted().mapRect(rect);

                qDebug() << "Rubberbanded inside = " << node->boundingRect().contains(transformed);
            }
        }
        // reset variables
        m_rubberBanding = false;
    }

    event->ignore();
}

void CellGLView::mouseMoveEvent(QMouseEvent *event)
{
    if ( m_panning ) {
        QPoint point = event->globalPos();
        m_panx += (point.x() - m_originPanning.x()) * DELTA_MOUSE_PANNING;
        m_pany += (point.y() - m_originPanning.y()) * DELTA_MOUSE_PANNING;
        m_originPanning = point;
        update();
    }
    if ( event->button() == Qt::RightButton && m_rubberBanding  ) {
        // NOTE I can update rubber band rect here and draw it
        // if I want to give the effect of the selection shape expanding
    }
    else if ( event->button() == Qt::LeftButton ) {
        QPointF point = event->localPos();
        foreach(GraphicItemGL *node, m_nodes) {
            const QPointF localPoint = nodeTransformations(node).inverted().map(point);
            QMouseEvent newEvent(
                event->type(),
                localPoint,
                event->windowPos(),
                event->screenPos(),
                event->button(),
                event->buttons(),
                event->modifiers()
            );

            if ( node->selectable() && node->contains(localPoint) ) {
                node->mouseMoveEvent(&newEvent);
            }
        }
    }

    event->ignore();
}

void CellGLView::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_Right:
        m_panx += DELTA_PANNING;
        break;
    case Qt::Key_Left:
        m_panx -= DELTA_PANNING;
        break;
    case Qt::Key_Up:
        m_pany -= DELTA_PANNING;
        break;
    case Qt::Key_Down:
        m_pany += DELTA_PANNING;
        break;
    default:
        break;
    }
    update();
    event->ignore();
}

const QTransform CellGLView::sceneTransformations() const
{
    QTransform transform;
    if ( m_rotate != 0.0 ) {
        //TOFIX should rotate around its center
        transform.rotate(m_rotate, Qt::XAxis);
    }
    if ( m_panx != 0.0 && m_pany != 0.0 ) {
        transform.translate(m_panx, m_pany);
    }
    if ( m_zoom != 1.0 ) {
        transform.scale(m_zoom, m_zoom);
    }
    return transform;
}

const QTransform CellGLView::nodeTransformations(GraphicItemGL *node) const
{
    const QSizeF viewSize = m_viewport.size();
    QTransform transform(Qt::Uninitialized);
    const GraphicItemGL::Anchor anchor = node->anchor();

    switch (anchor)
    {
        case GraphicItemGL::Center:
            transform = QTransform::fromTranslate(viewSize.width() * 0.5, viewSize.height() * 0.5);
            break;
        case GraphicItemGL::North:
            transform = QTransform::fromTranslate(viewSize.width() * 0.5, 0.0);
            break;
        case GraphicItemGL::NorthEast:
            transform = QTransform::fromTranslate(viewSize.width(), 0.0);
            break;
        case GraphicItemGL::East:
            transform = QTransform::fromTranslate(viewSize.width(), viewSize.height() * 0.5);
            break;
        case GraphicItemGL::SouthEast:
            transform = QTransform::fromTranslate(viewSize.width(), viewSize.height());
            break;
        case GraphicItemGL::South:
            transform = QTransform::fromTranslate(viewSize.width() * 0.5, viewSize.height());
            break;
        case GraphicItemGL::SouthWest:
            transform = QTransform::fromTranslate(0.0, viewSize.height());
            break;
        case GraphicItemGL::West:
           transform = QTransform::fromTranslate(0.0, viewSize.height() * 0.5);
            break;
        case GraphicItemGL::NorthWest:
            // fall-through
        default:
            transform = QTransform::fromTranslate(0.0, 0.0);
            break;
    }

    //if ( node->invertedX() || node->invertedY() ) {
   //     transform.scale(node->invertedX() ? -1.0 : 1.0, node->invertedY() ? -1.0 : 1.0);
   // }
    return node->transform() * transform ;
}
