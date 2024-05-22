# Use emcc to compile c/c++ code to wasm and generate web page
all:
	emcc -o MetroGenerator.html \
	src/main.cpp src/metro.cpp \
	-Wall -std=c++20 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os \
	-I. -I build/_deps/raylib-src/src -I build/_deps/raylib-src/src/external -I third_party/random/include -I src \
	-L. -L build/_deps/raylib-src/src -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1  \
	--shell-file build/_deps/raylib-src/src/shell.html build/_deps/raylib-src/src/libraylib.a \
	-DPLATFORM_WEB \
	-s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' -s EXPORTED_RUNTIME_METHODS=ccall

clean:
	rm -f *.html
	rm -f *.js
	rm -f *.wasm