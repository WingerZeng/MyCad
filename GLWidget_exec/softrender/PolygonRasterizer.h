#pragma once
#include "vrt.h"
#include "types.h"
#include <array>
namespace vrt {
	class PolygonRasterizer
	{
		struct Edge
		{
			int ymax;	//边的上端点的 y 坐标
			double x;		//边的下端点 x 坐标。在活化边链表中，表示扫描线与边的交点的 x 坐标
			double dx;	//边所在直线的斜率倒数
			Edge* nxt;	//指向下一条边指针
		};

		//边表中的一个元素
		struct ETElement {
			Edge* headEdge;
			int ymin;
		};

		typedef std::vector<ETElement> ET; //分类的边表,下端点的纵坐标 y 值等于 i 的边，归入第 i 类

		typedef Edge* AEL; //活化链表，由与当前扫描线相交的边组成
	};
}

