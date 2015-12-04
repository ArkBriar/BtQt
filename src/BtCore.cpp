#include <BtCore.h>
#include <QFile>

#include <QTcpSocket>
#include <QUdpSocket>
using namespace BtQt;

BtCore::BtCore(BtTorrent const &torrent, int listenPort)
    : torrent(torrent), uploaded(0), downloaded(0)
{
    localPeer = QSharedPointer<BtLocalPeer>::create(torrent, generatePeerId()
            , QHostAddress("0.0.0.0"), listenPort);
}

BtCore::~BtCore()
{
    localPeer.clear();
}

void BtCore::start()
{
    auto r = contactWithTracker(torrent.announce());
    if(!r.isEmpty()) trackerState.append(r);
    else return;

    startDownload();
}

void BtCore::pause()
{

}

void BtCore::stop()
{

}

BtTrackerResponse BtCore::contactWithTracker(QUrl trackerUrl,
        int numwant, BtTrackerDownloadEvent e) const
{
    BtTrackerRequest rq(
            torrent.infoHash(),
            localPeer->getPeerId(),
            localPeer->getPort(),
            uploaded,
            downloaded,
            torrent.length() - downloaded
            );
    rq.setNumwant(numwant);
    rq.setEvent(e);
    try {
        BtTrackerResponse r(
                parseTrackerResponse(sendTrackerRequest(
                        rq,
                        trackerUrl
                        )
                    )
                );
        return r;
    } catch (int e) {
        qDebug() << "Can not communicate with tracker: " << torrent.announce();
        return BtTrackerResponse(QMap<QString, QVariant>());
    }
}

void BtCore::startDownload()
{

}
