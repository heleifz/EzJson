# EzJSON

![](https://travis-ci.org/heleifz/EzJson.svg?branch=master)

## A Fast and minimalist JSON Library 

EzJSON is a fast and minimalist JSON library. 

## Installation

EzJSON need the compiler to support C++11. You can include the library source file in your project, or you can run ```make``` command to build a shared library (recommanded).

## Usage

As it name suggests, EzJSON is really easy to use. Use JSON string to contruct an EzJSON object, then you can access it like an array / map.

```c++
Ez::JSON j("[1, {\"foo\" : [3, 4]}, 2]");
std::cout << j[1]["foo"].asDouble();
```

You can modify the JSON tree by ```set()``` and ```remove()``` method.

```c++
Ez::JSON j("{}");
j.set("foo", "[1, 2, 3, 4]");
j["foo"].remove(2);
```

EzJSON has a baked-in pretty printing functionality.

```c++
Ez::JSON j("[1, {\"foo\" : [3, 4]}, 2]");
std::cout << j.serialize();
```

All EzJSON exceptions are derived from std::exception.

```c++
try
{
	Ez::JSON j("{");
}
catch (const std::exception& e)
{
	// get the error message
	std::cout << e.what();
}
```

# Performance

Inspired by rapidjson, EzJSON use a custom allocator to dramatically speed up the parse process. It's approximately 6x faster than dropbox's json11, and it took only 1.3 second for EzJSON to build complete AST for a very large (185MB) JSON file. 

## About

Released under the BSD Licence. (see license.txt)