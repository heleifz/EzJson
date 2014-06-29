# EzJSON : A Fast and Simple JSON Library 

![](https://travis-ci.org/heleifz/EzJson.svg?branch=master)

# Introduction

As it name suggests, EzJSON is really easy to use. Use JSON string to contruct an EzJSON object, then you can use it like an array / map.

```c++
Ez::JSON j("[1, {\"foo\" : [3, 4]}, 2]");
std::cout << j[1]["foo"].asDouble();

```