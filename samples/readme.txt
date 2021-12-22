g++ -c wrapper.cpp avc_hevc_parser.cpp bit_reader.cpp -I./include
ar -r libbase.a wrapper.o avc_hevc_parser.o bit_reader.o
g++ -o main main.c -L. -lbase -lmp4v2
或者
g++ main.c -o main wrapper.cpp avc_hevc_parser.cpp bit_reader.cpp -I./include -L. -lmp4v2

