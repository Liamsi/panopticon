import QtQuick 2.0
import Panopticon 1.0

/*
 * +-------+---------------------+-------+
 * |   S   |                     |   S   |
 * |   i   |                     |   i   |
 * |   d   |                     |   d   |
 * |   e   |      Workspace      |   e   |
 * |       |                     |       |
 * |   B   |                     |   B   |
 * |   a   |                     |   a   |
 * |   r   |                     |   r   |
 * +-------+---------------------+-------+
 */
Item {
	id: root

	property variant session: null

	state: "data"
	focus: true

	Keys.onPressed: {
		if(event.key == Qt.Key_Space) {
			if(state == "code") {
				state = "data"
			} else {
				state = "code"
			}
			event.accepted = true
		}
	}

	Component.onCompleted: {
		root.session.activeProcedure = root.session.procedures[0]
	}

	Item {
		id: workspace
		anchors.left: mainCode.right
		anchors.right: mainData.left
		anchors.top: parent.top
		anchors.bottom: parent.bottom
		clip: true

		Rectangle {
			anchors.fill: parent
			color: "#eeeeee"
		}

		Calls {
			anchors.fill: parent
			id: grph
			session: root.session
		}

		/*
		Graph {
			anchors.fill: parent
			id: grph
			session: root.session
			visible: root.state == "code"
		}

		Linear {
			anchors.fill: parent
			id: lst1
			session: root.session
			visible: root.state == "data"
			arrowBodyColor: "#111111"
			arrowHeadColor: "#aa1c1c"
			selectionColor: "#bed83f"
		}*/
	}

	SideMenu {
		id: mainCode

		height: parent.height
		width: 200
		x: root.state == "code" ? 0 : -1 * width

		model: root.session.procedures
		primaryColor: "#1c1c1c"
		secondaryColor: "#1bbc9b"
		alignLeft: true
		activeItem: root.session.activeProcedure
		icon: "func.png"

		onSelected: { root.session.activeProcedure = i }
	}

	SideMenu {
		id: mainData

		height: parent.height
		width: 200
		x: root.state == "data" ? parent.width - width : parent.width

		alignLeft: false
		model: root.session.procedures
		primaryColor: "#1c1c1c"
		secondaryColor: "#bed83f"
		activeItem: root.session.activeProcedure
		icon: "data.png"

		onSelected: { root.session.activeProcedure = i }
	}
}
