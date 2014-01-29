#ifndef RAZORMILL_TILERS_GOOGLE_H
#define RAZORMILL_TILERS_GOOGLE_H

#include "../Tiler.h"

namespace Razormill
{
	class GoogleTiler : public Tiler
	{
	protected:
		virtual void		 generateDirs()		const;
		virtual GDALDataset* baseRaster(int i)	const;
		virtual GDALDataset* quadRaster(int i)	const;
		virtual void         calcRegion(int z);
		virtual int          calcZoom()			const;
		
	public:
		GoogleTiler(const char* source, const char* target, Format* format);
	};
}

#endif

