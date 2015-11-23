import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4

ApplicationWindow {
    id: window
    visible: true

    width: 960
    height: 600

    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2

    Rectangle {
        width: parent.width
        height: parent.height
        radius: 20

        Button {
            anchors.centerIn: parent
            text: "Quit"
            onClicked: {
                Qt.quit()
            }
        }
    }
}
