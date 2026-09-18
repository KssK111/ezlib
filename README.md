# EZLIB - Collection of easy to understand, Rust-inspired C++ libraries

### How to use **Header-only libraries**
HOLs are self-contained, this means you can just include a single file into your project and it should work\
(However, for better code organisation libraries in this repo are not self-contained, dependencies are listed on the top of the files)
Not to get a linker error, you must also define an implementation macro, because HOLs only include the declarations by default\
This is because of the *One Definition Rule*, not to break the rule, you must only define the implementation macro once\
It will probably be in your main.cpp file, **before the inclusion of the header**\
In other files, include the header, without defining the macro
If you still get errors, try including just the header on the top of the file and implementation on the bottom

```cpp
// main.cpp
#define LIBNAME_IMPLEMENTATION
#include "path_to_lib/libname.h"

int main() {
/*
  Your code
*/
}
```

```cpp
// utils.h
#include "path_to_lib/libname.h"

/*
  Your code
*/
```
