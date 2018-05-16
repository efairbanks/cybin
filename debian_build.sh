g++ main.cpp $(pkg-config --cflags --libs luajit) -lm -ldl -lsoundio -pthread
