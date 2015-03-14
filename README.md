The objective of this framework is to provide a simple API to declare independent tasks that are executed for each combination of a n-dimensional range of indexes. Thus, code can be generated at compilation time to automatically parallelize such operations by using the computing resources available on the machine.

You can find a description of the API and some examples here

Compatibility note: map-reduce is based on a number of features provided by C++11 (e.g. variadic templates). Currently it is known to work with g++-4.7 using the flag -std=c++11
