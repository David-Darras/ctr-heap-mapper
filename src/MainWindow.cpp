#include "MainWindow.h"
#include <QMenu>
#include <QVBoxLayout>
#include <QSplitter>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("CTR-Heap-Mapper");
    resize(1280, 720);

    setupUi();

    QByteArray block = QByteArray::fromRawData(
        "\x48\x65\x6C\x6C\x6F\x2E\x2E\x2E\x2E\x2E\x2E\x2E\x2E\x2E\x2E\x2E"
        "\x42\x79\x65\x2E\x2E\x2E\x2E\x2E\x2E",
        25 // nombre d’octets
    );

    printMemoryBlock(block);
}

void MainWindow::setupUi()
{
    QWidget* central = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;
    QSplitter* splitter = new QSplitter(Qt::Vertical, central);

    treeView = new QTextEdit();
    memoryView = new QTextEdit();

    splitter->addWidget(treeView);
    splitter->addWidget(memoryView);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);

    central->setLayout(layout);
    setCentralWidget(central);
}

void MainWindow::printMemoryBlock(QByteArray data)
{
    QString output;

    for (u32 i = 0; i < data.size(); i += BYTES_PER_LINE)
    {
        QByteArray block = data.mid(i, BYTES_PER_LINE);

        // Offset
        QString offset = QString("%1  ").arg(i, 8, 16, QChar('0')).toUpper();

        // Bytes (Hex)
        QString hex;
        for (int j = 0; j < BYTES_PER_LINE; j++)
        {
            if (j >= block.size())
            {
                hex += "   ";
                continue;
            }

            hex += QString("%1 ").arg((u8)block[j], 2, 16, QChar('0')).toUpper();
        }
        hex += " ";

        // Ascii
        QString ascii;
        for (char c : block)
        {
            if (c >= 0x20 && c < 0x7F)
            {
                ascii += c;
            }
            else
            {
                ascii += ".";
            }
        }
        ascii += "\n";

        output.append(offset);
        output.append(hex);
        output.append(ascii);
    }

    memoryView->setPlainText(output);
}
