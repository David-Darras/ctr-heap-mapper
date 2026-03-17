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
    void onItemClicked(QTreeWidgetItem* item, int column);

private:
    u32 startAddress;
    QByteArray currentDump;

    void setupUi();
    void parseHeapData(const QByteArray& data, u32 baseAddr);

    QTreeWidget* treeView;
    QHexView* memoryView;
};

#endif // CTR_HEAP_MAPPER_MAINAPP_H
