/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#ifndef HEATMAP_H
#define HEATMAP_H

#include "math/Common.h"
#include "utils/Utils.h"

class QColor4ub;
class QImage;

// Heatmap is a convenience class containing functions to generate
// heatmap related data.
class Heatmap
{

public:
    enum SpectrumMode { SpectrumLinear, SpectrumLog, SpectrumExp };

    enum InterpolationColorMode { SpectrumRaibow, SpectrumLinearInterpolation };

    // convenience function to generate a heatmap spectrum image given specific mapping function
    // using the wave lenght spectra or a linear interpolation spectra between two colors
    // the input image will be transformed with the new colors
    static void createHeatMapImage(QImage& image,
                                   const qreal lowerbound,
                                   const qreal upperbound,
                                   const Globals::GeneColorMode& colorMode);

    // convenience function to generate a QColor color from a real value
    static QColor createHeatMapWaveLenghtColor(const qreal value);

    // convenience function to generate a QColor color from a real value given a range
    static QColor createHeatMapLinearColor(const qreal value, const qreal min, const qreal max);

    // convenience function to adjust the input value using a Linear - Exponential or Logaritmic
    // function
    static qreal normalizeValueSpectrumFunction(const qreal value,
                                                const Globals::GeneColorMode& colorMode);
};

#endif // HEATMAP_H //
