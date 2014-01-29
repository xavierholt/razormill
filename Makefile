CXXFLAGS = -std=c++11 -O3 -I/usr/include/gdal
LIBS     = -lgdal

razormill:\
		src/test/main.o\
		src/core/Format.o\
		src/core/Tiler.o\
		src/core/tilers/Google.o
	${CXX} $(CXXFLAGS) -o $@ $+ $(LIBS)
	
clean:
	find . -name '*.o' -delete
	rm razormill
	
*.o : *.cpp *.h
	${CXX} $(CXXFLAGS) -o $@ $<
