#include "camera.h"
namespace vrt{
Camera::Camera(QVector3D position, QVector3D up) :
	position(position),
	up(up),
	front(-position) {}

Camera::~Camera()
{

}

void Camera::initialize(float sceneWidth, float sceneHeight) {
	width = sceneWidth;
	height = sceneHeight;
}

//返回viewMatrix
QMatrix4x4 Camera::getViewMatrix()
{
	QMatrix4x4 view;
	view.lookAt(this->position, this->position + this->front, this->up);
	return view;
}

//返回坐标轴用的viewMatrix
QMatrix4x4 Camera::getViewMatrixForAxis()
{
	QMatrix4x4 view;
	view.lookAt(-this->front, QVector3D(0, 0, 0), this->up);
	return view;
}

//返回projectionMatrix
QMatrix4x4 Camera::getProjMatrix() {
	QMatrix4x4 proj;
	proj.ortho(-zoom_ * width / height, zoom_*width / height, -1 * zoom_, 1 * zoom_, 3 - MAX_ZOOM, 3 + MAX_ZOOM);
	return proj;
}

//返回鼠标拾取时使用的projMatrix，以此可实现视角拉近时选择内部物体
QMatrix4x4 Camera::getProjMatrixForSelection() {
	QMatrix4x4 proj;
	proj.ortho(-zoom_ * width / height, zoom_*width / height, -1 * zoom_, 1 * zoom_, 3 - 0.5*zoom_, 3 + MAX_ZOOM);
	return proj;
}

//返回坐标轴用的projMatrix
QMatrix4x4 Camera::getProjMatrixForAxis() {
	QMatrix4x4 proj;
	proj.ortho(-width / height, width / height, -1, 1, -5, 5);
	return proj;
}

//滚轮控制缩放
void Camera::zoomControl(int delta)
{
	if (delta > 0) {
		if (this->zoom_ >= MIN_ZOOM && this->zoom_ <= MAX_ZOOM)
			this->zoom_ = this->zoom_*0.9;
		if (this->zoom_ > MAX_ZOOM)
			this->zoom_ = MAX_ZOOM;
		if (this->zoom_ < MIN_ZOOM)
			this->zoom_ = MIN_ZOOM;
	}
	else {
		if (this->zoom_ >= MIN_ZOOM && this->zoom_ <= MAX_ZOOM)
			this->zoom_ = this->zoom_*1.2;
		if (this->zoom_ > MAX_ZOOM)
			this->zoom_ = MAX_ZOOM;
		if (this->zoom_ < MIN_ZOOM)
			this->zoom_ = MIN_ZOOM;
	}
}

//输入一个网格尺寸，据此设置缩放范围
void Camera::setMaxZoom(float meshSize) {
	if (meshSize > maxMeshSize_) {
		maxMeshSize_ = meshSize;
		if (width / height < 1.0f) {
			ZOOM = maxMeshSize_ * 2.0f * height / width;
			MAX_ZOOM = ZOOM * 2.5f;
			zoom_ = ZOOM;
		}
		else {
			ZOOM = maxMeshSize_ * 2.0f;
			MAX_ZOOM = ZOOM * 2.5f;
			zoom_ = ZOOM;
		}
	}
}

void vrt::Camera::fitToBound(const Bounds3f& bd)
{
	Point3f center;
	Float radius=1;
	//bd.BoundingSphere(&center, &radius);

	zoom_ = radius;

}
}
