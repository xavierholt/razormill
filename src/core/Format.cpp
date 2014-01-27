#include "Format.h"

#include <gdal_priv.h>

namespace Razormill
{
	Format::Format(const char* name, const char* driver, const char* extension, int w, int h, int b)
	{
		mName      = name;
		mDriver    = GetGDALDriverManager()->GetDriverByName(driver);
		mExtension = extension;
		mTileW     = w;
		mTileH     = h;
		mTileB     = b;
	}
	
	Format::~Format()
	{
		// Nothing to do.
	}
	
	GDALDataset* Format::create(const char* name) const
	{
		return mDriver->Create(name, mTileW, mTileH, mTileB, GDT_Byte, NULL);
	}
}

