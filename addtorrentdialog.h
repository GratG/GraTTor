#ifndef ADDTORRENTDIALOG_H
#define ADDTORRENTDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class AddTorrentDialog;
}

class AddTorrentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTorrentDialog(QWidget *parent = nullptr, QString fn = "");
    ~AddTorrentDialog();
    QString getPath() const;
    QString getDest() const;

private:
    Ui::AddTorrentDialog *ui;
    QString tPath;
    QString dPath;

private slots:
    void onAccept();
    void torrPathSelect();
    void destPathSelect();

};

#endif // ADDTORRENTDIALOG_H
