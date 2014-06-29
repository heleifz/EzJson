INCLUDES = $(wildcard ezjson/include/*.h)

runtest : test/test.cpp ezjson.so
	$(CXX) -O1 --std=c++11 -Iezjson test/test.cpp ezjson.so -o runtest
	./runtest

ezjson.so : ezjson/ezjson.cpp ezjson/ezjson.h ${INCLUDES}
	$(CXX) -O1  -fPIC -shared --std=c++11 -Iinclude ezjson/ezjson.cpp -o ezjson.so
