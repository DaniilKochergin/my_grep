#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "finder.h"

#include <QMainWindow>
#include <QVector>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void StopAll();
public slots:
    void scanDirectory(const QString& dir, const QString& s);
    void selectDirectory();
    void reciveEnterence(const QString& file_name, const QString& enterences, int count);
    void FinishSubdirectory();
    void buttonClicked();
private:
    int counted = 0;
    bool StopButton = false;
    QVector<QThread *> Threads_;
    QString Dir_;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
