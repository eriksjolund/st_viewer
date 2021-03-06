#ifndef GENESWIDGET_H
#define GENESWIDGET_H

#include <QDockWidget>
#include <QIcon>

#include "data/DataProxy.h"

class QPushButton;
class QLineEdit;
class GenesTableView;
class GeneFeatureItemModel;
class QSortFilterProxyModel;
class QColorDialog;

// This widget is componsed of the genes table
// a search field and the select and action menus so the user can
// select/deselect genes
// and change their color and/or threshold.
// Every action will affect what genes are shown in the cell view.
// Here when a dataset is opened its unique genes will be shown
// so the user can interact with them to visualize them in the cell view
class GenesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GenesWidget(QSharedPointer<DataProxy> dataProxy, QWidget *parent = 0);
    virtual ~GenesWidget();

    // clear focus/status and selections
    void clear();

    // forces an update of the table
    void updateModelTable();

signals:

    // signals emitted when the user selects or change colors of genes in the
    // table
    void signalSelectionChanged(DataProxy::GeneList);
    void signalColorChanged(DataProxy::GeneList);
    void signalCutOffChanged(DataProxy::GenePtr);

public slots:

    // the user has opened/edit/removed  a dataset
    void slotDatasetOpen(const QString &datasetId);
    void slotDatasetUpdated(const QString &datasetId);
    void slotDatasetRemoved(const QString &datasetId);

private slots:

    // slots triggered by the show/color controls in the gene table
    void slotSetColorAllSelected(const QColor &color);
    void slotSetVisibilityForSelectedRows(bool visible);
    void slotHideAllSelected();
    void slotShowAllSelected();

private:
    // internal function to configure created buttons
    // to avoid code duplication
    // TODO better approach would be to have factories somewhere else
    void configureButton(QPushButton *button, const QIcon &icon, const QString &tooltip);

    // internal function to retrieve the model and the proxy model of the gene
    // table
    QSortFilterProxyModel *getProxyModel();
    GeneFeatureItemModel *getModel();

    // some references needed to UI elements
    QScopedPointer<QLineEdit> m_lineEdit;
    QScopedPointer<GenesTableView> m_genes_tableview;
    QScopedPointer<QColorDialog> m_colorList;

    // reference to dataProxy
    QSharedPointer<DataProxy> m_dataProxy;

    Q_DISABLE_COPY(GenesWidget)
};

#endif // GENESWIDGET_H
