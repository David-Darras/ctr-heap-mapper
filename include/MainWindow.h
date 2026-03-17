#ifndef CTR_HEAP_MAPPER_MAINAPP_H
#define CTR_HEAP_MAPPER_MAINAPP_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTextEdit>

#include "ExpHeap.h"
#include "QHexView/qhexview.h"

#include "types.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void openFile();

private:
    u32 startAddress;
    ExpHeap heap;

    static const u32 BYTES_PER_LINE = 16;

    void setupUi();
    void printMemoryBlock(QByteArray data);

    QTreeWidgetItem* addHeap(u32 address, u32 size);
    void addMemoryBlock(QTreeWidgetItem* heap, u32 address, u32 size, bool isUsed);

    QTreeWidget* treeView;
    QHexView* memoryView;
};

#endif // CTR_HEAP_MAPPER_MAINAPP_H
