#pragma once
#include "vrt.h"
#include "types.h"
#include "Primitive.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
namespace vrt {
	class PLines:public GeometryPrimitive
	{
	public:
		PLines(std::vector<PType3f> pts, bool isLoop = false)
			:lp_(pts), isloop(isLoop) {}
		~PLines();
		virtual void initialize() override;
		virtual void paint(PaintInfomation* info) override;
		void doBeforePaint() override;

	private:
		std::vector<PType3f> lp_; // #PERF1 这个成员可以改成指针，用完释放
		std::vector<Float> pts_;
		bool isloop;

		QOpenGLShaderProgram lineShaderProgram; // #PERF1 着色器采用prototype模式优化
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;
	};
}

