/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/
#include "GenesTableView.h"

#include <QSortFilterProxyModel>
#include <QHeaderView>

#include "model/SortGenesProxyModel.h"
#include "model/GeneFeatureItemModel.h"

GenesTableView::GenesTableView(QWidget* parent)
    : QTableView(parent)
    , m_geneModel(nullptr)
    , m_sortGenesProxyModel(nullptr)
{
    // model
    m_geneModel = new GeneFeatureItemModel(this);

    // sorting model
    m_sortGenesProxyModel = new SortGenesProxyModel(this);
    m_sortGenesProxyModel->setSourceModel(m_geneModel);
    m_sortGenesProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_sortGenesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    // this is important because sort proxy will use the column 0 by default
    m_sortGenesProxyModel->setFilterKeyColumn(GeneFeatureItemModel::Name);
    setModel(m_sortGenesProxyModel);

    setSortingEnabled(true);
    setShowGrid(true);
    setWordWrap(true);
    setAlternatingRowColors(true);
    sortByColumn(GeneFeatureItemModel::Name, Qt::AscendingOrder);

    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);
    setGridStyle(Qt::SolidLine);
    setCornerButtonEnabled(false);
    setLineWidth(1);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    horizontalHeader()->setSectionResizeMode(GeneFeatureItemModel::Name, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(GeneFeatureItemModel::Color, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(GeneFeatureItemModel::Color, 50);
    horizontalHeader()->setSectionResizeMode(GeneFeatureItemModel::Show, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(GeneFeatureItemModel::Show, 50);
    horizontalHeader()->setSortIndicatorShown(true);
    verticalHeader()->hide();

    model()->submit(); // support for caching (speed up)
}

GenesTableView::~GenesTableView()
{
}

QItemSelection GenesTableView::geneTableItemSelection() const
{
    const auto selected = selectionModel()->selection();
    return m_sortGenesProxyModel->mapSelectionToSource(selected);
}

void GenesTableView::setGeneNameFilter(QString str)
{
    m_sortGenesProxyModel->setFilterFixedString(str);
}
