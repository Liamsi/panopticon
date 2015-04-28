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
		//calls: session != null ? session.calls : []
calls: [
			[1111111111,2222222222],
			[3333333333,2222222222],
			[3333333333,3333333333],
			[4444444444,5555555555],
			[5555555555,6666666666],
			[6666666666,4444444444],
			[4444444444,6666666666],
			[7777777777,1111111111],
			[8888888888,9999999999],
			[8888888888,3333333333]
		]
	}
}
