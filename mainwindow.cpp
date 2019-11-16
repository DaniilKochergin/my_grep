#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "finder.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));
    connect(ui->actionSelect_directory, &QAction::triggered, this, &MainWindow::selectDirectory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::buttonClicked);
    ui->currentDirectory->setReadOnly(true);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selectDirectory()
{
    Dir_ = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                             QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->currentDirectory->setText(Dir_);
}

void MainWindow::reciveEnterence(const QString& file_name, const QString &enterences, int count)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, file_name);
    item->setText(1, QString::number(count));
    QTreeWidgetItem *child = new QTreeWidgetItem(item);
    child->setText(0, enterences);
    ui->treeWidget->addTopLevelItem(item);
}

void MainWindow::buttonClicked()
{
    if (!StopButton) {
        ui->treeWidget->clear();
        auto s = ui->lineEdit->text();
        ui->pushButton->setDisabled(true);
        scanDirectory(Dir_, s);
        ui->pushButton->setEnabled(true);
        ui->pushButton->setText("stop");
        ui->actionSelect_directory->setDisabled(true);
    } else {
        ui->pushButton->setDisabled(true);
        for(const auto &thr: Threads_) {
            if (thr && thr->isRunning()) {
                thr->requestInterruption();
            }
        }
        for(const auto &thr: Threads_) {
            if (thr && thr->isRunning()) {
                thr->wait();
            }
            delete thr;
        }
        ui->pushButton->setEnabled(true);
        Threads_.clear();
        ui->pushButton->setText("find");
        ui->actionSelect_directory->setEnabled(true);
    }
    StopButton = !StopButton;
}

void MainWindow::scanDirectory(const QString &dir, const QString& s)
{
    QDir d(dir);
    QStringList dirs = d.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    for (const auto& item: dirs) {
        auto *finder = new Finder(dir + "/" + item, s);
        auto *thread = new QThread();
        finder->moveToThread(thread);
        Threads_.append(thread);
        connect(thread, &QThread::started, finder, &Finder::findSubstr);
        connect(finder, &Finder::onSubstrFound, this, &MainWindow::reciveEnterence);
        thread->start();
    }
    qDebug()<<"All threads connected";
}
