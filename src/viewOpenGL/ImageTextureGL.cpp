/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/
#include "ImageTextureGL.h"

#include <QGLBuilder>
#include <QGeometryData>
#include <QGLMaterial>
#include <QImage>
#include <QGLPainter>
#include <cmath>

ImageTextureGL::ImageTextureGL(QObject *parent) :
    GraphicItemGL(parent)
{
    setVisualOption(GraphicItemGL::Transformable, true);
    setVisualOption(GraphicItemGL::Visible, true);
    setVisualOption(GraphicItemGL::Selectable, false);
    setVisualOption(GraphicItemGL::Yinverted, false);
    setVisualOption(GraphicItemGL::Xinverted, false);
}

ImageTextureGL::~ImageTextureGL()
{
    clearTextures();
    clearNodes();
}

void ImageTextureGL::clearTextures()
{
    foreach( QGLTexture2D *texture, m_textures ) {
        if (texture) {
            texture->cleanupResources();
            texture->release();
            texture->clearImage();
            delete texture;
            texture = 0;
        }
    }
}

void ImageTextureGL::clearNodes()
{
    foreach( QGLSceneNode *node, m_nodes) {
        if ( node ) {
            delete node;
            node = 0;
        }
    }
}

void ImageTextureGL::draw(QGLPainter *painter)
{
    glEnable(GL_TEXTURE_2D);
    {
        foreach(QGLSceneNode *node, m_nodes ) {
            if ( node && node->material() && node->material()->texture() ) {
                node->material()->texture()->bind();
                QColor texture_color = Qt::black;
                texture_color.setAlphaF(m_intensity);
                node->material()->setColor(texture_color);
                node->draw(painter);
                node->material()->texture()->release();
            }
        }
    }
    glDisable(GL_TEXTURE_2D);
}

void ImageTextureGL::drawGeometry(QGLPainter *painter)
{
    Q_UNUSED(painter);
}

void ImageTextureGL::createTexture(const QImage& image)
{
   clearTextures();
   clearNodes();
   // we always tile, it is not secure to create one texture from the whole image
   createTiles(image);
   m_bounds = image.rect();
}

void ImageTextureGL::createTiles(const QImage &image)
{
    static const int tile_width = 512;
    static const int tile_height = 512;
    const int width = image.width();
    const int height = image.height();
    const int xCount = std::ceil( qreal(width) / qreal(tile_width) );
    const int yCount = std::ceil( qreal(height) / qreal(tile_height) );
    const int count = xCount * yCount;

    for (int i = 0; i < count; ++i) {

         // texture sizes
        const int x = tile_width * (i % xCount);
        const int y = tile_height * (i / xCount);
        const int texture_width = std::min(width -  x, tile_width);
        const int texture_height = std::min(height -  y, tile_height);

        // create sub image
        QImage sub_image = image.copy(QRect(x, y, texture_width, texture_height));

        // add texture
        addTexture(sub_image, x, y);
    }
}

void ImageTextureGL::addTexture(const QImage& image, const int x, const int y)
{
    const qreal width = qreal(image.width());
    const qreal height = qreal(image.height());

    QGLBuilder builder;
    QGeometryData data;

    QVector2D a(x, y);
    QVector2D b(x + width, y);
    QVector2D c(x + width, y + height);
    QVector2D d(x, y + height);
    QVector2D ta(0.0f, 0.0f);
    QVector2D tb(1.0f, 0.0f);
    QVector2D tc(1.0f, 1.0f);
    QVector2D td(0.0f, 1.0f);

    data.appendVertex(a, b, c, d);
    data.appendTexCoord(ta, tb, tc, td);

    builder.addQuads(data);
    QGLSceneNode *node  = builder.finalizedSceneNode();

    QGLTexture2D *m_texture = new QGLTexture2D;
    m_texture->setImage(image);
    m_texture->setVerticalWrap(QGL::ClampToEdge);
    m_texture->setHorizontalWrap(QGL::ClampToEdge);
    m_texture->setBindOptions(QGLTexture2D::LinearFilteringBindOption
                              | QGLTexture2D::MipmapBindOption);
    m_texture->setSize(QSize(width, height));

    QGLMaterial *mat = new QGLMaterial;
    mat->setColor(Qt::black);
    mat->setTexture(m_texture);
    node->setMaterial(mat);
    node->setEffect(QGL::LitDecalTexture2D);

    m_nodes.append(node);
}

const QRectF ImageTextureGL::boundingRect() const
{
    return m_bounds;
}

void ImageTextureGL::setIntensity(qreal intensity)
{
    if ( m_intensity != intensity ) {
        m_intensity = intensity;
        emit updated();
    }
}