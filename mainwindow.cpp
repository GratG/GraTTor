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

    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"name", "status", "progress", "download", "upload"});
    ui->mainView->setModel(model);
    ui->mainView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


    //setup connections
    connect(newTorrAction, &QAction::triggered, this, &MainWindow::addTorrentTrigger);

}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::addTorrentTrigger() {

    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "/home/", "All .torrent files(*.torrent)");
    QString destinationDir = "/home/downloads/";
    addTorrent(fileName, destinationDir);



}


bool MainWindow::addTorrent(QString &fileName, QString &destDir)
{
    //TODO check if file exists

    Client *client = new Client(this);

    client->setTorrent(fileName);

    connect(client, &Client::updateProg, this, &MainWindow::updateProg);

    Task task;
    task.client = client;
    //TODO get/set path directory
    //TODO get proper name for torrent
    task.torrFileName = task.client->getTorrName();
    tasks.append(task);


    QList<QStandardItem*> item;
    item << new QStandardItem(task.torrFileName);
    item << new QStandardItem("0");
    item << new QStandardItem("temp");
    item << new QStandardItem("N/A");
    item << new QStandardItem("N/A");

    model->appendRow(item);
    return true;
}


void MainWindow::updateProg(double percent)
{
    //get row of client
    Client *c = qobject_cast<Client *>(sender());

    int row;
    for(int i = 0; i < tasks.size(); i++){
        if(tasks[i].client == c){
            row = i;
            break;
        }
    }

    model->setItem(row, 2, new QStandardItem(QString::number(percent)));
    qDebug() << percent;
}

