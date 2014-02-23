/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/
#include "CellViewPageToolBar.h"

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QWidgetAction>
#include <QComboBox>
#include <QToolButton>


#include "SpinBoxSlider.h"

static const int GENE_INTENSITY_MIN = 1;
static const int GENE_INTENSITY_MAX = 10;
static const int GENE_SIZE_MIN = 10;
static const int GENE_SIZE_MAX = 30;
static const int GENE_SHIMME_MIN = 1;
static const int GENE_SHIMME_MAX = 10;
static const int BRIGHTNESS_MIN = 1;
static const int BRIGHTNESS_MAX = 10;

void addWidgetToMenu(const QString &str, QMenu *menu, QWidget *widget) {
    Q_ASSERT(menu != nullptr);
    Q_ASSERT(widget != nullptr);
    QWidgetAction *widgetAction = new QWidgetAction(menu);
    widgetAction->setDefaultWidget(widget);
    menu->addAction(str);
    menu->addAction(widgetAction);
}

void addSliderToMenu(QWidget *parent, const QString &str, const QString &tooltipStr, QMenu *menu, QSlider **slider_ptr, int rangeMin, int rangeMax) {
    Q_ASSERT(parent != nullptr);
    Q_ASSERT(slider_ptr != nullptr);
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
    addWidgetToMenu(str, menu, slider);
}

CellViewPageToolBar::CellViewPageToolBar(QWidget *parent) :
    QToolBar(parent)
{
    createActions();

    setIconSize(QSize(35, 35));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    //tool bar actions
    addAction(m_actionNavigate_goBack);
    addSeparator();
    addAction(m_actionZoom_zoomIn);
    addAction(m_actionZoom_zoomOut);
    addSeparator();
    addAction(m_actionSelection_showSelectionDialog);
    addSeparator();
    addAction(m_actionSave_save);
    addAction(m_actionSave_print);
    addSeparator();

    // menu gene plotter actions
    m_menu_genePlotter = new QMenu(this);
    m_menu_genePlotter->setTitle(tr("Gene Plotter"));
    m_menu_genePlotter->addAction(m_actionShow_showGrid);
    m_menu_genePlotter->addAction(m_actionShow_showGenes);
    m_menu_genePlotter->addSeparator();
    m_menu_genePlotter->addAction(m_actionColor_selectColorGenes);
    m_menu_genePlotter->addAction(m_actionColor_selectColorGrid);
    m_menu_genePlotter->addSeparator();

    //color modes
    m_actionGroup_toggleVisualMode = new QActionGroup(m_menu_genePlotter);
    m_actionGroup_toggleVisualMode->setExclusive(true);
    m_actionGroup_toggleVisualMode->addAction(m_actionShow_toggleNormal);
    m_actionGroup_toggleVisualMode->addAction(m_actionShow_toggleDynamicRange);
    m_actionGroup_toggleVisualMode->addAction(m_actionShow_toggleHeatMap);
    m_menu_genePlotter->addActions(m_actionGroup_toggleVisualMode->actions());
    m_menu_genePlotter->addSeparator();

    //threshold slider
    Q_ASSERT(m_geneHitsThreshold == nullptr);
    m_geneHitsThreshold = new SpinBoxSlider(this);
    m_geneHitsThreshold->setToolTip(tr("Limit of the number of transcripts."));
    addWidgetToMenu(tr("Transcripts Threshold:"), m_menu_genePlotter, m_geneHitsThreshold);

    // transcripts intensity
    addSliderToMenu(this,
                    tr("Opacity:"),
                    tr("Intensity of the transcripts."),
                    m_menu_genePlotter,
                    &m_geneIntensitySlider,
                    GENE_INTENSITY_MIN,
                    GENE_INTENSITY_MAX);
    addSliderToMenu(this,
                    tr("Size:"),
                    tr("Size of the transcripts."),
                    m_menu_genePlotter,
                    &m_geneSizeSlider,
                    GENE_SIZE_MIN,
                    GENE_SIZE_MAX);
    addSliderToMenu(this,
                    tr("Shine:"),
                    tr("Shine"),
                    m_menu_genePlotter,
                    &m_geneShineSlider,
                    GENE_SHIMME_MIN,
                    GENE_SHIMME_MAX);

    m_menu_genePlotter->addSeparator();

    // shape of the genes
    Q_ASSERT(m_geneShapeComboBox == nullptr);
    m_geneShapeComboBox = new QComboBox(this);
    m_geneShapeComboBox->addItem("Circles", Globals::GeneShape::Circle);
    m_geneShapeComboBox->addItem("Crosses", Globals::GeneShape::Cross);
    m_geneShapeComboBox->addItem("Squares", Globals::GeneShape::Square);
    m_geneShapeComboBox->setCurrentIndex(Globals::GeneShape::Circle);
    m_geneShapeComboBox->setToolTip(tr("Shape of the genes."));
    addWidgetToMenu(tr("Shape:"), m_menu_genePlotter, m_geneShapeComboBox);

    //second menu
    QToolButton* toolButtonGene = new QToolButton(this);
    toolButtonGene->setMenu(m_menu_genePlotter);
    toolButtonGene->setPopupMode(QToolButton::InstantPopup);
    toolButtonGene->setIcon(QIcon(QStringLiteral(":/images/settings2.png")));
    toolButtonGene->setToolTip(tr("Configuration of Genes"));
    toolButtonGene->setText(tr("Configuration of Genes"));
    addWidget(toolButtonGene);
    addSeparator();

    // cell tissue menu
    m_menu_cellTissue = new QMenu(this);
    m_menu_cellTissue->setTitle(tr("Cell Tissue"));

    addSliderToMenu(this,
                    tr("Brightness:"), 
                    tr("Brightness"),
                    m_menu_cellTissue,
                    &m_geneBrightnessSlider,
                    BRIGHTNESS_MIN,
                    BRIGHTNESS_MAX);

    m_menu_cellTissue->addAction(m_actionRotation_rotateLeft);
    m_menu_cellTissue->addAction(m_actionRotation_rotateRight);

    m_actionGroup_cellTissue = new QActionGroup(m_menu_cellTissue);
    m_actionGroup_cellTissue->setExclusive(true);
    m_actionGroup_cellTissue->addAction(m_actionShow_cellTissueBlue);
    m_actionGroup_cellTissue->addAction(m_actionShow_cellTissueRed);
    m_menu_cellTissue->addActions(m_actionGroup_cellTissue->actions());
    m_menu_cellTissue->addAction(m_actionShow_showCellTissue);
    QToolButton* toolButtonCell = new QToolButton();
    toolButtonCell->setMenu(m_menu_cellTissue);
    toolButtonCell->setPopupMode(QToolButton::InstantPopup);
    toolButtonCell->setIcon(QIcon(QStringLiteral(":/images/settings.png")));
    toolButtonCell->setToolTip(tr("Configuration of Cell Tissue"));
    toolButtonCell->setText(tr("Configuration of Cell Tissue"));
    addWidget(toolButtonCell);
    addSeparator();

    createConnections();
}

void CellViewPageToolBar::resetTresholdActions(int min, int max)
{
    Q_ASSERT(m_geneHitsThreshold != nullptr);
    //threshold slider
    m_geneHitsThreshold->setMinimumValue(min);
    m_geneHitsThreshold->setMaximumValue(max);
    m_geneHitsThreshold->setLowerValue(min);
    m_geneHitsThreshold->setUpperValue(max);
    m_geneHitsThreshold->setTickInterval(1);
}

void CellViewPageToolBar::resetActions()
{
    // reset visual modes

    m_actionShow_toggleDynamicRange->setChecked(false);
    m_actionShow_toggleHeatMap->setChecked(false);
    m_actionShow_toggleNormal->setChecked(true);

    m_actionShow_showCellTissue->setChecked(true);

    m_actionShow_showGrid->setChecked(false);

    // reset genes to show
    m_actionShow_showGenes->setChecked(true);

    // gene controls
    Q_ASSERT(m_geneIntensitySlider);
    m_geneIntensitySlider->setValue(GENE_INTENSITY_MAX);
    Q_ASSERT(m_geneSizeSlider);
    m_geneSizeSlider->setValue(GENE_SIZE_MIN);
    Q_ASSERT(m_geneShapeComboBox);
    m_geneShapeComboBox->setCurrentIndex(Globals::GeneShape::Circle);

    resetTresholdActions(Globals::GENE_THRESHOLD_MIN, Globals::GENE_THRESHOLD_MAX);
}

void CellViewPageToolBar::createActions()
{
    //show grid/genes
    m_actionShow_showGrid = new QAction(QIcon(QStringLiteral(":/images/grid-icon-md.png")), tr("Show Grid"), this);
    m_actionShow_showGrid->setCheckable(true);
    m_actionShow_showGenes = new QAction(QIcon(QStringLiteral(":/images/genes.png")), tr("Show &Genes"), this);
    m_actionShow_showGenes->setCheckable(true);

    //zomming
    m_actionZoom_zoomIn = new QAction(QIcon(QStringLiteral(":/images/Zoom-In-icon.png")), tr("Zoom &In"), this);
    m_actionZoom_zoomOut = new QAction(QIcon(QStringLiteral(":/images/Zoom-Out-icon.png")), tr("Zoom &Out"), this);

    //cell tissue controls
    m_actionShow_cellTissueBlue = new QAction(QIcon(QStringLiteral(":/images/blue-icon.png")), tr("Load &Blue Cell Tissue"), this);
    m_actionShow_cellTissueBlue->setCheckable(true);
    m_actionShow_cellTissueRed = new QAction(QIcon(QStringLiteral(":/images/red-icon.png")), tr("Load &Red Cell Tissue"), this);
    m_actionShow_cellTissueRed->setCheckable(true);
    m_actionShow_showCellTissue = new QAction(QIcon(QStringLiteral(":/images/biology.png")), tr("Show Cell &Tissue"), this);
    m_actionShow_showCellTissue->setCheckable(true);

    //navigation
    m_actionNavigate_goBack = new QAction(QIcon(QStringLiteral(":/images/back.png")), tr("Go Back"), this);
    m_actionNavigate_goBack->setAutoRepeat(false);

    //color modes
    m_actionShow_toggleNormal = new QAction(QIcon(QStringLiteral(":/images/blue-icon.png")), tr("Normal Mode"), this);
    m_actionShow_toggleNormal->setCheckable(true);
    m_actionShow_toggleNormal->setProperty("mode", Globals::GeneVisualMode::NormalMode);
    m_actionShow_toggleDynamicRange = new QAction(QIcon(QStringLiteral(":/images/dynamicrange.png")), tr("Dynamic Range Mode"), this);
    m_actionShow_toggleDynamicRange->setCheckable(true);
    m_actionShow_toggleDynamicRange->setProperty("mode", Globals::GeneVisualMode::DynamicRangeMode);
    m_actionShow_toggleHeatMap = new QAction(QIcon(QStringLiteral(":/images/heatmap.png")), tr("Heat Map Mode"), this);
    m_actionShow_toggleHeatMap->setCheckable(true);
    m_actionShow_toggleHeatMap->setProperty("mode", Globals::GeneVisualMode::HeatMapMode);

     //save print
    m_actionSave_save = new QAction(QIcon(QStringLiteral(":/images/filesave.png")), tr("Save Cell Tissue"),  this);
    m_actionSave_print = new QAction(QIcon(QStringLiteral(":/images/printer.png")), tr("Print Cell Tissue"), this);

    //selection actions
    m_actionSelection_showSelectionDialog = new QAction(QIcon(QStringLiteral(":/images/reg_search.png")), tr("Select Genes"), this);

    // color dialogs
    m_actionColor_selectColorGenes = new QAction(QIcon(QStringLiteral(":/images/select-by-color-icon.png")), tr("Choose &Color Genes"), this);
    m_actionColor_selectColorGrid = new QAction(QIcon(QStringLiteral(":/images/edit_color.png")), tr("Choose Color Grid"), this);

    m_actionRotation_rotateLeft = new QAction(QIcon(QStringLiteral(":/images/rotate_left.png")), tr("Rotate &left"), this);
    m_actionRotation_rotateRight = new QAction(QIcon(QStringLiteral(":/images/rotate_right.png")), tr("Rotate &right"), this);
}

void CellViewPageToolBar::createConnections()
{
    //threshold slider signal
    Q_ASSERT(m_geneHitsThreshold);
    connect(m_geneHitsThreshold, SIGNAL(lowerValueChanged(int)), this, SIGNAL(thresholdLowerValueChanged(int)));
    connect(m_geneHitsThreshold, SIGNAL(upperValueChanged(int)), this, SIGNAL(thresholdUpperValueChanged(int)));

    //gene attributes signals
    Q_ASSERT(m_geneIntensitySlider);
    connect(m_geneIntensitySlider, SIGNAL(valueChanged(int)), this, SLOT(slotGeneIntensity(int)));
    Q_ASSERT(m_geneSizeSlider);
    connect(m_geneSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(slotGeneSize(int)));
    Q_ASSERT(m_geneShineSlider);
    connect(m_geneShineSlider, SIGNAL(valueChanged(int)), this, SLOT(slotGeneShine(int)));
    Q_ASSERT(m_geneBrightnessSlider);
    connect(m_geneBrightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(slotGeneBrightness(int)));
    Q_ASSERT(m_geneShapeComboBox);
    connect(m_geneShapeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGeneShape(int)));
    Q_ASSERT(m_actionRotation_rotateLeft);
    connect(m_actionRotation_rotateLeft, SIGNAL(triggered(bool)), this, SLOT(slotRotateLeft()));
    Q_ASSERT(m_actionRotation_rotateRight);
    connect(m_actionRotation_rotateRight, SIGNAL(triggered(bool)), this, SLOT(slotRotateRight()));
}

void CellViewPageToolBar::slotGeneShape(int geneShape)
{
    const Globals::GeneShape shape = static_cast<Globals::GeneShape>(geneShape);
    emit shapeIndexChanged(shape);
}

//input is expected to be >= 1 and <= 10
void CellViewPageToolBar::slotGeneIntensity(int geneIntensity)
{
    const qreal decimal = static_cast<qreal>(geneIntensity) / 10;
    emit intensityValueChanged(decimal);
}

//input is expected to be >= 10 and <= 30
void CellViewPageToolBar::slotGeneSize(int geneSize)
{
    const qreal decimal = static_cast<qreal>(geneSize) / 10;
    emit sizeValueChanged(decimal);
}

void CellViewPageToolBar::slotGeneShine(int geneShine)
{
    const qreal decimal = static_cast<qreal>(geneShine) / 10;
    emit shineValueChanged(decimal);
}

void CellViewPageToolBar::slotGeneBrightness(int geneBrightness)
{
    const qreal decimal = static_cast<qreal>(geneBrightness) / 10;
    emit brightnessValueChanged(decimal);
}

void CellViewPageToolBar::slotRotateLeft()
{
    emit rotateView(-45.0f);
}

void CellViewPageToolBar::slotRotateRight()
{
    emit rotateView(45.0f);
}