#ifndef CTR_HEAP_MAPPER_MAINAPP_H
#define CTR_HEAP_MAPPER_MAINAPP_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTextEdit>

#include "ExpHeap.h"
#include "QHexView/qhexview.h"

#include "types.h"

struct RegionData
{
    u32 startAddress;
    QByteArray data;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void openFile();
    void onItemClicked(QTreeWidgetItem* item, int column);

private:
    u32 startAddress;
    QList<RegionData> loadedRegions;

    void setupUi();
    const void* getMemoryPointer(u32 address, u32 size);
    void parseExpHeapList(u32 startCoreAddr, QTreeWidgetItem* parentItem, QSet<u32>& parsedCores);
    void parseHeapBlocks(const ExpHeap* heap, QTreeWidgetItem* heapItem);

    QTreeWidget* treeView;
    QHexView* memoryView;
};

#endif // CTR_HEAP_MAPPER_MAINAPP_H
