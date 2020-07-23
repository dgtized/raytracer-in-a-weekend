raytracer: src/main.cc
	g++ src/main.cc -o raytracer

clean:
	rm -f raytracer src/*.o
