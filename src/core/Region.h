#ifndef RAZORMILL_REGION_H
#define RAZORMILL_REGION_H

struct Region
{
	int x;
	int y;
	int z;
	int w;
	int h;
	int n;
	
	int upscale()
	{
		int x1 = x;
		int y1 = y;
		int x2 = x + w;
		int y2 = y + h;
		
		x = x1 / 2;
		y = y1 / 2;
		w = x2 / 2 - x + 1;
		h = y2 / 2 - y + 1;
		n = w * h;
		z = z - 1;
		
		return z;
	}
};

#endif

