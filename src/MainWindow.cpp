#include "MainWindow.h"
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
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
        25
    );

    printMemoryBlock(block);

    QTreeWidgetItem* heap1 = addHeap(0x08000000, 0x10000);
    addMemoryBlock(heap1, 0x08000000, 0x1000, true);
    addMemoryBlock(heap1, 0x08001000, 0x500, false);

    QTreeWidgetItem* heap2 = addHeap(0x08200000, 0x20000);
    addMemoryBlock(heap2, 0x08200000, 0x2000, true);
}

void MainWindow::setupUi()
{
    QWidget* central = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;
    QSplitter* splitter = new QSplitter(Qt::Vertical, central);

    QMenu* fileMenu = menuBar()->addMenu("File");
    QAction* openAction = new QAction("Open", this);
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAction);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    treeView = new QTreeWidget();
    treeView->setColumnCount(3);
    treeView->setHeaderLabels(QStringList() << "Address" << "Size" << "State");
    treeView->setColumnWidth(0, 150);
    treeView->setColumnWidth(1, 120);
    treeView->setColumnWidth(2, 100);

    memoryView = new QTextEdit();
    memoryView->setReadOnly(true);

    splitter->addWidget(treeView);
    splitter->addWidget(memoryView);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);

    central->setLayout(layout);
    setCentralWidget(central);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Binary File", "", "All Files (*);;Binary Files (*.bin *.dat)");

    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not open file: " + file.errorString());
        return;
    }

    QByteArray data = file.read(0x100);
    file.close();

    printMemoryBlock(data);
}

QTreeWidgetItem* MainWindow::addHeap(u32 address, u32 size)
{
    QTreeWidgetItem* heapItem = new QTreeWidgetItem(treeView);

    heapItem->setText(0, QString("0x%1").arg(address, 8, 16, QChar('0')));
    heapItem->setText(1, QString("0x%1").arg(size, 8, 16, QChar('0')));
    heapItem->setText(2, QString("---"));

    heapItem->setExpanded(true);
    treeView->addTopLevelItem(heapItem);
    return heapItem;
}

void MainWindow::addMemoryBlock(QTreeWidgetItem* heap, u32 address, u32 size, bool isUsed)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(heap);

    item->setText(0, QString("0x%1").arg(address, 8, 16, QChar('0')));
    item->setText(1, QString("0x%1").arg(size, 8, 16, QChar('0')));
    item->setText(2, isUsed ? "Used" : "Free");

    heap->addChild(item);
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
