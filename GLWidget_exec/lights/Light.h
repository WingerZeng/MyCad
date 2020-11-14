#pragma once
#include "vrt.h"
#include "types.h"
namespace vrt {
	class PaintInfomation;

	class Light
	{
	public:
		virtual ~Light() {};
		virtual ColorType getAmbient() = 0;
		virtual ColorType getDiffuse() = 0;
		virtual ColorType getSpecular() = 0;
		virtual Point3f getPosition(PaintInfomation* info=nullptr) = 0;

		enum PositionType
		{
			L_ABSOLUTE = 0,
			L_RELATIVE,
		};
		virtual PositionType getPositionType() { return posType_; }
	protected:
		PositionType posType_;
	};

	class ConstantLight:public Light
	{
	public:
		ConstantLight(ColorType Ambient, ColorType Diffuse, ColorType Specular, Point3f pos);

		virtual ColorType getAmbient() 	{ return ambient_;};
		virtual ColorType getDiffuse()	{ return diffuse_;}	;
		virtual ColorType getSpecular() { return specular_; };
		virtual Point3f getPosition(PaintInfomation* info = nullptr) { return pos_; }
	private:
		ColorType ambient_;
		ColorType diffuse_;
		ColorType specular_;
		Point3f pos_;
	};
}
