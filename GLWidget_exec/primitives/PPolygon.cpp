#include "PPolygon.h"
#include "glut.h"
#include "PaintInformation.h"
#include "utilities.h"
#include "lights/Light.h"
#include "CadCore.h"
#include "vrt.h"
#include "shaders.h"
#include <thread>
#include <mutex>
namespace vrt {			

	PPolygon::~PPolygon()
	{
	}

	void PPolygon::initialize()
	{
		this->initializeOpenGLFunctions();

		if (!checkNormal(this->normal_)) {
			//TODO
			return;
		}

		if((tessPolygon(lps_, &tessPts_, &drawTypes_))) return;
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

			attr = LineShader::ptr()->attributeLocation("aPos");
			LineShader::ptr()->setAttributeBuffer(attr, GL_FLOAT, 0, 3, 0);
			LineShader::ptr()->enableAttributeArray(attr);
		}


		for (int i = 0; i < lps_.size(); i++) {
			boundPts_.emplace_back();
			for (int j = 0; j < lps_[i].size(); j++) {
				boundPts_.back().push_back(lps_[i][j].x());
				boundPts_.back().push_back(lps_[i][j].y());
				boundPts_.back().push_back(lps_[i][j].z());
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

	void PPolygon::paint(PaintInfomation* info)
	{
		doBeforePaint();
		if (vaos.empty()) return;
		if (info->fillmode == FILL || info->fillmode == FILL_WIREFRAME) {
			QOpenGLShaderProgram* shader;
			auto viewNormal = QVector3D((info->viewMat).inverted().transposed()*QVector4D(QVector3D(normal_), 0));
			//�����濿ǰ
			if (viewNormal.z() < 0) {
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(4, 4);
			}
			if (info->lights.size() && !selected())
			{
				shader = LightShader::ptr();
				shader->bind();
				shader->setUniformValue("modelMat", QMatrix4x4());
				shader->setUniformValue("viewMat", info->viewMat);
				shader->setUniformValue("projMat", info->projMat);
				shader->setUniformValue("ourColor", color().x(), color().y(), color().z(), 1.0f);
				shader->setUniformValue("lightCount", GLint(info->lights.size()));
				//auto viewNormal = QVector3D((info->viewMat).inverted().transposed()*QVector4D(QVector3D(normal_), 0));
				//if (viewNormal.z() < 0) viewNormal = -viewNormal;
				shader->setUniformValue("normal", viewNormal);
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
			for (int i = 0; i < vaos.size();i++) {
				vaos[i]->bind();
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glDrawArrays(drawTypes_[i], 0, tessPts_[i].size() / 3);

				glDisable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(0, 0);
			}
			shader->release();
		}
		if (info->fillmode == WIREFRAME || info->fillmode == FILL_WIREFRAME || !selected()) {  //ѡ��ʱ����
			LineShader::ptr()->bind();
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
				glDrawArrays(GL_LINE_LOOP, 0, boundPts_[i].size()/3);
			}
			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(0, 0);
			LineShader::ptr()->release();
		}
		doAfterPaint();
	}

	bool PPolygon::checkNormal(Vec3f& normal)
	{
		if (lps_.empty()) return false;
		const auto&  pts = lps_[0];
		if (pts.size() <= 2) return false;

		//�ж϶�����Ƿ���yzƽ����
		int n = pts.size();
		double result = 0;
		for (int i = 1; i < pts.size(); i++) {
			int aaa = (i - 1) % n;
			result += abs(Normalize(pts[i%n] - pts[(i - 1 + n) % n]).dot(Vec3f(1, 0, 0)));
		}
		int axis;
		if (result > 0.5) axis = 0; //����yzƽ���ϣ�ѡ��x��
		else axis = 1; //����ѡ��y��

		int maxVal = -Infinity;
		int maxPt = -1;
		for (int i = 0; i < pts.size();i++) {
			if (pts[i][axis] > maxVal) {
				maxVal = pts[i][axis];
				maxPt = i;
			}
		}

		Vec3f trueNormal = Normalize((pts[maxPt] - pts[(maxPt - 1 + n) % n]).cross(pts[(maxPt + 1) % n] - pts[maxPt]));

		bool first = 1;
		result = 0;
		int cnt = 0;
		for (const auto& lp : lps_) {
			for (int i = 1; i < pts.size() - 1; i++) {
				Vec3f tempNormal = (pts[i] - pts[(i - 1 + n) % n]).cross(pts[(i + 1) % n] - pts[i]); //
				if (tempNormal.length() < 1e-14) continue;
				tempNormal.normalize();
				result += abs(tempNormal.dot(trueNormal));
				cnt++;
				if (first) {
					first = false;
				}
			}
		}

		if (first) return false;
		else if (abs(cnt - result)/cnt > 1e-6) {
			return false;
		}
		normal = trueNormal;

		return true;
	}

	//vrt::Bounds3f PPolygon::getBound()
	//{
	//	Bounds3f bd;
	//	for (const auto& lp : lps_)
	//	{
	//		for (const auto& pt : lp)
	//		{
	//			bd = Union(bd, Point3f(pt));
	//		}
	//	}
	//	return bd;
	//}

	std::mutex tessMtx;
	//����glu��ԭ�򣬴˴�ֻ����ȫ�ֱ����������ص������ṩ���涥��������Ϣ��λ��
	std::vector<std::vector<Float>>* glbTessPts;
	std::vector<GLenum>* glbDrawTypes;
	bool glbError;

	void CALLBACK vertexCallback(GLvoid* vertex)
	{
		double* cd = (double*)vertex;
		glbTessPts->back().push_back(cd[0]);
		glbTessPts->back().push_back(cd[1]);
		glbTessPts->back().push_back(cd[2]);
	}

	void CALLBACK beginCallback(GLenum type)
	{
		glbDrawTypes->push_back(type);
		glbTessPts->emplace_back();
	}

	void CALLBACK endCallback()
	{

	}

	void CALLBACK errorCallback(GLenum errorCode)
	{
		glbError = true;
		qDebug() << "error:" << errorCode;
	}

	int tessPolygon(const std::vector<std::vector<PType3f>>& lps, std::vector<std::vector<Float>>* tessPts, std::vector<GLenum>* drawTypes)
	{
		std::vector<std::vector<std::vector<PType3f>>> plgs{lps};
		return tessPolygons(plgs, tessPts, drawTypes);
	}

	int tessPolygons(const std::vector<std::vector<std::vector<PType3f>>>& plgs, std::vector<std::vector<Float>>* tessPts, std::vector<GLenum>* drawTypes)
	{
		GLUtesselator * tessobj;
		tessobj = gluNewTess();
		tessPts->clear();
		drawTypes->clear();
		//ע��ص�����  
		gluTessCallback(tessobj, GLU_TESS_VERTEX, (void (CALLBACK *)())vertexCallback);
		gluTessCallback(tessobj, GLU_TESS_BEGIN, (void (CALLBACK *)())beginCallback);
		gluTessCallback(tessobj, GLU_TESS_END, (void (CALLBACK *)())endCallback);
		gluTessCallback(tessobj, GLU_TESS_ERROR, (void (CALLBACK *)())errorCallback);

		gluTessProperty(tessobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
		//gluTessNormal(tessobj, normal_.x(), normal_.y(), normal_.z());

		tessMtx.lock();
		glbTessPts = tessPts;
		glbDrawTypes = drawTypes;
		glbError = false;

		int count = 0;
		for (const auto& lps : plgs) {
			for (const auto& lp : lps)
			{
				for (const PType3f& pt : lp)
				{
					++count; //ͳ�����ж�����
				}
			}
		}

		//����Ҫ��һ���ֲ������Ŀռ����洢����ֵ����������ʱ���飬Ҳ������һ����vector����push_back���棬��Ϊpushback�����е�ַ���
		std::vector<GLdouble> tempCd(count * 3);

		count = 0;
		for (const auto& plg : plgs) {
			gluTessBeginPolygon(tessobj, NULL);
			for (const auto& lp : plg)
			{
				gluTessBeginContour(tessobj);//���ö���εı��� 	
				for (const PType3f& pt : lp)
				{
					tempCd[count * 3 + 0] = pt.x();
					tempCd[count * 3 + 1] = pt.y();
					tempCd[count * 3 + 2] = pt.z();
					gluTessVertex(tessobj, &tempCd[count * 3], &tempCd[count * 3]);
					++count;
				}
				gluTessEndContour(tessobj);
			}
			gluTessEndPolygon(tessobj);
		}

		tessMtx.unlock();
		gluDeleteTess(tessobj);
		if (glbError) return -1;
		return 0;
	}

	std::vector<std::shared_ptr<vrt::PPolygon>> solidToPolygons(bpSolid* solid)
	{
		bpFace* fc = solid->getFace();
		std::vector<std::shared_ptr<vrt::PPolygon>> plgs;
		for (auto it = fc->begin(); it != fc->end(); it++) {
			std::vector<std::vector<Point3f>> lps;
			for (auto lpit = (*it)->Floops()->begin(); lpit != (*it)->Floops()->end(); lpit++) {
				lps.push_back(std::vector<Point3f>());
				bpHalfEdge* he = (*lpit)->getFirstHalfEdge();
				bpHalfEdge* firstHe = he;
				do
				{
					lps.back().push_back(he->getBeginVtx()->getCoord());
					he = he->Nxt();
				} while (he != firstHe);
			}
			plgs.emplace_back(new vrt::PPolygon(lps));
		}
		return plgs;
	}
}