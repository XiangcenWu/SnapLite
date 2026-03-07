#pragma once

#include <QWidget>

class QVTKOpenGLNativeWidget;
class vtkGenericOpenGLRenderWindow;
class vtkImageData;
class vtkRenderer;

class VolumeRenderPlaceholder : public QWidget {
    Q_OBJECT

public:
    explicit VolumeRenderPlaceholder(QWidget *parent = nullptr);

    void setVolume(vtkImageData *imageData);

private:
    QVTKOpenGLNativeWidget *m_vtkWidget = nullptr;
    vtkGenericOpenGLRenderWindow *m_renderWindow = nullptr;
    vtkRenderer *m_renderer = nullptr;
};
