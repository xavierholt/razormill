#ifndef RAZORMILL_TILERS_GOOGLE_H
#define RAZORMILL_TILERS_GOOGLE_H

#include "../Tiler.h"

namespace Razormill
{
	class GoogleTiler : public Tiler
	{
	protected:
		void loadQuad(GDALDataset* quad, int x, int y, int dx, int dy) const;
		
		virtual GDALDataset* baseRaster(int i)	           const;
		virtual void         calcRegion(int z);
		virtual int          calcZoom()			           const;
		virtual void		 generateDirs()		           const;
		virtual GDALDataset* quadRaster(int i, char* path) const;
		
	public:
		GoogleTiler(const char* source, const char* target, Format* format);
	};
}

#endif

