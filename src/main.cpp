#include <QApplication>

#include "config.h"
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Set up style
    {
        app.setStyle("Fusion");

        QFont font(FONT_FAMILY, FONT_SIZE);
        app.setFont(font);
    }

    MainWindow window;
    window.show();

    return app.exec();
}
