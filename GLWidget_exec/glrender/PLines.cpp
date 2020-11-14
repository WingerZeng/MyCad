#include "PLines.h"
#include "PPolygon.h"
#include "utilities.h"
#include "PaintInformation.h"
namespace vrt{
	
	PLines::~PLines()
	{
		vbo.destroy();
		vao.destroy();
	}

	void PLines::initialize()
	{
		this->initializeOpenGLFunctions();

		for (const auto& pts : lp_) {
			pts_.push_back(pts.x());
			pts_.push_back(pts.y());
			pts_.push_back(pts.z());
		}
		if (!compileVrtShader(lineShaderProgram, vertexShaderSource_Mesh, geoShaderSource_Line, fragmentShaderSource_Mesh)) return;

		vao.create();
		vao.bind();
		vbo.create();
		vbo.bind();
		vbo.allocate(&pts_[0], pts_.size() * sizeof(Float));

		int attr = -1;
		attr = lineShaderProgram.attributeLocation("aPos");
		lineShaderProgram.setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		lineShaderProgram.enableAttributeArray(attr);
	}

	void PLines::paint(PaintInfomation* info)
	{
		doBeforePaint();

		// #PERF2 改了vao，着色器可以不用重分配么？
		vao.bind();
		lineShaderProgram.bind();
		lineShaderProgram.setUniformValue("modelMat", QMatrix4x4());
		lineShaderProgram.setUniformValue("viewMat", info->viewMat);
		lineShaderProgram.setUniformValue("projMat", info->projMat);
		lineShaderProgram.setUniformValue("ourColor",QVector4D(color().x(),color().y(),color().z(), 1.0f));
		lineShaderProgram.setUniformValue("u_viewportSize", info->width, info->height);
		lineShaderProgram.setUniformValue("u_thickness", GLfloat(info->lineWidth));
		if(isloop)
			glDrawArrays(GL_LINE_LOOP, 0, pts_.size() / 3);
		else
			glDrawArrays(GL_LINE_STRIP, 0, pts_.size() / 3);

		lineShaderProgram.release();
		doAfterPaint();
	}

	void PLines::doBeforePaint()
	{
		this->glPolygonOffset(-2, -2);
		glEnable(GL_POLYGON_OFFSET_FILL);
	}

}