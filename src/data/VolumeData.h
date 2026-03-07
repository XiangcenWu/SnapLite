#pragma once

#include <QImage>
#include <QString>
#include <QVector>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>

class VolumeData {
public:
    VolumeData() = default;

    void generateSynthetic(int width, int height, int depth);
    bool loadNifti(const QString &filePath, QString *errorMessage = nullptr);

    bool isValid() const;

    int width() const { return m_width; }
    int height() const { return m_height; }
    int depth() const { return m_depth; }
    float minValue() const { return m_minValue; }
    float maxValue() const { return m_maxValue; }

    vtkImageData *imageData() const { return m_imageData; }

    QImage axialSlice(int z) const;
    QImage axialSlice(int z, float blackPoint, float whitePoint) const;
    QImage coronalSlice(int y) const;
    QImage coronalSlice(int y, float blackPoint, float whitePoint) const;
    QImage sagittalSlice(int x) const;
    QImage sagittalSlice(int x, float blackPoint, float whitePoint) const;

private:
    int index(int x, int y, int z) const;
    float voxel(int x, int y, int z) const;
    unsigned char toGray(float value, float blackPoint, float whitePoint) const;
    bool loadFromImageData(vtkImageData *imageData, QString *errorMessage);

    int m_width = 0;
    int m_height = 0;
    int m_depth = 0;
    float m_minValue = 0.0f;
    float m_maxValue = 1.0f;
    QVector<float> m_data;
    vtkSmartPointer<vtkImageData> m_imageData;
};
