#pragma once

#include "core/Primitive.h"
#include "glut.h"
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
#include "core/vrt.h"
#include "core/types.h"
namespace vrt {
void CALLBACK vertexCallback(GLvoid * vertex);
void CALLBACK beginCallback(GLenum type);
void CALLBACK endCallback();
void CALLBACK errorCallback(GLenum errorCode);
class PaintInfomation;
class bpSolid;

extern const char *vertexShaderSource_Mesh;
extern const char *geoShaderSource_Line;
extern const char *fragmentShaderSource_Mesh;

class PPolygon : public GeometryPrimitive
{
public:
	PPolygon(std::vector<PType3f> pts) //one loop
		:lps_{ pts } {
		setColor({0.6,0.6,0.6});
	};
	PPolygon(std::vector<std::vector<PType3f>> lps) //multi loop
		:lps_(lps) {
		setColor({ 0.6,0.6,0.6 });
	};
	~PPolygon();
	virtual void initialize() override;
	virtual void paint(PaintInfomation* info) override;
	bool checkNormal(Vec3f& normal); //Judge the normal of polygon
	static PPolygon* currentTessPolygon;

	friend void CALLBACK vertexCallback(GLvoid * vertex);
	friend void CALLBACK beginCallback(GLenum type);
	friend void CALLBACK endCallback();
	friend void CALLBACK errorCallback(GLenum errorCode);
private:
	std::vector<std::vector<PType3f>> lps_; // #PERF1 这个成员可以改成指针，用完释放
	std::vector<std::vector<Float>> boundPts_;
	std::vector<std::vector<Float>> tessPts_;
	std::vector<GLenum> drawTypes_;

	QOpenGLShaderProgram shaderProgram;
	QOpenGLShaderProgram lightShaderProgram;
	QOpenGLShaderProgram lineShaderProgram; // #PERF1 着色器采用prototype模式优化
	std::vector<std::shared_ptr<QOpenGLBuffer>> vbos;
	std::vector<std::shared_ptr<QOpenGLVertexArrayObject>> vaos;

	std::vector<std::shared_ptr<QOpenGLBuffer>> linevbos;
	std::vector<std::shared_ptr<QOpenGLVertexArrayObject>> linevaos;

	Vector3f normal_;
};

/*Render Interface of solid*/
std::vector<std::shared_ptr<PPolygon>> solidToPolygons(bpSolid* solid);
}
