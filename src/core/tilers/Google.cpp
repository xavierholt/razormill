#include "Google.h"

#include <gdal_priv.h>
#include <gdalwarper.h>

#include <cmath>
#include <string>

#include <sys/stat.h>

namespace Razormill
{
	GoogleTiler::GoogleTiler(const char* source, const char* target, Format* format): Tiler("EPSG:900913", source, target, format)
	{
		// All done.
	}
	
/**
 * Calculates the region in tile space covered by the input raster.
 *
 * @param z  The desired zoom level.
 */
	void GoogleTiler::calcRegion(int z)
	{
		int    w;
		int    h;
		double g[6];
		double m[4];
		
		void* args = GDALCreateGenImgProjTransformer(mSource, mSrcProj, NULL, mDstProj, FALSE, 0, 1);
		GDALSuggestedWarpOutput2(mSource, GDALGenImgProjTransform, args, g, &w, &h, m, 0);
		GDALDestroyGenImgProjTransformer(args);
		
		double xmin = m[0];
		double ymin = m[1];
		double xmax = m[2];
		double ymax = m[3];
		
		log("World Region:\n  n: %12.6f\n  e: %f\n  s: %f\n  w: %f\n", ymax, xmax, ymin, xmin);
		
		double adj = M_PI * 6378137;
		double upt = M_PI * 6378137 / pow(2, z-1);
		
		mRegion.x = floor((xmin + adj) / upt);
		mRegion.y = floor((ymin + adj) / upt);
		mRegion.z = z;
		mRegion.w = floor((xmax + adj) / upt) - mRegion.x + 1;
		mRegion.h = floor((ymax + adj) / upt) - mRegion.y + 1;
		mRegion.n = mRegion.w * mRegion.h;
		
		log("Tile Region:\n  x: %i\n  y: %i\n  z: %i\n  w: %i\n  h: %i\n  n: %i\n", mRegion.x, mRegion.y, mRegion.z, mRegion.w, mRegion.h, mRegion.n);
	}
	
	int GoogleTiler::calcZoom() const
	{
		return -1;
	}

/**
 * Creates a file system of the format <base>/<z>/<x>/ to store intermediate and final tiles.
 */
	void GoogleTiler::generateDirs() const
	{
		char destination[256];
		
		sprintf(destination, "%s/%d/", mTarget, mRegion.z);
		createDir(destination);
		
		for(int i = mRegion.x; i < mRegion.x + mRegion.w; i++)
		{
			sprintf(destination, "%s/%d/%d/", mTarget, mRegion.z, i);
			createDir(destination);
		}
	}

/**
 * Returns a new GDAL raster for the indexed tile.
 *
 * @param  i  The linear index of the desired tile.
 * @result    A new raster with its SRID and GeoTransform set appropriately.
 */
	GDALDataset* GoogleTiler::baseRaster(int i) const
	{
		int x = i % mRegion.w + mRegion.x;
		int y = i / mRegion.w + mRegion.y;
		int z = mRegion.z;
		
		double adj = M_PI * 6378137;
		double upt = M_PI * 6378137 / pow(2, z-1);
		
		double geomap[6];
		geomap[0] = x * upt - adj;
		geomap[1] = upt / mFormat->w();
		geomap[2] = 0;
		geomap[3] = (y+1) * upt - adj;
		geomap[4] = 0;
		geomap[5] = upt / -mFormat->h();
		
		char buffer[128];
		sprintf(buffer, "%s/%i/%i/%i.%s", mTarget, z, x, y, mFormat->extension());
		if(mResume && isfile(buffer)) return NULL;
		log("Writing %s...\n", buffer);
		
		GDALDataset* ret = mFormat->create(buffer);
		ret->SetProjection(mDstProj);
		ret->SetGeoTransform(geomap);
		return ret;
	}
	
	void GoogleTiler::loadQuad(GDALDataset* quad, int x, int y, int dx, int dy) const
	{
		int w = mFormat->w();
		int h = mFormat->h();
		int b = mFormat->b();
		char data[w * h * b];
		char buffer[128];
		
		sprintf(buffer, "%s/%i/%i/%i.%s", mTarget, mRegion.z+1, 2*x+dx, 2*y+dy, mFormat->extension());
		if(isfile(buffer))
		{
			GDALDataset* temp = (GDALDataset*) GDALOpen(buffer, GA_ReadOnly);
			temp->RasterIO(GF_Read,  0, 0, w, h, data, w, h, GDT_Byte, b, NULL, 0, 0, 0);
			quad->RasterIO(GF_Write, dx*w, (1-dy)*h, w, h, data, w, h, GDT_Byte, b, NULL, 0, 0, 0);
			GDALClose(temp);
		}
	}

	GDALDataset* GoogleTiler::quadRaster(int i, char* path)	const
	{
		int x = i % mRegion.w + mRegion.x;
		int y = i / mRegion.w + mRegion.y;
		
		sprintf(path, "%s/%i/%i/%i.mem", mTarget, mRegion.z, x, y);
		if(mResume && isfile(path)) return NULL;
		GDALDataset* quad = mMemory->create(path);
		
		loadQuad(quad, x, y, 0, 0);
		loadQuad(quad, x, y, 1, 0);
		loadQuad(quad, x, y, 0, 1);
		loadQuad(quad, x, y, 1, 1);
		
		sprintf(path, "%s/%i/%i/%i.%s", mTarget, mRegion.z, x, y, mFormat->extension());
		return quad;
	}
}

