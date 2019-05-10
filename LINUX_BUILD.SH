g++ main.cpp -o cybin $(pkg-config --cflags --libs luajit --libs jack) -lm -ldl -lsndfile -ldl -lc -pthread
