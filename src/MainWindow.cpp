#include "MainWindow.h"
#include <QMenu>
#include <QVBoxLayout>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
  setWindowTitle("CTR-Heap-Mapper");
  resize(1280, 720);

  setupUi();
}

void MainWindow::setupUi()
{
  QWidget *central = new QWidget;
  QVBoxLayout *layout = new QVBoxLayout;
  QSplitter *splitter = new QSplitter(Qt::Vertical, central);

  treeView = new QTextEdit();
  memoryView = new QTextEdit();

  splitter->addWidget(treeView);
  splitter->addWidget(memoryView);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(splitter);

  central->setLayout(layout);
  setCentralWidget(central);
}
