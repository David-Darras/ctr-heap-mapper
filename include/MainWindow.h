#ifndef CTR_HEAP_MAPPER_MAINAPP_H
#define CTR_HEAP_MAPPER_MAINAPP_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTextEdit>

#include "types.h"

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    static const u32 BYTES_PER_LINE = 16;

    void setupUi();
    void printMemoryBlock(QByteArray data);

    QTreeWidgetItem* addHeap(u32 address, u32 size);
    void addMemoryBlock(QTreeWidgetItem* heap, u32 address, u32 size, bool isUsed);

    QTreeWidget* treeView;
    QTextEdit* memoryView;
};

#endif // CTR_HEAP_MAPPER_MAINAPP_H
