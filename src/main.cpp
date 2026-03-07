#include <QApplication>
#include <QSurfaceFormat>

#include <QVTKOpenGLNativeWidget.h>

#include "ui/MainWindow.h"

int main(int argc, char *argv[]) {
    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication app(argc, argv);

    QApplication::setApplicationName("QtSnapLite");
    QApplication::setOrganizationName("VarReg");

    MainWindow window;
    window.resize(1280, 820);
    window.show();

    return app.exec();
}
