#pragma once
#include "vrt.h"
#include "types.h"
#include <array>
namespace vrt {
	class PolygonRasterizer
	{
		struct Edge
		{
			int ymax;	//�ߵ��϶˵�� y ����
			double x;		//�ߵ��¶˵� x ���ꡣ�ڻ�������У���ʾɨ������ߵĽ���� x ����
			double dx;	//������ֱ�ߵ�б�ʵ���
			Edge* nxt;	//ָ����һ����ָ��
		};

		//�߱��е�һ��Ԫ��
		struct ETElement {
			Edge* headEdge;
			int ymin;
		};

		typedef std::vector<ETElement> ET; //����ı߱�,�¶˵�������� y ֵ���� i �ıߣ������ i ��

		typedef Edge* AEL; //��������뵱ǰɨ�����ཻ�ı����
	};
}

