Build instructions:

git clone https://github.com/ClassiCube/ClassiCube

cd ClassiCube

cc -fno-math-errno src/*.c -o ClassiCube -rdynamic -lpthread -lX11 -lXi -lGL -ldl -lm
