#include "tracker.h"

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "bencoding.h"

Tracker::Tracker(Torrent *t, QObject *parent)
{
    torrent = t;
}

void Tracker::request()
{
    //TODO UDP request
    qDebug() << "Http get...";
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    qDebug() << torrent->getAnnounce();
    QUrl url(torrent->getAnnounce());

    QUrlQuery query;
    query.addQueryItem("info_hash", QString::fromUtf8(torrent->getSHA1().toPercentEncoding()));
    query.addQueryItem("peer_id", "12345612345612345678");
    query.addQueryItem("port", QString::number(6881));
    //TODO get proper values for uploaded, downloaded and left
    query.addQueryItem("uploaded", QString::number(0));
    query.addQueryItem("downloaded", QString::number(0));
    query.addQueryItem("left", QString::number(torrent->getLength()));
    query.addQueryItem("compact", "1");
    //TODO query events
    url.setQuery(query);

    QNetworkRequest httpRequest(url);
    QNetworkReply *reply = manager->get(httpRequest);

    QList<QVariant> responseData;

    connect(reply, &QNetworkReply::finished, this, &Tracker::onReplyFinished);



    //qDebug() << responseData[0];

}

void Tracker::onReplyFinished(){
    qDebug() << "tracker reply recieved";

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if(!reply) return;

    QByteArray response = reply->readAll();
    bencoding decoder;
    decoder.loadString(response);

    qDebug() << response;
    //QList<QVariant> decodedResponse = decoder.decode();

    //qDebug() << decodedResponse[0];
}
