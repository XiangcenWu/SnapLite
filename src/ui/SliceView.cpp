#include "ui/SliceView.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QStandardPaths>
#include <QStyle>
#include <QToolButton>
#include <QWheelEvent>

namespace {
constexpr int kControlButtonSize = 22;
constexpr int kControlButtonSpacing = 6;

QString controlButtonStyle() {
    return QStringLiteral(
        "QToolButton {"
        " background-color: rgba(40, 40, 40, 180);"
        " border: 1px solid rgba(255, 255, 255, 60);"
        " border-radius: 3px;"
        " color: white;"
        " font-weight: 600;"
        "}"
        "QToolButton:hover {"
        " background-color: rgba(70, 70, 70, 210);"
        "}"
        "QToolButton:checked {"
        " background-color: rgba(170, 45, 45, 220);"
        "}");
}
}

SliceView::SliceView(const QString &title, QWidget *parent)
    : QWidget(parent), m_title(title) {
    setMinimumSize(280, 220);

    m_fullSizeButton = new QToolButton(this);
    m_fullSizeButton->setAutoRaise(true);
    m_fullSizeButton->setCursor(Qt::PointingHandCursor);
    m_fullSizeButton->setFixedSize(kControlButtonSize, kControlButtonSize);
    m_fullSizeButton->setStyleSheet(controlButtonStyle());
    connect(m_fullSizeButton, &QToolButton::clicked, this, &SliceView::fullSizeToggled);

    m_saveImageButton = new QToolButton(this);
    m_saveImageButton->setAutoRaise(true);
    m_saveImageButton->setCursor(Qt::PointingHandCursor);
    m_saveImageButton->setFixedSize(kControlButtonSize, kControlButtonSize);
    m_saveImageButton->setStyleSheet(controlButtonStyle());
    m_saveImageButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
    m_saveImageButton->setToolTip("Save PNG");
    m_saveImageButton->setEnabled(false);
    connect(m_saveImageButton, &QToolButton::clicked, this, &SliceView::saveCurrentImage);

    m_crosshairButton = new QToolButton(this);
    m_crosshairButton->setAutoRaise(true);
    m_crosshairButton->setCursor(Qt::PointingHandCursor);
    m_crosshairButton->setFixedSize(kControlButtonSize, kControlButtonSize);
    m_crosshairButton->setStyleSheet(controlButtonStyle());
    m_crosshairButton->setCheckable(true);
    m_crosshairButton->setChecked(true);
    m_crosshairButton->setText("C");
    connect(m_crosshairButton, &QToolButton::toggled, this, [this](bool checked) {
        m_crosshairVisible = checked;
        updateCrosshairButton();
        update();
    });

    updateFullSizeButton();
    updateCrosshairButton();
    positionControlButtons();
}

void SliceView::setSliceImage(const QImage &image) {
    m_image = image;
    if (m_saveImageButton != nullptr) {
        m_saveImageButton->setEnabled(!m_image.isNull());
    }
    update();
}

void SliceView::setCrosshair(const QPointF &normalizedPos) {
    m_crosshair.setX(qBound(0.0, normalizedPos.x(), 1.0));
    m_crosshair.setY(qBound(0.0, normalizedPos.y(), 1.0));
    update();
}

void SliceView::setExpanded(bool expanded) {
    if (m_expanded == expanded) {
        return;
    }

    m_expanded = expanded;
    updateFullSizeButton();
}

void SliceView::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter p(this);
    p.fillRect(rect(), QColor(25, 25, 25));

    QRect drawRect = rect().adjusted(10, 34, -10, -10);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!m_image.isNull()) {
        QImage scaled = m_image.scaled(drawRect.size(), Qt::KeepAspectRatio, Qt::FastTransformation);
        const int x = drawRect.center().x() - (scaled.width() / 2);
        const int y = drawRect.center().y() - (scaled.height() / 2);
        QRect imageRect(x, y, scaled.width(), scaled.height());
        p.drawImage(imageRect.topLeft(), scaled);

        if (m_crosshairVisible) {
            const int crossX = imageRect.left() + static_cast<int>(m_crosshair.x() * (imageRect.width() - 1));
            const int crossY = imageRect.top() + static_cast<int>(m_crosshair.y() * (imageRect.height() - 1));
            p.setPen(QPen(QColor(220, 40, 40), 1));
            p.drawLine(imageRect.left(), crossY, imageRect.right(), crossY);
            p.drawLine(crossX, imageRect.top(), crossX, imageRect.bottom());
        }
    } else {
        p.setPen(Qt::gray);
        p.drawRect(drawRect);
        p.drawText(drawRect, Qt::AlignCenter, "No slice loaded");
    }

    p.setPen(Qt::white);
    const int titleRight = (m_fullSizeButton != nullptr) ? m_fullSizeButton->geometry().left() - 8 : width() - 10;
    p.drawText(QRect(10, 6, qMax(0, titleRight - 10), 22), Qt::AlignLeft | Qt::AlignVCenter, m_title);
}

void SliceView::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    positionControlButtons();
}

void SliceView::wheelEvent(QWheelEvent *event) {
    const int angleDeltaY = event->angleDelta().y();
    if (angleDeltaY != 0) {
        m_wheelAngleRemainder += angleDeltaY;
        const int steps = m_wheelAngleRemainder / 120;
        m_wheelAngleRemainder %= 120;
        if (steps != 0) {
            emit sliceScrollRequested(steps);
        }
        event->accept();
        return;
    }

    const int pixelDeltaY = event->pixelDelta().y();
    if (pixelDeltaY != 0) {
        emit sliceScrollRequested(pixelDeltaY > 0 ? 1 : -1);
        event->accept();
        return;
    }

    QWidget::wheelEvent(event);
}

void SliceView::updateFullSizeButton() {
    if (m_fullSizeButton == nullptr) {
        return;
    }

    m_fullSizeButton->setIcon(
        style()->standardIcon(m_expanded ? QStyle::SP_TitleBarNormalButton : QStyle::SP_TitleBarMaxButton));
    m_fullSizeButton->setToolTip(m_expanded ? "Restore view" : "Expand view");
}

void SliceView::updateCrosshairButton() {
    if (m_crosshairButton == nullptr) {
        return;
    }

    m_crosshairButton->setToolTip(m_crosshairVisible ? "Hide crosshair" : "Show crosshair");
}

void SliceView::positionControlButtons() {
    const int x = width() - kControlButtonSize - 8;
    int y = 6;

    if (m_fullSizeButton != nullptr) {
        m_fullSizeButton->move(x, y);
        m_fullSizeButton->raise();
        y += kControlButtonSize + kControlButtonSpacing;
    }

    if (m_saveImageButton != nullptr) {
        m_saveImageButton->move(x, y);
        m_saveImageButton->raise();
        y += kControlButtonSize + kControlButtonSpacing;
    }

    if (m_crosshairButton != nullptr) {
        m_crosshairButton->move(x, y);
        m_crosshairButton->raise();
    }
}

QImage SliceView::buildExportImage() const {
    if (m_image.isNull()) {
        return {};
    }

    QImage exportImage = m_image.convertToFormat(QImage::Format_ARGB32);
    if (!m_crosshairVisible) {
        return exportImage;
    }

    QPainter painter(&exportImage);
    painter.setPen(QPen(QColor(220, 40, 40), 1));

    const int crossX = qBound(0, static_cast<int>(m_crosshair.x() * (exportImage.width() - 1)), exportImage.width() - 1);
    const int crossY = qBound(0, static_cast<int>(m_crosshair.y() * (exportImage.height() - 1)), exportImage.height() - 1);
    painter.drawLine(0, crossY, exportImage.width() - 1, crossY);
    painter.drawLine(crossX, 0, crossX, exportImage.height() - 1);

    return exportImage;
}

void SliceView::saveCurrentImage() {
    if (m_image.isNull()) {
        QMessageBox::information(this, "Save PNG", "No slice image is available to save.");
        return;
    }

    QString picturesDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (picturesDir.isEmpty()) {
        picturesDir = QDir::homePath();
    }

    const QString suggestedFileName = QString("%1-%2.png")
                                          .arg(m_title.toLower())
                                          .arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Slice PNG",
        QDir(picturesDir).filePath(suggestedFileName),
        "PNG Image (*.png)");
    if (filePath.isEmpty()) {
        return;
    }

    const QString normalizedFilePath = filePath.endsWith(".png", Qt::CaseInsensitive)
                                           ? filePath
                                           : QString("%1.png").arg(filePath);
    QImage exportImage = buildExportImage();
    if (!exportImage.save(normalizedFilePath, "PNG")) {
        QMessageBox::warning(this, "Save PNG", "Failed to save the PNG image.");
    }
}
