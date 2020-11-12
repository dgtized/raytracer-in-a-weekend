raytracer: src/main.cc
	g++ -std=c++11 -pg src/main.cc -o raytracer

clean:
	rm -f raytracer src/*.o
