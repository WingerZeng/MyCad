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
		mutable const SubdivHalfEdge* subEdge=nullptr; //ϸ�ֲ����Ķ�Ӧ�Ӱ��
	};

	struct SubdivFace
	{
		std::unique_ptr<const SubdivHalfEdge*[]> lps; //�������飬ÿ����Ԫ��¼ÿ��lp�ױ�
		int nLp;
		Point3f centroid;
		SubdivFace* subFace=nullptr; //ϸ�ֲ����Ķ�Ӧ����
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
				for (int idx = 0; idx < nLpIdx; i++) {
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

	std::vector<std::shared_ptr<vrt::GeometryPrimitive>> SubdivPolygonMesh::createPrimitives()
	{
		//TODO
		return std::vector<std::shared_ptr<vrt::GeometryPrimitive>>();
	}

	std::vector<std::shared_ptr<vrt::GeometryPrimitive>> dooSabinSubdivide(int nLevels, int nPlgs, const int* plgLoopNums, const int *loopIndicesNums, const int *indices, int nPts, const Point3f* pts)
	{
		std::unique_ptr<SubdivPolygonMesh> mesh(new SubdivPolygonMesh(nPlgs,plgLoopNums,loopIndicesNums, indices,nPts,pts));
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
						const Point3f& pt = edge->pt[1];	//����
						const Point3f& pevPt = (edge->pt[0]+ edge->pt[1])/2;	//�ߵ�
						const Point3f& nxtPt = (edge->pt[1] + edge->nxt->pt[1]) / 2;	//�ߵ�
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
					} while (edge == firstEdge);
					//������ĩ�ڵ�
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

			//����ԭʼ���е�ԭʼ����㣬�������м�����������棬�������˹�ϵ
			for (int pt = 0; pt < mesh->points.size(); pt++) {
				const SubdivHalfEdge* firstEdge = mesh->points[pt].firstEdge;
				const SubdivHalfEdge* edge = mesh->points[pt].firstEdge;
				//���ɶ�������
				subMesh->faces.emplace_back();
				int vtxSubFaceIdx = subMesh->faces.size();
				SubdivFace& vtxSubFace = subMesh->faces.back();
				vtxSubFace.nLp = 1;
				vtxSubFace.lps.reset(new const SubdivHalfEdge *[1]);
				vtxSubFace.lps[0] = nullptr;

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

					//�������˹�ϵ
					if (!firstVtxSubEdge) firstVtxSubEdge = vtxSubEdge;
					else {
						lastVtxSubEdge->nxt = vtxSubEdge;
						vtxSubEdge->pev = lastVtxSubEdge;
					}
					lastVtxSubEdge = vtxSubEdge;
					//�ƶ�����һ�������
					edge = edge->pev->adj;
				} while (edge == firstEdge);
				vtxSubFace.lps[0] = firstVtxSubEdge;

				lastVtxSubEdge->nxt = firstVtxSubEdge;
				firstVtxSubEdge->pev = lastVtxSubEdge;
			}

			//�����������������
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

			//��mesh�滻Ϊ��mesh
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