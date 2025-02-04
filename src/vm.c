#include "include/common.h"  
#include "include/vm.h"      // Virtual machine (VM) definitions
#include "include/debug.h"
#include <stdio.h>
#include <stdlib.h>

VM vm;  // Global VM instance

static void resetStack() {
    vm.stackTop = vm.stack;
}

// Initialize the virtual machine
void initVM() {
    resetStack();
}

// Free resources used by the virtual machine
void freeVM() {
    // Cleanup logic goes here
}


// stack operation
/**
 * @brief Pushes a value onto the stack.
 *
 * This function takes a Value and pushes it onto the stack for further
 * processing or evaluation within the virtual machine.
 *
 * @param value The Value to be pushed onto the stack.
 */
void push(Value value) {
    if (vm.stackTop > vm.stack + STACK_MAX) {
        fprintf(stderr, "Stack overflow\n");
        exit(1);
    }

    *vm.stackTop = value;
    vm.stackTop++;
}


/**
 * @brief Pops a value from the stack.
 *
 * This function removes the top value from the stack and returns it.
 * It is assumed that the stack is not empty when this function is called.
 *
 * @return The value that was popped from the stack.
 */
Value pop() {
    if (vm.stackTop == vm.stack) {
        fprintf(stderr, "Stack underflow!\n");
        exit(1); // Terminate with an error
    }
    vm.stackTop--;
    return *vm.stackTop;
}



/**
 * @brief Reads a long index from the bytecode.
 *
 * This function reads a 32-bit unsigned integer from the bytecode stream.
 *
 * @return The 32-bit unsigned integer read from the bytecode.
 */
static uint32_t readLongIndex() {
    uint32_t index = 0;
    index |= (uint32_t)(*vm.ip++) << 16;  // Read the first byte (most significant)
    index |= (uint32_t)(*vm.ip++) << 8;   // Read the second byte
    index |= (uint32_t)(*vm.ip++);        // Read the third byte (least significant)
    return index;
}



// Execute bytecode instructions
static InterpretResult run() {
    /**
     * @brief Macro to read a constant value from the chunk's constants.
     *
     * This macro reads a byte from the bytecode and uses it as an index to 
     * access the corresponding constant value from the chunk's constants array.
     *
     * @return The constant value from the chunk's constants array.
     */
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    
    /**
     * @brief Macro to perform a binary operation.
     *
     * This macro takes a binary operator as an argument and performs the
     * corresponding operation. It is typically used to simplify the implementation
     * of binary operations in the virtual machine.
     *
     * @param op The binary operator to be used in the operation.
     */
    #define BINARY_OP(op) \
        do { \
            Value b = pop(); \
            Value a = pop(); \
            push(a op b); \
        } while (false)


    // Read the next byte from the instruction pointer using (*) and advance it
    #define READ_BYTE() (*vm.ip++)

    // Continuously execute instructions
    for (;;) {

        #ifdef DEBUG_TRACE_EXECUTION
            printf("          ");
            for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }
            printf("\n");

            printf("[DEBUG] Executing instruction at offset %ld\n", (vm.ip - vm.chunk->code));
            disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));    // calculates the offset of the current ip within the bytecode array

        #endif

        // printf("Executing instruction at offset %d\n", (int)(vm.ip - vm.chunk->code));

        uint8_t instruction = READ_BYTE();  // Fetch the next instruction
        switch (instruction) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);     // loads a value into the stack
                break;
            }

            case OP_CONSTANT_LONG: {
                // read a 3-byte index and fetch the constant
                uint32_t index = readLongIndex();
                Value constant = vm.chunk->constants.values[index];
                push(constant);
                break;
            }

            // for binary operations
            case OP_ADD: {
                BINARY_OP(+);
                break;
            }

            case OP_SUBTRACT: {
                BINARY_OP(-);
                break;
            }

            case OP_MULTIPLY: {
                BINARY_OP(*);
                break;
            }

            case OP_DIVIDE: {
                BINARY_OP(/);
                break;
            }

            // for unary operators
            case OP_NEGATE: {
                push(-pop());       // negates the popped value (unary operator)
                break;
            }

            case OP_RETURN: {
                // printValue(pop());
                printf("\n");
                return INTERPRET_OK;  
            }  
        }
    }

    // Clean up the macro
    #undef READ_BYTE  
    #undef READ_CONSTANT
    #undef BINARY_OP
}

// Interpret a chunk of bytecode
InterpretResult interpret(Chunk* chunk) {
    vm.chunk = chunk;          // Set the current chunk
    vm.ip = vm.chunk->code;    // Set instruction pointer to the start of the chunk
    return run();              
}