# Compile all source files
mkdir dist/
gcc -c -I. -Iinclude nsigii-codec/nsigii_codec.c -o dist/nsigii_codec.o -lm
gcc -c -I. -Iinclude rift-000/rift_000_tokenizer.c -o dist/rift_000.o
gcc -c -I. -Iinclude rift-001/rift_001_process.c -o dist/rift_001.o
gcc -c -I. -Iinclude rift-333/rift_333_ast.c -o dist/rift_333.o
gcc -c -I. -Iinclude rift-444/rift_444_target.c -o dist/rift_444.o
gcc -c -I. -Iinclude rift-555/rift_555_bridge.c -o dist/rift_555.o
gcc -c -I. -Iinclude main.c -o dist/main.o

# Link executable
gcc dist/*.o -o rift -lm -lpthread
