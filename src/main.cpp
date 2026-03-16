#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Set up style
    {
        app.setStyle("Fusion");

        QFont font("JetBrains Mono", 11);
        app.setFont(font);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
