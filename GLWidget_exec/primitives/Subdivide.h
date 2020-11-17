#pragma once
#include "vrt.h"
#include "types.h"
#include "Primitive.h"
namespace vrt {
	/*Subdivide schemes for general topology*/
	int dooSabinSubdivPolygonMesh(int id, int nlevels);

	int catmullClarkSubdivPolygonMesh(int id, int nlevels);

	std::vector<std::shared_ptr<Primitive>> dooSabinSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh);

	std::vector<std::shared_ptr<Primitive>> catmullClarkSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh);

	/*Subdivide schemes for triangular mesh*/
	std::vector<std::shared_ptr<Primitive>> dooSabinSubdivideTri(int nLevels, int nIndices,
		const int *indices, int nPts, const Point3f* pts);

	std::vector<std::shared_ptr<Primitive>> catmullClarkSubdivideTri(int nLevels, int nIndices,
		const int *indices, int nPts, const Point3f* pts);

	std::vector<std::shared_ptr<Primitive>> loopSubdivideTri(int nLevels, int nIndices,
		const int *indices, int nPts, const Point3f* pts);
}