/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms. 
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#ifndef GENEEXPORTER_H
#define GENEEXPORTER_H

#include <QObject>
#include <QSharedPointer>
#include <QVector>

#include "model/FeatureExtended.h"

class QIODevice;

// Interface defining gene export functionality
class GeneExporter : public QObject
{
public:
    
    typedef FeatureExtended FeatureType;
    typedef QSharedPointer<FeatureType> FeaturePtr;
    typedef QVector<FeaturePtr> FeatureList;

    GeneExporter(QObject *parent = 0);
    virtual ~GeneExporter();

    virtual void exportItem(QIODevice *device, const FeatureList &featureList, const QObject &context) const = 0;

    typedef QList<QString> PropertyList;
    void addExportProperty(const QString &property);
    void addExportProperty(const PropertyList &properties);

    static const QString encodePropertyList(const PropertyList &properties);
    static const PropertyList decodePropertyList(const QString &properties);

protected:
    
    static const QString PROPERTY_LIST_DELIMITER;
    const QList<QString> &exportPropertyList() const;

    PropertyList m_propertyList;
};

#endif // GENEEXPORTER_H //
