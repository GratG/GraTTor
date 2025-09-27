#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "client.h"
#include "torrent.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void addTorrentTrigger();

private:
    Ui::MainWindow *ui;

    bool addTorrent(QString &fileName, QString &destDir);

    struct Task {
        Client *client;
        QString torrFileName;
        QString destinationDir;
    };

    QList<Task> tasks;

    //bencoding* bencode;
};
#endif // MAINWINDOW_H
