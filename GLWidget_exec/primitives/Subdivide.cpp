#include "Subdivide.h"
#include <set>
#include <vector>
#include <memory>
#include "ui/MainWindow.h"
#include "ui/ItemManager.h"
#include "glrender/Scene.h"
#include "PPolygonMesh.h"
namespace vrt{
	class SubdivPoint;
	class SubdivHalfEdge;
	class SubdivPolygon;

	struct SubdivPoint
	{
		SubdivPoint(const Point3f& pt, const SubdivHalfEdge* eg=nullptr)
			:p(pt), firstEdge(eg) {}
		SubdivPoint() = default;
		inline int getDegree() const;
		Point3f p;
		const SubdivHalfEdge* firstEdge=nullptr;
		int subPoint = -1; //细分子顶点，用于catmull细分

	};

	struct SubdivHalfEdge
	{
		SubdivHalfEdge(int pt1, int pt2, int face)
			:pt{ pt1,pt2 },fc(face) {}
		SubdivHalfEdge() = default;
		bool operator<(const SubdivHalfEdge& rhs) const{
			if (rhs.pt[0] != pt[0]) return pt[0] < rhs.pt[0];
			return pt[1] < rhs.pt[1];
		}
		int pt[2] = { -1 ,-1 };  //Edge point from pt[0] to pt[1];
		mutable int fc= -1;
		mutable const SubdivHalfEdge* adj = nullptr,* pev = nullptr,* nxt = nullptr;
		//#PERF5 这里两个变量在每个算法中只用其一，有空间效率问题
		mutable const SubdivHalfEdge* subEdge=nullptr; //细分产生的对应子半边
		mutable int subEdgePoint = -1; //细分产生的边点，用于catmull细分
	};

	struct SubdivFace
	{
		std::unique_ptr<const SubdivHalfEdge*[]> lps; //整数数组，每个单元记录每个lp首边
		int nLp;
		Point3f centroid;
		SubdivFace* subFace=nullptr; //细分产生的对应子面;
		int subFacePoint = -1; //子面点，用于catmull细分
	};

	struct SubdivPolygonMesh
	{
		int addPoint(Point3f pt) {
			points.push_back(SubdivPoint(pt));
			return points.size() - 1;
		}

		SubdivPolygonMesh(int nPlgs, const int* plgLoopNums, const int *loopIndicesNums, const int *indices, int nPts, const Point3f* pts);
		SubdivPolygonMesh(std::shared_ptr<PPolygonMesh> plgmesh);
		std::vector<std::shared_ptr<vrt::Primitive>> createPrimitives();
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
				for (int idx = 0; idx < nLpIdx; idx++) {
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

	int SubdivPoint::getDegree() const {
		int count = 0;
		if (!firstEdge) return 0;
		const SubdivHalfEdge* he = firstEdge;
		const SubdivHalfEdge* fhe = he;
		do {
			count++;
			he = he->adj->nxt;
		} while (he != firstEdge);
		return count;
	}

	SubdivPolygonMesh::SubdivPolygonMesh(std::shared_ptr<PPolygonMesh> plgmesh)
	{
		int nPts = plgmesh->getPts().size();
		points.resize(nPts);
		for (int i = 0; i < nPts; i++) {
			points[i] = SubdivPoint(plgmesh->getPts()[i]);
		}
		//遍历每一多边形
		for (int i = 0; i < plgmesh->getPlgs().size(); i++) {
			faces.push_back(SubdivFace());
			SubdivFace& curFace = faces.back();
			int curFaceIdx = faces.size() - 1;
			const int &nLp = plgmesh->getPlgs()[i].lps_.size();
			curFace.nLp = nLp;
			curFace.lps.reset(new const SubdivHalfEdge*[nLp]);
			//用于计算质心
			int ptCount = 0;
			Point3f ptSum(0, 0, 0);
			//遍历每一Loop
			for (int lp = 0; lp < nLp; lp++) {
				const auto& curLp = plgmesh->getPlgs()[i].lps_[lp];
				const int &nLpIdx = curLp.size();
				DCHECK(nLpIdx >= 3);
				//构造边拓扑
				int lastP = curLp[curLp.size()-1]; //第一个点与最后一个点相连
				const SubdivHalfEdge* lastEdge = nullptr;
				const SubdivHalfEdge* firstEdge = nullptr;
				for (int idx = 0; idx < nLpIdx; idx++) {
					int curP = curLp[idx];
					ptCount++;
					ptSum += points[curP].p;
					DCHECK(edges.find(SubdivHalfEdge(lastP, curP, -1)) == edges.end());
					//加入边
					edges.insert(SubdivHalfEdge(lastP, curP, curFaceIdx));
					const SubdivHalfEdge* curEdge = &(*edges.find(SubdivHalfEdge(lastP, curP, curFaceIdx)));
					//给点分配首边
					if (!points[lastP].firstEdge) points[lastP].firstEdge = curEdge;
					//给loop分配首边
					//配对相邻边
					auto it = edges.find(SubdivHalfEdge(curP, lastP, -1));
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
			}
			//计算质心
			curFace.centroid = ptSum / ptCount;
		}
	}

	std::vector<std::shared_ptr<vrt::Primitive>> SubdivPolygonMesh::createPrimitives()
	{
		std::vector<Point3f>  pts;
		std::vector<PPolygonMesh::Polygon> plgs;
		for (const auto& pt : points) {
			pts.push_back(pt.p);
		}
		for (const auto& fc : faces) {
			plgs.emplace_back();
			for (int i = 0; i < fc.nLp; i++) {
				plgs.back().lps_.emplace_back();
				const SubdivHalfEdge* he = fc.lps[i];
				auto firstHe = he;
				do {
					plgs.back().lps_.back().push_back(he->pt[0]);
					he = he->nxt;
				} while (firstHe != he);
			}
		}
		return {std::make_shared<PPolygonMesh>(plgs,pts)};
	}

	int dooSabinSubdivPolygonMesh(int id, int nlevels)
	{
		std::shared_ptr<PPolygonMesh> plgs;
		MAIPTR->itemMng()->getItem(id,plgs);
		if (!plgs) return -1;
		auto output = dooSabinSubdivide(nlevels,plgs);
		MAIPTR->itemMng()->delItem(plgs);
		MAIPTR->itemMng()->addItems(output); 
		//MAIPTR->getScene()->update();
		return 0;
	}

	int catmullClarkSubdivPolygonMesh(int id, int nlevels)
	{
		std::shared_ptr<PPolygonMesh> plgs;
		MAIPTR->itemMng()->getItem(id, plgs);
		if (!plgs) return -1;
		auto output = catmullClarkSubdivide(nlevels, plgs);
		MAIPTR->itemMng()->delItem(plgs);
		MAIPTR->itemMng()->addItems(output);
		//MAIPTR->getScene()->update();
		return 0;
	}

	std::vector<std::shared_ptr<vrt::Primitive>> dooSabinSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh)
	{
		std::unique_ptr<SubdivPolygonMesh> mesh(new SubdivPolygonMesh(plgMesh));
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
						const Point3f& pt = mesh->points[edge->pt[1]].p;	//顶点
						const Point3f& pevPt = (mesh->points[edge->pt[0]].p + mesh->points[edge->pt[1]].p)/2;	//边点
						const Point3f& nxtPt = (mesh->points[edge->pt[1]].p + mesh->points[edge->nxt->pt[1]].p) / 2;	//边点
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
					} while (edge != firstEdge);
					//连接首末节点
					subMesh->edges.insert(SubdivHalfEdge(subMesh->points.size()-1, firstPt, curFaceIdx));
					const SubdivHalfEdge* curSubEdge = &(*subMesh->edges.find(SubdivHalfEdge(subMesh->points.size() - 1, firstPt, curFaceIdx)));
					curSubEdge->nxt = firstSubEdge;
					firstSubEdge->pev = curSubEdge;
					curSubEdge->pev = lastSubEdge;
					lastSubEdge->nxt = curSubEdge;
					firstEdge->subEdge = curSubEdge; 
					subMesh->points[subMesh->points.size() - 1].firstEdge = curSubEdge;
				}
			}

			//遍历原始所有的原始网格点，生成所有脊子面与点子面，连接拓扑关系
			for (int pt = 0; pt < mesh->points.size(); pt++) {
				const SubdivHalfEdge* firstEdge = mesh->points[pt].firstEdge;
				const SubdivHalfEdge* edge = mesh->points[pt].firstEdge;
				//生成顶点子面
				subMesh->faces.emplace_back();
				int vtxSubFaceIdx = subMesh->faces.size()-1;
				subMesh->faces[vtxSubFaceIdx].nLp = 1;
				subMesh->faces[vtxSubFaceIdx].lps.reset(new const SubdivHalfEdge *[1]);
				subMesh->faces[vtxSubFaceIdx].lps[0] = nullptr;

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
					adjEdge->adj = vtxSubEdge;

					//连接拓扑关系
					if (!firstVtxSubEdge) firstVtxSubEdge = vtxSubEdge;
					else {
						lastVtxSubEdge->nxt = vtxSubEdge;
						vtxSubEdge->pev = lastVtxSubEdge;
					}
					lastVtxSubEdge = vtxSubEdge;
					//移动到下一条顶点边
					edge = edge->pev->adj;
				} while (edge != firstEdge);
				subMesh->faces[vtxSubFaceIdx].lps[0] = firstVtxSubEdge;

				lastVtxSubEdge->nxt = firstVtxSubEdge;
				firstVtxSubEdge->pev = lastVtxSubEdge;
			}

			//计算所有子面的质心
			//以下这段是bug代码，可以制造出石头效果
			//for (auto& face : subMesh->faces) {
			//	Point3f ptSum(0, 0, 0);
			//	int ptCount=0;
			//	for (int i = 0; i < face.nLp; i++) {
			//		auto edge = face.lps[i];
			//		auto firstEdge = edge;
			//		do {
			//			ptSum += subMesh->points[edge->pt[0]].p;
			//			ptCount++;
			//		} while (edge != firstEdge);
			//	}
			//	face.centroid = ptSum / ptCount;
			//}
			for (auto& face : subMesh->faces) {
				Point3f ptSum(0, 0, 0);
				int ptCount = 0;
				for (int i = 0; i < face.nLp; i++) {
					auto edge = face.lps[i];
					auto firstEdge = edge;
					do {
						ptSum += subMesh->points[edge->pt[0]].p;
						ptCount++;
						edge = edge->nxt;
					} while (edge != firstEdge);
				}
				face.centroid = ptSum / ptCount;
			}

			//父mesh替换为子mesh
			mesh = std::move(subMesh);
		}

		//#TODO2 将mesh移到极限点

		return mesh->createPrimitives();
	}

	std::vector<std::shared_ptr<vrt::Primitive>> catmullClarkSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh)
	{
		std::unique_ptr<SubdivPolygonMesh> mesh(new SubdivPolygonMesh(plgMesh));

		for (int i = 0; i < nLevels; i++) {
			std::unique_ptr<SubdivPolygonMesh> subMesh(new SubdivPolygonMesh);
			//预分配数组空间
			subMesh->faces.reserve(mesh->faces.size() * 3);
			subMesh->points.reserve(mesh->points.size() * 2);

			//计算面点
			for (auto& face : mesh->faces) {
				face.subFacePoint = subMesh->points.size();
				subMesh->addPoint(face.centroid);
			}

			//计算边点
			for (const auto& he : mesh->edges) {
				if (he.subEdgePoint == -1) {
					//获取面点
					const Point3f& fcpt1 = mesh->faces[he.fc].centroid; 
					const Point3f& fcpt2 = mesh->faces[he.adj->fc].centroid;
					Point3f edgept = (fcpt1 + fcpt2 + mesh->points[he.pt[0]].p + mesh->points[he.pt[1]].p) / 4;
					he.subEdgePoint=subMesh->addPoint(edgept);
					he.adj->subEdgePoint = he.subEdgePoint;
				}
			}

			//计算顶点
			for (auto& pt : mesh->points) {
				const SubdivHalfEdge* he = pt.firstEdge;
				const SubdivHalfEdge* firstHe = he;
				Point3f sumOfFacePt(0, 0, 0);
				Point3f sumOfEdgePt(0, 0, 0);
				int count = 0;
				do {
					sumOfEdgePt += subMesh->points[he->subEdgePoint].p;
					sumOfFacePt += subMesh->points[mesh->faces[he->fc].subFacePoint].p;
					count++;
					he = he->adj->nxt;
				} while (he!=firstHe);

				int n = pt.getDegree();
				pt.subPoint = subMesh->addPoint(((sumOfFacePt + 2 * sumOfEdgePt) / count + (n - 3)*pt.p) / n);
			}

			//为每个原始面，生成子面
			for (auto&fc: mesh->faces) {
				for (int i = 0; i < fc.nLp; i++) {
					const SubdivHalfEdge* he = fc.lps[i]; 
					const SubdivHalfEdge* firsthe = he;

					do 
					{
						int ep1 = he->subEdgePoint;
						int ep2 = he->nxt->subEdgePoint;
						int pp = mesh->points[he->pt[1]].subPoint;
						int fp = mesh->faces[he->fc].subFacePoint;

						subMesh->faces.emplace_back();
						SubdivFace& newFace = subMesh->faces.back();
						newFace.nLp = 1;

						subMesh->edges.insert(SubdivHalfEdge(fp, ep1, subMesh->faces.size() - 1));
						const SubdivHalfEdge* inhe1 = &*subMesh->edges.find(SubdivHalfEdge(fp, ep1, subMesh->faces.size() - 1));

						subMesh->edges.insert(SubdivHalfEdge(ep1, pp, subMesh->faces.size() - 1));
						const SubdivHalfEdge* outhe1 = &*subMesh->edges.find(SubdivHalfEdge(ep1, pp, subMesh->faces.size() - 1));

						subMesh->edges.insert(SubdivHalfEdge(pp, ep2, subMesh->faces.size() - 1));
						const SubdivHalfEdge* inhe2 = &*subMesh->edges.find(SubdivHalfEdge(pp, ep2, subMesh->faces.size() - 1));

						subMesh->edges.insert(SubdivHalfEdge(ep2, fp, subMesh->faces.size() - 1));
						const SubdivHalfEdge* outhe2 = &*subMesh->edges.find(SubdivHalfEdge(ep2, fp, subMesh->faces.size() - 1));

						//建立拓扑关系
						if (!subMesh->points[fp].firstEdge) subMesh->points[fp].firstEdge = inhe1;
						if (!subMesh->points[ep1].firstEdge) subMesh->points[ep1].firstEdge = outhe1;
						if (!subMesh->points[pp].firstEdge) subMesh->points[pp].firstEdge = inhe2;
						if (!subMesh->points[ep2].firstEdge) subMesh->points[ep2].firstEdge = outhe2;

						inhe1->nxt = outhe1;
						outhe1->pev = inhe1;

						outhe1->nxt = inhe2;
						inhe2->pev = outhe1;

						inhe2->nxt = outhe2;
						outhe2->pev = inhe2;

						outhe2->nxt = inhe1;
						inhe1->pev = outhe2;

						newFace.lps.reset(new const SubdivHalfEdge*[1]{ inhe1 });

						//计算质心
						const Point3f&	pep1 = subMesh->points[ep1].p;
						const Point3f&	pep2 = subMesh->points[ep2].p;
						const Point3f&	ppp = subMesh->points[pp].p;
						const Point3f&	pfp = subMesh->points[fp].p;
						newFace.centroid = (pep1 + pep2 + ppp + pfp) / 4;

						he = he->nxt;
					} while (he!=firsthe);
				}
			}

			//建立边的adj关系
			for (auto it = subMesh->edges.begin(); it != subMesh->edges.end();it++){
				if(it->adj) continue;
				auto adjit = subMesh->edges.find(SubdivHalfEdge(it->pt[1], it->pt[0], -1));
				DCHECK(adjit != subMesh->edges.end());
				it->adj = &*adjit;
				adjit->adj = &*it;
			}
			
			mesh = std::move(subMesh);
		}
		//#TODO2 将mesh移到极限点

		return mesh->createPrimitives();
	}

	std::vector<std::shared_ptr<vrt::Primitive>> dooSabinSubdivideTri(int nLevels, int nIndices, const int *indices, int nPts, const Point3f* pts)
	{
		//TODO
		return std::vector<std::shared_ptr<vrt::Primitive>>();
	}

	std::vector<std::shared_ptr<vrt::Primitive>> catmullClarkSubdivideTri(int nLevels, int nIndices, const int *indices, int nPts, const Point3f* pts)
	{
		//TODO
		return std::vector<std::shared_ptr<vrt::Primitive>>();
	}

	std::vector<std::shared_ptr<vrt::Primitive>> loopSubdivideTri(int nLevels, int nIndices, const int *indices, int nPts, const Point3f* pts)
	{
		//TODO
		return std::vector<std::shared_ptr<vrt::Primitive>>();
	}
}