
/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "CellViewPage.h"

#include <QDebug>
#include <QMetaObject>
#include <QMetaMethod>
#include <QString>
#include <QStringList>
#include <QtConcurrent>
#include <QFuture>
#include <QPrintDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPrinter>
#include <QColorDialog>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>
#include <QWidgetAction>
#include <QComboBox>
#include <QSlider>
#include <QRadioButton>
#include <QGroupBox>
#include <QFutureWatcher>

#include "error/Error.h"
#include "io/GeneExporter.h"
#include "io/FeatureExporter.h"
#include "utils/Utils.h"
#include "dialogs/SelectionDialog.h"
#include "dialogs/CreateSelectionDialog.h"
#include "viewOpenGL/CellGLView.h"
#include "viewOpenGL/ImageTextureGL.h"
#include "viewOpenGL/GridRendererGL.h"
#include "viewOpenGL/HeatMapLegendGL.h"
#include "viewOpenGL/MiniMapGL.h"
#include "viewOpenGL/GeneRendererGL.h"
#include "analysis/AnalysisFRD.h"
#include "customWidgets/SpinBoxSlider.h"
#include "utils/SetTips.h"
#include <algorithm>

#include "ui_cellview.h"

static const int GENE_INTENSITY_MIN = 1;
static const int GENE_INTENSITY_MAX = 10;
static const int GENE_SIZE_MIN = 5;
static const int GENE_SIZE_MAX = 30;
static const int BRIGHTNESS_MIN = 1;
static const int BRIGHTNESS_MAX = 10;

using namespace Globals;

namespace
{

void addWidgetToMenu(const QString &str, QMenu *menu, QWidget *widget)
{
    Q_ASSERT(menu != nullptr);
    Q_ASSERT(widget != nullptr);
    QWidgetAction *widgetAction = new QWidgetAction(menu);
    widgetAction->setDefaultWidget(widget);
    menu->addAction(str);
    menu->addAction(widgetAction);
}

void addSliderToMenu(QWidget *parent, const QString &str,
                     const QString &tooltipStr, const QString &statustipStr,
                     QMenu *menu, QSlider **slider_ptr, int rangeMin,
                     int rangeMax)
{

    Q_ASSERT(parent != nullptr);
    Q_ASSERT(*slider_ptr == nullptr);
    Q_ASSERT(menu != nullptr);

    QSlider *slider = new QSlider(parent);
    *slider_ptr = slider;
    slider->setRange(rangeMin, rangeMax);
    slider->setSingleStep(1);
    slider->setValue(rangeMax);
    slider->setInvertedAppearance(false);
    slider->setInvertedControls(false);
    slider->setTracking(true);
    slider->setOrientation(Qt::Horizontal);
    slider->setTickPosition(QSlider::TicksAbove);
    slider->setTickInterval(1);
    slider->setToolTip(tooltipStr);
    slider->setStatusTip(statustipStr);
    addWidgetToMenu(str, menu, slider);
}

bool imageFormatHasWriteSupport(const QString &format) {
    QStringList supportedImageFormats;
    for (auto imageformat : QImageWriter::supportedImageFormats()) {
        supportedImageFormats << QString(imageformat).toLower();
    }
    return (std::find(supportedImageFormats.begin(),
                      supportedImageFormats.end(),
                      format) != supportedImageFormats.end());
}

}


CellViewPage::CellViewPage(QPointer<DataProxy> dataProxy, QWidget *parent)
    : Page(dataProxy, parent),
      m_minimap(nullptr),
      m_legend(nullptr),
      m_gene_plotter(nullptr),
      m_image(nullptr),
      m_grid(nullptr),
      m_view(nullptr),
      m_colorDialogGrid(nullptr),
      m_ui(new Ui::CellView()),
      m_FDH(nullptr),
      m_colorLinear(nullptr),
      m_colorLog(nullptr),
      m_colorExp(nullptr),
      m_poolingGenes(nullptr),
      m_poolingReads(nullptr),
      m_poolingTPMs(nullptr),
      m_geneHitsThreshold(nullptr),
      m_geneIntensitySlider(nullptr),
      m_geneSizeSlider(nullptr),
      m_geneShineSlider(nullptr),
      m_geneBrightnessSlider(nullptr),
      m_geneShapeComboBox(nullptr)
{
    m_ui->setupUi(this);
    //setting style to main UI Widget (frame and widget must be set specific to avoid propagation)
    setWindowFlags(Qt::FramelessWindowHint);
    m_ui->cellViewPageWidget->setStyleSheet("QWidget#cellViewPageWidget " + PAGE_WIDGETS_STYLE);
    m_ui->frame->setStyleSheet("QFrame#frame " + PAGE_FRAME_STYLE);

    // instantiante FDH
    m_FDH = new AnalysisFRD();
    Q_ASSERT(m_FDH);

    // color dialogs
    m_colorDialogGrid = new QColorDialog(GridRendererGL::DEFAULT_COLOR_GRID, this);
    // OSX native color dialog gives problems
    m_colorDialogGrid->setOption(QColorDialog::DontUseNativeDialog, true);

    // init OpenGL graphical objects
    initGLView();

    //create menus and connections
    createMenusAndConnections();

    //connect data proxy signals
    connect(m_dataProxy.data(),
            SIGNAL(signalImageAlignmentDownloaded(DataProxy::DownloadStatus)),
            this, SLOT(slotImageAlignmentDownloaded(DataProxy::DownloadStatus)));
    connect(m_dataProxy.data(),
            SIGNAL(signalDatasetContentDownloaded(DataProxy::DownloadStatus)),
            this, SLOT(slotDatasetContentDownloaded(DataProxy::DownloadStatus)));
}

CellViewPage::~CellViewPage()
{
    m_minimap->deleteLater();
    m_minimap = nullptr;

    m_legend->deleteLater();
    m_legend = nullptr;

    m_gene_plotter->deleteLater();
    m_gene_plotter = nullptr;

    m_image->deleteLater();
    m_image = nullptr;

    m_grid->deleteLater();
    m_grid = nullptr;

    m_view->deleteLater();
    m_view = nullptr;

    m_colorDialogGrid->deleteLater();
    m_colorDialogGrid = nullptr;

    m_FDH->deleteLater();
    m_FDH = nullptr;
}

void CellViewPage::onEnter()
{
    //reset tool buttons
    setEnableButtons(false);

    if (m_dataProxy->getSelectedDataset().isNull()) {
        //no dataset selected, nothing to do here
        setStatusTip(tr("No dataset loaded"));
        return;
    }

    setWaiting(true);
    //first we need to download the image alignment
    m_dataProxy->loadImageAlignment();
    m_dataProxy->activateCurrentDownloads();

}

void CellViewPage::slotImageAlignmentDownloaded(const DataProxy::DownloadStatus status)
{
    // if status == OK
    if (status == DataProxy::Success) {
        // download the rest of the content
        m_dataProxy->loadDatasetContent();
        m_dataProxy->activateCurrentDownloads();
    } else {
        setWaiting(false);
    }
}

void CellViewPage::slotDatasetContentDownloaded(const DataProxy::DownloadStatus status)
{
    //if error or abort
    if (status != DataProxy::Success) {
        setWaiting(false);
        return;
    }

    // enable toolbar controls
    setEnableButtons(true);

    // update Status tip with the name of the currently selected dataset
    const auto dataset = m_dataProxy->getSelectedDataset();
    Q_ASSERT(!dataset.isNull());
    setStatusTip(tr("Dataset loaded %1").arg(dataset->name()));

    // get image alignment and chip
    const auto imageAlignment = m_dataProxy->getImageAlignment();
    Q_ASSERT(!imageAlignment.isNull());
    const auto currentChip = m_dataProxy->getChip();
    Q_ASSERT(!currentChip.isNull());
    const QTransform alignment = imageAlignment->alignment();
    const qreal min = dataset->statisticsMin();
    const qreal max = dataset->statisticsMax();
    const QRectF chip_rect = QRectF(
                QPointF(currentChip->x1(), currentChip->y1()),
                QPointF(currentChip->x2(), currentChip->y2())
                );
    const QRectF chip_border = QRectF(
                QPointF(currentChip->x1Border(), currentChip->y1Border()),
                QPointF(currentChip->x2Border(), currentChip->y2Border())
                );

    // load features and threshold boundaries into FDH
    m_FDH->computeData(m_dataProxy->getFeatureList(), min, max);

    // updade grid size and data
    m_grid->setDimensions(chip_border, chip_rect);
    m_grid->generateData();
    m_grid->setTransform(alignment);

    // update gene size and data
    m_gene_plotter->setDimensions(chip_border);
    m_gene_plotter->generateData();
    m_gene_plotter->setTransform(alignment);
    m_gene_plotter->setHitCount(min, max);

    // load cell tissue (async)
    slotLoadCellFigure();

    // load min-max to the threshold sliders in tool bar
    // do this the last as it will trigger some slots in the gene_plotter and FDH
    m_geneHitsThreshold->setMinimumValue(min);
    m_geneHitsThreshold->setMaximumValue(max);
    m_geneHitsThreshold->setLowerValue(min);
    m_geneHitsThreshold->setUpperValue(max);
    m_geneHitsThreshold->setTickInterval(1);

    //load min max values to legend
    m_legend->setLowerLimit(min);
    m_legend->setUpperLimit(max);

    // reset main variabless
    resetActionStates();

    // refresh view
    m_view->update();

    setWaiting(false);
}

void CellViewPage::setEnableButtons(bool enable)
{
    m_ui->selection->setEnabled(enable);
    m_ui->save->setEnabled(enable);
    m_ui->print->setEnabled(enable);
    m_ui->regexpselection->setEnabled(enable);
    m_ui->zoomin->setEnabled(enable);
    m_ui->zoomout->setEnabled(enable);
    m_ui->histogram->setEnabled(enable);
    m_ui->genemenu->setEnabled(enable);
    m_ui->cellmenu->setEnabled(enable);
    m_ui->genesWidget->setEnabled(enable);
    m_ui->selectionsWidget->setEnabled(enable);
    m_ui->area->setEnabled(enable);
}

void CellViewPage::onExit()
{
    //reset genes and selections model
    m_ui->genesWidget->clear();
    m_ui->selectionsWidget->clear();

    //reset visualization objects
    m_grid->clearData();
    m_image->clearData();
    m_gene_plotter->clearData();
    m_view->clearData();
    m_view->update();
}

//TODO split into two
void CellViewPage::createMenusAndConnections()
{
    //set some default properties for some actions
    m_ui->actionShow_toggleNormal->setProperty("mode", GeneRendererGL::NormalMode);
    m_ui->actionShow_toggleDynamicRange->setProperty("mode", GeneRendererGL::DynamicRangeMode);
    m_ui->actionShow_toggleHeatMap->setProperty("mode", GeneRendererGL::HeatMapMode);
    m_ui->action_toggleLegendTopRight->setProperty("mode", Globals::Anchor::NorthEast);
    m_ui->action_toggleLegendTopLeft->setProperty("mode", Globals::Anchor::NorthWest);
    m_ui->action_toggleLegendDownRight->setProperty("mode", Globals::Anchor::SouthEast);
    m_ui->action_toggleLegendDownLeft->setProperty("mode", Globals::Anchor::SouthWest);
    m_ui->action_toggleMinimapTopRight->setProperty("mode", Globals::Anchor::NorthEast);
    m_ui->action_toggleMinimapTopLeft->setProperty("mode", Globals::Anchor::NorthWest);
    m_ui->action_toggleMinimapDownRight->setProperty("mode", Globals::Anchor::SouthEast);
    m_ui->action_toggleMinimapDownLeft->setProperty("mode", Globals::Anchor::SouthWest);

    // menu gene plotter actions
    QMenu *menu_genePlotter = new QMenu(this);
    menu_genePlotter->setTitle(tr("Gene Plotter"));
    setToolTipAndStatusTip(tr("Tools for visualization of genes"), menu_genePlotter);
    menu_genePlotter->addAction(m_ui->actionShow_showGrid);
    menu_genePlotter->addAction(m_ui->actionShow_showGenes);
    menu_genePlotter->addSeparator();
    menu_genePlotter->addAction(m_ui->actionColor_selectColorGrid);
    menu_genePlotter->addSeparator();

    //color modes
    QActionGroup *actionGroup_toggleVisualMode = new QActionGroup(menu_genePlotter);
    actionGroup_toggleVisualMode->setExclusive(true);
    actionGroup_toggleVisualMode->addAction(m_ui->actionShow_toggleNormal);
    actionGroup_toggleVisualMode->addAction(m_ui->actionShow_toggleDynamicRange);
    actionGroup_toggleVisualMode->addAction(m_ui->actionShow_toggleHeatMap);
    menu_genePlotter->addActions(actionGroup_toggleVisualMode->actions());
    menu_genePlotter->addSeparator();

    //color computation modes
    QGroupBox *colorComputationMode = new QGroupBox(this);
    colorComputationMode->setFlat(true);
    setToolTipAndStatusTip(
            tr("Chooses the type of color computation."),
            colorComputationMode);
    m_colorLinear = new QRadioButton(tr("Linear"));
    m_colorLog = new QRadioButton(tr("Log"));
    m_colorExp = new QRadioButton(tr("Exp"));
    m_colorLinear->setChecked(true);
    QHBoxLayout *hboxColor = new QHBoxLayout();
    hboxColor->addWidget(m_colorLinear);
    hboxColor->addWidget(m_colorLog);
    hboxColor->addWidget(m_colorExp);
    hboxColor->addStretch(1);
    colorComputationMode->setLayout(hboxColor);
    addWidgetToMenu(tr("Color computation:"), menu_genePlotter, colorComputationMode);

    //color modes
    QGroupBox *poolingMode = new QGroupBox(this);
    poolingMode->setFlat(true);
    setToolTipAndStatusTip(
            tr("Chooses the type of pooling mode for dynamic range and heatmap."),
            poolingMode);
    m_poolingGenes = new QRadioButton(tr("Genes"));
    m_poolingReads = new QRadioButton(tr("Reads"));
    m_poolingTPMs = new QRadioButton(tr("TPMs"));
    m_poolingReads->setChecked(true);
    QHBoxLayout *hboxPooling = new QHBoxLayout();
    hboxPooling->addWidget(m_poolingReads);
    hboxPooling->addWidget(m_poolingGenes);
    hboxPooling->addWidget(m_poolingTPMs);
    hboxPooling->addStretch(1);
    poolingMode->setLayout(hboxPooling);
    addWidgetToMenu(tr("Pooling modes:"), menu_genePlotter, poolingMode);
    menu_genePlotter->addSeparator();

    //threshold slider
    m_geneHitsThreshold = new SpinBoxSlider(this, SpinBoxSlider::onlySpinBoxes);
    setToolTipAndStatusTip(
            tr("Limit of the number of reads per feature."),
            m_geneHitsThreshold);
    addWidgetToMenu(tr("Transcripts Threshold:"), menu_genePlotter, m_geneHitsThreshold);
    menu_genePlotter->addSeparator();

    // transcripts intensity and size sliders
    addSliderToMenu(this,
                    tr("Opacity:"),
                    tr("Set the intensity of the genes"),
                    tr("Set the intensity of the genes"),
                    menu_genePlotter,
                    &m_geneIntensitySlider,
                    GENE_INTENSITY_MIN,
                    GENE_INTENSITY_MAX);
    addSliderToMenu(this,
                    tr("Size:"),
                    tr("Set the size of the genes"),
                    tr("Set the size of the genes"),
                    menu_genePlotter,
                    &m_geneSizeSlider,
                    GENE_SIZE_MIN,
                    GENE_SIZE_MAX);
    menu_genePlotter->addSeparator();

    // shape of the genes
    m_geneShapeComboBox = new QComboBox(this);
    m_geneShapeComboBox->addItem("Circles", GeneRendererGL::Circle);
    m_geneShapeComboBox->addItem("Crosses", GeneRendererGL::Cross);
    m_geneShapeComboBox->addItem("Squares", GeneRendererGL::Square);
    m_geneShapeComboBox->setCurrentIndex(GeneRendererGL::Circle);
    setToolTipAndStatusTip(tr("Set the shape of the genes"), m_geneShapeComboBox);
    addWidgetToMenu(tr("Shape:"), menu_genePlotter, m_geneShapeComboBox);

    //add menu to toolbutton in top bar
    m_ui->genemenu->setMenu(menu_genePlotter);

    // cell tissue menu
    QMenu *menu_cellTissue = new QMenu(this);
    menu_cellTissue->setTitle(tr("Cell Tissue"));
    setToolTipAndStatusTip(tr("Tools for visualization of the cell tissue"), menu_cellTissue);
    menu_cellTissue->addAction(m_ui->actionShow_showMiniMap);
    menu_cellTissue->addAction(m_ui->actionShow_showLegend);

    //group legend positions actions into one
    menu_cellTissue->addSeparator()->setText(tr("Legend Position"));
    QActionGroup *actionGroup_toggleLegendPosition = new QActionGroup(menu_cellTissue);
    actionGroup_toggleLegendPosition->setExclusive(true);
    actionGroup_toggleLegendPosition->addAction(m_ui->action_toggleLegendTopRight);
    actionGroup_toggleLegendPosition->addAction(m_ui->action_toggleLegendTopLeft);
    actionGroup_toggleLegendPosition->addAction(m_ui->action_toggleLegendDownRight);
    actionGroup_toggleLegendPosition->addAction(m_ui->action_toggleLegendDownLeft);
    menu_cellTissue->addActions(actionGroup_toggleLegendPosition->actions());
    menu_cellTissue->addSeparator();

    //group minimap positions actions into one
    menu_cellTissue->addSeparator()->setText(tr("Minimap Position"));
    QActionGroup *actionGroup_toggleMinimapPosition = new QActionGroup(menu_cellTissue);
    actionGroup_toggleMinimapPosition->setExclusive(true);
    actionGroup_toggleMinimapPosition->addAction(m_ui->action_toggleMinimapTopRight);
    actionGroup_toggleMinimapPosition->addAction(m_ui->action_toggleMinimapTopLeft);
    actionGroup_toggleMinimapPosition->addAction(m_ui->action_toggleMinimapDownRight);
    actionGroup_toggleMinimapPosition->addAction(m_ui->action_toggleMinimapDownLeft);
    menu_cellTissue->addActions(actionGroup_toggleMinimapPosition->actions());
    menu_cellTissue->addSeparator();

    //load red/blue actions and show/hide action for cell tissue
    QActionGroup *actionGroup_cellTissue = new QActionGroup(menu_cellTissue);
    actionGroup_cellTissue->setExclusive(true);
    actionGroup_cellTissue->addAction(m_ui->actionShow_cellTissueBlue);
    actionGroup_cellTissue->addAction(m_ui->actionShow_cellTissueRed);
    menu_cellTissue->addActions(actionGroup_cellTissue->actions());
    menu_cellTissue->addAction(m_ui->actionShow_showCellTissue);
    menu_cellTissue->addSeparator();

    //brightness of the cell tissue image
    addSliderToMenu(this,
                    tr("Brightness:"),
                    tr("Brightness level of the Cell Tissue"),
                    tr("Brightness level of the Cell Tissue"),
                    menu_cellTissue,
                    &m_geneBrightnessSlider,
                    BRIGHTNESS_MIN,
                    BRIGHTNESS_MAX);

    //add menu to tool button in top bar
    m_ui->cellmenu->setMenu(menu_cellTissue);

    // go back signal
    connect(m_ui->back, SIGNAL(clicked()), this, SIGNAL(moveToPreviousPage()));
    // go next signal
    connect(m_ui->next, SIGNAL(clicked()), this, SIGNAL(moveToNextPage()));

    // cell tissue
    connect(m_ui->actionShow_cellTissueBlue,
            SIGNAL(triggered(bool)), this, SLOT(slotLoadCellFigure()));
    connect(m_ui->actionShow_cellTissueRed,
            SIGNAL(triggered(bool)), this, SLOT(slotLoadCellFigure()));

    // graphic view signals
    connect(m_ui->zoomin, SIGNAL(clicked()), m_view.data(), SLOT(zoomIn()));
    connect(m_ui->zoomout, SIGNAL(clicked()), m_view.data(), SLOT(zoomOut()));

    // print canvas
    connect(m_ui->save, SIGNAL(clicked()), this, SLOT(slotSaveImage()));
    connect(m_ui->print, SIGNAL(clicked()), this, SLOT(slotPrintImage()));

    // selection mode
    connect(m_ui->selection, &QPushButton::clicked,
            [=]{ m_view->setSelectionMode(m_ui->selection->isChecked()); });
    connect(m_ui->regexpselection, SIGNAL(clicked()), this, SLOT(slotSelectByRegExp()));

    //color selectors
    connect(m_ui->actionColor_selectColorGrid, &QAction::triggered,
            [=]{ m_colorDialogGrid->show(); });

    //gene attributes signals
    connect(m_geneIntensitySlider, SIGNAL(valueChanged(int)), this, SLOT(slotGeneIntensity(int)));
    connect(m_geneSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slotGeneSize(int)));
    connect(m_geneBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(slotGeneBrightness(int)));
    connect(m_geneShapeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGeneShape(int)));

    //color and pooling signals
    connect(m_colorExp.data(), &QRadioButton::clicked,
            [=]{ m_legend->setColorComputingMode(Globals::ExpColor);
                 m_gene_plotter->setColorComputingMode(Globals::ExpColor);});
    connect(m_colorLinear.data(), &QRadioButton::clicked,
            [=]{ m_legend->setColorComputingMode(Globals::LinearColor);
                 m_gene_plotter->setColorComputingMode(Globals::LinearColor);});
    connect(m_colorLog.data(), &QRadioButton::clicked,
            [=]{ m_legend->setColorComputingMode(Globals::LogColor);
                 m_gene_plotter->setColorComputingMode(Globals::LogColor);});
    connect(m_poolingGenes.data(), &QRadioButton::clicked,
            [=]{ m_gene_plotter->setPoolingMode(GeneRendererGL::PoolNumberGenes); });
    connect(m_poolingReads.data(), &QRadioButton::clicked,
            [=]{ m_gene_plotter->setPoolingMode(GeneRendererGL::PoolReadsCount); });
    connect(m_poolingTPMs.data(), &QRadioButton::clicked,
            [=]{ m_gene_plotter->setPoolingMode(GeneRendererGL::PoolTPMs); });

    //connect gene list model to gene plotter
    connect(m_ui->genesWidget, SIGNAL(signalSelectionChanged(DataProxy::GeneList)),
            m_gene_plotter.data(),
            SLOT(updateVisible(DataProxy::GeneList)));
    connect(m_ui->genesWidget, SIGNAL(signalColorChanged(DataProxy::GeneList)),
            m_gene_plotter.data(),
            SLOT(updateColor(DataProxy::GeneList)));

    //connect gene selection signals from selectionsWidget
    connect(m_ui->selectionsWidget, SIGNAL(signalClearSelection()),
            m_gene_plotter.data(), SLOT(clearSelection()));
    connect(m_ui->selectionsWidget, SIGNAL(signalExportGenesSelection()),
            this, SLOT(slotExportGenesSelection()));
    connect(m_ui->selectionsWidget, SIGNAL(signalExportFeaturesSelection()),
            this, SLOT(slotExportFeaturesSelection()));
    connect(m_ui->selectionsWidget, SIGNAL(signalSaveSelection()),
            this, SLOT(slotSaveSelection()));

    //connect gene plotter to gene selection model
    connect(m_gene_plotter.data(), SIGNAL(selectionUpdated()),
            this, SLOT(slotSelectionUpdated()));

    //threshold slider signal
    connect(m_geneHitsThreshold.data(), SIGNAL(lowerValueChanged(int)),
            m_gene_plotter.data(), SLOT(setLowerLimit(int)));
    connect(m_geneHitsThreshold.data(), SIGNAL(upperValueChanged(int)),
            m_gene_plotter.data(), SLOT(setUpperLimit(int)));

    //show/not genes signal
    connect(m_ui->actionShow_showGenes,
            SIGNAL(triggered(bool)), m_gene_plotter.data(), SLOT(setVisible(bool)));

    //visual mode signal
    connect(actionGroup_toggleVisualMode, SIGNAL(triggered(QAction*)), this,
            SLOT(slotSetGeneVisualMode(QAction*)));

    // grid signals
    connect(m_colorDialogGrid.data(), SIGNAL(colorSelected(const QColor&)),
            m_grid.data(), SLOT(setColor(const QColor&)));
    connect(m_ui->actionShow_showGrid, SIGNAL(triggered(bool)),
            m_grid.data(), SLOT(setVisible(bool)));

    // cell tissue canvas
    connect(m_ui->actionShow_showCellTissue, SIGNAL(triggered(bool)),
            m_image.data(), SLOT(setVisible(bool)));

    // legend signals
    connect(m_ui->actionShow_showLegend, SIGNAL(toggled(bool)),
            m_legend.data(), SLOT(setVisible(bool)));
    connect(actionGroup_toggleLegendPosition,
            SIGNAL(triggered(QAction*)), this,
            SLOT(slotSetLegendAnchor(QAction*)));

    // minimap signals
    connect(m_ui->actionShow_showMiniMap, SIGNAL(toggled(bool)),
            m_minimap.data(), SLOT(setVisible(bool)));
    connect(actionGroup_toggleMinimapPosition,
            SIGNAL(triggered(QAction*)), this,
            SLOT(slotSetMiniMapAnchor(QAction*)));

    // Features Histogram Distribution
    connect(m_geneHitsThreshold.data(), SIGNAL(lowerValueChanged(int)),
            m_FDH.data(), SLOT(setLowerLimit(int)));
    connect(m_geneHitsThreshold.data(), SIGNAL(upperValueChanged(int)),
            m_FDH.data(), SLOT(setUpperLimit(int)));
    connect(m_ui->histogram, &QPushButton::clicked, [=]{ m_FDH->show(); });
}

void CellViewPage::resetActionStates()
{
    // load data for gene model
    m_ui->genesWidget->slotLoadModel(m_dataProxy->getGeneList());

    // reset color dialogs
    m_colorDialogGrid->setCurrentColor(GridRendererGL::DEFAULT_COLOR_GRID);

    // reset cell image to show
    m_image->setVisible(true);
    m_image->setAnchor(Globals::DEFAULT_ANCHOR_IMAGE);

    // reset gene grid to not show
    m_grid->setVisible(false);
    m_grid->setAnchor(Globals::DEFAULT_ANCHOR_GRID);

    // reset gene plotter to visible
    m_gene_plotter->setVisible(true);
    m_gene_plotter->setAnchor(Globals::DEFAULT_ANCHOR_GENE);

    // reset minimap to visible true
    m_minimap->setVisible(true);
    m_minimap->setAnchor(Globals::DEFAULT_ANCHOR_MINIMAP);

    // reset legend to visible true
    m_legend->setVisible(false);
    m_legend->setAnchor(Globals::DEFAULT_ANCHOR_LEGEND);

    // reset visual modes
    m_ui->actionShow_toggleNormal->setChecked(true);

    // reset show cell tissue
    m_ui->actionShow_showCellTissue->setChecked(true);

    // reset show grids
    m_ui->actionShow_showGrid->setChecked(false);

    // reset genes to show
    m_ui->actionShow_showGenes->setChecked(true);

    // gene controls
    m_geneIntensitySlider->setValue(GENE_INTENSITY_MAX);
    m_geneSizeSlider->setValue(GENE_SIZE_MIN);
    m_geneShapeComboBox->setCurrentIndex(GeneRendererGL::Circle);

    // selection mode
    m_ui->selection->setChecked(false);

    // anchor signals
    m_ui->action_toggleLegendTopRight->setChecked(true);

    // minimap signals
    m_ui->action_toggleMinimapDownRight->setChecked(true);

    // show legend and minimap
    m_ui->actionShow_showLegend->setChecked(false);
    m_ui->actionShow_showMiniMap->setChecked(true);

    // reset pooling mode
    m_poolingReads->setChecked(true);

    // reset color mode
    m_colorLinear->setChecked(true);

    // restrict interface
    const auto user = m_dataProxy->getUser();
    Q_ASSERT(!user.isNull());
    m_ui->actionShow_cellTissueRed->setVisible(user->hasSpecialRole());
}

void CellViewPage::initGLView()
{
    //ui->area contains the openGL window
    m_view = new CellGLView();
    m_ui->area->initializeView(m_view);

    // Setting stretch factors in the QSplitter to make the opengl window occupy more space
    m_ui->horizontalSplitter->setStretchFactor(0, 0);
    m_ui->horizontalSplitter->setStretchFactor(1, 8);

    // image texture graphical object
    m_image = new ImageTextureGL();
    m_image->setAnchor(Globals::DEFAULT_ANCHOR_IMAGE);
    m_view->addRenderingNode(m_image.data());

    // grid graphical object
    m_grid = new GridRendererGL();
    m_grid->setAnchor(Globals::DEFAULT_ANCHOR_GRID);
    m_view->addRenderingNode(m_grid.data());

    // gene plotter component
    m_gene_plotter = new GeneRendererGL(m_dataProxy);
    m_gene_plotter->setAnchor(Globals::DEFAULT_ANCHOR_GENE);
    m_view->addRenderingNode(m_gene_plotter.data());

    // heatmap component
    m_legend = new HeatMapLegendGL();
    m_legend->setAnchor(Globals::DEFAULT_ANCHOR_LEGEND);
    m_view->addRenderingNode(m_legend.data());

    // minimap component
    m_minimap = new MiniMapGL();
    m_minimap->setAnchor(Globals::DEFAULT_ANCHOR_MINIMAP);
    m_view->addRenderingNode(m_minimap.data());

    // minimap needs to be notified when the canvas is resized and when the image
    // is zoomed or moved
    connect(m_minimap.data(), SIGNAL(signalCenterOn(QPointF)),
            m_view.data(), SLOT(centerOn(QPointF)));
    connect(m_view.data(), SIGNAL(signalSceneUpdated(QRectF)),
            m_minimap.data(), SLOT(setScene(QRectF)));
    connect(m_view.data(), SIGNAL(signalViewPortUpdated(QRectF)),
            m_minimap.data(), SLOT(setViewPort(QRectF)));
    connect(m_view.data(), SIGNAL(signalSceneTransformationsUpdated(const QTransform)),
            m_minimap.data(), SLOT(setParentSceneTransformations(const QTransform)));
}

void CellViewPage::slotLoadCellFigure()
{
    const bool forceRedFigure = QObject::sender() == m_ui->actionShow_cellTissueRed;
    const bool forceBlueFigure = QObject::sender() == m_ui->actionShow_cellTissueBlue;
    const bool loadRedFigure = forceRedFigure && !forceBlueFigure;

    // retrieve the image (red or blue)
    const QByteArray image = loadRedFigure ? m_dataProxy->getFigureRed()
                                           : m_dataProxy->getFigureBlue();

    //update checkboxes
    m_ui->actionShow_cellTissueBlue->setChecked(!loadRedFigure);
    m_ui->actionShow_cellTissueRed->setChecked(loadRedFigure);

    // add image to the texture image holder
    // run it concurrently as it takes time
    QFutureWatcher<void> *futureWatcher = new QFutureWatcher<void>(this);
    futureWatcher->setFuture(m_image->createTexture(image));
    // set the scene size in the view to the image bounding box (image must have been tiled and
    // textured before)
    connect(futureWatcher, &QFutureWatcher<void>::finished,
            [=]{ m_view->setScene(m_image->boundingRect()); });
}

void CellViewPage::slotPrintImage()
{
    QPrinter printer;
    printer.setColorMode(QPrinter::Color);
    printer.setOrientation(QPrinter::Landscape);

    QScopedPointer<QPrintDialog> dialog(new QPrintDialog(&printer, this));
    if (dialog->exec() != QDialog::Accepted) {
        return;
    }

    QPainter painter(&printer);
    QRect rect = painter.viewport();
    QImage image = m_view->grabPixmapGL();
    QSize size = image.size();
    size.scale(rect.size(), Qt::KeepAspectRatio);
    painter.setViewport(QRect(QPoint(0, 0), size));
    painter.setWindow(image.rect());
    painter.drawImage(0, 0, image);
}

void CellViewPage::slotSaveImage()
{
    QString filename =
            QFileDialog::getSaveFileName(this, tr("Save Image"), QDir::homePath(),
                                         QString("%1;;%2;;%3").
                                         arg(tr("JPEG Image Files (*.jpg *.jpeg)")).
                                         arg(tr("PNG Image Files (*.png)")).
                                         arg(tr("BMP Image Files (*.bmp)")));
    // early out
    if (filename.isEmpty()) {
        return;
    }

    const QFileInfo fileInfo(filename);
    const QFileInfo dirInfo(fileInfo.dir().canonicalPath());
    if (!fileInfo.exists() && !dirInfo.isWritable()) {
        showError(tr("Save image"), tr("The directory is not writable"));
        return;
    }

    const QString format = fileInfo.suffix().toLower();
    if (!imageFormatHasWriteSupport(format)) {
        // This should never happen because getSaveFileName() automatically
        // adds the suffix from the "Save as type" choosen.
        // But this would be triggered if somehow there is no jpg, png or bmp support
        // compiled in the application
        showError(tr("Save image"), tr("The image format is not supported"));
        return;
    }

    const int quality = 100; //quality format (100 max, 0 min, -1 default)
    QImage image = m_view->grabPixmapGL();
    if (!image.save(filename, format.toStdString().c_str(), quality)) {
        showError(tr("Save Image"), tr("Error saving image."));
    }
}

void CellViewPage::slotExportGenesSelection()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), QDir::homePath(),
                                                    QString("%1").
                                                    arg(tr("Text Files (*.txt)")));
    // early out
    if (filename.isEmpty()) {
        return;
    }

    const QFileInfo fileInfo(filename);
    const QFileInfo dirInfo(fileInfo.dir().canonicalPath());
    if (!fileInfo.exists() && !dirInfo.isWritable()) {
        showError(tr("Export Genes Selection"), tr("The directory is not writable"));
        return;
    }

    // get selected genes
    const auto& geneSelection = m_gene_plotter->getSelectedGenes();

    //create file
    QFile textFile(filename);

    //export selection
    if (textFile.open(QFile::WriteOnly | QFile::Truncate)) {
        GeneExporter exporter = GeneExporter(GeneExporter::SimpleFull,
                                             GeneExporter::TabDelimited);
        exporter.exportItem(textFile, geneSelection);
        showInfo(tr("Export Genes Selection"), tr("Genes selection was exported successfully"));
    }

    textFile.close();
}

void CellViewPage::slotExportFeaturesSelection()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Export File"), QDir::homePath(),
                                                    QString("%1").
                                                    arg(tr("Text Files (*.txt)")));
    // early out
    if (filename.isEmpty()) {
        return;
    }

    const QFileInfo fileInfo(filename);
    const QFileInfo dirInfo(fileInfo.dir().canonicalPath());
    if (!fileInfo.exists() && !dirInfo.isWritable()) {
        showError(tr("Export Features Selection"), tr("The directory is not writable"));
        return;
    }

    // get selected features
    const auto& featuresSelection = m_gene_plotter->getSelectedFeatures();

    //create file
    QFile textFile(filename);

    //export selection
    if (textFile.open(QFile::WriteOnly | QFile::Truncate)) {
        FeatureExporter exporter = FeatureExporter(FeatureExporter::SimpleFull,
                                                   FeatureExporter::TabDelimited);
        exporter.exportItem(textFile, featuresSelection);
        showInfo(tr("Export Features Selection"), tr("Features selection was exported successfully"));
    }

    textFile.close();
}

void CellViewPage::slotSetGeneVisualMode(QAction *action)
{
    const QVariant variant = action->property("mode");
    if (variant.canConvert(QVariant::Int)) {
        const GeneRendererGL::GeneVisualMode mode =
                static_cast<GeneRendererGL::GeneVisualMode>(variant.toInt());
        m_gene_plotter->setVisualMode(mode);
    } else {
        Q_ASSERT("[CellViewPage] Undefined gene visual mode!");
    }
}

void CellViewPage::slotSetMiniMapAnchor(QAction *action)
{
    const QVariant variant = action->property("mode");
    if (variant.canConvert(QVariant::Int)) {
        const Globals::Anchor mode = static_cast<Globals::Anchor>(variant.toInt());
        m_minimap->setAnchor(mode);
    } else {
        Q_ASSERT("[CellViewPage] Undefined minimap anchor!");
    }
}

void CellViewPage::slotSetLegendAnchor(QAction *action)
{
    const QVariant variant = action->property("mode");
    if (variant.canConvert(QVariant::Int)) {
        const Globals::Anchor mode = static_cast<Globals::Anchor>(variant.toInt());
        m_legend->setAnchor(mode);
    } else {
        Q_ASSERT("[CellViewPage] Undefined legend anchor!");
    }
}

void CellViewPage::slotSelectByRegExp()
{
    const DataProxy::GeneList& geneList = SelectionDialog::selectGenes(m_dataProxy, this);
    m_gene_plotter->selectGenes(geneList);
}

void CellViewPage::slotSelectionUpdated()
{
    // get selected genes
    const auto& geneSelection = m_gene_plotter->getSelectedGenes();
    m_ui->selectionsWidget->slotLoadModel(geneSelection);
}

void CellViewPage::slotSaveSelection()
{
    QScopedPointer<CreateSelectionDialog>
            createSelection(new CreateSelectionDialog(this,
                                                      Qt::CustomizeWindowHint
                                                      | Qt::WindowTitleHint));
    if (createSelection->exec() == CreateSelectionDialog::Accepted) {

        if (createSelection->getName().isNull() || createSelection->getName().isEmpty()) {
            return;
        }

        // get selected genes
        const auto& geneSelection = m_gene_plotter->getSelectedGenes();

        //create the selection object
        GeneSelection selection;
        selection.name(createSelection->getName());
        selection.comment(createSelection->getComment());
        selection.enabled(true);

        //add dataset ID
        const auto dataset = m_dataProxy->getSelectedDataset();
        Q_ASSERT(!dataset.isNull());
        selection.datasetId(dataset->id());

        //add type of selection
        selection.type("Rubberband selection");

        //add account
        const auto user = m_dataProxy->getUser();
        Q_ASSERT(!user.isNull());
        selection.userId(user->id());

        //add selected genes
        selection.selectedItems(geneSelection);

        //save the selection object
        m_dataProxy->addGeneSelection(selection);
        m_dataProxy->activateCurrentDownloads();
    }
}

void CellViewPage::slotGeneShape(int geneShape)
{
    const GeneRendererGL::GeneShape shape = static_cast<GeneRendererGL::GeneShape>(geneShape);
    Q_ASSERT(!m_gene_plotter.isNull());
    m_gene_plotter->setShape(shape);
}

//input is expected to be >= 1 and <= 10
void CellViewPage::slotGeneIntensity(int geneIntensity)
{
    Q_ASSERT(geneIntensity >= 1 && geneIntensity <= 10);
    Q_ASSERT(!m_gene_plotter.isNull());
    const qreal decimal = static_cast<qreal>(geneIntensity) / 10;
    m_gene_plotter->setIntensity(decimal);
}

//input is expected to be >= 5 and <= 30
void CellViewPage::slotGeneSize(int geneSize)
{
    Q_ASSERT(geneSize >= 5 && geneSize <= 30);
    Q_ASSERT(!m_gene_plotter.isNull());
    const qreal decimal = static_cast<qreal>(geneSize) / 10;
    m_gene_plotter->setSize(decimal);
}

//input is expected to be >= 1 and <= 10
void CellViewPage::slotGeneBrightness(int geneBrightness)
{
    Q_ASSERT(geneBrightness >= 1 && geneBrightness <= 10);
    Q_ASSERT(!m_image.isNull());
    const qreal decimal = static_cast<qreal>(geneBrightness) / 10;
    m_image->setIntensity(decimal);
}
