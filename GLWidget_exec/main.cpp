#include <QtWidgets/QApplication>
#include <algorithm>
#include <math.h>
#include "glrender/Scene.h"
#include "primitives/PPolygon.h"
#include "core/CadCore.h"
#include "ui/MainWindow.h"
#include "ui/Scripts.h"
#include "ui/ProjectMagager.h"
using namespace vrt;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	PROPTR->setSoftwarePath(a.applicationDirPath());
	MAIPTR->show();

	return a.exec();
}