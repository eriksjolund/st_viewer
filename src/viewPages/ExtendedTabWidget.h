/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#ifndef EXTENDEDTABWIDGET_H
#define EXTENDEDTABWIDGET_H

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QMouseEvent>
#include <QPointer>

class InitPage;
class DatasetPage;
class CellViewPage;
class ExperimentPage;
class QIcon;
class QComboBox;
class QStackedWidget;
class QVBoxLayout;
class QHBoxLayout;
class Error;
class AuthorizationManager;
class DataProxy;

// This class is a customized tab manager based on a QStackedWidget and a QButtonGroup.
// It pretty much has the same functionality as the QTabWidget and it is used as the main view
// of the application so users can navigate different pages.
// The pages are member variables
class ExtendedTabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ExtendedTabWidget(QPointer<DataProxy> dataProxy,
                               QPointer<AuthorizationManager> authManager,
                               QWidget* parent = 0);
    virtual ~ExtendedTabWidget();

    // simple functions needed here for convenience
    QSize sizeHint() const;
    int count() const;
    int currentIndex() const;

public slots:

    // TODO slots should have the prefix "slot"

    // navigation signals
    void moveToNextPage();
    void moveToPreviousPage();

    // sets the start page as current page going trough all
    // the previuos pages (needed when we clean cache)
    void resetStatus();

private slots:

    // to handle when the user clicks on a tab's button
    void slotTabPressed(const int index);

private:
    // returns the widget(Page) for a given index
    QWidget* widget(int index);

    // adds page to the tab manager
    void insertPage(QWidget* page, const QIcon& icon = QIcon(), const QString& title = QString());

    // set the current index in the tab manager
    void setCurrentIndex(int index);

    // makes a page transition
    void tabChanged(int toIndex, int fromIndex = -1);

    // helper function to instanciate the pages and create the connections
    void createPages();
    void createConnections();

    // pages
    QPointer<InitPage> m_startpage;
    QPointer<DatasetPage> m_datasets;
    QPointer<CellViewPage> m_cellview;
    QPointer<ExperimentPage> m_experiments;

    // UI components
    QPointer<QStackedWidget> m_stackWidget;
    QPointer<QButtonGroup> m_buttonGroup;
    QPointer<QHBoxLayout> m_layout;
    QPointer<QVBoxLayout> m_buttonLayout;

    // refence to dataProxy and auth manager so it can be passed to the pages
    QPointer<DataProxy> m_dataProxy;
    QPointer<AuthorizationManager> m_authManager;

    Q_DISABLE_COPY(ExtendedTabWidget)
};
#endif /* // EXTENDEDTABWIDGET_H */
