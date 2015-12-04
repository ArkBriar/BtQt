#pragma once

#ifndef __BTCORE_H__
#define __BTCORE_H__

#include <BtDefs.h>
#include <BtPeer.h>
#include <BtTracker.h>
#include <BtTorrent.h>
#include <BtDebug.h>

#include <QList>
#include <QSharedPointer>

NAMESPACE_BEGIN(BtQt)

class BtCore {
public:
    BtCore(BtTorrent const &torrent, int listenPort);
    ~BtCore();
    /* Methods */
    void start();
    void pause();
    void stop();

private:
    BtTrackerResponse contactWithTracker(QUrl trackerUrl, int numwant = 50, BtTrackerDownloadEvent e = BtTrackerDownloadEvent::empty) const;

    void startDownload();
    const BtTorrent& torrent;
    QSharedPointer<BtLocalPeer> localPeer;
    QList<BtRemotePeer> remotePeerList;
    QList<BtTrackerResponse> trackerState;

    quint64 uploaded;
    quint64 downloaded;
};

NAMESPACE_END(BtQt)

#endif // __BTCORE_H__
