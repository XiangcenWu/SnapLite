#include "rendering/VolumeRenderPlaceholder.h"

#include <QVBoxLayout>

#include <QVTKOpenGLNativeWidget.h>

#include <vtkColorTransferFunction.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

VolumeRenderPlaceholder::VolumeRenderPlaceholder(QWidget *parent)
    : QWidget(parent) {
    setMinimumSize(280, 220);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_vtkWidget = new QVTKOpenGLNativeWidget(this);
    layout->addWidget(m_vtkWidget);

    vtkNew<vtkGenericOpenGLRenderWindow> window;
    m_renderWindow = window;
    m_vtkWidget->setRenderWindow(window);

    vtkNew<vtkRenderer> renderer;
    m_renderer = renderer;
    m_renderer->SetBackground(0.08, 0.1, 0.14);
    m_renderWindow->AddRenderer(renderer);
}

void VolumeRenderPlaceholder::setVolume(vtkImageData *imageData) {
    m_renderer->RemoveAllViewProps();

    if (imageData == nullptr) {
        m_renderWindow->Render();
        return;
    }

    double range[2] = {0.0, 1.0};
    imageData->GetScalarRange(range);
    const double minValue = range[0];
    const double maxValue = (range[1] > range[0]) ? range[1] : range[0] + 1.0;
    const double midValue = minValue + (maxValue - minValue) * 0.45;

    vtkNew<vtkSmartVolumeMapper> mapper;
    mapper->SetInputData(imageData);

    vtkNew<vtkColorTransferFunction> color;
    color->AddRGBPoint(minValue, 0.0, 0.0, 0.0);
    color->AddRGBPoint(midValue, 0.85, 0.85, 0.85);
    color->AddRGBPoint(maxValue, 1.0, 1.0, 1.0);

    vtkNew<vtkPiecewiseFunction> opacity;
    opacity->AddPoint(minValue, 0.0);
    opacity->AddPoint(midValue, 0.12);
    opacity->AddPoint(maxValue, 0.65);

    vtkNew<vtkVolumeProperty> property;
    property->SetColor(color);
    property->SetScalarOpacity(opacity);
    property->SetInterpolationTypeToLinear();
    property->ShadeOff();

    vtkNew<vtkVolume> volume;
    volume->SetMapper(mapper);
    volume->SetProperty(property);

    m_renderer->AddVolume(volume);
    m_renderer->ResetCamera();
    m_renderWindow->Render();
}
