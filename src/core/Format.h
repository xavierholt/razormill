#ifndef RAZORMILL_FORMAT_H
#define RAZORMILL_FORMAT_H

class GDALDriver;
class GDALDataset;

namespace Razormill
{
	class Format
	{
	protected:
		const char* mName;
		GDALDriver* mDriver;
		const char* mExtension;
		int         mTileW;
		int         mTileH;
		int         mTileB;
		
	public:
		Format(const char* name, const char* driver, const char* extension, int w, int h, int b);
		~Format();
		
		int         b()         const {return mTileB;}
		const char* extension() const {return mExtension;}
		int         h()         const {return mTileH;}
		const char* name()      const {return mName;}
		int         w()         const {return mTileW;}
		
		GDALDataset* create(const char* name) const;
	};
}

#endif

