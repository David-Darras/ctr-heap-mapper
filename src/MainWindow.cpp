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
    treeView->setColumnCount(1);
    treeView->setHeaderHidden(true);

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

    if (fileName.isEmpty()) return;

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
            parseHeapData(block, startAddress);
            return;
        }
    }

    QMessageBox::critical(this, "Error", "Heap not found.");
}

void MainWindow::parseHeapData(const QByteArray& data, u32 baseAddr)
{
    treeView->clear();
    if ((u32)data.size() < sizeof(ExpHeap)) return;

    const ExpHeap* heap = reinterpret_cast<const ExpHeap*>(data.constData());
    if (!heap->isValid())
    {
        QMessageBox::warning(this, "Error", "ExpHeap signature (HPXE) not found at the specified address.");
        return;
    }

    QString heapStr = QString("ExpHeap [Start: 0x%1 | Size: 0x%2 | Allocs: %3 | Policy: %4]")
                      .arg(heap->core.start, 8, 16, QChar('0'))
                      .arg(heap->getTotalSize(), 0, 16, QChar('0'))
                      .arg(heap->allocCount)
                      .arg(heap->core.isBestFit ? "Best Fit" : "First Fit");

    QTreeWidgetItem* heapItem = new QTreeWidgetItem(treeView);
    heapItem->setText(0, heapStr);
    heapItem->setForeground(0, Qt::darkRed);
    heapItem->setExpanded(true);

    struct BlockInfo
    {
        u32 address;
        const MemoryBlockHeader* header;
    };
    QList<BlockInfo> blocks;

    auto traverseBlocks = [&](u32 startNodeAddr)
    {
        u32 currAddr = startNodeAddr;
        while (currAddr != 0)
        {
            if (currAddr < baseAddr || (currAddr - baseAddr + sizeof(MemoryBlockHeader)) > (u32)data.size())
                break;

            const MemoryBlockHeader* header = reinterpret_cast<const MemoryBlockHeader*>(data.constData() + (currAddr -
                baseAddr));
            blocks.append({currAddr, header});

            if (header->next == currAddr) break;
            currAddr = header->next;
        }
    };

    traverseBlocks(heap->core.usedBlocks.head);
    traverseBlocks(heap->core.freeBlocks.head);

    std::sort(blocks.begin(), blocks.end(), [](const BlockInfo& a, const BlockInfo& b)
    {
        return a.address < b.address;
    });

    for (const auto& block : blocks)
    {
        u32 payloadAddr = block.address + sizeof(MemoryBlockHeader);
        QString stateStr = block.header->isUsed() ? "Used" : (block.header->isFree() ? "Free" : "Unknown");
        QString allocDir = block.header->isRearAlloc ? "Rear" : "Front";

        QString blockStr = QString("MemBlock [Addr: 0x%1 | Size: 0x%2 | State: %3 | Align: %4 | Dir: %5]")
                           .arg(payloadAddr, 8, 16, QChar('0'))
                           .arg(block.header->payloadSize, 0, 16, QChar('0'))
                           .arg(stateStr)
                           .arg(block.header->alignment)
                           .arg(allocDir);

        QTreeWidgetItem* item = new QTreeWidgetItem(heapItem);
        item->setText(0, blockStr);
        item->setData(0, Qt::UserRole, payloadAddr);
        if (block.header->isUsed()) item->setForeground(0, Qt::darkBlue);
        else item->setForeground(0, Qt::darkGreen);
    }
}
