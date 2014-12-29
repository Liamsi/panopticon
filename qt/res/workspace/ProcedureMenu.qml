import QtQuick 2.2;

PathView {
	id: root

	property var procedure: null


	width: 40; height: 40
	model: ["", "rename", "delete", "decompile"]

	delegate: Rectangle {
		width: 20; height: 20
		id: icon
		color: "black"
		opacity: (modelData == "" ? 0 : 1)

		Text {
			anchors.centerIn: icon
			text: modelData
		}
	}

	path: Path {
		startX: root.width; startY: root.height

		PathArc {
			x: 0; y: 0
			radiusX: root.width; radiusY: root.height
		}
	}
}
			/*
Item {
	id: root

	property var procedure: null

	width: 40; height: 40

	Canvas {
		id: icon
		anchors.fill: parent

		onPaint: {
			var ctx = icon.getContext("2d")

			ctx.strokeStyle = "black"
			ctx.lineWidth = 1

			ctx.beginPath()
			ctx.moveTo(0,0);
			ctx.arc(icon.width, 0, icon.width, Math.PI, Math.PI * 1.5, true);
			ctx.stroke()
		}
	}

	MouseArea {
		id: mouseArea
		anchors.fill: icon

		onPressed: {
			console.log("Hello: " + procedure.name)
		}
	}

	Item {
		id: options
		anchors.fill: parent

		Repeater {
			id: itemRep
			anchors.fill: parent

			model: ["rename", "delete", "decompile"]
			delegate: Item {
				height: 20; width: 20

				y: Math.cos(Math.PI + (Math.PI / 2) * (index / (itemRep.model.length - 1))) * -40
				x: Math.sin(Math.PI + (Math.PI / 2) * (index / (itemRep.model.length - 1))) * 40 + 20

				Component.onCompleted: {
					console.log(index / (itemRep.model.length - 1));
				}


				Canvas {
					id: item
					anchors.fill: parent

					onPaint: {
						var ctx = item.getContext('2d')

						ctx.beginPath()
						ctx.arc(10, 10, 10, 0, Math.PI * 2, true);
						ctx.fill()
					}
				}
			}
		}
	}
}*/
