#include "../razormill.h"

#include <gdal.h>

using namespace Razormill;

int main()
{
	GDALAllRegister();
	
	const char* source = "/home/holt/Desktop/tile-test/tile_2-21.tif";
	const char* target = "tiles";
	Format*     format = new Razormill::GeoTIFF(256, 256, 3);
	int         zoom   = 15;
	
	Razormill::GoogleTiler tiler(source, target, format);
	tiler.setNThreads(1);
	tiler.setResume(true);
	tiler.setZoom(zoom, zoom);
	tiler.run();
	
	return 0;
}

