#pragma once
#include "vrt.h"
#include "types.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include "Primitive.h"

namespace vrt {
	class PPolygonMesh : public GeometryPrimitive
	{
	public:
		struct Polygon {
			std::vector<std::vector<int>> lps_;
		};
	public:
		PPolygonMesh(const std::vector<Polygon>& plgs,const std::vector<Point3f>& pts) //multi loop
			:plgs_(plgs),pts_(pts){
			setColor({ 0.6,0.6,0.6 });
		};

		virtual void initialize() override;
		virtual void paint(PaintInfomation* info) override;
	private:
		std::vector<Polygon> plgs_;
		std::vector<Point3f> pts_;
		std::vector<std::vector<Float>> boundPts_;//用于绘制边界
		std::vector<std::vector<Float>> tessPts_;
		std::vector<GLenum> drawTypes_;

		std::vector<std::shared_ptr<QOpenGLBuffer>> vbos;
		std::vector<std::shared_ptr<QOpenGLVertexArrayObject>> vaos;

		std::vector<std::shared_ptr<QOpenGLBuffer>> linevbos;
		std::vector<std::shared_ptr<QOpenGLVertexArrayObject>> linevaos;
	};
}

