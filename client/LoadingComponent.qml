import QtQuick 2.2
import QtQuick.Controls 2.2

Rectangle {
    width: parent.width
    height: parent.height
    z: 9999
    color: "black"
    opacity: 0.5
    Image {
        width: 100
        height: 100
        id: loadingImage
        source: "./loading.svg"  // Replace with the path to your loading image
        anchors.centerIn: parent
        transformOrigin: Item.Center

        RotationAnimation {
            id: rotationAnimation
            target: loadingImage
            property: "rotation"
            duration: 3000  // Duration of one complete rotation in milliseconds
            from: 0
            to: 360
            loops: Animation.Infinite  // Infinite loop for continuous rotation
            running: true
        }
    }
}
