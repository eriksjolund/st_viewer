#include "GraphicItemGL.h"

#include <QVector3D>

GraphicItemGL::GraphicItemGL(QObject *parent) :
    QGLSceneNode(parent)
{

}

GraphicItemGL::~GraphicItemGL()
{

}

GraphicItemGL::VisualOptions GraphicItemGL::visualOptions() const
{
    return m_visualOptions;
}

void GraphicItemGL::setVisualOptions(GraphicItemGL::VisualOptions visualOptions)
{
    if ( m_visualOptions != visualOptions ) {
        m_visualOptions = visualOptions;
        emit updated();
    }
}

void GraphicItemGL::setVisualOption(GraphicItemGL::VisualOption visualOption, bool value)
{
    if (value) {
        m_visualOptions |= visualOption;
     }
    else {
        m_visualOptions &= ~visualOption;
    }
    emit updated();
}

bool GraphicItemGL::visible() const
{
    return m_visualOptions & VisualOption::Visible;
}

bool GraphicItemGL::selectable() const
{
    return m_visualOptions & VisualOption::Selectable;
}

bool GraphicItemGL::transformable() const
{
    return m_visualOptions & VisualOption::Transformable;
}

bool GraphicItemGL::invertedY() const
{
    return m_visualOptions & VisualOption::Yinverted;
}

bool GraphicItemGL::invertedX() const
{
    return m_visualOptions & VisualOption::Xinverted;
}

void GraphicItemGL::setVisible(bool value)
{
    setVisualOption(VisualOption::Visible, value);
}

GraphicItemGL::Anchor GraphicItemGL::anchor() const
{
    return m_anchor;
}

void GraphicItemGL::setAnchor(GraphicItemGL::Anchor anchor)
{
    if ( m_anchor != anchor ) {
        m_anchor = anchor;
        emit updated();
    }
}

const QTransform GraphicItemGL::transform() const
{
    return adjustForAnchor(m_transform);
}

void GraphicItemGL::setTransform(const QTransform& transform)
{
    m_transform = transform;
}

bool GraphicItemGL::contains(const QPointF& point) const
{
    return boundingRect().contains(point);
}

const QTransform GraphicItemGL::adjustForAnchor(const QTransform& transform) const
{
    const QRectF rect = boundingRect();
    QTransform adjustedTransform = transform;
    switch (m_anchor)
    {
        case GraphicItemGL::Center:
            adjustedTransform.translate((rect.x() + rect.width()) * -0.5f, (rect.y() + rect.height()) * -0.5f);
            break;
        case GraphicItemGL::North:
            adjustedTransform.translate((rect.x() + rect.width()) * -0.5f, 0.0f);
            break;
        case GraphicItemGL::NorthEast:
            adjustedTransform.translate((rect.x() + rect.width()) * -1.0f, 0.0f);
            break;
        case GraphicItemGL::East:
            adjustedTransform.translate((rect.x() + rect.width()) * -1.0f, (rect.y() + rect.height()) * -0.5f);
            break;
        case GraphicItemGL::SouthEast:
            adjustedTransform.translate((rect.x() + rect.width()) * -1.0f, (rect.y() + rect.height()) * -1.0f);
            break;
        case GraphicItemGL::South:
            adjustedTransform.translate((rect.x() + rect.width()) * -0.5f, (rect.y() + rect.height()) * -1.0f);
            break;
        case GraphicItemGL::SouthWest:
            adjustedTransform.translate(0.0f, (rect.y() + rect.height()) * -1.0f);
            break;
        case GraphicItemGL::West:
            adjustedTransform.translate(0.0f, (rect.y() + rect.height()) * -0.5f);
            break;
        case GraphicItemGL::NorthWest:
            // fall-through
        default:
            break;
    }
    return adjustedTransform;
}
