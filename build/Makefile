emblem:
	clang++ ../src/*.cpp -g -O0 -std=c++17 -o em -I/opt/homebrew/include/SDL2 -L /opt/homebrew/lib -lSDL2_image -lSDL2 -lSDL2_ttf

withimgui:
	clang++ ../src/*.cpp ../imgui/*.cpp ../imgui/backends/imgui_impl_sdlrenderer.cpp ../imgui/backends/imgui_impl_sdl.cpp -g -O0 -std=c++17 -o em -I/opt/homebrew/include/SDL2 -I../imgui/ -I../imgui/backends/ -L /opt/homebrew/lib -lSDL2_image -lSDL2 -lSDL2_ttf
