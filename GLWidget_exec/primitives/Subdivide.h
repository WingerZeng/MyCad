#pragma once
#include "vrt.h"
#include "types.h"
#include "Primitive.h"
namespace vrt {
	/*Subdivide schemes for general topology*/
	std::vector<std::shared_ptr<GeometryPrimitive>> dooSabinSubdivide(int nLevels, int nPlgs,const int* plgLoopNums, const int *loopIndicesNums,
		const int *indices, int nPts, const Point3f* pts);

	std::vector<std::shared_ptr<GeometryPrimitive>> catmullClarkSubdivide(int nLevels, int nPlgs, const int* plgLoopNums, const int *loopIndicesNums,
		const int *indices, int nPts, const Point3f* pts);

	/*Subdivide schemes for triangular mesh*/
	std::vector<std::shared_ptr<GeometryPrimitive>> dooSabinSubdivideTri(int nLevels, int nIndices,
		const int *indices, int nPts, const Point3f* pts);

	std::vector<std::shared_ptr<GeometryPrimitive>> catmullClarkSubdivideTri(int nLevels, int nIndices,
		const int *indices, int nPts, const Point3f* pts);

	std::vector<std::shared_ptr<GeometryPrimitive>> loopSubdivideTri(int nLevels, int nIndices,
		const int *indices, int nPts, const Point3f* pts);
}

