#include <QApplication>
#include "GUI.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	GUI top;

	top.show();
    return a.exec();
}
