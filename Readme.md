# mmx

My single header libraries for C/C++.

|library | lastest version | category | LoC | license | description
| --------------------- | ---- | -------- | --- | --- | --------------------------------
|**json.h** | 1.00 | parser | 848 | zlib | non-allocating json parser
|**sched.h** | 1.00 | multithreading | 699 | zlib | multithreaded task scheduler
|**web.h** | 1.00 | network | 1455 | BSD |  lightweight webserver
|**sdefl.h** | 1.00 | compression | 525 | public domain |  deflate compressor
|**sinfl.h** | 1.00 | decompression | 320 | public domain |  deflate decompressor

Total libraries: 5  
Total lines of C code: 4487

## FAQ

#### Why single-file headers?
Windows doesn't have standard directories where libraries
live. That makes deploying libraries in Windows a lot more
painful than open source developers on Unix-derivates generally
realize. (It also makes library dependencies a lot worse in Windows.)

There's also a common problem in Windows where a library was built
against a different version of the runtime library, which causes
link conflicts and confusion. Shipping the libs as headers means
you normally just compile them straight into your project without
making libraries, thus sidestepping that problem.

Making them a single file makes it very easy to just
drop them into a project that needs them. (Of course you can
still put them in a proper shared library tree if you want.)

Why not two files, one a header and one an implementation?
The difference between 10 files and 9 files is not a big deal,
but the difference between 2 files and 1 file is a big deal.
You don't need to zip or tar the files up, you don't have to
remember to attach *two* files, etc.

#### Where is the documentation?
Each file has documentation, basic ussage description and
examples at the top of the file. In addition each API function,
struct and member variables are documented as well.
Finally each library has a corresponding test file inside the
test directory for additional working examples.

#### Why C?
Personally I primarily use C instead of C++ and since I want to
support both C and C++ and C++ is not useable from C I therefore focus
on C.

#### Why C89?
I use C89 instead of C99/C11 for its portability between different compilers
and accessiblity for other languages.

## References
- [Sean Barretts single header libraries](https://github.com/nothings/stb)
- [Other single header libraries](https://github.com/nothings/stb/blob/master/docs/other_libs.md)
- [enkiTS: source implementation for mm_sched.h](https://github.com/dougbinks/enkiTS)
- [Webby: source implementation for mm_web.h](https://github.com/deplinenoise/webby)

