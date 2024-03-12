g++ -c wrapper.cpp avc_hevc_parser.cpp bit_reader.cpp  -I../output/include
ar -r libbase.a wrapper.o avc_hevc_parser.o bit_reader.o
g++ -o main main.c -I../output/include -L../output/lib -L./ -lbase -lmp4v2
或者
g++ main.c -o main wrapper.cpp avc_hevc_parser.cpp bit_reader.cpp -I../output/include -L../output/lib -lmp4v2

