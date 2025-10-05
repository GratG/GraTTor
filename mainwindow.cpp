#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include <QDebug>
#include "QFileDialog"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    QAction *newTorrAction = new QAction("Add torrent", this);
    ui->menuFile->addAction(newTorrAction);

    //setup connections
    connect(newTorrAction, &QAction::triggered, this, &MainWindow::addTorrentTrigger);

}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::addTorrentTrigger() {
    qDebug() <<"SUCCESS!!!";


    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "/home/", "All .torrent files(*.torrent)");
    QString destinationDir = "/home/downloads/";
    addTorrent(fileName, destinationDir);



}

bool MainWindow::addTorrent(QString &fileName, QString &destDir)
{
    //TODO check if file exists

    Client *client = new Client(this);

    client->setTorrent(fileName);


    Task task;
    task.client = client;
    //TODO get/set path directory
    //TODO get proper name for torrent
    task.torrFileName = "PLACE HOLDER";
    tasks.append(task);

    return true;
}

