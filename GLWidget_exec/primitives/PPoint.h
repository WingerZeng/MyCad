#pragma once
#include "vrt.h"
#include "types.h"
#include "Primitive.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
namespace vrt {
	class PPoint:public GeometryPrimitive
	{
	public:
		PPoint(const Point3f& coord)
			:cd{coord.x(),coord.y(),coord.z()}{}
		~PPoint();
		virtual void initialize() override;
		virtual void paint(PaintInfomation* info) override;

	private:
		Point3f coord_; // #PERF1 �����Ա���Ըĳ�ָ�룬�����ͷ�
		Float cd[3];

		QOpenGLShaderProgram lineShaderProgram; // #PERF1 ��ɫ������prototypeģʽ�Ż�
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;
	};
}

