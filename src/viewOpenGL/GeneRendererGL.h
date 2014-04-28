/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/
#ifndef GENERENDERERGL_H
#define GENERENDERERGL_H

#include "GraphicItemGL.h"

#include "math/QuadTree.h"
#include "SelectionEvent.h"
#include "data/DataProxy.h"
#include "utils/Utils.h"
#include "dataModel/GeneSelection.h"
#include "GeneData.h"

class QGLPainter;
class QGLTexture2D;
class QVector2DArray;
class QGLShaderProgramEffect;

class GeneRendererGL : public GraphicItemGL
{
    Q_OBJECT

public:

    // selection set
    typedef QVector<GeneSelection> GeneSelectedSet;
    typedef QMap<QString, GeneSelection> GeneInfoSelected;

    explicit GeneRendererGL(QObject *parent = 0);
    virtual ~GeneRendererGL();

    // data builders
    void generateData();
    void clearData();

    //set the dimensions of the bounding rect, also for the QuadTree
    void setDimensions(const QRectF border);

    //selection functions
    void selectGenes(const DataProxy::GeneList&);
    void selectFeatures(const DataProxy::FeatureList&);
    GeneSelectedSet getSelectedFeatures();

public slots:

    void setIntensity(qreal intensity);
    void setSize(qreal size);
    void setShine(qreal shine);

    void setShape(Globals::GeneShape shape);

    void setLowerLimit(int limit);
    void setUpperLimit(int limit);

    void setVisualMode(const Globals::GeneVisualMode &mode);

    void setHitCount(int min, int max, int pooledMin, int pooledMax);

    void updateColor(DataProxy::GenePtr);
    void updateSelection(DataProxy::GenePtr);

    void updateAllColor(const QColor color);
    void updateAllSelection(bool selected);

    void clearSelection();

    void setSelectionArea(const SelectionEvent *event);

    void draw(QGLPainter *painter);

    const QRectF boundingRect() const;

signals:

    void selectionUpdated();

private:

    //adds a new GeneSelection to the map
    //if not present creates if present accumulates reads
    void addGeneSelection(const DataProxy::FeaturePtr);
    void delGeneSelection(const DataProxy::FeaturePtr);

    // internal rendering functions
    void updateSize();
    void updateVisual();

    // reset quad tree to rect size
    void resetQuadTree(const QRectF &rect);

    // compiles and loads the shaders
    void setupShaders();

    // lookup maps
    typedef QHash<DataProxy::FeaturePtr, int> GeneInfoByIdMap;
    typedef QHash<int, DataProxy::FeaturePtr> GeneInfoReverseMap;

    // lookup quadtree
    typedef QuadTree<int, 8> GeneInfoQuadTree;

    // gene visual data
    GeneData m_geneData;
    QGLSceneNode *m_geneNode = nullptr;

    // gene lookup data
    GeneInfoByIdMap m_geneInfoById;
    GeneInfoReverseMap m_geneInfoReverse; //TODO can probably be removed
    GeneInfoQuadTree m_geneInfoQuadTree;

    // gene selection data
    GeneInfoSelected m_geneInfoSelection;

    // visual attributes
    qreal m_intensity;
    qreal m_size;
    qreal m_shine;
    Globals::GeneShape m_shape;

    // threshold limits for gene hits
    int m_thresholdLower;
    int m_thresholdUpper;
    int m_thresholdLowerPooled;
    int m_thresholdUpperPooled;

    // dataset Statistics
    int m_min;
    int m_max;
    int m_pooledMin;
    int m_pooledMax;

    // bounding rect area
    QRectF m_border;

    // visual mode
    Globals::GeneVisualMode m_visualMode;

    // shader programs
    typedef QMap<uint, QGLShaderProgramEffect *> ShaderProgramList;
    ShaderProgramList m_shaderProgramList;
    QGLShaderProgramEffect *m_shaderProgram = nullptr;

    // tells if something has changed
    bool m_isDirty = false;

    Q_DISABLE_COPY(GeneRendererGL)
};


#endif // GENERENDERERGL_H
