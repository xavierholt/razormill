#ifndef RAZORMILL_FORMAT_MEMORY_H
#define RAZORMILL_FORMAT_MEMORY_H

#include "../Format.h"

namespace Razormill
{
	class Memory : public Format
	{
	public:
		Memory(int w, int h, int b): Format("Memory", "MEM", "", w, h, b)
		{
			// That's all.
		}
	};
}

#endif

