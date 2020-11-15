#pragma once
#include "vrt.h"
#include "types.h"
namespace vrt {
	class PaintInfomation;
class Primitive : public OPENGLCLASS
{
public:
	Primitive();
	virtual ~Primitive();

	/* 属性接口 */
	unsigned int id() const { return id_; }
	bool operator<(const Primitive& rhs) {
		return this->id_ < rhs.id_;
	}
	vrt::Bounds3f getBound() { return vrt::Bounds3f(); }

	/* OpenGL绘制接口 */
	virtual void initialize() = 0;
	virtual void paint(PaintInfomation* info) = 0;

	/* 界面信息接口 */
	virtual QString name() { return ""; }; //#TODO Primitive名称功能
	bool selected() { return selected_; }
	virtual void setSelected(bool selected) { selected_ = selected; }
private:
	unsigned int id_;
	static unsigned int globalId;
	bool selected_ = false;
};

inline Primitive::Primitive()
	:id_(globalId++)
{
}

inline Primitive::~Primitive()
{
}

class GeometryPrimitive : public Primitive
{
public:
	void setColor(ColorType color) { color_ = color; };
	virtual ColorType color() { return color_; };
	void setSelected(bool selected) override;
	virtual void doBeforePaint();
	virtual void doAfterPaint();
private:
	ColorType color_;
	ColorType tempColor_;
};
}
