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
		std::vector<PType3f> lp_; // #PERF1 �����Ա���Ըĳ�ָ�룬�����ͷ�
		std::vector<Float> pts_;
		bool isloop;

		QOpenGLShaderProgram lineShaderProgram; // #PERF1 ��ɫ������prototypeģʽ�Ż�
		QOpenGLBuffer vbo;
		QOpenGLVertexArrayObject vao;
	};
}

