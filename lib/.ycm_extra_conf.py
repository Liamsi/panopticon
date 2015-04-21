import os;

def FlagsForFile(a):
	dname = os.path.dirname(os.path.abspath(__file__ ))
	return {'flags': ['-Wall','-std=c++11','-x','c++','-I/usr/include/libarchive','-I/usr/include/qt5/QtQml','-I/usr/include/qt5/QtQuick','-I/usr/include/qt5/QtGui','-I/usr/include/qt5/QtWidgets','-I/usr/include/qt5/QtCore','-I',dname + "/../lib/include",'-I',dname + "/include",'-I',dname + "/../3rdparty/"], 'do_cache': True}
