#include "../razormill.h"

#include <gdal.h>

using namespace Razormill;

int main()
{
	GDALAllRegister();
	
	const char* source = "/home/twoods/Public/tiler-test/source/tile_2-21.tif";
	const char* target = "/home/twoods/Public/tiler-test/target";
	Format*     format = new Razormill::GeoTIFF(256, 256, 3);
	int         zoom   = 15;
	
	Razormill::GoogleTiler tiler(source, target, format);
	tiler.setNThreads(1);
	tiler.setResume(true);
	tiler.setZoom(zoom, zoom);
	tiler.run();
	
	return 0;
}

