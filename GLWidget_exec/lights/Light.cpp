#include "Light.h"

namespace vrt {
	ConstantLight::ConstantLight(ColorType Ambient, ColorType Diffuse, ColorType Specular,Point3f pos)
		:ambient_(Ambient), diffuse_(Diffuse), specular_(Specular), pos_(pos)
	{
	}
}

