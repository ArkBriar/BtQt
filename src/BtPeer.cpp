#include <BtQt.h>

using namespace BtQt;

QByteArray BtQt::generatePeerId()
{
    QByteArray peer_id = "-";

    if(AzureusClientId.size() != 2) peer_id.append(AzureusClientId.mid(0, 2));
    else peer_id.append(AzureusClientId);
    if(versionInString.size() != 4) peer_id.append(versionInString.mid(0, 4));
    else peer_id.append(versionInString);
    peer_id.append('-');

    for(auto i = 0; i < 12; ++ i) {
        peer_id.append(QByteArray::number(qrand() % 10));
    }

    Q_ASSERT(peer_id.size() == 20);

    return peer_id;
}
