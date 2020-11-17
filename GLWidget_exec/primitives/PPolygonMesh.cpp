#include "PPolygonMesh.h"
#include "PPolygon.h"
#include "shaders.h"
#include "PaintInformation.h"
#include "lights/Light.h"
namespace vrt{
	
	PPolygonMesh::~PPolygonMesh()
	{
		if(vbo)vbo->destroy();
		if(vao)vao->destroy();
		if(linevbo)linevbo->destroy();
		if(linevao)linevao->destroy();
	}

	void PPolygonMesh::initialize()
	{
		this->initializeOpenGLFunctions();

		std::vector<std::vector<std::vector<Point3f>>> plgPts;
		for (const auto& plg : plgs_) {
			plgPts.emplace_back();
			for (const auto& lp : plg.lps_) {
				plgPts.back().emplace_back();
				for (const auto& ptIdx : lp) {
					plgPts.back().back().push_back(pts_[ptIdx]);
				}
			}
		}

		//将多边形离散为面片
		if ((tessPolygons(plgPts, &tessPts_, &drawInfo_))) return;
		if (tessPts_.empty()) 
			return;

		vao = std::make_shared<QOpenGLVertexArrayObject>();
		vbo = std::make_shared<QOpenGLBuffer>();

		vao->create();
		vao->bind();
		vbo->create();
		vbo->bind();
		vbo->allocate(&tessPts_[0], tessPts_.size() * sizeof(Float));

		int attr = -1;
		attr = CommonShader::ptr()->attributeLocation("aPos");
		CommonShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		CommonShader::ptr()->enableAttributeArray(attr);

		attr = LightPerFragShader::ptr()->attributeLocation("aPos");
		LightPerFragShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		LightPerFragShader::ptr()->enableAttributeArray(attr);

		//生成边界点
		for (const auto& plg : plgs_) {
			for (const auto& lp : plg.lps_) {
				boundDrawInfo_.push_back(DrawSingleObjInfo(GL_LINE_LOOP,boundPts_.size() / 3,lp.size()));
				for (const auto& ptIdx : lp) {
					boundPts_.push_back(pts_[ptIdx].x());
					boundPts_.push_back(pts_[ptIdx].y());
					boundPts_.push_back(pts_[ptIdx].z());
				}
			}
		}

		linevao = std::make_shared<QOpenGLVertexArrayObject>();
		linevbo = std::make_shared<QOpenGLBuffer>();

		linevao->create();
		linevao->bind();
		linevbo->create();
		linevbo->bind();
		linevbo->allocate(&boundPts_[0], boundPts_.size() * sizeof(Float));

		attr = -1;
		attr = CommonShader::ptr()->attributeLocation("aPos");
		CommonShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
		CommonShader::ptr()->enableAttributeArray(attr);

		readyToDraw = true;
	}

	void PPolygonMesh::paint(PaintInfomation* info)
	{
		doBeforePaint();
		if (!readyToDraw) return;
		if (info->fillmode == FILL || info->fillmode == FILL_WIREFRAME) {
			QOpenGLShaderProgram* shader;
			if (info->lights.size() && !selected())
			{
				shader = LightPerFragShader::ptr();
				shader->bind();
				shader->setUniformValue("modelMat", QMatrix4x4());
				shader->setUniformValue("viewMat", info->viewMat);
				shader->setUniformValue("projMat", info->projMat);
				shader->setUniformValue("ourColor", color().x(), color().y(), color().z(), 1.0f);
				shader->setUniformValue("lightCount", GLint(info->lights.size()));
				//auto viewNormal = QVector3D((info->viewMat).inverted().transposed()*QVector4D(QVector3D(normal_), 0));
				//if (viewNormal.z() < 0) viewNormal = -viewNormal;
				for (int j = 0; j < info->lights.size(); j++) {
					std::string lightname = ("lights[" + std::to_string(j) + "]").c_str();
					shader->setUniformValue((lightname + ".ambient").c_str(), QVector3D(info->lights[j]->getAmbient()));
					shader->setUniformValue((lightname + ".pos").c_str(), QVector3D(info->lights[j]->getPosition()));
					shader->setUniformValue((lightname + ".diffuse").c_str(), QVector3D(info->lights[j]->getDiffuse()));
				}
			}
			else {
				shader = CommonShader::ptr();
				shader->bind();
				shader->setUniformValue("modelMat", QMatrix4x4());
				shader->setUniformValue("viewMat", info->viewMat);
				shader->setUniformValue("projMat", info->projMat);

				shader->setUniformValue("ourColor", color().x(), color().y(), color().z(), 1.0f);
			}

			vao->bind();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			for(int i =0;i<drawInfo_.size();i++)
				glDrawArrays(drawInfo_[i].type, drawInfo_[i].offset, drawInfo_[i].size);

			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);

			shader->release();
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME) {  //选中时隐藏
			LineShader::ptr()->bind();
			LineShader::ptr()->setUniformValue("modelMat", QMatrix4x4());
			LineShader::ptr()->setUniformValue("viewMat", info->viewMat);
			LineShader::ptr()->setUniformValue("projMat", info->projMat);
			LineShader::ptr()->setUniformValue("ourColor", .0, .0, .0, 1.0f);
			LineShader::ptr()->setUniformValue("u_viewportSize", info->width, info->height);
			LineShader::ptr()->setUniformValue("u_thickness", GLfloat(info->lineWidth));
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);

			linevao->bind();

			for (int i = 0; i < boundDrawInfo_.size(); i++)
				glDrawArrays(boundDrawInfo_[i].type, boundDrawInfo_[i].offset, boundDrawInfo_[i].size);
			
			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}
		doAfterPaint();
	}

}