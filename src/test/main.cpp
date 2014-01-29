#include "../razormill.h"

#include <gdal.h>
#include <unistd.h>
#include <string.h>

using namespace Razormill;

int main(int argc, char **argv)
{
	GDALAllRegister();
	
	const char* source		= NULL;
	const char* target		= NULL;
	Format*     format		= NULL;
	int         minzoom		= 15;
	int			maxzoom		= 15;
	int			threadcount = 1;
	bool		resume		= false;
	bool		verbose		= false;
	int			flag;
	
	
	while((flag = getopt (argc, argv, "rz:f:t:v")) != -1)
	{
		switch(flag)
		{
			case 'r':
				resume = true;
				break;
			case 'v':
				verbose = true;
				break;
			case 'f':
				if(optarg == "GeoTIFF")
				{
					format = new Razormill::GeoTIFF(256, 256, 3);
				}
				else if(optarg == "PNG")
				{
					format = new Razormill::PNG(256, 256, 3);
				}
				else
				{
					printf("Currently acceptable formats are: \"GeoTIFF\" and \"PNG\"\nDefaulting format to GeoTIFF.\n");
				}
				break;
			case 't':
				threadcount = atoi(optarg);
				break;
			case 'z':
				int read = sscanf(optarg, "%i-%i", &minzoom, &maxzoom);
				if(read == 1)
				{
					maxzoom = minzoom;
				}
				else if(read == 2)
				{
					// Do nothing
				}
				else
				{
					printf("Zoom argument of incorrect format. Please use one integer, or two integers separated by a hyphen.\nDefaulting to zoom level 15.\n");
				}
				break;
		}
	}
	
	if(format == NULL)
	{
		format = new Razormill::GeoTIFF(256, 256, 3);
	}
	
	if(argc - optind < 2)
	{
		printf("USAGE: %s [options] <source-file> <target-dir>\n", argv[0]);
		exit(-1);
	}
	
	source = argv[optind];
	target = argv[optind+1];
	
	Razormill::GoogleTiler tiler(source, target, format);
	tiler.setNThreads(threadcount);
	tiler.setResume(resume);
	tiler.setZoom(minzoom, maxzoom);
	tiler.setVerbose(verbose);
	tiler.run();
	
	return 0;
}

