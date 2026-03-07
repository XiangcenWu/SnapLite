#include "ui/MainWindow.h"

#include <QAction>
#include <QDockWidget>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QtMath>

#include "rendering/VolumeRenderPlaceholder.h"
#include "ui/SliceView.h"

namespace {
constexpr int kContrastSliderMax = 1000;
constexpr int kNiftiFilePathRole = Qt::UserRole + 1;

void adjustSliderValue(QSlider *slider, int steps) {
    if (slider == nullptr || steps == 0) {
        return;
    }

    slider->setValue(qBound(slider->minimum(), slider->value() + steps, slider->maximum()));
}

bool isNiftiFile(const QString &fileName) {
    const QString lowerName = fileName.toLower();
    return lowerName.endsWith(".nii") || lowerName.endsWith(".nii.gz");
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    buildUi();
    buildMenus();
    connectSignals();
    generateDemoVolume();
}

void MainWindow::resetView() {
    const QSignalBlocker blockX(m_xSlider);
    const QSignalBlocker blockY(m_ySlider);
    const QSignalBlocker blockZ(m_zSlider);
    m_xSlider->setValue(m_xSlider->maximum() / 2);
    m_ySlider->setValue(m_ySlider->maximum() / 2);
    m_zSlider->setValue(m_zSlider->maximum() / 2);
    updateSlices();
}

void MainWindow::generateDemoVolume() {
    m_volume.generateSynthetic(192, 192, 144);
    applyLoadedVolume("synthetic demo volume");
}

void MainWindow::openNiftiFile() {
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "Open NIfTI Volume",
        m_niftiDirectoryPath,
        "NIfTI Files (*.nii *.nii.gz);;All Files (*)");
    if (filePath.isEmpty()) {
        return;
    }

    loadVolumeFromFile(filePath);
}

void MainWindow::openNiftiDirectory() {
    const QString directoryPath = QFileDialog::getExistingDirectory(
        this,
        "Open NIfTI Directory",
        m_niftiDirectoryPath);
    if (directoryPath.isEmpty()) {
        return;
    }

    populateNiftiFileList(directoryPath);
}

void MainWindow::loadNiftiFromListItem(QListWidgetItem *item) {
    if (item == nullptr) {
        return;
    }

    loadVolumeFromFile(item->data(kNiftiFilePathRole).toString());
}

void MainWindow::updateSlices() {
    if (!m_volume.isValid()) {
        return;
    }

    const int x = m_xSlider->value();
    const int y = m_ySlider->value();
    const int z = m_zSlider->value();
    const float blackPoint = static_cast<float>(m_contrastLevel - (m_contrastWindow * 0.5));
    const float whitePoint = static_cast<float>(m_contrastLevel + (m_contrastWindow * 0.5));

    m_axialView->setSliceImage(m_volume.axialSlice(z, blackPoint, whitePoint));
    m_coronalView->setSliceImage(m_volume.coronalSlice(y, blackPoint, whitePoint));
    m_sagittalView->setSliceImage(m_volume.sagittalSlice(x, blackPoint, whitePoint));

    const int xMax = qMax(1, m_xSlider->maximum());
    const int yMax = qMax(1, m_ySlider->maximum());
    const int zMax = qMax(1, m_zSlider->maximum());

    m_axialView->setCrosshair(QPointF(
        static_cast<double>(x) / xMax,
        static_cast<double>(y) / yMax));
    m_coronalView->setCrosshair(QPointF(
        static_cast<double>(x) / xMax,
        static_cast<double>(z) / zMax));
    m_sagittalView->setCrosshair(QPointF(
        static_cast<double>(y) / yMax,
        static_cast<double>(z) / zMax));

    statusBar()->showMessage(QString("Voxel: x=%1, y=%2, z=%3").arg(x).arg(y).arg(z));
}

void MainWindow::buildUi() {
    auto *central = new QWidget(this);
    m_viewGrid = new QGridLayout(central);
    m_viewGrid->setContentsMargins(8, 8, 8, 8);
    m_viewGrid->setSpacing(8);

    m_axialView = new SliceView("Axial", central);
    m_coronalView = new SliceView("Coronal", central);
    m_sagittalView = new SliceView("Sagittal", central);
    m_volumeView = new VolumeRenderPlaceholder(central);

    m_viewGrid->addWidget(m_axialView, 0, 0);
    m_viewGrid->addWidget(m_coronalView, 0, 1);
    m_viewGrid->addWidget(m_sagittalView, 1, 0);
    m_viewGrid->addWidget(m_volumeView, 1, 1);
    m_viewGrid->setRowStretch(0, 1);
    m_viewGrid->setRowStretch(1, 1);
    m_viewGrid->setColumnStretch(0, 1);
    m_viewGrid->setColumnStretch(1, 1);

    setCentralWidget(central);

    auto *browserDock = new QDockWidget("NIfTI Browser", this);
    browserDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    browserDock->setMinimumWidth(280);
    browserDock->setWidget(buildNiftiBrowserPanel());
    addDockWidget(Qt::LeftDockWidgetArea, browserDock);

    auto *controlsDock = new QDockWidget("Controls", this);
    controlsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    controlsDock->setWidget(buildControlPanel());
    addDockWidget(Qt::RightDockWidgetArea, controlsDock);

    resizeDocks({browserDock, controlsDock}, {320, 320}, Qt::Horizontal);

    m_statusDims = new QLabel("Volume: -", this);
    statusBar()->addPermanentWidget(m_statusDims);
}

void MainWindow::buildMenus() {
    auto *fileMenu = menuBar()->addMenu("&File");
    auto *viewMenu = menuBar()->addMenu("&View");

    auto *openNiftiAction = new QAction("&Open NIfTI...", this);
    auto *openDirectoryAction = new QAction("Open &Folder...", this);
    auto *loadDemoAction = new QAction("Load &Demo Volume", this);
    auto *quitAction = new QAction("&Quit", this);
    auto *resetAction = new QAction("&Reset View", this);

    fileMenu->addAction(openNiftiAction);
    fileMenu->addAction(openDirectoryAction);
    fileMenu->addAction(loadDemoAction);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);
    viewMenu->addAction(resetAction);

    auto *mainTools = addToolBar("Main");
    mainTools->addAction(openNiftiAction);
    mainTools->addAction(openDirectoryAction);
    mainTools->addAction(loadDemoAction);
    mainTools->addAction(resetAction);

    connect(openNiftiAction, &QAction::triggered, this, &MainWindow::openNiftiFile);
    connect(openDirectoryAction, &QAction::triggered, this, &MainWindow::openNiftiDirectory);
    connect(loadDemoAction, &QAction::triggered, this, &MainWindow::generateDemoVolume);
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetView);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::connectSignals() {
    connect(m_xSlider, &QSlider::valueChanged, this, &MainWindow::updateSlices);
    connect(m_ySlider, &QSlider::valueChanged, this, &MainWindow::updateSlices);
    connect(m_zSlider, &QSlider::valueChanged, this, &MainWindow::updateSlices);
    connect(m_openDirectoryButton, &QPushButton::clicked, this, &MainWindow::openNiftiDirectory);
    connect(m_niftiFileList, &QListWidget::itemActivated, this, &MainWindow::loadNiftiFromListItem);
    connect(m_niftiFileList, &QListWidget::itemClicked, this, &MainWindow::loadNiftiFromListItem);
    connect(m_axialView, &SliceView::sliceScrollRequested, this, [this](int steps) {
        adjustSliderValue(m_zSlider, steps);
    });
    connect(m_coronalView, &SliceView::sliceScrollRequested, this, [this](int steps) {
        adjustSliderValue(m_ySlider, steps);
    });
    connect(m_sagittalView, &SliceView::sliceScrollRequested, this, [this](int steps) {
        adjustSliderValue(m_xSlider, steps);
    });
    connect(m_axialView, &SliceView::fullSizeToggled, this, [this]() {
        toggleSliceViewExpanded(m_axialView);
    });
    connect(m_coronalView, &SliceView::fullSizeToggled, this, [this]() {
        toggleSliceViewExpanded(m_coronalView);
    });
    connect(m_sagittalView, &SliceView::fullSizeToggled, this, [this]() {
        toggleSliceViewExpanded(m_sagittalView);
    });
    connect(m_contrastWindowSlider, &QSlider::valueChanged, this, &MainWindow::updateContrastWindow);
    connect(m_contrastLevelSlider, &QSlider::valueChanged, this, &MainWindow::updateContrastLevel);
    connect(m_resetContrastButton, &QPushButton::clicked, this, &MainWindow::resetContrast);
}

QWidget *MainWindow::buildNiftiBrowserPanel() {
    auto *panel = new QWidget(this);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);
    panel->setMinimumWidth(280);

    auto *browserBox = new QGroupBox("Folder", panel);
    auto *browserLayout = new QVBoxLayout(browserBox);
    auto *browserSplitter = new QSplitter(Qt::Vertical, browserBox);
    browserSplitter->setChildrenCollapsible(false);

    auto *browserSummary = new QWidget(browserSplitter);
    auto *summaryLayout = new QVBoxLayout(browserSummary);
    summaryLayout->setContentsMargins(0, 0, 0, 0);
    summaryLayout->setSpacing(8);

    m_openDirectoryButton = new QPushButton("Open Folder...", browserSummary);
    summaryLayout->addWidget(m_openDirectoryButton);

    summaryLayout->addWidget(new QLabel("Current Folder", browserSummary));
    m_niftiDirectoryValue = new QLabel("No folder selected", browserSummary);
    m_niftiDirectoryValue->setWordWrap(true);
    summaryLayout->addWidget(m_niftiDirectoryValue);

    m_niftiFileCountValue = new QLabel("0 files", browserSummary);
    summaryLayout->addWidget(m_niftiFileCountValue);

    m_niftiFileList = new QListWidget(browserSplitter);
    m_niftiFileList->setAlternatingRowColors(true);
    m_niftiFileList->setSelectionMode(QAbstractItemView::SingleSelection);
    browserSplitter->addWidget(browserSummary);
    browserSplitter->addWidget(m_niftiFileList);
    browserSplitter->setStretchFactor(0, 0);
    browserSplitter->setStretchFactor(1, 1);
    browserSplitter->setSizes({150, 400});

    browserLayout->addWidget(browserSplitter, 1);
    layout->addWidget(browserBox, 1);

    return panel;
}

QWidget *MainWindow::buildControlPanel() {
    auto *panel = new QWidget(this);
    auto *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(12);

    auto *sliceBox = new QGroupBox("Slice Navigation", panel);
    auto *sliceLayout = new QVBoxLayout(sliceBox);
    sliceLayout->addWidget(new QLabel("Sagittal (X)", sliceBox));
    m_xSlider = new QSlider(Qt::Horizontal, sliceBox);
    sliceLayout->addWidget(m_xSlider);

    sliceLayout->addWidget(new QLabel("Coronal (Y)", sliceBox));
    m_ySlider = new QSlider(Qt::Horizontal, sliceBox);
    sliceLayout->addWidget(m_ySlider);

    sliceLayout->addWidget(new QLabel("Axial (Z)", sliceBox));
    m_zSlider = new QSlider(Qt::Horizontal, sliceBox);
    sliceLayout->addWidget(m_zSlider);

    auto *toolsBox = new QGroupBox("Tools", panel);
    auto *toolsLayout = new QVBoxLayout(toolsBox);

    auto *contrastBox = new QGroupBox("Image Contrast", toolsBox);
    auto *contrastLayout = new QGridLayout(contrastBox);

    contrastLayout->addWidget(new QLabel("Window", contrastBox), 0, 0);
    m_contrastWindowSlider = new QSlider(Qt::Horizontal, contrastBox);
    m_contrastWindowSlider->setRange(0, kContrastSliderMax);
    contrastLayout->addWidget(m_contrastWindowSlider, 0, 1);
    m_contrastWindowValue = new QLabel("-", contrastBox);
    contrastLayout->addWidget(m_contrastWindowValue, 0, 2);

    contrastLayout->addWidget(new QLabel("Level", contrastBox), 1, 0);
    m_contrastLevelSlider = new QSlider(Qt::Horizontal, contrastBox);
    m_contrastLevelSlider->setRange(0, kContrastSliderMax);
    contrastLayout->addWidget(m_contrastLevelSlider, 1, 1);
    m_contrastLevelValue = new QLabel("-", contrastBox);
    contrastLayout->addWidget(m_contrastLevelValue, 1, 2);

    m_resetContrastButton = new QPushButton("Reset Contrast", contrastBox);
    contrastLayout->addWidget(m_resetContrastButton, 2, 0, 1, 3);

    toolsLayout->addWidget(contrastBox);
    toolsLayout->addWidget(new QLabel("Additional paint and threshold tools can be added here.", toolsBox));

    layout->addWidget(sliceBox);
    layout->addWidget(toolsBox);
    layout->addStretch(1);

    return panel;
}

bool MainWindow::loadVolumeFromFile(const QString &filePath) {
    if (filePath.isEmpty()) {
        return false;
    }

    QString errorMessage;
    if (!m_volume.loadNifti(filePath, &errorMessage)) {
        statusBar()->showMessage(QString("Failed to load volume: %1").arg(errorMessage), 6000);
        return false;
    }

    const QFileInfo info(filePath);
    m_niftiDirectoryPath = info.absolutePath();
    applyLoadedVolume(info.fileName());
    return true;
}

void MainWindow::populateNiftiFileList(const QString &directoryPath) {
    m_niftiDirectoryPath = directoryPath;
    m_niftiDirectoryValue->setText(QDir::toNativeSeparators(directoryPath));
    m_niftiFileList->clear();

    QDir rootDir(directoryPath);
    QStringList relativePaths;

    QDirIterator it(directoryPath, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QFileInfo info = it.fileInfo();
        if (isNiftiFile(info.fileName())) {
            relativePaths.append(rootDir.relativeFilePath(info.filePath()));
        }
    }

    relativePaths.sort(Qt::CaseInsensitive);
    for (const QString &relativePath : relativePaths) {
        auto *item = new QListWidgetItem(relativePath, m_niftiFileList);
        const QString absolutePath = rootDir.absoluteFilePath(relativePath);
        item->setData(kNiftiFilePathRole, absolutePath);
        item->setToolTip(QDir::toNativeSeparators(absolutePath));
    }

    m_niftiFileCountValue->setText(QString("%1 file%2")
                                       .arg(relativePaths.size())
                                       .arg(relativePaths.size() == 1 ? "" : "s"));
    statusBar()->showMessage(
        relativePaths.isEmpty()
            ? QString("No NIfTI files found in %1").arg(QDir::toNativeSeparators(directoryPath))
            : QString("Found %1 NIfTI file%2")
                  .arg(relativePaths.size())
                  .arg(relativePaths.size() == 1 ? "" : "s"),
        4000);
}

void MainWindow::applyLoadedVolume(const QString &sourceLabel) {
    if (!m_volume.isValid()) {
        return;
    }

    m_xSlider->setRange(0, m_volume.width() - 1);
    m_ySlider->setRange(0, m_volume.height() - 1);
    m_zSlider->setRange(0, m_volume.depth() - 1);

    {
        const QSignalBlocker blockX(m_xSlider);
        const QSignalBlocker blockY(m_ySlider);
        const QSignalBlocker blockZ(m_zSlider);
        m_xSlider->setValue(m_xSlider->maximum() / 2);
        m_ySlider->setValue(m_ySlider->maximum() / 2);
        m_zSlider->setValue(m_zSlider->maximum() / 2);
    }

    m_scalarMin = m_volume.minValue();
    m_scalarMax = m_volume.maxValue();
    m_volumeView->setVolume(m_volume.imageData());
    resetContrast();

    m_statusDims->setText(QString("Volume: %1 x %2 x %3")
                              .arg(m_volume.width())
                              .arg(m_volume.height())
                              .arg(m_volume.depth()));
    statusBar()->showMessage(QString("Loaded %1").arg(sourceLabel), 4000);
}

void MainWindow::updateContrastWindow(int sliderValue) {
    setContrastWindowLevel(contrastWindowFromSlider(sliderValue), m_contrastLevel, false);
}

void MainWindow::updateContrastLevel(int sliderValue) {
    setContrastWindowLevel(m_contrastWindow, contrastLevelFromSlider(sliderValue), false);
}

void MainWindow::resetContrast() {
    const double range = qMax(1e-6, m_scalarMax - m_scalarMin);
    setContrastWindowLevel(range, m_scalarMin + (range * 0.5), true);
}

void MainWindow::setContrastWindowLevel(double window, double level, bool syncControls) {
    const double range = qMax(1e-6, m_scalarMax - m_scalarMin);
    const double minWindow = qMax(range / static_cast<double>(kContrastSliderMax), 1e-6);

    m_contrastWindow = qBound(minWindow, window, range);
    m_contrastLevel = qBound(m_scalarMin, level, m_scalarMax);

    if (syncControls) {
        const QSignalBlocker blockWindow(m_contrastWindowSlider);
        const QSignalBlocker blockLevel(m_contrastLevelSlider);
        m_contrastWindowSlider->setValue(contrastWindowToSlider(m_contrastWindow));
        m_contrastLevelSlider->setValue(contrastLevelToSlider(m_contrastLevel));
    }

    updateContrastLabels();

    if (m_volume.isValid()) {
        updateSlices();
    }
}

double MainWindow::contrastWindowFromSlider(int sliderValue) const {
    const double range = qMax(1e-6, m_scalarMax - m_scalarMin);
    const double minWindow = qMax(range / static_cast<double>(kContrastSliderMax), 1e-6);
    const double ratio = static_cast<double>(qBound(0, sliderValue, kContrastSliderMax)) / kContrastSliderMax;
    return minWindow + (ratio * (range - minWindow));
}

double MainWindow::contrastLevelFromSlider(int sliderValue) const {
    const double range = qMax(1e-6, m_scalarMax - m_scalarMin);
    const double ratio = static_cast<double>(qBound(0, sliderValue, kContrastSliderMax)) / kContrastSliderMax;
    return m_scalarMin + (ratio * range);
}

int MainWindow::contrastWindowToSlider(double window) const {
    const double range = qMax(1e-6, m_scalarMax - m_scalarMin);
    const double minWindow = qMax(range / static_cast<double>(kContrastSliderMax), 1e-6);
    if (range <= minWindow) {
        return 0;
    }

    const double ratio = (qBound(minWindow, window, range) - minWindow) / (range - minWindow);
    return qBound(0, qRound(ratio * kContrastSliderMax), kContrastSliderMax);
}

int MainWindow::contrastLevelToSlider(double level) const {
    const double range = qMax(1e-6, m_scalarMax - m_scalarMin);
    if (range <= 1e-6) {
        return 0;
    }

    const double ratio = (qBound(m_scalarMin, level, m_scalarMax) - m_scalarMin) / range;
    return qBound(0, qRound(ratio * kContrastSliderMax), kContrastSliderMax);
}

void MainWindow::updateContrastLabels() {
    const int precision = (m_scalarMax - m_scalarMin >= 100.0) ? 0 : 2;
    m_contrastWindowValue->setText(QString::number(m_contrastWindow, 'f', precision));
    m_contrastLevelValue->setText(QString::number(m_contrastLevel, 'f', precision));
}

void MainWindow::toggleSliceViewExpanded(SliceView *view) {
    if (view == nullptr || m_viewGrid == nullptr) {
        return;
    }

    if (m_expandedSliceView == view) {
        m_expandedSliceView = nullptr;

        m_axialView->show();
        m_coronalView->show();
        m_sagittalView->show();
        m_volumeView->show();

        m_viewGrid->addWidget(m_axialView, 0, 0);
        m_viewGrid->addWidget(m_coronalView, 0, 1);
        m_viewGrid->addWidget(m_sagittalView, 1, 0);
        m_viewGrid->addWidget(m_volumeView, 1, 1);
    } else {
        m_expandedSliceView = view;

        m_axialView->setVisible(view == m_axialView);
        m_coronalView->setVisible(view == m_coronalView);
        m_sagittalView->setVisible(view == m_sagittalView);
        m_volumeView->hide();

        m_viewGrid->addWidget(view, 0, 0, 2, 2);
        view->show();
    }

    syncSliceViewExpansionButtons();
}

void MainWindow::syncSliceViewExpansionButtons() {
    m_axialView->setExpanded(m_expandedSliceView == m_axialView);
    m_coronalView->setExpanded(m_expandedSliceView == m_coronalView);
    m_sagittalView->setExpanded(m_expandedSliceView == m_sagittalView);
}
