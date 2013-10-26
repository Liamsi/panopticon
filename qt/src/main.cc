#include <QtCore>
#include <QApplication>
#include <QQmlApplicationEngine>

#include <graphscene.hh>
#include <linearscene.hh>
#include <window.hh>

using namespace std;

int main(int argc, char *argv[])
{
	qmlRegisterType<GraphScenePath>("Panopticon",1,0,"Path");
	qmlRegisterType<GraphSceneItem>("Panopticon",1,0,"Graph");
	qmlRegisterType<LinearSceneModel>("Panopticon",1,0,"LinearSceneModel");
	qmlRegisterType<Element>("Panopticon",1,0,"Element");
	qmlRegisterType<Header>("Panopticon",1,0,"Block");
	qmlRegisterType<GraphScenePen>("Panopticon",1,0,"Pen");

	QApplication app(argc, argv);
	Window win;

	win.show();
	/*QScrollArea scroll;
	QDeclarativeView view(QUrl("qrc:/Procedure.qml"));

	scroll.setWidget(&view);
	scroll.setWidgetResizable(true);
	scroll.show();*/
	return app.exec();
}
