g++ main.cpp $(pkg-config --cflags --libs luajit) -lm -ldl -lsoundio -pthread -pagezero_size 10000 -image_base 100000000
