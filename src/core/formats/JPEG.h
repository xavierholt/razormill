#ifndef RAZORMILL_FORMAT_JPEG_H
#define RAZORMILL_FORMAT_JPEG_H

#include "../Format.h"

namespace Razormill
{
	class JPEG : public Format
	{
	public:
		JPEG(int w, int h, int b = 3): Format("JPEG", "JPEG", "jpg", w, h, b)
		{
			// That's all.
		}
	};
}

#endif

