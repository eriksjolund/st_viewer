/*
    Copyright (C) 2012  Spatial Transcriptomics AB,
    read LICENSE for licensing terms.
    Contact : Jose Fernandez Navarro <jose.fernandez.navarro@scilifelab.se>

*/

#include "ScatterPlot.h"

#include <QPushButton>

#include "ui_plotCanvas.h"

ScatterPlot::ScatterPlot(QWidget *parent) :
    QWidget(parent),
    m_ui(nullptr),
    m_customPlot(nullptr)
{

    // create UI
    m_ui = new Ui::plotCanvas;
    m_ui->setupUi(this);

    // creating plotting object
    m_customPlot = new QCustomPlot(m_ui->plotWidget);

    //make connections
    connect(m_ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
    //TODO handle the saving with a slot that launches a file chooser
    connect(m_ui->saveButton, &QPushButton::clicked,
            [=]() { m_customPlot->savePdf("scatter_plot"); });

}

ScatterPlot::~ScatterPlot()
{
    if (m_ui != nullptr) {
        delete m_ui;
    }
    m_ui = nullptr;

    m_customPlot->deleteLater();
    m_customPlot = nullptr;
}
