g++ main.cpp -o cybin $(pkg-config --cflags --libs luajit) -lm -ldl -lsoundio -lsndfile -pthread
