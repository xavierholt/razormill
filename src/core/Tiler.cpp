#include "Tiler.h"
#include "formats/Memory.h"

#include <gdal_priv.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>
#include <cpl_string.h>

#include <thread>

namespace Razormill
{
	Tiler::Tiler(const char* projection, const char* source, const char* target, Format* format)
	{
		mSource = (GDALDataset*) GDALOpen(source, GA_ReadOnly);
		mSrcProj = mSource->GetProjectionRef();
		
		mTarget   = target;
		mFormat   = format;
		mMemory   = new Memory(2 * format->w(), 2 * format->h(), format->b());
		
		mNThreads = std::thread::hardware_concurrency();
		mMinZoom  = 0;
		mMaxZoom  = -1;
		mResume   = false;
		
		OGRSpatialReference ogref;
		ogref.SetWellKnownGeogCS(projection);
		ogref.exportToWkt(&mDstProj);
	}
	
	Tiler::~Tiler()
	{
		GDALClose(mSource);
	}
	
	void Tiler::createDir(char* target) const
	{
		int status = mkdir(target, 0755);
		if(status != 0)
		{
			if(errno != EEXIST)
			{
				printf("Directory creation failed:\n\%s\n", strerror(errno));
				exit(-1);
			}
		}
	}
	
/**
 * 
 */
	
	void Tiler::baseWorker()
	{
		int i = nextIndex();
		if(i >= lastIndex())
		{
			return;
		}
		
		GDALWarpOptions*  warp_opts = GDALCreateWarpOptions();
		GDALWarpOperation warp_oper;
		CPLStringList     warp_data;
		warp_data.AddNameValue("INIT_DEST", "0");
		
		warp_opts->eResampleAlg     = GRA_Bilinear;
		warp_opts->hSrcDS           = mSource;
		warp_opts->nBandCount       = mFormat->b();
		warp_opts->panSrcBands      = (int*) malloc(sizeof(int) * mFormat->b());
		warp_opts->panDstBands      = (int*) malloc(sizeof(int) * mFormat->b());
		warp_opts->papszWarpOptions = warp_data.StealList();
		warp_opts->pfnTransformer   = GDALGenImgProjTransform;
		
		
		for(int b = 0; b < mFormat->b(); ++b)
		{
			warp_opts->panSrcBands[b] = b + 1;
			warp_opts->panDstBands[b] = b + 1;
		}
		
		while(i < lastIndex())
		{
			GDALDataset* target = baseRaster(i);
			
			if(target != NULL)
			{
				warp_opts->hDstDS = target;
				warp_opts->pTransformerArg = GDALCreateGenImgProjTransformer(mSource, mSrcProj, target, mDstProj, FALSE, 0.0, 1 );
				warp_oper.Initialize(warp_opts);
				warp_oper.WarpRegion(0, 0, mFormat->w(), mFormat->h());
				GDALClose(target);
			}
			
			i = nextIndex();
		}
	}
	
	void Tiler::buildBase()
	{
		mIndex = 0;
		generateDirs();
		std::thread** threads = new std::thread*[mNThreads];
		
		for(int i = 0; i < mNThreads; ++i)
		{
			log("Spawning worker (this = %p)...\n", this);
			threads[i] = new std::thread(&Tiler::baseWorker, this);
		}
		
		for(int i = 0; i < mNThreads; ++i)
		{
			threads[i]->join();
			delete threads[i];
		}
		
		delete [] threads;
	}
	
	void Tiler::buildZoom()
	{
		std::thread** threads = new std::thread*[mNThreads];
		
		while(mRegion.upscale() >= mMinZoom)
		{
			mIndex = 0;
			generateDirs();
			
			for(int i = 0; i < mNThreads; ++i)
			{
				threads[i] = new std::thread(&Tiler::zoomWorker, this);
			}
			
			for(int i = 0; i < mNThreads; ++i)
			{
				threads[i]->join();
				delete threads[i];
			}
		}
		
		delete [] threads;
	}
	
	int Tiler::lastIndex() const
	{
		return mRegion.n;
	}
	
	int Tiler::nextIndex()
	{
		return mIndex.fetch_add(1);
	}
	
	bool Tiler::isfile(const char* path) const
	{
		struct stat buffer;
		return (stat(path, &buffer) == 0);
	}
	
	void Tiler::run()
	{
		calcRegion(mMaxZoom);
		buildBase();
		buildZoom();
	}
	
	void Tiler::setNThreads(int n)
	{
		mNThreads = n;
	}
	
	void Tiler::setResume(bool resume)
	{
		mResume = resume;
	}
	
	void Tiler::setZoom(int min, int max)
	{
		mMinZoom = min;
		mMaxZoom = max;
	}
	
	void Tiler::setVerbose(bool verbose)
	{
		mVerbose = verbose;
	}
	
	void Tiler::log(const char* format, ...) const
	{
		if(mVerbose)
		{
			va_list argptr;
			va_start(argptr,format);
			vprintf(format, argptr);
			va_end(argptr);
		}
	}
	
	void Tiler::zoomWorker()
	{
		int i = nextIndex();
		if(i >= lastIndex())
		{
			return;
		}
		
		char buffer[128];
		
		while(i < lastIndex())
		{
			GDALDataset* source = quadRaster(i, buffer);
			
			if(source != NULL)
			{
				log("Writing %s...\n", buffer);
				GDALDataset* target = mFormat->create(buffer);
				
				for(int i = 1; i <= mFormat->b(); ++i)
				{
					GDALRasterBand* sband = source->GetRasterBand(i);
					GDALRasterBand* tband = target->GetRasterBand(i);
					GDALRegenerateOverviews(sband, 1, (void**) &tband, "AVERAGE", NULL, NULL);
				}
				
				GDALClose(source);
				GDALClose(target);
			}
			
			i = nextIndex();
		}
	}
}

