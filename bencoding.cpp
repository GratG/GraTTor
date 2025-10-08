#include "bencoding.h"

#include <QFile>
#include <QTextStream>
#include <iostream>

#include <QDebug>
bencoding::bencoding() {
    pos = 0;

}

void bencoding::loadFile(const QString file)
{
    filePath = file;
    //read file
    bEncodedData = readFile(filePath);
    pos = 0;
}

void bencoding::loadString(const QString data){
    bEncodedData = data.toUtf8();
    pos = 0;
}

void bencoding::loadString(const QByteArray data){
    bEncodedData = data;
    pos = 0;
}

QList<QVariant> bencoding::decode()
{

    QList<QVariant> resBuffer;
    qDebug() << "decoding...";
    while(pos < bEncodedData.length()){
        resBuffer.append(parse());
    }
    qDebug() << "decoding finished";
    return resBuffer;

}

QVariant bencoding::parse()
{
    QChar c = currentChar();
    if(c == 'i'){
        return parseInt();
    } else if(c == 'l'){
        return parseList();
    } else if(c == 'd'){
        return parseDict();
    } else if(c.isNumber()){
        return parseString();
    }

    pos++;
    QVariant x;
    return x;
}

QByteArray bencoding::infoSection()
{
    return bEncodedData.mid(infoStart, infoLen);
}





QByteArray bencoding::readFile(const QString &path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Failed to open file");
        return {};
    }

    QByteArray data = file.readAll();
    file.close();
    return data;
}



int bencoding::parseInt()
{
    if(currentChar() != 'i')
        return 0;
    //skip i
    pos++;
    //get length of int till 'e'
    int length = bEncodedData.indexOf('e', pos) - pos;
    //if is not valid (0 or less) ret 0
    if(!(length > 0))
        return 0;
    //get value from pos + length
    int value = bEncodedData.sliced(pos, length).toInt();
    //move pos past 'e'
    pos += length + 1;

    return value;
}

QByteArray bencoding::parseString()
{
    int strLength = bEncodedData.indexOf(':', pos) - pos;
    if(!(strLength > 0))
        return QByteArray();
    int length = bEncodedData.sliced(pos, strLength).toInt();
    pos += strLength+1;

    if(!(length > 0))
        return QByteArray();

    QByteArray value = bEncodedData.sliced(pos, length);
    pos += length;
    return value;
}

QList<QVariant> bencoding::parseList()
{
    QList<QVariant> listBuffer;
    if(currentChar() != 'l')
        return listBuffer;

    pos++;

    while(currentChar() != 'e'){

        listBuffer.append(parse());
    }
    pos++;
    return listBuffer;
}

QHash<QString, QVariant> bencoding::parseDict()
{
    QHash<QString, QVariant> hashBuffer;
    if(currentChar() != 'd')
        return hashBuffer;

    QByteArray key;
    QVariant value;
    pos++;
    while(currentChar() != 'e'){
        key = parseString();
        if(key == "info"){
            infoStart = pos;
        }
        value = parse();
        if(key.isEmpty() || value.isNull()){
            return hashBuffer;
        }
        hashBuffer.insert(key, value);
        if(key == "info"){
            infoLen = pos - infoStart;
        }
        //qDebug() << key;
        key.clear();
        value.clear();
    }
    pos++;
    return hashBuffer;
}

QChar bencoding::currentChar()
{
    if(pos >= bEncodedData.length())
        return {};
    return bEncodedData.at(pos);
}
