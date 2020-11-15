#include "PPolygonMesh.h"
#include "PPolygon.h"
#include "shaders.h"
#include "PaintInformation.h"
#include "lights/Light.h"
namespace vrt{
	
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
		if ((tessPolygons(plgPts, &tessPts_, &drawTypes_))) return;
		if (tessPts_.empty()) return;

		for (int i = 0; i < tessPts_.size();) {
			if (tessPts_[i].empty()) {
				tessPts_.erase(tessPts_.begin() + i);
				drawTypes_.erase(drawTypes_.begin() + i);
			}
			else i++;
		}

		for (int i = 0; i < tessPts_.size(); i++) {
			vaos.emplace_back(new QOpenGLVertexArrayObject);
			auto& vao = vaos.back();

			vbos.emplace_back(new QOpenGLBuffer);
			auto& vbo = vbos.back();

			vao->create();
			vao->bind();
			vbo->create();
			vbo->bind();
			vbo->allocate(&tessPts_[i][0], tessPts_[i].size() * sizeof(Float));

			int attr = -1;
			attr = CommonShader::ptr()->attributeLocation("aPos");
			CommonShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
			CommonShader::ptr()->enableAttributeArray(attr);

			attr = LightPerFragShader::ptr()->attributeLocation("aPos");
			LightPerFragShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
			LightPerFragShader::ptr()->enableAttributeArray(attr);
		}

		//生成边界点
		for (const auto& plg : plgs_) {
			for (const auto& lp : plg.lps_) {
				boundPts_.emplace_back();
				for (const auto& ptIdx : lp) {
					boundPts_.back().push_back(pts_[ptIdx].x());
					boundPts_.back().push_back(pts_[ptIdx].y());
					boundPts_.back().push_back(pts_[ptIdx].z());
				}
			}
		}

		for (int i = 0; i < boundPts_.size(); i++) {
			linevaos.emplace_back(new QOpenGLVertexArrayObject);
			linevbos.emplace_back(new QOpenGLBuffer);
			auto& vao = linevaos.back();
			auto& vbo = linevbos.back();

			vao->create();
			vao->bind();
			vbo->create();
			vbo->bind();
			vbo->allocate(&boundPts_[i][0], boundPts_[i].size() * sizeof(Float));

			int attr = -1;
			attr = CommonShader::ptr()->attributeLocation("aPos");
			CommonShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
			CommonShader::ptr()->enableAttributeArray(attr);
		}
	}

	void PPolygonMesh::paint(PaintInfomation* info)
	{
		doBeforePaint();
		if (vaos.empty()) return;
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
			for (int i = 0; i < vaos.size(); i++) {
				vaos[i]->bind();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glDrawArrays(drawTypes_[i], 0, tessPts_[i].size() / 3);

				glDisable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(0, 0);
			}
			shader->release();
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME || !selected()) {  //选中时隐藏
			LightPerFragShader::ptr()->bind();
			LineShader::ptr()->setUniformValue("modelMat", QMatrix4x4());
			LineShader::ptr()->setUniformValue("viewMat", info->viewMat);
			LineShader::ptr()->setUniformValue("projMat", info->projMat);
			LineShader::ptr()->setUniformValue("ourColor", .0, .0, .0, 1.0f);
			LineShader::ptr()->setUniformValue("u_viewportSize", info->width, info->height);
			LineShader::ptr()->setUniformValue("u_thickness", GLfloat(info->lineWidth));
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);
			for (int i = 0; i < linevaos.size(); i++) {
				linevaos[i]->bind();
				glDrawArrays(GL_LINE_LOOP, 0, boundPts_[i].size() / 3);
			}
			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}
		doAfterPaint();
	}

}