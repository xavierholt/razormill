#ifndef RAZORMILL_TILER_H
#define RAZORMILL_TILER_H

#include "Format.h"
#include "Region.h"

#include <atomic>
#include <iostream>
#include <thread>

#define MARKER() std::cout << std::this_thread::get_id() << ' ' << __FUNCTION__ << ' ' << __FILE__ << ' ' << __LINE__ << '\n';

namespace Razormill
{
	class Tiler
	{
	protected:
		GDALDataset*  mSource;
		const char*   mTarget;
		const char*   mSrcProj;
		char*         mDstProj;
		const Format* mFormat;
		const Format* mMemory;
		
		int           mNThreads;
		int           mMinZoom;
		int           mMaxZoom;
		bool          mResume;
		
		Region           mRegion;
		std::atomic_long mIndex;
		
	protected:
		void createDir(char* target) const;
		void baseWorker();
		void buildBase();
		void buildZoom();
		void zoomWorker();
		int  lastIndex() const;
		int  nextIndex();
		
		virtual void		 generateDirs()		const = 0;
		virtual GDALDataset* baseRaster(int i)     const = 0;
		virtual GDALDataset* quadRaster(int i)	const = 0;
		virtual void         calcRegion(int z)       = 0;
		virtual int          calcZoom()        const = 0;
		
	public:
		Tiler(const char* projection, const char* source, const char* target, Format* format);
		~Tiler();
		
		void run();
		void setNThreads(int n);
		void setResume(bool resume);
		void setZoom(int min, int max);
	};
}

#endif

