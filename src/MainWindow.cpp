#include "MainWindow.h"
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSplitter>

#include "config.h"
#include "GatewayRAMDump.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), startAddress(0x08000000)
{
    setWindowTitle("CTR-Heap-Mapper");
    resize(1280, 720);

    setupUi();

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

    memoryView = new QHexView();
    memoryView->setReadOnly(true);
    QFont font(FONT_FAMILY, FONT_SIZE);
    memoryView->setFont(font);

    splitter->addWidget(treeView);
    splitter->addWidget(memoryView);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(splitter);

    central->setLayout(layout);
    setCentralWidget(central);
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Binary File", "",
                                                    "All Files (*);;Binary Files (*.bin *.dat)");

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Error", "Could not open file: " + file.errorString());
        return;
    }

    QByteArray headerData = file.read(sizeof(GatewayRAMDump));
    const GatewayRAMDump* header = reinterpret_cast<const GatewayRAMDump*>(headerData.constData());

    QByteArray regionData = file.read(sizeof(RegionEntry) * header->regionCount);
    const RegionEntry* regions = reinterpret_cast<const RegionEntry*>(regionData.constData());

    for (u32 i = 0; i < header->regionCount; i++)
    {
        if (regions[i].startAddress == startAddress)
        {
            file.seek(regions[i].fileOffset);
            QByteArray block = file.read(regions[i].size);
            memoryView->setData(block);
            memoryView->setBaseAddress(startAddress);
            return;
        }
    }

    QMessageBox::critical(this, "Error", "Heap not found.");
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
