#include "PPoint.h"
#include "PPolygon.h"
#include "utilities.h"
#include "PaintInformation.h"

namespace vrt{
	const char *geoShaderSource_Point =
		"#version 330																							   \n"
		"																										   \n"
		"layout(points) in;                              // now we can access 2 vertices						   \n"
		"layout(triangle_strip, max_vertices = 4) out;  // always (for now) producing 2 triangles (so 4 vertices)  \n"
		"																										   \n"
		"uniform vec2  u_viewportSize;																			   \n"
		"uniform float pointSize = 10;																			   \n"
		"																										   \n"
		"void main()																							   \n"
		"{																										   \n"
		"	vec4 p1 = gl_in[0].gl_Position;																		   \n"
		"	float xoffset =pointSize/ u_viewportSize.x;															   \n"
		"	float yoffset =pointSize/ u_viewportSize.y; 																						\n"
		"																											\n"
		"	gl_Position = p1 + vec4(xoffset, yoffset, 0.0, 0.0);												   \n"
		"	EmitVertex();																						   \n"
		"	gl_Position = p1 + vec4(-xoffset,yoffset, 0.0, 0.0);												   \n"
		"	EmitVertex();																						   \n"
		"	gl_Position = p1 + vec4(xoffset,-yoffset, 0.0, 0.0);												   \n"
		"	EmitVertex();																						   \n"
		"	gl_Position = p1 + vec4(-xoffset,-yoffset, 0.0, 0.0);												   \n"
		"	EmitVertex();																						   \n"
		"																										   \n"
		"	EndPrimitive();																						   \n"
		"}																										   \n";

	PPoint::~PPoint()
	{
		vao.destroy();
		vbo.destroy();
	}

	void PPoint::initialize()
	{
		this->initializeOpenGLFunctions();

		if (!compileVrtShader(lineShaderProgram, vertexShaderSource_Mesh, geoShaderSource_Point, fragmentShaderSource_Mesh)) return;

		vao.create();
		vao.bind();
		vbo.create();
		vbo.bind();
		vbo.allocate(cd, 3 * sizeof(Float));

		int attr = -1;
		attr = lineShaderProgram.attributeLocation("aPos");
		lineShaderProgram.setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		lineShaderProgram.enableAttributeArray(attr);
	}

	void PPoint::paint(PaintInfomation* info)
	{
		doBeforePaint();
		glPolygonOffset(-1, -1);

		// #PERF2 改了vao，着色器可以不用重分配么？
		vao.bind();
		lineShaderProgram.bind();
		lineShaderProgram.setUniformValue("modelMat", QMatrix4x4());
		lineShaderProgram.setUniformValue("viewMat", info->viewMat);
		lineShaderProgram.setUniformValue("projMat", info->projMat);
		lineShaderProgram.setUniformValue("ourColor", QVector4D(color().x(), color().y(), color().z(), 1.0f));
		lineShaderProgram.setUniformValue("u_viewportSize", info->width, info->height);
		lineShaderProgram.setUniformValue("pointSize", GLfloat(info->pointSize));

		glDrawArrays(GL_POINTS, 0, 1);

		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);

		lineShaderProgram.release();
		doAfterPaint();
	}

}