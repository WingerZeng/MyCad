#include "Subdivide.h"
#include <set>
#include <vector>
#include <memory>
namespace vrt{
	class SubdivPoint;
	class SubdivHalfEdge;
	class SubdivPolygon;

	struct SubdivPoint
	{
		SubdivPoint(const Point3f& pt, const SubdivHalfEdge* eg=nullptr)
			:p(pt), firstEdge(eg) {}
		SubdivPoint() = default;
		Point3f p;
		const SubdivHalfEdge* firstEdge=nullptr;
	};

	struct SubdivHalfEdge
	{
		SubdivHalfEdge(int pt1, int pt2, int face)
			:pt{ pt1,pt2 },fc(face) {}
		SubdivHalfEdge() = default;
		bool operator<(const SubdivHalfEdge& rhs) const{
			if (rhs.pt[0] == pt[0]) return pt[0] < rhs.pt[0];
			return pt[1] < rhs.pt[1];
		}
		int pt[2] = { -1 ,-1 };  //Edge point from pt[0] to pt[1];
		mutable int fc= -1;
		mutable const SubdivHalfEdge* adj = nullptr,* pev = nullptr,* nxt = nullptr;
		mutable const SubdivHalfEdge* subEdge=nullptr; //细分产生的对应子半边
	};

	struct SubdivFace
	{
		std::unique_ptr<const SubdivHalfEdge*[]> lps; //整数数组，每个单元记录每个lp首边
		int nLp;
		Point3f centroid;
		SubdivFace* subFace=nullptr; //细分产生的对应子面
	};

	struct SubdivPolygonMesh
	{
		SubdivPolygonMesh(int nPlgs, const int* plgLoopNums, const int *loopIndicesNums, const int *indices, int nPts, const Point3f* pts);
		std::vector<std::shared_ptr<vrt::GeometryPrimitive>> createPrimitives();
		SubdivPolygonMesh() = default;
		std::set<SubdivHalfEdge> edges;
		std::vector<SubdivFace> faces;
		std::vector<SubdivPoint> points;
	};

	//构造多边形网格拓扑
	SubdivPolygonMesh::SubdivPolygonMesh(int nPlgs, const int* plgLoopNums, const int *loopIndicesNums, const int *indices, int nPts, const Point3f* pts)
	{
		points.resize(nPts);
		for (int i = 0; i < nPts; i++) {
			points[i] = SubdivPoint(pts[i]);
		}
		int lpOffset = 0;
		int idxOffset = 0;
		//遍历每一多边形
		for (int i = 0; i < nPlgs; i++) {
			faces.push_back(SubdivFace());
			SubdivFace& curFace = faces.back();
			int curFaceIdx = faces.size()-1;
			const int &nLp = plgLoopNums[i];
			curFace.nLp = nLp;
			curFace.lps.reset(new const SubdivHalfEdge*[nLp]);
			//用于计算质心
			int ptCount = 0;
			Point3f ptSum(0, 0, 0);
			//遍历每一Loop
			for (int lp = 0; lp < nLp; lp++) {
				const int &nLpIdx = loopIndicesNums[lpOffset+lp];
				DCHECK(nLpIdx >= 3);
				//构造边拓扑
				int lastP = indices[idxOffset + nLpIdx - 1]; //第一个点与最后一个点相连
				const SubdivHalfEdge* lastEdge = nullptr;
				const SubdivHalfEdge* firstEdge = nullptr;
				for (int idx = 0; idx < nLpIdx; i++) {
					int curP = indices[idxOffset + idx];
					ptCount++;
					ptSum += points[curP].p;
					DCHECK(edges.find(SubdivHalfEdge(lastP, curP, -1)) == edges.end());
					//加入边
					edges.insert(SubdivHalfEdge(lastP,curP, curFaceIdx));
					const SubdivHalfEdge* curEdge = &(*edges.find(SubdivHalfEdge(lastP, curP, curFaceIdx)));
					//给点分配首边
					if (!points[lastP].firstEdge) points[lastP].firstEdge = curEdge;
					//给loop分配首边
					//配对相邻边
					auto it = edges.find(SubdivHalfEdge(curP, lastP,-1));
					if (it != edges.end()) {
						it->adj = curEdge;
						curEdge->adj = &(*it);
					}
					//配对前后边
					if (lastEdge) {
						lastEdge->nxt = curEdge;
						curEdge->pev = lastEdge;
					}
					lastEdge = curEdge;
					if (!firstEdge) {
						firstEdge = curEdge;
						curFace.lps[lp] = curEdge;
					}
					lastP = curP;
				}
				//配对第一边与最后一边
				firstEdge->pev = lastEdge;
				lastEdge->nxt = firstEdge;

				idxOffset += nLpIdx;
			}
			//计算质心
			curFace.centroid = ptSum / ptCount;
			lpOffset += nLp;
		}
	}

	std::vector<std::shared_ptr<vrt::GeometryPrimitive>> SubdivPolygonMesh::createPrimitives()
	{
		//TODO
		return std::vector<std::shared_ptr<vrt::GeometryPrimitive>>();
	}

	std::vector<std::shared_ptr<vrt::GeometryPrimitive>> dooSabinSubdivide(int nLevels, int nPlgs, const int* plgLoopNums, const int *loopIndicesNums, const int *indices, int nPts, const Point3f* pts)
	{
		std::unique_ptr<SubdivPolygonMesh> mesh(new SubdivPolygonMesh(nPlgs,plgLoopNums,loopIndicesNums, indices,nPts,pts));
		//进行nLevel次细分
		for (int i = 0; i < nLevels; i++) {
			std::unique_ptr<SubdivPolygonMesh> subMesh(new SubdivPolygonMesh);
			//预分配数组空间
			subMesh->faces.reserve(mesh->faces.size() * 2);
			subMesh->points.reserve(mesh->points.size());

			//首先生成所有面内部的细分面，点和半边
			for (auto& face : mesh->faces) {
				//生成新面
				subMesh->faces.emplace_back();
				int curFaceIdx = subMesh->faces.size()-1;
				SubdivFace& curFace = subMesh->faces.back();
				face.subFace = &curFace;
				curFace.nLp = face.nLp;
				curFace.lps.reset(new const SubdivHalfEdge *[curFace.nLp]);
				for (int lp = 0; lp < face.nLp; lp++) {
					const SubdivHalfEdge* edge = face.lps[lp];
					const SubdivHalfEdge* firstEdge = edge;
					int firstPt = subMesh->points.size();
					bool first = true;
					const SubdivHalfEdge* firstSubEdge = nullptr;
					const SubdivHalfEdge* lastSubEdge = nullptr;
					//生成细分节点与半边
					do {
						const Point3f& pt = edge->pt[1];	//顶点
						const Point3f& pevPt = (edge->pt[0]+ edge->pt[1])/2;	//边点
						const Point3f& nxtPt = (edge->pt[1] + edge->nxt->pt[1]) / 2;	//边点
						Point3f avgPt = (pt + pevPt + nxtPt + face.centroid) / 4;	//面点
						subMesh->points.push_back(SubdivPoint(avgPt));
						if (first) first = false;
						else { //生成半边
							subMesh->edges.insert(SubdivHalfEdge(subMesh->points.size() - 2, subMesh->points.size() - 1, curFaceIdx));
							const SubdivHalfEdge* curSubEdge = &(*subMesh->edges.find(SubdivHalfEdge(subMesh->points.size() - 2, subMesh->points.size() - 1, curFaceIdx)));
							//绑定到父边
							edge->subEdge = curSubEdge;
							subMesh->points[subMesh->points.size() - 2].firstEdge = curSubEdge;
							//分配loop的首边
							curFace.lps[lp] = curSubEdge;
							//绑定半边前后关系
							if (!firstSubEdge) {
								firstSubEdge = curSubEdge;
							}
							else {
								lastSubEdge->nxt = curSubEdge;
								curSubEdge->pev = lastSubEdge;
							}
							lastSubEdge = curSubEdge;
						}
						edge = edge->nxt;
					} while (edge == firstEdge);
					//连接首末节点
					subMesh->edges.insert(SubdivHalfEdge(subMesh->points.size()-1, firstPt, curFaceIdx));
					const SubdivHalfEdge* curSubEdge = &(*subMesh->edges.find(SubdivHalfEdge(subMesh->points.size() - 1, firstPt, curFaceIdx)));
					curSubEdge->nxt = firstSubEdge;
					firstSubEdge->pev = curSubEdge;
					curSubEdge->pev = lastSubEdge;
					lastSubEdge->nxt = curSubEdge;
					firstEdge->pev->subEdge = curSubEdge; 
					subMesh->points[subMesh->points.size() - 1].firstEdge = curSubEdge;
				}
			}

			//遍历原始所有的原始网格点，生成所有脊子面与点子面，连接拓扑关系
			for (int pt = 0; pt < mesh->points.size(); pt++) {
				const SubdivHalfEdge* firstEdge = mesh->points[pt].firstEdge;
				const SubdivHalfEdge* edge = mesh->points[pt].firstEdge;
				//生成顶点子面
				subMesh->faces.emplace_back();
				int vtxSubFaceIdx = subMesh->faces.size();
				SubdivFace& vtxSubFace = subMesh->faces.back();
				vtxSubFace.nLp = 1;
				vtxSubFace.lps.reset(new const SubdivHalfEdge *[1]);
				vtxSubFace.lps[0] = nullptr;

				const SubdivHalfEdge* lastVtxSubEdge = nullptr;
				const SubdivHalfEdge* firstVtxSubEdge = nullptr;
				//遍历一个原始网格点周围的边
				do {
					const SubdivHalfEdge* subedge = edge->subEdge;
					if (!subedge->adj) { //该子边对应的脊面没有连接，连接脊面
						const SubdivHalfEdge* oppoSubEdge = edge->adj->subEdge;

						subMesh->faces.emplace_back();
						SubdivFace& newSubFace = subMesh->faces.back();
						newSubFace.nLp = 1;
						newSubFace.lps.reset(new const SubdivHalfEdge *[1]);

						//生成脊面四个边，并且连接拓扑
						subMesh->edges.insert(SubdivHalfEdge(subedge->pt[1], subedge->pt[0], subMesh->faces.size() - 1));
						const SubdivHalfEdge* edgeAdj = &*subMesh->edges.find(SubdivHalfEdge(subedge->pt[1], subedge->pt[0], subMesh->faces.size() - 1));
						edgeAdj->adj = subedge;
						subedge->adj = edgeAdj;

						subMesh->edges.insert(SubdivHalfEdge(oppoSubEdge->pt[1], oppoSubEdge->pt[0], subMesh->faces.size() - 1));
						const SubdivHalfEdge* oppoEdgeAdj = &*subMesh->edges.find(SubdivHalfEdge(oppoSubEdge->pt[1], oppoSubEdge->pt[0], subMesh->faces.size() - 1));
						oppoEdgeAdj->adj = oppoSubEdge;
						oppoSubEdge->adj = oppoEdgeAdj;

						subMesh->edges.insert(SubdivHalfEdge(subedge->pt[0], oppoSubEdge->pt[1], subMesh->faces.size() - 1));
						const SubdivHalfEdge* edgeAdjNxt = &*subMesh->edges.find(SubdivHalfEdge(subedge->pt[0], oppoSubEdge->pt[1], subMesh->faces.size() - 1));

						subMesh->edges.insert(SubdivHalfEdge(oppoSubEdge->pt[0], subedge->pt[1], subMesh->faces.size() - 1));
						const SubdivHalfEdge* oppoEdgeAdjNxt = &*subMesh->edges.find(SubdivHalfEdge(oppoSubEdge->pt[0], subedge->pt[1], subMesh->faces.size() - 1));

						edgeAdj->nxt = edgeAdjNxt;
						edgeAdjNxt->pev = edgeAdj;

						oppoEdgeAdj->pev = edgeAdjNxt;
						edgeAdjNxt->nxt = oppoEdgeAdj;

						edgeAdj->pev = oppoEdgeAdjNxt;
						oppoEdgeAdjNxt->nxt = edgeAdj;

						oppoEdgeAdj->nxt = oppoEdgeAdjNxt;
						oppoEdgeAdjNxt->pev = oppoEdgeAdj;

						newSubFace.lps[0] = edgeAdj;
					}
					const SubdivHalfEdge* adjEdge = edge->subEdge->adj->nxt; //与顶点面的边相邻的边

					subMesh->edges.insert(SubdivHalfEdge(adjEdge->pt[1], adjEdge->pt[0], vtxSubFaceIdx));
					//生成顶点子面的一条边
					const SubdivHalfEdge* vtxSubEdge = &*subMesh->edges.find(SubdivHalfEdge(adjEdge->pt[1], adjEdge->pt[0], vtxSubFaceIdx));
					vtxSubEdge->adj = adjEdge;

					//连接拓扑关系
					if (!firstVtxSubEdge) firstVtxSubEdge = vtxSubEdge;
					else {
						lastVtxSubEdge->nxt = vtxSubEdge;
						vtxSubEdge->pev = lastVtxSubEdge;
					}
					lastVtxSubEdge = vtxSubEdge;
					//移动到下一条顶点边
					edge = edge->pev->adj;
				} while (edge == firstEdge);
				vtxSubFace.lps[0] = firstVtxSubEdge;

				lastVtxSubEdge->nxt = firstVtxSubEdge;
				firstVtxSubEdge->pev = lastVtxSubEdge;
			}

			//计算所有子面的质心
			for (auto& face : subMesh->faces) {
				Point3f ptSum(0, 0, 0);
				int ptCount;
				for (int i = 0; i < face.nLp; i++) {
					auto edge = face.lps[i];
					auto firstEdge = edge;
					do {
						ptSum += subMesh->points[edge->pt[0]].p;
						ptCount++;
					} while (edge != firstEdge);
				}
				face.centroid = ptSum / ptCount;
			}

			//父mesh替换为子mesh
			mesh = std::move(subMesh);
		}
		//TODO
		return std::vector<std::shared_ptr<vrt::GeometryPrimitive>>();
	}

	std::vector<std::shared_ptr<vrt::GeometryPrimitive>> catmullClarkSubdivide(int nLevels, int nPlgs, const int* plgLoopNums, const int *loopIndicesNums, const int *indices, int nPts, const Point3f* pts)
	{
		//TODO
		return std::vector<std::shared_ptr<vrt::GeometryPrimitive>>();
	}

	std::vector<std::shared_ptr<vrt::GeometryPrimitive>> dooSabinSubdivideTri(int nLevels, int nIndices, const int *indices, int nPts, const Point3f* pts)
	{
		//TODO
		return std::vector<std::shared_ptr<vrt::GeometryPrimitive>>();
	}

	std::vector<std::shared_ptr<vrt::GeometryPrimitive>> catmullClarkSubdivideTri(int nLevels, int nIndices, const int *indices, int nPts, const Point3f* pts)
	{
		//TODO
		return std::vector<std::shared_ptr<vrt::GeometryPrimitive>>();
	}

	std::vector<std::shared_ptr<vrt::GeometryPrimitive>> loopSubdivideTri(int nLevels, int nIndices, const int *indices, int nPts, const Point3f* pts)
	{
		//TODO
		return std::vector<std::shared_ptr<vrt::GeometryPrimitive>>();
	}
}