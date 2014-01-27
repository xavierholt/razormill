#include "Tiler.h"

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
		mSource ->MarkAsShared();
		
		mTarget   = target;
		mFormat   = format;
		
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
	
	void Tiler::baseWorker()
	{
		printf("baseWorker: %p\n", this);
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
		
		MARKER();
		while(i < lastIndex())
		{
			GDALDataset* target = raster(i);
			
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
		MARKER();
		mIndex = 0;
		std::thread** threads = new std::thread*[mNThreads];
		
		for(int i = 0; i < mNThreads; ++i)
		{
			printf("Spawning worker (this = %p)...\n", this);
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
		MARKER();
		std::thread** threads = new std::thread*[mNThreads];
		
		while(mRegion.upscale() >= mMinZoom)
		{
			mIndex = 0;
			
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
	
	void Tiler::run()
	{
		calcRegion(mMaxZoom);
		buildBase();
		//buildZoom();
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
	
	void Tiler::zoomWorker()
	{
		//TODO!
	}
}

