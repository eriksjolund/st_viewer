/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "ColorListEditor.h"

#include <QVariant>
#include <QPixmap>
#include <QDebug>

#include "color/ColorPalette.h"

ColorListEditor::ColorListEditor(QWidget* widget)
    : QComboBox(widget)
{
    HSVPalette palette(this);
    populateList(&palette);
}

ColorListEditor::~ColorListEditor()
{

}

const QColor ColorListEditor::color() const
{
    return qvariant_cast<QColor>(itemData(currentIndex(), Qt::UserRole));
}

void ColorListEditor::setColor(const QColor& color)
{
    const int res = findData(color, Qt::UserRole);
    Q_ASSERT(res != -1);
    setCurrentIndex(res);
}

void ColorListEditor::populateList(const ColorPalette* palette)
{
    const ColorPalette::ColorList colorList = palette->colorList();
    for (int i = 0; i < colorList.size(); ++i) {
        const ColorPalette::ColorPair pair = colorList[i];
        QPixmap pix(QSize(20,20));
        pix.fill(pair.first);
        insertItem(i, QIcon(pix), pair.second, pair.first);
    }
}
