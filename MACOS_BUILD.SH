g++ main.cpp -o cybin $(pkg-config --cflags --libs luajit) $(pkg-config --cflags --libs jack) -lm -ldl -lsndfile -framework Cocoa -pthread -pagezero_size 10000 -image_base 100000000
