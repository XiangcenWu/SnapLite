#include "data/VolumeData.h"

#include <QtMath>

#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkNIFTIImageReader.h>
#include <vtkPointData.h>

void VolumeData::generateSynthetic(int width, int height, int depth) {
    m_width = width;
    m_height = height;
    m_depth = depth;
    m_minValue = 0.0f;
    m_maxValue = 4095.0f;
    m_data.resize(m_width * m_height * m_depth);

    const float cx = (m_width - 1) * 0.5f;
    const float cy = (m_height - 1) * 0.5f;
    const float cz = (m_depth - 1) * 0.5f;
    const float radius = qMin(qMin(m_width, m_height), m_depth) * 0.32f;

    m_imageData = vtkSmartPointer<vtkImageData>::New();
    m_imageData->SetDimensions(m_width, m_height, m_depth);
    m_imageData->AllocateScalars(VTK_FLOAT, 1);

    for (int z = 0; z < m_depth; ++z) {
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                const float dx = x - cx;
                const float dy = y - cy;
                const float dz = z - cz;
                const float dist = qSqrt(dx * dx + dy * dy + dz * dz);

                float value = 400.0f;
                if (dist < radius) {
                    value = 3200.0f - dist * 35.0f;
                }

                const float ridge = 450.0f * qSin((x * 0.08f) + (z * 0.06f));
                value += ridge;
                value = qBound(0.0f, value, 4095.0f);

                m_data[index(x, y, z)] = value;
                static_cast<float *>(m_imageData->GetScalarPointer(x, y, z))[0] = value;
            }
        }
    }
}

bool VolumeData::loadNifti(const QString &filePath, QString *errorMessage) {
    vtkNew<vtkNIFTIImageReader> reader;
    reader->SetFileName(filePath.toUtf8().constData());
    reader->Update();

    return loadFromImageData(reader->GetOutput(), errorMessage);
}

bool VolumeData::isValid() const {
    return m_width > 0 && m_height > 0 && m_depth > 0 && m_data.size() == (m_width * m_height * m_depth);
}

QImage VolumeData::axialSlice(int z) const {
    return axialSlice(z, m_minValue, m_maxValue);
}

QImage VolumeData::axialSlice(int z, float blackPoint, float whitePoint) const {
    if (!isValid()) {
        return {};
    }

    z = qBound(0, z, m_depth - 1);
    QImage img(m_width, m_height, QImage::Format_Grayscale8);
    for (int y = 0; y < m_height; ++y) {
        uchar *row = img.scanLine(y);
        for (int x = 0; x < m_width; ++x) {
            row[x] = toGray(voxel(x, y, z), blackPoint, whitePoint);
        }
    }
    return img;
}

QImage VolumeData::coronalSlice(int y) const {
    return coronalSlice(y, m_minValue, m_maxValue);
}

QImage VolumeData::coronalSlice(int y, float blackPoint, float whitePoint) const {
    if (!isValid()) {
        return {};
    }

    y = qBound(0, y, m_height - 1);
    QImage img(m_width, m_depth, QImage::Format_Grayscale8);
    for (int z = 0; z < m_depth; ++z) {
        uchar *row = img.scanLine(z);
        for (int x = 0; x < m_width; ++x) {
            row[x] = toGray(voxel(x, y, z), blackPoint, whitePoint);
        }
    }
    return img;
}

QImage VolumeData::sagittalSlice(int x) const {
    return sagittalSlice(x, m_minValue, m_maxValue);
}

QImage VolumeData::sagittalSlice(int x, float blackPoint, float whitePoint) const {
    if (!isValid()) {
        return {};
    }

    x = qBound(0, x, m_width - 1);
    QImage img(m_height, m_depth, QImage::Format_Grayscale8);
    for (int z = 0; z < m_depth; ++z) {
        uchar *row = img.scanLine(z);
        for (int y = 0; y < m_height; ++y) {
            row[y] = toGray(voxel(x, y, z), blackPoint, whitePoint);
        }
    }
    return img;
}

int VolumeData::index(int x, int y, int z) const {
    return z * (m_width * m_height) + y * m_width + x;
}

float VolumeData::voxel(int x, int y, int z) const {
    return m_data[index(x, y, z)];
}

unsigned char VolumeData::toGray(float value, float blackPoint, float whitePoint) const {
    const float low = qMin(blackPoint, whitePoint);
    const float high = qMax(low + 1e-6f, qMax(blackPoint, whitePoint));
    const float normalized = (value - low) / (high - low);
    return static_cast<unsigned char>(qBound(0, static_cast<int>(normalized * 255.0f), 255));
}

bool VolumeData::loadFromImageData(vtkImageData *imageData, QString *errorMessage) {
    if (imageData == nullptr) {
        if (errorMessage != nullptr) {
            *errorMessage = "Reader returned an empty image.";
        }
        return false;
    }

    int dims[3] = {0, 0, 0};
    imageData->GetDimensions(dims);
    if (dims[0] <= 0 || dims[1] <= 0 || dims[2] <= 0) {
        if (errorMessage != nullptr) {
            *errorMessage = "Image dimensions are invalid.";
        }
        return false;
    }

    vtkDataArray *scalars = imageData->GetPointData()->GetScalars();
    if (scalars == nullptr || scalars->GetNumberOfTuples() <= 0) {
        if (errorMessage != nullptr) {
            *errorMessage = "Image has no scalar voxels.";
        }
        return false;
    }

    m_width = dims[0];
    m_height = dims[1];
    m_depth = dims[2];

    const vtkIdType voxelCount = static_cast<vtkIdType>(m_width) * m_height * m_depth;
    if (scalars->GetNumberOfTuples() < voxelCount) {
        if (errorMessage != nullptr) {
            *errorMessage = "Scalar voxel count does not match image dimensions.";
        }
        return false;
    }

    m_data.resize(static_cast<int>(voxelCount));
    for (vtkIdType i = 0; i < voxelCount; ++i) {
        m_data[static_cast<int>(i)] = static_cast<float>(scalars->GetComponent(i, 0));
    }

    double range[2] = {0.0, 1.0};
    scalars->GetRange(range, 0);
    m_minValue = static_cast<float>(range[0]);
    m_maxValue = static_cast<float>(range[1]);
    if (m_minValue == m_maxValue) {
        m_maxValue = m_minValue + 1.0f;
    }

    m_imageData = vtkSmartPointer<vtkImageData>::New();
    m_imageData->DeepCopy(imageData);
    return true;
}
