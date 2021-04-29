.PHONY: vendor

raytracer: src/main.cc
	g++ -std=c++11 src/main.cc -o raytracer

vendor/stb/stb.h:
	git clone https://github.com/nothings/stb vendor/stb

vendor: vendor/stb/stb.h
	echo "dependencies fetched"

clean:
	rm -f raytracer src/*.o
