#ifndef DATASETSTABLEVIEW_H
#define DATASETSTABLEVIEW_H

#include <QTableView>
#include <QPointer>

class DatasetItemModel;
class QSortFilterProxyModel;

// An abstraction of QTableView for the datasets table
class DatasetsTableView : public QTableView
{
    Q_OBJECT

public:
    explicit DatasetsTableView(QWidget *parent = 0);
    virtual ~DatasetsTableView();

    // returns the current selection mapped to the sorting model
    QItemSelection datasetsTableItemSelection() const;

private:
    // references to model and proxy model
    QScopedPointer<DatasetItemModel> m_datasetModel;
    QScopedPointer<QSortFilterProxyModel> m_sortDatasetsProxyModel;

    Q_DISABLE_COPY(DatasetsTableView)
};

#endif // DATASETSTABLEVIEW_H //
