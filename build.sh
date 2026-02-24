echo "编译中"
x86_64-w64-mingw32-g++ main.cpp -o main.exe \
                       -I./include \
                       -L./lib \
                       -lgraphics \
                       -lgdiplus -luuid -lmsimg32 -lgdi32 -limm32 -lole32 -loleaut32 -lwinmm \
                       -mwindows \
                       -static
echo "编译完成"
