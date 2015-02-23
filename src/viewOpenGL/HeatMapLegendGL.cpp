/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "HeatMapLegendGL.h"

#include "math/Common.h"
#include "color/HeatMap.h"

#include <QPainter>
#include <QImage>
#include <QApplication>
#include <QVector2D>
#include <QLabel>

static const qreal legend_x = 0.0;
static const qreal legend_y = 0.0;
static const qreal legend_width = 25.0;
static const qreal legend_height = 150.0;
static const qreal bars_width = 35.0;

HeatMapLegendGL::HeatMapLegendGL(QObject* parent)
    : GraphicItemGL(parent),
      m_maxReads(1),
      m_minReads(1),
      m_minGenes(1),
      m_maxGenes(1),
      m_colorComputingMode(Globals::LinearColor),
      m_texture(QOpenGLTexture::Target2D),
      m_textureText(QOpenGLTexture::Target2D),
      m_valueComputation(Reads)
{
    setVisualOption(GraphicItemGL::Transformable, false);
    setVisualOption(GraphicItemGL::Visible, false);
    setVisualOption(GraphicItemGL::Selectable, false);
    setVisualOption(GraphicItemGL::Yinverted, false);
    setVisualOption(GraphicItemGL::Xinverted, false);
    setVisualOption(GraphicItemGL::RubberBandable, false);
}

HeatMapLegendGL::~HeatMapLegendGL()
{
    clearData();
}

void HeatMapLegendGL::clearData()
{
    if (m_texture.isCreated()) {
        m_texture.release();
        m_texture.destroy();
    }

    m_texture_vertices.clear();
    m_texture_cords.clear();
    m_texture_cords.clear();

    if (m_textureText.isCreated()) {
        m_textureText.release();
        m_textureText.destroy();
    }

    //m_valueComputation = Reads;
}

void HeatMapLegendGL::draw()
{
    glEnable(GL_TEXTURE_2D);
    {
        m_texture.bind();
        glBegin(GL_QUADS);
        {
            for (int i = 0; i < m_texture_vertices.size(); ++i) {
                glTexCoord2f(m_texture_cords.at(i).x(), m_texture_cords.at(i).y());
                glVertex2f(m_texture_vertices.at(i).x(), m_texture_vertices.at(i).y());
            }
        }
        glEnd();
        m_texture.release();

        glBegin(GL_LINE_LOOP);
        {
            glColor4f(1.0, 1.0, 1.0, 1);
            foreach(QVector2D indice, m_texture_vertices) {
                glVertex2f(indice.x(), indice.y());
            }
            glEnd();
        }

        // draw text (add 5 pixels offset to the right)
        const int min = m_valueComputation == Reads ? m_minReads : m_minGenes;
        const int max = m_valueComputation == Reads ? m_maxReads : m_maxGenes;
        drawText(QPointF(legend_x + legend_width + 5, 0), QString::number(max));
        drawText(QPointF(legend_x + legend_width + 5, legend_height), QString::number(min));
    }
    glDisable(GL_TEXTURE_2D);
}

void HeatMapLegendGL::setSelectionArea(const SelectionEvent *)
{

}

void HeatMapLegendGL::setLowerLimitReads(const int limit)
{
    if (m_minReads != limit) {
        m_minReads = limit;
        generateHeatMap();
    }
}

void HeatMapLegendGL::setUpperLimitReads(const int limit)
{
    if (m_maxReads != limit) {
        m_maxReads = limit;
        generateHeatMap();
    }
}

void HeatMapLegendGL::setValueComputation(ValueComputation mode)
{
    if (m_valueComputation != mode) {
        m_valueComputation = mode;
        generateHeatMap();
    }
}

void HeatMapLegendGL::setColorComputingMode(const Globals::GeneColorMode &mode)
{
    // update color computing mode
    if (m_colorComputingMode != mode) {
        m_colorComputingMode = mode;
        generateHeatMap();
    }
}

void HeatMapLegendGL::setLowerLimitGenes(const int limit)
{
    if (m_minGenes != limit) {
        m_minGenes = limit;
        generateHeatMap();
    }
}

void HeatMapLegendGL::setUpperLimitGenes(const int limit)
{
    if (m_maxGenes != limit) {
        m_maxGenes = limit;
        generateHeatMap();
    }
}

void HeatMapLegendGL::generateHeatMap()
{
    clearData();

    const int min = m_valueComputation == Reads ? m_minReads : m_minGenes;
    const int max = m_valueComputation == Reads ? m_maxReads : m_maxGenes;

    // generate image texture
    QImage image(legend_width, legend_height, QImage::Format_ARGB32);
    //here we can chose the type of Spectrum (linear, log or exp) and the type
    //of color mapping (wavelenght or linear interpolation)
    Heatmap::createHeatMapImage(image, min, max, m_colorComputingMode);

    m_texture.create();
    m_texture.setData(image);
    m_texture.setMinificationFilter(QOpenGLTexture::Linear);
    m_texture.setMagnificationFilter(QOpenGLTexture::Linear);
    m_texture.setWrapMode(QOpenGLTexture::ClampToEdge);

    m_texture_vertices.append(QVector2D(legend_x, legend_y));
    m_texture_vertices.append(QVector2D(legend_x + legend_width, legend_y));
    m_texture_vertices.append(QVector2D(legend_x + legend_width, legend_y + legend_height));
    m_texture_vertices.append(QVector2D(legend_x, legend_y + legend_height));

    m_texture_cords.append(QVector2D(0.0, 0.0));
    m_texture_cords.append(QVector2D(1.0, 0.0));
    m_texture_cords.append(QVector2D(1.0, 1.0));
    m_texture_cords.append(QVector2D(0.0, 1.0));

    emit updated();
}

void HeatMapLegendGL::drawText(const QPointF &posn, const QString& str)
{
    QFont monoFont("Courier", 12, QFont::Normal);
    QFontMetrics metrics(monoFont);
    QRect textRect = metrics.boundingRect(str);

    QImage image(textRect.size(), QImage::Format_ARGB32);
    image.fill(0);
    QPainter qpainter(&image);
    qpainter.setFont(monoFont);
    qpainter.setPen(Qt::white);
    qpainter.setRenderHint(QPainter::Antialiasing, true);
    qpainter.drawText(textRect.x(), metrics.ascent(), str);
    qpainter.end();

    m_textureText.destroy();
    m_textureText.create();
    m_textureText.setData(image.mirrored());
    const int x = posn.x();
    const int y = posn.y();

    QVector<QVector2D> vertices;
    vertices.append(QVector2D(x + textRect.x(), y + metrics.ascent()));
    vertices.append(QVector2D(x + textRect.x(), y - metrics.descent()));
    vertices.append(QVector2D(x + textRect.x() + textRect.width(), y - metrics.descent()));
    vertices.append(QVector2D(x + textRect.x() + textRect.width(), y + metrics.ascent()));

    QVector<QVector2D> texCoord;
    texCoord.append(QVector2D(0.0, 0.0));
    texCoord.append(QVector2D(0.0, 1.0));
    texCoord.append(QVector2D(1.0, 1.0));
    texCoord.append(QVector2D(1.0, 0.0));

    m_textureText.bind();
    glBegin(GL_QUADS);
    {
        for (int i = 0; i < vertices.size(); ++i) {
            glTexCoord2f(texCoord.at(i).x(), texCoord.at(i).y());
            glVertex2f(vertices.at(i).x(), vertices.at(i).y());
        }
    }
    glEnd();
    m_textureText.release();
}

const QRectF HeatMapLegendGL::boundingRect() const
{
    return QRectF(legend_x, legend_y,
                  legend_width + bars_width, legend_height);
}
