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

GenesTableView::GenesTableView(QWidget *parent)
    : QTableView(parent),
      m_geneModel(nullptr),
      m_sortGenesProxyModel(nullptr)
{
    // model
    m_geneModel = new GeneFeatureItemModel(this);
    
    // sorting model
    m_sortGenesProxyModel = new SortGenesProxyModel(this);
    m_sortGenesProxyModel->setSourceModel(m_geneModel);
    m_sortGenesProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_sortGenesProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_sortGenesProxyModel->setFilterRegExp(QRegExp("(ambiguous)*|(^[0-9])*",
                                            Qt::CaseInsensitive)); //not show ambiguous genes or numbers
    setModel(m_sortGenesProxyModel);

    setSortingEnabled(true);
    sortByColumn(GeneFeatureItemModel::Name, Qt::AscendingOrder);
    horizontalHeader()->setSortIndicatorShown(true);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setEditTriggers(QAbstractItemView::AllEditTriggers);

    resizeColumnsToContents();
    resizeRowsToContents();

    horizontalHeader()->setSectionResizeMode(GeneFeatureItemModel::Name, QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(GeneFeatureItemModel::Color, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(GeneFeatureItemModel::Show, QHeaderView::Fixed);

    model()->submit(); //support for caching (speed up)

    verticalHeader()->hide();
}

GenesTableView::~GenesTableView()
{
    m_geneModel->deleteLater();
    m_geneModel = nullptr;

    m_sortGenesProxyModel->deleteLater();
    m_sortGenesProxyModel = nullptr;
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
