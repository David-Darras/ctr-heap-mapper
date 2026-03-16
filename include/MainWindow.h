#ifndef CTR_HEAP_MAPPER_MAINAPP_H
#define CTR_HEAP_MAPPER_MAINAPP_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTextEdit>

class MainWindow : public QMainWindow {
public:
  explicit MainWindow(QWidget *parent = nullptr);

private:
  void setupUi();

  QTextEdit *treeView;
  QTextEdit *memoryView;
};

#endif // CTR_HEAP_MAPPER_MAINAPP_H
