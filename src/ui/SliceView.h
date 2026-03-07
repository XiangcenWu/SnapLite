#pragma once

#include <QImage>
#include <QString>
#include <QWidget>

class QResizeEvent;
class QToolButton;
class QWheelEvent;

class SliceView : public QWidget {
    Q_OBJECT

public:
    explicit SliceView(const QString &title, QWidget *parent = nullptr);

    void setSliceImage(const QImage &image);
    void setCrosshair(const QPointF &normalizedPos);
    void setExpanded(bool expanded);

signals:
    void sliceScrollRequested(int steps);
    void fullSizeToggled();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QImage buildExportImage() const;
    void positionControlButtons();
    void saveCurrentImage();
    void updateCrosshairButton();
    void updateFullSizeButton();

    QString m_title;
    QImage m_image;
    QPointF m_crosshair = QPointF(0.5, 0.5);
    int m_wheelAngleRemainder = 0;
    QToolButton *m_fullSizeButton = nullptr;
    QToolButton *m_saveImageButton = nullptr;
    QToolButton *m_crosshairButton = nullptr;
    bool m_expanded = false;
    bool m_crosshairVisible = true;
};
