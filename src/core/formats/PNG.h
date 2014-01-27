#ifndef RAZORMILL_FORMAT_PNG_H
#define RAZORMILL_FORMAT_PNG_H

#include "../Format.h"

namespace Razormill
{
	class PNG : public Format
	{
	public:
		PNG(int w, int h, int b): Format("PNG", "PNG", "png", w, h, b)
		{
			// That's all.
		}
	};
}

#endif

