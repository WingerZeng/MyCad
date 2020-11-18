#pragma once

// Global Include Files
#include <type_traits>
#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>
#include <QOpenGLFunctions_3_3_Core>

#ifndef BUILD_STATIC
# if defined(GLWIDGET_LIB)
#  define GLWIDGET_EXPORT Q_DECL_EXPORT
# else
#  define GLWIDGET_EXPORT Q_DECL_IMPORT
# endif
#else
# define GLWIDGET_EXPORT
#endif

//Global Types
#define OPENGLCLASS QOpenGLFunctions_3_3_Core
typedef float Float;
////Global Constants
//#define MaxFloat std::numeric_limits<Float>::max()
//#define Infinity std::numeric_limits<Float>::infinity()

namespace vrt {
	// Global Forward Declarations
	template <typename T>
	class Vector2;
	template <typename T>
	class Vector3;
	template <typename T>
	class Point3;
	template <typename T>
	class Point2;
	template <typename T>
	class Normal3;
	template <typename T>
	class Bound2;
	template <typename T>
	class Bound3;
	class Transform;

	class Scene;
	class bpSolid;
	class bpFace;
	class bpLoop;
	class bpHalfEdge;
	class bpVertex;
	class bpEdge;
	class Primitive;
	class PPolygonMesh;
	class PTriMesh;

#ifdef FLOAT_AS_DOUBLE
	typedef double Float;
#else
	typedef float Float;
#endif

	// Global Constants
	const Float MaxFloat = std::numeric_limits<Float>::max();
	const Float MinFloat = std::numeric_limits<Float>::min();
	const Float Infinity = std::numeric_limits<Float>::infinity();
	const Float MachineEpsilon = (std::numeric_limits<Float>::epsilon() * 0.5);
	const int MAX_LIGHT_COUNT = 10;
	const Float PI = 3.14159265358979323846;
}