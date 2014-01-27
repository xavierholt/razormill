#include "Google.h"

#include <gdal_priv.h>
#include <gdalwarper.h>

#include <cmath>

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
		
		std::cout << "World Region:\n" <<
		"  n: " << xmax << '\n' <<
		"  e: " << ymax << '\n' <<
		"  s: " << xmin << '\n' <<
		"  w: " << ymin << '\n';
		
		double adj = M_PI * 6378137;
		double upt = M_PI * 6378137 / pow(2, z-1);
		
		mRegion.x = floor((xmin + adj) / upt);
		mRegion.y = floor((ymin + adj) / upt);
		mRegion.z = z;
		mRegion.w = floor((xmax + adj) / upt) - mRegion.x + 1;
		mRegion.h = floor((ymax + adj) / upt) - mRegion.y + 1;
		mRegion.n = mRegion.w * mRegion.h;
		
		
		std::cout << "Tile Region:\n" <<
		"  x: " << mRegion.x << '\n' <<
		"  y: " << mRegion.y << '\n' <<
		"  z: " << mRegion.z << '\n' <<
		"  w: " << mRegion.w << '\n' <<
		"  h: " << mRegion.h << '\n' <<
		"  n: " << mRegion.n << '\n';
	}
	
	int GoogleTiler::calcZoom() const
	{
		return -1;
	}

/**
 * Returns a new GDAL raster for the indexed tile.
 *
 * @param  i  The linear index of the desired tile.
 * @result    A new raster with its SRID and GeoTransform set appropriately.
 */
	GDALDataset* GoogleTiler::raster(int i) const
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
		geomap[3] = y * upt - adj;
		geomap[4] = 0;
		geomap[5] = upt / mFormat->h();
		
		char buffer[128];
		//sprintf(buffer, "%s/%i/%i/%i.%s", base, z, x, y, mFormat->extension());
		sprintf(buffer, "%s/%ix%i.%s", mTarget, x, y, mFormat->extension());
		std::cout << "Writing " << buffer << "...\n";
		
		GDALDataset* ret = mFormat->create(buffer);
		ret->SetProjection(mDstProj);
		ret->SetGeoTransform(geomap);
		return ret;
	}
}

