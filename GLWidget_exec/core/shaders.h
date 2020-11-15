#pragma once
#include "vrt.h"
#include "types.h"
#include <fstream>
#include <sstream>
#include "QOpenGLShaderProgram"
#include "utilities.h"
#include "ui/ProjectMagager.h"
namespace vrt {
//---------------------------------
//
//单体着色器模板与宏定义
//
//---------------------------------
#define DEF_SHADER_SOURCE(Name)\
	class Name {\
	public:\
		static std::string text(){\
			std::ifstream fin(PROPTR->swPath().toStdString()+"/shaders/"+#Name);\
			std::stringstream buffer;\
			buffer << fin.rdbuf();\
			return buffer.str();\
		}\
	};

	//单体着色器模板
	template <typename ...Shaders>
	class Shader
	{
	public:
		static QOpenGLShaderProgram* ptr();
	private:
		static QOpenGLShaderProgram* shader;
	};

	template <typename ...Shaders>
	QOpenGLShaderProgram* vrt::Shader<Shaders...>::shader = nullptr;

	template <typename ...Shaders>
	QOpenGLShaderProgram* vrt::Shader<Shaders...>::ptr()
	{
		if (!shader) {
			shader = new QOpenGLShaderProgram;
			if (!compileVrtShader(*shader, (Shaders::text().c_str())...)) {
				shader = nullptr;
				delete shader;
			}
		}
		return shader;
	}

//---------------------------------
//
//定义着色器代码
//
//---------------------------------
	DEF_SHADER_SOURCE(VertexShader_Mesh);

	DEF_SHADER_SOURCE(FragShader_Mesh);

	DEF_SHADER_SOURCE(VertexShader_LightedMesh_SingleNormal);

	DEF_SHADER_SOURCE(FragShader_LightedMesh);

	DEF_SHADER_SOURCE(GeoShader_LightedMesh_CalNormal);

	DEF_SHADER_SOURCE(GeoShader_Line);

	DEF_SHADER_SOURCE(GeoShader_Point);

	DEF_SHADER_SOURCE(VertexShader_OutputPos);

	DEF_SHADER_SOURCE(FragShader_LightPerFrag);
#undef DEF_SHADER_SOURCE
//---------------------------------
//
//定义着色器程序
//
//---------------------------------

	using CommonShader = Shader<VertexShader_Mesh, FragShader_Mesh>;
	using LightShader = Shader<VertexShader_LightedMesh_SingleNormal, FragShader_LightedMesh>;
	using LineShader = Shader<VertexShader_Mesh, GeoShader_Line, FragShader_Mesh>;
	using PointShader = Shader<VertexShader_Mesh, GeoShader_Point, FragShader_Mesh>;
	using LightPerFragShader = Shader<VertexShader_OutputPos, GeoShader_LightedMesh_CalNormal, FragShader_LightPerFrag>;
}

