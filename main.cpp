#include "mainwindow.h"
#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Устанавливаем иконку приложения
    app.setWindowIcon(QIcon(":/appicon.png"));

    // Устанавливаем стиль приложения (опционально)
    app.setStyle("Fusion");

    MainWindow window;
    window.show();

    return app.exec();
}
