rm BFSDISK 
rm *.gch
cp ../BFSDISK .
gcc *.c *.h -o main
./main
rm BFSDISK 
cp ../BFSDISK .
rm main

# rm BFSDISK 
# make clean
# cp ../BFSDISK .
# make all
# ./main
# make clean
