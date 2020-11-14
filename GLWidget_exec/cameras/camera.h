#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QMouseEvent>
#include "core/types.h"
namespace vrt {
	class Camera {
	public:
		Camera(QVector3D position = QVector3D(0.0f, 0.0f, 3.0f), QVector3D up = QVector3D(0.0f, 1.0f, 0.0f));
		~Camera();

		//视图矩阵
		QMatrix4x4 getViewMatrix();
		//为坐标轴单独确定一个视图矩阵
		QMatrix4x4 getViewMatrixForAxis();

		//投影矩阵
		QMatrix4x4 getProjMatrix();
		//为鼠标拾取确定一个投影矩阵，以实现拉近视角选择内部物体
		QMatrix4x4 getProjMatrixForSelection();
		//为坐标轴单独确定一个投影矩阵
		QMatrix4x4 getProjMatrixForAxis();

		QVector3D position;
		QVector3D up;
		QVector3D front;

		//缩放
		float zoom() { return zoom_; }
		void setZoom(float zoom) { zoom_ = zoom; }
		void fitToBound(const Bounds3f& bd);

		//合适的缩放大小
		float fitZoom() { return ZOOM; }

		//输入网格尺寸，据此设置相机缩放范围以及默认缩放大小
		void setMaxZoom(float meshSize);

		//返回目前相机确定投影范围所依据的网格尺寸
		float maxMeshSize() { return maxMeshSize_; }
		//重置已存储的网格尺寸
		void cleanMeshSize() { maxMeshSize_ = 0.0f; }

		void initialize(float sceneWidth, float sceneHeight);
		void zoomControl(int delta);

	private:
		float width;
		float height;

		float zoom_ = 1.0f;

		//相机缩放默认值及范围
		float ZOOM = 1.0f;
		float MAX_ZOOM = 1e4f;
		float MIN_ZOOM = 1e-5f;

		float maxMeshSize_ = 0.0f;

	};

}


#endif // CAMERA_H