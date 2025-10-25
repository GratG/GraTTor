#include "tracker.h"

#include <QDebug>
#include <QUrl>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

#include "bencoding.h"

Tracker::Tracker(QObject *parent)
{

    connect(&httpManager, &QNetworkAccessManager::finished, this, &Tracker::onReplyFinished);
}

void Tracker::addTorrent(Torrent *t)
{
    torrent = t;
}

void Tracker::start(){

    fetchPeerList();
}

QList<QPair<QString, quint16> > Tracker::getPeerList()
{
    return peerList;
}
void Tracker::fetchPeerList()
{
    //TODO UDP request
    qDebug() << "Http get...";

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

    httpManager.get(httpRequest);



}

void Tracker::onReplyFinished(QNetworkReply *reply){
    //when reply::finished, obtain the response and decode it
    qDebug() << "tracker reply recieved";

    //QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    if(!reply) return;

    QByteArray response = reply->readAll();
    bencoding decoder;
    decoder.loadString(response);


    QList<QVariant> decodedResponse = decoder.decode();

    QHash dict = decodedResponse[0].toHash();

    //obtain the peer ip and port from dict as a QPair
    if(dict.contains("peers")){
        QVariant peerTemp = dict.value("peers");
        //peer can either be a QList or a QByteArray
        //TODO peer list
        if(peerTemp.userType() == QMetaType::QVariantList){
            QList<QVariant> peers = peerTemp.toList();
            for(int i=0; i < peers.size(); i++){

            }
        } else{
            QByteArray peers = peerTemp.toByteArray();
            for (int i=0; i+6 <= peers.size(); i += 6){
                uint8_t set1 = peers[i];
                uint8_t set2 = peers[i+1];
                uint8_t set3 = peers[i+2];
                uint8_t set4 = peers[i+3];
                QString ip = QString("%1.%2.%3.%4").arg(set1).arg(set2).arg(set3).arg(set4);

                quint16 port = (static_cast<quint8>(peers[i+4]) << 8) |
                    static_cast<quint8>(peers[i+5]);

                peerList.append({ip, port});
            }
        }

    }

    //TODO intervals?
    if(dict.contains("interval")){

    }

    emit peersFound(peerList);
    emit replyFinished();

}
