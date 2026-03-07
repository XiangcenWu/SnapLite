#pragma once

#include <QMainWindow>
#include "data/VolumeData.h"

class QLabel;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QGridLayout;
class QSlider;
class SliceView;
class VolumeRenderPlaceholder;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void resetView();
    void generateDemoVolume();
    void openNiftiFile();
    void openNiftiDirectory();
    void loadNiftiFromListItem(QListWidgetItem *item);
    void updateSlices();
    void updateContrastWindow(int sliderValue);
    void updateContrastLevel(int sliderValue);
    void resetContrast();

private:
    void buildUi();
    void buildMenus();
    void connectSignals();
    QWidget *buildNiftiBrowserPanel();
    QWidget *buildControlPanel();
    bool loadVolumeFromFile(const QString &filePath);
    void populateNiftiFileList(const QString &directoryPath);
    void applyLoadedVolume(const QString &sourceLabel);
    void setContrastWindowLevel(double window, double level, bool syncControls);
    double contrastWindowFromSlider(int sliderValue) const;
    double contrastLevelFromSlider(int sliderValue) const;
    int contrastWindowToSlider(double window) const;
    int contrastLevelToSlider(double level) const;
    void updateContrastLabels();
    void toggleSliceViewExpanded(SliceView *view);
    void syncSliceViewExpansionButtons();

    SliceView *m_axialView = nullptr;
    SliceView *m_coronalView = nullptr;
    SliceView *m_sagittalView = nullptr;
    VolumeRenderPlaceholder *m_volumeView = nullptr;
    QGridLayout *m_viewGrid = nullptr;
    SliceView *m_expandedSliceView = nullptr;

    QSlider *m_xSlider = nullptr;
    QSlider *m_ySlider = nullptr;
    QSlider *m_zSlider = nullptr;
    QSlider *m_contrastWindowSlider = nullptr;
    QSlider *m_contrastLevelSlider = nullptr;
    QListWidget *m_niftiFileList = nullptr;
    QLabel *m_statusDims = nullptr;
    QLabel *m_niftiDirectoryValue = nullptr;
    QLabel *m_niftiFileCountValue = nullptr;
    QLabel *m_contrastWindowValue = nullptr;
    QLabel *m_contrastLevelValue = nullptr;
    QPushButton *m_openDirectoryButton = nullptr;
    QPushButton *m_resetContrastButton = nullptr;

    QString m_niftiDirectoryPath;
    VolumeData m_volume;
    double m_scalarMin = 0.0;
    double m_scalarMax = 1.0;
    double m_contrastWindow = 1.0;
    double m_contrastLevel = 0.5;
};
