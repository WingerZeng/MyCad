#include "Scene.h"
#include <QOpenGLDebugLogger>
#include "Primitive.h"
#include "primitives/PPolygon.h"
#include "PaintInformation.h"
#include "lights/Light.h"
namespace vrt {
Scene::Scene(QWidget* parent)
	: QOpenGLWidget(parent)
{
	QSurfaceFormat format;
#ifdef _DEBUG
	//输出调试信息
	format.setMajorVersion(3);
	format.setMinorVersion(3);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setOption(QSurfaceFormat::DebugContext);
#endif
	//采样倍数
	format.setSamples(32);
	this->setFormat(format);
}

QOpenGLDebugLogger* Scene::logger = nullptr;

Scene::~Scene()
{
	
}

void Scene::initializeGL()
{
	this->initializeOpenGLFunctions();

	this->glClearColor(0.7f, 0.7f, 0.8f, 1.0f);
	this->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//相机初始化
	camera.initialize(width(), height());

	//坐标轴初始化
	axis.initialize();
	axis.updateViewMat(camera.getViewMatrixForAxis());
	axis.updateProjMat(camera.getProjMatrixForAxis());

	doPrimAdd();

#ifdef _DEBUG
	logger = new QOpenGLDebugLogger(this);
	logger->initialize();
#endif
}

void Scene::resizeGL(int w, int h)
{
	this->glViewport(0, 0, w, h);
	camera.initialize(w, h);
}

void Scene::paintGL()
{
	this->glEnable(GL_DEPTH_TEST);
	this->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	debugOpenGL();

	doPrimAdd();

	PaintInfomation info;
	info.projMat = camera.getProjMatrix();
	info.viewMat = camera.getViewMatrix();
	info.lights.push_back(std::shared_ptr<Light>(new ConstantLight(Vector3f(0.5, 0.5, 0.5), Vector3f(0.9, 0.9, 0.9), Vector3f(0.0f), Point3f(4000, 4000, -5000))));
	
	if(wfmode_) info.fillmode = FILL_WIREFRAME;
	else info.fillmode = FILL;

	info.lineWidth = 1.5f;
	info.pointSize = 8.0f;
	info.width = width();
	info.height = height();

	for(const auto& prim : prims_) {
		prim.second->paint(&info);
	}

	//坐标轴
	this->glClear(GL_DEPTH_BUFFER_BIT);
	axis.updateViewMat(camera.getViewMatrixForAxis());
	axis.updateProjMat(camera.getProjMatrixForAxis());
	axis.paint();
}

void Scene::mouseMoveEvent(QMouseEvent *ev)
{
	//鼠标右键控制旋转
	if (ev->buttons() & Qt::LeftButton)
	{
		QVector2D rot;
		QQuaternion rotation;
		rot.setX((ev->x() - mousePos.x()) / width());
		rot.setY((ev->y() - mousePos.y()) / height());

		//横向旋转
		rotation = QQuaternion::fromAxisAndAngle(currentUp, -200 * rot.x());
		camera.up = rotation.rotatedVector(currentUp);
		camera.position = rotation.rotatedVector(currentPosition);
		camera.front = rotation.rotatedVector(currentFront);

		//纵向旋转
		rotation = QQuaternion::fromAxisAndAngle(QVector3D::crossProduct(-currentFront, currentUp).normalized(), 200 * rot.y());
		camera.up = rotation.rotatedVector(camera.up);
		camera.position = rotation.rotatedVector(camera.position);
		camera.front = rotation.rotatedVector(camera.front);

		update();
	}
	//鼠标中键控制平移
	if (ev->buttons() & Qt::MidButton)
	{
		QVector2D move;
		move.setX((ev->x() - wheelPos.x()) / width());
		move.setY((ev->y() - wheelPos.y()) / height());

		camera.position = currentPosition + QVector3D::crossProduct(-camera.front, camera.up).normalized() * 3 * camera.zoom()*move.x()
			+ camera.up.normalized() * 3 * camera.zoom()*move.y();

		update();
	}
}

//滚轮控制缩放
void Scene::wheelEvent(QWheelEvent *ev) {
	camera.zoomControl(ev->delta());
	update();
}

void Scene::mousePressEvent(QMouseEvent *ev)
{
	//获取点击时状态
	if (ev->button() == Qt::LeftButton) {
		mousePos.setX(ev->x());
		mousePos.setY(ev->y());

		currentPosition = camera.position;
		currentFront = camera.front;
		currentUp = camera.up;
	}

	if (ev->button() == Qt::MidButton) {
		wheelPos.setX(ev->x());
		wheelPos.setY(ev->y());

		currentPosition = camera.position;
	}
}

void Scene::doPrimAdd()
{
	while (!primsToAdd.empty()) {
		const auto& prim = primsToAdd.back();
		if (prims_.find(prim->id()) == prims_.end()) {
			prims_[prim->id()] = prim;
			prim->initialize();
		}
		primsToAdd.pop_back();
	}
}

//调整合适大小
void Scene::zoomFit() {
	camera.fitZoom();
}

void Scene::delPrimitive(int id)
{
	auto it = prims_.find(id);
	if (it != prims_.end())
		prims_.erase(it);
	for (auto it = primsToAdd.begin(); it != primsToAdd.end();) {
		if ((*it)->id() == id) it = primsToAdd.erase(it); // #PERF5 此处如果PrimsToAdd较多，会出现性能问题
		else it++;
	}
	for (int i = 0; i < primsToAdd.size(); i++) {
	}
}

void Scene::debugOpenGL()
{
#ifdef _DEBUG
	const QList<QOpenGLDebugMessage> messages = logger->loggedMessages();
	for (const QOpenGLDebugMessage &message : messages)
		qDebug() << message;
#endif
}

int Scene::wireFrameMode(bool wfmode)
{
	wfmode_ = wfmode;
	return 0;
}

void vrt::Scene::addPrimitive(std::shared_ptr<Primitive> prim)
{
	primsToAdd.push_back(prim);
}
}
