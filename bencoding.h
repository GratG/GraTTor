#ifndef BENCODING_H
#define BENCODING_H

#include "QString"

#include <QObject>
#include <QVariant>




class bencoding : public QObject{
    Q_OBJECT
    public:
        bencoding();
        void loadFile(const QString file);
        void loadString(const QString data);
        QList<QVariant> decode();
        QByteArray infoSection();

    private:
        QString filePath;
        QString bEncodedStr; //the encoded string
        QByteArray bEncodedData;
        QByteArray readFile(const QString& path);
        QVariant parse();
        int64_t pos;


        int64_t infoStart;
        int64_t infoLen;

        int parseInt();
        QByteArray parseString();
        QList<QVariant> parseList();
        QHash<QString, QVariant> parseDict();
        QChar currentChar();

};

#endif // BENCODING_H
