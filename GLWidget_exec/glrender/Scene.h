#pragma once

#include <set>
#include "vrt.h"
#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLFunctions_3_3_Core>
#include <QVector3D>
#include "axis.h"
#include "cameras/camera.h"

class QOpenGLDebugLogger;

namespace vrt {
	class Primitive;

	class Scene : public QOpenGLWidget, public OPENGLCLASS
	{
		Q_OBJECT

	public:
		Scene(QWidget* parent = nullptr);
		~Scene();

		virtual void zoomFit();
		void addPrimitive(std::shared_ptr<Primitive> prim);
		void delPrimitive(int id);
		template <class T>
		void delPrimitives(const std::vector < std::shared_ptr<T>>& prims);
		template <class T>
		void addPrimitives(const std::vector < std::shared_ptr<T>>& prims);
		static void debugOpenGL();
	protected:
		virtual void initializeGL() override;
		virtual void resizeGL(int w, int h) override;
		virtual void paintGL() override;
		virtual void mouseMoveEvent(QMouseEvent *ev);
		virtual void wheelEvent(QWheelEvent *ev);
		virtual void mousePressEvent(QMouseEvent *ev);
		void doPrimAdd();
	private:
		Camera camera;
		Axis axis;

		QVector2D mousePos;
		QVector2D wheelPos;

		QVector3D currentPosition;
		QVector3D currentFront;
		QVector3D currentUp;

		QOpenGLBuffer rbo;

		//鼠标点选相关
		bool hitted = false;
		QVector2D hitPoint;

		std::map<int, std::shared_ptr<Primitive>> prims_;
		std::vector<std::shared_ptr<Primitive>> primsToAdd;

		static QOpenGLDebugLogger* logger;
	};

	template<class T>
	void Scene::addPrimitives(const std::vector < std::shared_ptr<T>>& prims)
	{
		for (const auto& prim : prims) {
			addPrimitive(prim);
		}
	}
	
	template <class T>
		void Scene::delPrimitives(const std::vector < std::shared_ptr<T>>& prims)
	{
		for (const auto& prim : prims) {
			delPrimitive(prim->id());
		}
	}
}