#!/bin/sh

# setup disk and test file
cc main.cpp -o test_compiled -lm
rm test_file.txt
echo "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccdddddddddddddddddddddddddddddddddddddddddddddddd:)" > test_file.txt
./test_compiled 1 8 test_disk

# move test file to disk
./test_compiled 2 test_file.txt test_disk
./test_compiled 6 test_disk
./test_compiled 7 test_disk
mv test_file.txt test_file2.txt

# move test file from disk
./test_compiled 3 test_disk test_file.txt
# same as before
cat test_file2.txt
cat test_file.txt

# cant move test file to disk because it is already there 
./test_compiled 2 test_file.txt test_disk
./test_compiled 6 test_disk
./test_compiled 7 test_disk

# move second test file to disk (it allocates new blocks)
./test_compiled 2 test_file2.txt test_disk
./test_compiled 6 test_disk
./test_compiled 7 test_disk

# remove first test file from disk (it deallocates blocks)
./test_compiled 5 test_file.txt test_disk
./test_compiled 6 test_disk
./test_compiled 7 test_disk

# again move first test file to disk (it allocates deallocated blocks)
./test_compiled 2 test_file.txt test_disk
./test_compiled 6 test_disk
./test_compiled 7 test_disk

# remove disk
./test_compiled 4 test_disk

rm test_file.txt
rm test_file2.txt
rm test_compiled