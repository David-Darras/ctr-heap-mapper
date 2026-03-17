#include "MainWindow.h"
#include <QMenuBar>
#include <QAction>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QSplitter>

#include "config.h"
#include "ExpHeap.h"
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
    connect(treeView, &QTreeWidget::itemClicked, this, &MainWindow::onItemClicked);

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

const void* MainWindow::getMemoryPointer(u32 address, u32 size)
{
    if (size == 0) return nullptr;

    for (const auto& region : loadedRegions)
    {
        if (address >= region.startAddress && (address + size) <= (region.startAddress + region.data.size()))
        {
            u32 offset = address - region.startAddress;
            return region.data.constData() + offset;
        }
    }
    return nullptr;
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

    treeView->clear();
    loadedRegions.clear();
    QSet<u32> parsedCores;

    for (u32 i = 0; i < header->regionCount; i++)
    {
        file.seek(regions[i].fileOffset);
        QByteArray dump = file.read(regions[i].size);
        loadedRegions.append({regions[i].startAddress, dump});
    }

    const ExpHeap* initialHeap = reinterpret_cast<const ExpHeap*>(getMemoryPointer(startAddress, sizeof(ExpHeap)));

    if (initialHeap && initialHeap->isValid())
    {
        u32 currentCoreAddr = startAddress + offsetof(ExpHeap, core);

        while (true)
        {
            u32 heapAddr = currentCoreAddr - offsetof(ExpHeap, core);
            const ExpHeap* h = reinterpret_cast<const ExpHeap*>(getMemoryPointer(heapAddr, sizeof(ExpHeap)));

            if (!h || !h->isValid() || h->core.siblings.prev == 0) break;
            if (parsedCores.contains(h->core.siblings.prev)) break;

            currentCoreAddr = h->core.siblings.prev;
        }

        parseExpHeapList(currentCoreAddr, treeView->invisibleRootItem(), parsedCores);
    }
    else
    {
        QMessageBox::warning(this, "Avertissement",
                             QString("Aucun ExpHeap valide trouvé à l'adresse de départ (0x%1).")
                             .arg(startAddress, 8, 16, QChar('0')));
    }
}

void MainWindow::parseExpHeapList(u32 startCoreAddr, QTreeWidgetItem* parentItem, QSet<u32>& parsedCores)
{
    u32 currentCoreAddr = startCoreAddr;

    while (currentCoreAddr != 0)
    {
        if (parsedCores.contains(currentCoreAddr)) break;
        parsedCores.insert(currentCoreAddr);

        u32 heapAddr = currentCoreAddr - offsetof(ExpHeap, core);
        const ExpHeap* heap = reinterpret_cast<const ExpHeap*>(getMemoryPointer(heapAddr, sizeof(ExpHeap)));

        if (!heap || !heap->isValid()) break;

        QString heapStr = QString("ExpHeap [Addr: 0x%1 | Start: 0x%2 | Size: 0x%3 | Allocs: %4]")
                          .arg(heapAddr, 8, 16, QChar('0'))
                          .arg(heap->core.start, 8, 16, QChar('0'))
                          .arg(heap->getTotalSize(), 0, 16, QChar('0'))
                          .arg(heap->allocCount);

        QTreeWidgetItem* heapItem = new QTreeWidgetItem(parentItem);
        heapItem->setText(0, heapStr);
        heapItem->setForeground(0, Qt::darkRed);
        heapItem->setExpanded(true);
        heapItem->setData(0, Qt::UserRole, heapAddr);
        heapItem->setData(0, Qt::UserRole + 1, heap->core.end - heapAddr); // Taille approximative dans la RAM

        if (heap->core.subHeaps.head != 0 || heap->core.subHeaps.count > 0)
        {
            QTreeWidgetItem* subHeapsItem = new QTreeWidgetItem(heapItem);
            subHeapsItem->setText(0, QString("SubHeaps [Count: %1 | Head: 0x%2 | Tail: 0x%3]")
                                     .arg(heap->core.subHeaps.count)
                                     .arg(heap->core.subHeaps.head, 8, 16, QChar('0'))
                                     .arg(heap->core.subHeaps.tail, 8, 16, QChar('0')));
            subHeapsItem->setForeground(0, Qt::darkCyan);
            if (heap->core.subHeaps.head != 0)
            {
                parseExpHeapList(heap->core.subHeaps.head, subHeapsItem, parsedCores);
            }
        }

        parseHeapBlocks(heap, heapItem);
        currentCoreAddr = heap->core.siblings.next;
    }
}

void MainWindow::parseHeapBlocks(const ExpHeap* heap, QTreeWidgetItem* heapItem)
{
    struct BlockInfo
    {
        u32 address;
        const MemoryBlockHeader* header;
    };
    QList<BlockInfo> blocks;

    auto traverseBlocks = [&](u32 startNodeAddr)
    {
        u32 currAddr = startNodeAddr;
        QSet<u32> visitedBlocks;

        while (currAddr != 0)
        {
            if (visitedBlocks.contains(currAddr)) break;
            visitedBlocks.insert(currAddr);

            const MemoryBlockHeader* header = reinterpret_cast<const MemoryBlockHeader*>(
                getMemoryPointer(currAddr, sizeof(MemoryBlockHeader))
            );

            if (!header) break;

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
        item->setData(0, Qt::UserRole + 1, block.header->payloadSize);

        if (block.header->isUsed()) item->setForeground(0, Qt::darkBlue);
        else item->setForeground(0, Qt::darkGreen);
    }
}

void MainWindow::onItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column);
    if (loadedRegions.isEmpty()) return;

    u32 targetAddr = item->data(0, Qt::UserRole).toUInt();
    u32 targetSize = item->data(0, Qt::UserRole + 1).toUInt();

    if (targetSize == 0) return;

    for (const auto& region : loadedRegions)
    {
        if (targetAddr >= region.startAddress && targetAddr < region.startAddress + (u32)region.data.size())
        {
            u32 offset = targetAddr - region.startAddress;
            u32 availableSize = (u32)region.data.size() - offset;
            u32 readSize = qMin(targetSize, availableSize);

            QByteArray slice = region.data.mid(offset, readSize);
            memoryView->setData(slice);
            memoryView->setBaseAddress(targetAddr);
            return;
        }
    }
}
