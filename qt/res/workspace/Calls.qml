import QtQuick 2.0
import QtQuick.Controls 1.0
import Panopticon 1.0

Item {
	id: root
	property var session: null

	Radial {
		anchors.fill: parent
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		anchors.topMargin: 10
		anchors.bottomMargin: 10
		calls: { session == null ? [] : session.calls }
	}
}
