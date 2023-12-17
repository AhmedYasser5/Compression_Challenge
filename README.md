# Compression using Huffman

This is a program to compress and decompress text files using Huffman encoding that relies on variable-length encoding and frequency of characters used in the file

## Usage

To use the program, run the following line first to compile it:
```bash
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j
```

To run the program, run the following command:
```bash
$ ./build/bin/Compression <operation> <filename>
```

The supported operations are:
1. `c`: compress the file, the resulting file will be of the same name but suffixed with `.huf`
2. `d`: decompress the file, the resulting file will be of the same name but suffixed with `.fuh`

## Tests

To compile the tests, run the following line:
```bash
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON && cmake --build build -j
```

To run the tests, run the following command:
```bash
$ ctest --test-dir build
```

## TODO

- Support unicode characters
- Support multiple files
