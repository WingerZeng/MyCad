#pragma once
#include "vrt.h"
#include "types.h"
#include "Primitive.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
namespace vrt {
	class PTriMesh:public GeometryPrimitive
	{
	public:
		PTriMesh(const std::vector<unsigned int>& tris, const std::vector<Point3f>& pts);

		virtual void initialize() override;
		virtual void paint(PaintInfomation* info) override;

		const std::vector<unsigned int>& getIndices() const { return tris_; }
		const std::vector<Float>& getPts() const { return pts_; }

	private:
		bool readyToDraw = false;
		
		std::vector<unsigned int> tris_;
		std::vector<Float> pts_;

		std::shared_ptr<QOpenGLBuffer> ebo;
		std::shared_ptr<QOpenGLBuffer> vbo;
		std::shared_ptr<QOpenGLVertexArrayObject> vao;
	};
}

