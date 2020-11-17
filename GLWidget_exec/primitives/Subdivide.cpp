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
		int subPoint = -1; //ϸ���Ӷ��㣬����catmullϸ��

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
		//#PERF5 ��������������ÿ���㷨��ֻ����һ���пռ�Ч������
		mutable const SubdivHalfEdge* subEdge=nullptr; //ϸ�ֲ����Ķ�Ӧ�Ӱ��
		mutable int subEdgePoint = -1; //ϸ�ֲ����ıߵ㣬����catmullϸ��
	};

	struct SubdivFace
	{
		std::unique_ptr<const SubdivHalfEdge*[]> lps; //�������飬ÿ����Ԫ��¼ÿ��lp�ױ�
		int nLp;
		Point3f centroid;
		SubdivFace* subFace=nullptr; //ϸ�ֲ����Ķ�Ӧ����;
		int subFacePoint = -1; //����㣬����catmullϸ��
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

	//����������������
	SubdivPolygonMesh::SubdivPolygonMesh(int nPlgs, const int* plgLoopNums, const int *loopIndicesNums, const int *indices, int nPts, const Point3f* pts)
	{
		points.resize(nPts);
		for (int i = 0; i < nPts; i++) {
			points[i] = SubdivPoint(pts[i]);
		}
		int lpOffset = 0;
		int idxOffset = 0;
		//����ÿһ�����
		for (int i = 0; i < nPlgs; i++) {
			faces.push_back(SubdivFace());
			SubdivFace& curFace = faces.back();
			int curFaceIdx = faces.size()-1;
			const int &nLp = plgLoopNums[i];
			curFace.nLp = nLp;
			curFace.lps.reset(new const SubdivHalfEdge*[nLp]);
			//���ڼ�������
			int ptCount = 0;
			Point3f ptSum(0, 0, 0);
			//����ÿһLoop
			for (int lp = 0; lp < nLp; lp++) {
				const int &nLpIdx = loopIndicesNums[lpOffset+lp];
				DCHECK(nLpIdx >= 3);
				//���������
				int lastP = indices[idxOffset + nLpIdx - 1]; //��һ���������һ��������
				const SubdivHalfEdge* lastEdge = nullptr;
				const SubdivHalfEdge* firstEdge = nullptr;
				for (int idx = 0; idx < nLpIdx; idx++) {
					int curP = indices[idxOffset + idx];
					ptCount++;
					ptSum += points[curP].p;
					DCHECK(edges.find(SubdivHalfEdge(lastP, curP, -1)) == edges.end());
					//�����
					edges.insert(SubdivHalfEdge(lastP,curP, curFaceIdx));
					const SubdivHalfEdge* curEdge = &(*edges.find(SubdivHalfEdge(lastP, curP, curFaceIdx)));
					//��������ױ�
					if (!points[lastP].firstEdge) points[lastP].firstEdge = curEdge;
					//��loop�����ױ�
					//������ڱ�
					auto it = edges.find(SubdivHalfEdge(curP, lastP,-1));
					if (it != edges.end()) {
						it->adj = curEdge;
						curEdge->adj = &(*it);
					}
					//���ǰ���
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
				//��Ե�һ�������һ��
				firstEdge->pev = lastEdge;
				lastEdge->nxt = firstEdge;

				idxOffset += nLpIdx;
			}
			//��������
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
		//����ÿһ�����
		for (int i = 0; i < plgmesh->getPlgs().size(); i++) {
			faces.push_back(SubdivFace());
			SubdivFace& curFace = faces.back();
			int curFaceIdx = faces.size() - 1;
			const int &nLp = plgmesh->getPlgs()[i].lps_.size();
			curFace.nLp = nLp;
			curFace.lps.reset(new const SubdivHalfEdge*[nLp]);
			//���ڼ�������
			int ptCount = 0;
			Point3f ptSum(0, 0, 0);
			//����ÿһLoop
			for (int lp = 0; lp < nLp; lp++) {
				const auto& curLp = plgmesh->getPlgs()[i].lps_[lp];
				const int &nLpIdx = curLp.size();
				DCHECK(nLpIdx >= 3);
				//���������
				int lastP = curLp[curLp.size()-1]; //��һ���������һ��������
				const SubdivHalfEdge* lastEdge = nullptr;
				const SubdivHalfEdge* firstEdge = nullptr;
				for (int idx = 0; idx < nLpIdx; idx++) {
					int curP = curLp[idx];
					ptCount++;
					ptSum += points[curP].p;
					DCHECK(edges.find(SubdivHalfEdge(lastP, curP, -1)) == edges.end());
					//�����
					edges.insert(SubdivHalfEdge(lastP, curP, curFaceIdx));
					const SubdivHalfEdge* curEdge = &(*edges.find(SubdivHalfEdge(lastP, curP, curFaceIdx)));
					//��������ױ�
					if (!points[lastP].firstEdge) points[lastP].firstEdge = curEdge;
					//��loop�����ױ�
					//������ڱ�
					auto it = edges.find(SubdivHalfEdge(curP, lastP, -1));
					if (it != edges.end()) {
						it->adj = curEdge;
						curEdge->adj = &(*it);
					}
					//���ǰ���
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
				//��Ե�һ�������һ��
				firstEdge->pev = lastEdge;
				lastEdge->nxt = firstEdge;
			}
			//��������
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
		//����nLevel��ϸ��
		for (int i = 0; i < nLevels; i++) {
			std::unique_ptr<SubdivPolygonMesh> subMesh(new SubdivPolygonMesh);
			//Ԥ��������ռ�
			subMesh->faces.reserve(mesh->faces.size() * 2);
			subMesh->points.reserve(mesh->points.size());

			//���������������ڲ���ϸ���棬��Ͱ��
			for (auto& face : mesh->faces) {
				//��������
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
					//����ϸ�ֽڵ�����
					do {
						const Point3f& pt = mesh->points[edge->pt[1]].p;	//����
						const Point3f& pevPt = (mesh->points[edge->pt[0]].p + mesh->points[edge->pt[1]].p)/2;	//�ߵ�
						const Point3f& nxtPt = (mesh->points[edge->pt[1]].p + mesh->points[edge->nxt->pt[1]].p) / 2;	//�ߵ�
						Point3f avgPt = (pt + pevPt + nxtPt + face.centroid) / 4;	//���
						subMesh->points.push_back(SubdivPoint(avgPt));
						if (first) first = false;
						else { //���ɰ��
							subMesh->edges.insert(SubdivHalfEdge(subMesh->points.size() - 2, subMesh->points.size() - 1, curFaceIdx));
							const SubdivHalfEdge* curSubEdge = &(*subMesh->edges.find(SubdivHalfEdge(subMesh->points.size() - 2, subMesh->points.size() - 1, curFaceIdx)));
							//�󶨵�����
							edge->subEdge = curSubEdge;
							subMesh->points[subMesh->points.size() - 2].firstEdge = curSubEdge;
							//����loop���ױ�
							curFace.lps[lp] = curSubEdge;
							//�󶨰��ǰ���ϵ
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
					//������ĩ�ڵ�
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

			//����ԭʼ���е�ԭʼ����㣬�������м�����������棬�������˹�ϵ
			for (int pt = 0; pt < mesh->points.size(); pt++) {
				const SubdivHalfEdge* firstEdge = mesh->points[pt].firstEdge;
				const SubdivHalfEdge* edge = mesh->points[pt].firstEdge;
				//���ɶ�������
				subMesh->faces.emplace_back();
				int vtxSubFaceIdx = subMesh->faces.size()-1;
				subMesh->faces[vtxSubFaceIdx].nLp = 1;
				subMesh->faces[vtxSubFaceIdx].lps.reset(new const SubdivHalfEdge *[1]);
				subMesh->faces[vtxSubFaceIdx].lps[0] = nullptr;

				const SubdivHalfEdge* lastVtxSubEdge = nullptr;
				const SubdivHalfEdge* firstVtxSubEdge = nullptr;
				//����һ��ԭʼ�������Χ�ı�
				do {
					const SubdivHalfEdge* subedge = edge->subEdge;
					if (!subedge->adj) { //���ӱ߶�Ӧ�ļ���û�����ӣ����Ӽ���
						const SubdivHalfEdge* oppoSubEdge = edge->adj->subEdge;

						subMesh->faces.emplace_back();
						SubdivFace& newSubFace = subMesh->faces.back();
						newSubFace.nLp = 1;
						newSubFace.lps.reset(new const SubdivHalfEdge *[1]);

						//���ɼ����ĸ��ߣ�������������
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
					const SubdivHalfEdge* adjEdge = edge->subEdge->adj->nxt; //�붥����ı����ڵı�

					subMesh->edges.insert(SubdivHalfEdge(adjEdge->pt[1], adjEdge->pt[0], vtxSubFaceIdx));
					//���ɶ��������һ����
					const SubdivHalfEdge* vtxSubEdge = &*subMesh->edges.find(SubdivHalfEdge(adjEdge->pt[1], adjEdge->pt[0], vtxSubFaceIdx));
					vtxSubEdge->adj = adjEdge;
					adjEdge->adj = vtxSubEdge;

					//�������˹�ϵ
					if (!firstVtxSubEdge) firstVtxSubEdge = vtxSubEdge;
					else {
						lastVtxSubEdge->nxt = vtxSubEdge;
						vtxSubEdge->pev = lastVtxSubEdge;
					}
					lastVtxSubEdge = vtxSubEdge;
					//�ƶ�����һ�������
					edge = edge->pev->adj;
				} while (edge != firstEdge);
				subMesh->faces[vtxSubFaceIdx].lps[0] = firstVtxSubEdge;

				lastVtxSubEdge->nxt = firstVtxSubEdge;
				firstVtxSubEdge->pev = lastVtxSubEdge;
			}

			//�����������������
			//���������bug���룬���������ʯͷЧ��
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

			//��mesh�滻Ϊ��mesh
			mesh = std::move(subMesh);
		}

		//#TODO2 ��mesh�Ƶ����޵�

		return mesh->createPrimitives();
	}

	std::vector<std::shared_ptr<vrt::Primitive>> catmullClarkSubdivide(int nLevels, std::shared_ptr<PPolygonMesh> plgMesh)
	{
		std::unique_ptr<SubdivPolygonMesh> mesh(new SubdivPolygonMesh(plgMesh));

		for (int i = 0; i < nLevels; i++) {
			std::unique_ptr<SubdivPolygonMesh> subMesh(new SubdivPolygonMesh);
			//Ԥ��������ռ�
			subMesh->faces.reserve(mesh->faces.size() * 3);
			subMesh->points.reserve(mesh->points.size() * 2);

			//�������
			for (auto& face : mesh->faces) {
				face.subFacePoint = subMesh->points.size();
				subMesh->addPoint(face.centroid);
			}

			//����ߵ�
			for (const auto& he : mesh->edges) {
				if (he.subEdgePoint == -1) {
					//��ȡ���
					const Point3f& fcpt1 = mesh->faces[he.fc].centroid; 
					const Point3f& fcpt2 = mesh->faces[he.adj->fc].centroid;
					Point3f edgept = (fcpt1 + fcpt2 + mesh->points[he.pt[0]].p + mesh->points[he.pt[1]].p) / 4;
					he.subEdgePoint=subMesh->addPoint(edgept);
					he.adj->subEdgePoint = he.subEdgePoint;
				}
			}

			//���㶥��
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

			//Ϊÿ��ԭʼ�棬��������
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

						//�������˹�ϵ
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

						//��������
						const Point3f&	pep1 = subMesh->points[ep1].p;
						const Point3f&	pep2 = subMesh->points[ep2].p;
						const Point3f&	ppp = subMesh->points[pp].p;
						const Point3f&	pfp = subMesh->points[fp].p;
						newFace.centroid = (pep1 + pep2 + ppp + pfp) / 4;

						he = he->nxt;
					} while (he!=firsthe);
				}
			}

			//�����ߵ�adj��ϵ
			for (auto it = subMesh->edges.begin(); it != subMesh->edges.end();it++){
				if(it->adj) continue;
				auto adjit = subMesh->edges.find(SubdivHalfEdge(it->pt[1], it->pt[0], -1));
				DCHECK(adjit != subMesh->edges.end());
				it->adj = &*adjit;
				adjit->adj = &*it;
			}
			
			mesh = std::move(subMesh);
		}
		//#TODO2 ��mesh�Ƶ����޵�

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