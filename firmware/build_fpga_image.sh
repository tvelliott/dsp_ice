cd fpga/src/
touch *.v
make -f Makefile
cd ../../
java compress_fpga_image fpga/src/fpga_top.bin >fpga_image.h
touch fpga.c
