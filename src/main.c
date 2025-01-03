#include "include/common.h"
#include "include/chunk.h"
#include "include/debug.h"

int main(int argc, const char* argv[]) {
    Chunk chunk;
    initChunk(&chunk);                  //  Initialize the chunk
    writeChunk(&chunk, OP_RETURN);      // Write the byte to the chunk

    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);                  // Free the chunk
    return 0;
}