g++ main.cpp -o cybin $(pkg-config --cflags --libs luajit) -lm -ldl -lsndfile -ldl -lc -pthread -D NOJACK
