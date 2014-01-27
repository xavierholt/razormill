#ifndef RAZORMILL_FORMAT_GEOTIFF_H
#define RAZORMILL_FORMAT_GEOTIFF_H

#include "../Format.h"

namespace Razormill
{
	class GeoTIFF : public Format
	{
	public:
		GeoTIFF(int w, int h, int b): Format("GeoTIFF", "GTiff", "tif", w, h, b)
		{
			// That's all.
		}
	};
}

#endif

