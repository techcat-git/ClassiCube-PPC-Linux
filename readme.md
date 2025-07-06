Build instructions:

cc -fno-math-errno src/*.c -o ClassiCube -rdynamic -lpthread -lX11 -lXi -lGL -ldl -lm
