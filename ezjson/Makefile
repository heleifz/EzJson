INCLUDES = $(wildcard include/*.h)

ezjson.so : ezjson.cpp ezjson.h ${INCLUDES}
	g++ -O1 -shared --std=c++11 -Iinclude -o ezjson.so ezjson.cpp 