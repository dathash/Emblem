emblem:
	clang++ ../src/*.cpp -g -O0 -std=c++14 -o em -I/opt/homebrew/include/SDL2 -I../ext/ -L /opt/homebrew/lib -lSDL2_image -lSDL2 -lSDL2_ttf

withimgui:
	clang++ ../src/*.cpp ../ext/imgui/*.cpp ../ext/imgui/backends/imgui_impl_sdlrenderer.cpp -I../ext/ ../ext/imgui/backends/imgui_impl_sdl.cpp -g -O0 -std=c++14 -o em -I/opt/homebrew/include/SDL2 -I../ext/imgui/ -I../ext/imgui/backends/ -L /opt/homebrew/lib -lSDL2_image -lSDL2 -lSDL2_ttf
