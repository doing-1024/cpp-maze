"g++.exe" main.cpp -o main.exe \
                       -I./include \
                       -L./lib \
                       -lgraphics \
                       -lgdiplus -luuid -lmsimg32 -lgdi32 -limm32 -lole32 -loleaut32 -lwinmm \
                       -mwindows \
                       -static
