g++ main.cpp -o cybin $(pkg-config --cflags --libs luajit) -lm -ldl -lsoundio -lsndfile -ldl -lc -lglut -lGL -lGLU -DGL_GLEXT_PROTOTYPES -pthread
