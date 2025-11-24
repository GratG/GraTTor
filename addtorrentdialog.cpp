#include "addtorrentdialog.h"
#include "ui_addtorrentdialog.h"

#include <QFile>
#include <QMessageBox>
#include <QFileDialog>

AddTorrentDialog::AddTorrentDialog(QWidget *parent, QString fn)
    : QDialog(parent)
    , ui(new Ui::AddTorrentDialog)
{
    ui->setupUi(this);
    ui->torrFileValLabel->setText(fn);
    ui->destValLabel->setText("/home/downloads/");
    connect(ui->torrFileBtn, &QPushButton::clicked, this, &AddTorrentDialog::torrPathSelect);
    connect(ui->destBtn, &QPushButton::clicked, this, &AddTorrentDialog::destPathSelect);
}

AddTorrentDialog::~AddTorrentDialog()
{
    delete ui;
}

QString AddTorrentDialog::getPath() const
{
    //get torrent path
    return ui->torrFileValLabel->text();
}

QString AddTorrentDialog::getDest() const
{
    return ui->destValLabel->text();
}

void AddTorrentDialog::onAccept()
{
    if(tPath.isEmpty()){
        QMessageBox::warning(this, "Error", "Select torrent file");
        return;
    }

    if(!QFile::exists(tPath)){
        QMessageBox::critical(this, "Invalid File", "Selected file not found");
    }

    accept();
}

void AddTorrentDialog::torrPathSelect()
{
    qDebug() << "TORR PATH TRIGGER";
    QString p = QFileDialog::getOpenFileName(this, "Open File", "/home/", "All .torrent files(*.torrent)");
    tPath = p;
    ui->torrFileValLabel->setText(p);
}

void AddTorrentDialog::destPathSelect()
{
    qDebug() << "DEST PATH TRIGGER";
    QString d = QFileDialog::getExistingDirectory(this, "Select folder", "/home/");
    dPath = d;
    ui->destValLabel->setText(d);
}
