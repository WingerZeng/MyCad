#include "Primitive.h"

namespace vrt {
unsigned int Primitive::globalId = 0;

void GeometryPrimitive::setSelected(bool selected)
{
	if (selected == this->selected()) return;
	if (selected) {
		tempColor_ = color();
		setColor({ 1,0,0 }); //Ñ¡ÖÐÉ«
	}
	else {
		setColor(tempColor_);
	}
	Primitive::setSelected(selected);
}

void GeometryPrimitive::doBeforePaint()
{
	if (selected()) {
		this->glEnable(GL_POLYGON_OFFSET_FILL);
		this->glPolygonOffset(-1, -1);
	}
}

void GeometryPrimitive::doAfterPaint()
{
	if (selected()) {
		this->glDisable(GL_POLYGON_OFFSET_FILL);
		this->glPolygonOffset(0, 0);
	}
}
}
