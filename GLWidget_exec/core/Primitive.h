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

	/* ���Խӿ� */
	unsigned int id() const { return id_; }
	bool operator<(const Primitive& rhs) {
		return this->id_ < rhs.id_;
	}
	vrt::Bounds3f getBound() { return vrt::Bounds3f(); }

	/* OpenGL���ƽӿ� */
	virtual void initialize() = 0;
	virtual void paint(PaintInfomation* info) = 0;

	/* ������Ϣ�ӿ� */
	virtual QString name() { return ""; }; //#TODO Primitive���ƹ���
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
