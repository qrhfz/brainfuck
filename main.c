#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 30000
#define INSTRUCTION_INITIAL_SIZE 64

typedef enum
{
  OP_MOVE,
  OP_ADD,
  OP_BEGIN_LOOP,
  OP_END_LOOP,
  OP_IN,
  OP_OUT,
  OP_END,
} Op;

typedef struct
{
  Op op;
  uint64_t payload;
} Instruction;

typedef struct
{
  size_t length;
  size_t capacity;
  Instruction* array;
} InstructionArray;

InstructionArray
new_instruction_array()
{
  Instruction* array = calloc(INSTRUCTION_INITIAL_SIZE, sizeof(Instruction));
  InstructionArray instructions = {
    .length = 0,
    .capacity = INSTRUCTION_INITIAL_SIZE,
    .array = array,
  };
  return instructions;
}

void
push_instruction(InstructionArray* array, Instruction value)
{
  if (array->length == array->capacity) {
    array->array =
      realloc(array->array, sizeof(Instruction) * array->capacity * 2);

    array->capacity *= 2;
  }

  array->array[array->length] = value;
  array->length++;
}

typedef uint64_t Cell;
typedef struct
{
  size_t size;
  size_t current;
  Cell* array;
} MemoryArray;

MemoryArray
new_memory_array()
{
  Cell* array = calloc(INITIAL_SIZE, sizeof(Cell));
  MemoryArray mem = {
    .size = INITIAL_SIZE,
    .current = 0,
    .array = array,
  };
  return mem;
}

void
free_memory_array(MemoryArray* mem)
{
  Cell* array = mem->array;
  free(array);
  mem->array = NULL;
}

void
grow_to_right(MemoryArray* mem)
{
  mem->size *= 2;
  mem->array = realloc(mem->array, mem->size * sizeof(Cell));
}

void
grow_to_left(MemoryArray* mem)
{

  size_t old_size = mem->size;
  mem->size *= 2;
  mem->array = realloc(mem->array, mem->size * sizeof(Cell));

  memmove(mem->array + old_size, mem->array, sizeof(Cell) * old_size);
  memset(mem->array, 0, sizeof(Cell) * old_size);
  mem->current += old_size;
}

void
move(MemoryArray* mem, long int count)
{
  while (count < 0 && -count > (long int)mem->current) {
    grow_to_left(mem);
  }

  mem->current += count;

  while (mem->current >= mem->size) {
    grow_to_right(mem);
  }
}

size_t
jump_forward(Instruction* instructions, size_t i)
{
  i++;

  int count = 0;
  for (;;) {
    if (instructions[i].op == OP_END) {
      break;
    }

    if (instructions[i].op == OP_END_LOOP && count == 0) {
      i++;
      break;
    }

    if (instructions[i].op == OP_BEGIN_LOOP) {
      count++;
    } else if (instructions[i].op == OP_END_LOOP) {
      count--;
    }

    i++;
  }

  return i;
}

size_t
jump_back(Instruction* instructions, size_t i)
{
  i--;

  int count = 0;
  for (;;) {
    if (i == 0) {
      break;
    }

    if (instructions[i].op == OP_BEGIN_LOOP && count == 0) {
      i++;
      break;
    }

    if (instructions[i].op == OP_END_LOOP) {
      count++;
    } else if (instructions[i].op == OP_BEGIN_LOOP) {
      count--;
    }

    i--;
  }

  return i;
}

void
interpret(InstructionArray instructions)
{
  MemoryArray mem = new_memory_array();
  for (size_t i = 0; i < instructions.length;) {
    // printf("%d %ld\n", instructions.array[i].op, i);
    switch (instructions.array[i].op) {
      case OP_MOVE:
        move(&mem, instructions.array[i].payload);
        i++;
        break;
      case OP_ADD:
        mem.array[mem.current] += instructions.array[i].payload;
        i++;
        break;
      case OP_BEGIN_LOOP:
        if (mem.array[mem.current] == 0) {
          i = jump_forward(instructions.array, i);
          break;
        }
        i++;
        break;
      case OP_END_LOOP:
        if (mem.array[mem.current] != 0) {
          i = jump_back(instructions.array, i);
          break;
        }
        i++;
        break;
      case OP_IN:
        mem.array[mem.current] = getchar();
        i++;
        break;
      case OP_OUT:
        putchar(mem.array[mem.current]);
        i++;
        break;
      default:
        i++;
        break;
    }
  }
}

size_t
instruction_move(InstructionArray* array, const char* source, size_t i)
{

  uint64_t payload = 0;
  while (source[i] == '>' || source[i] == '<') {
    payload += (source[i] == '>') ? 1 : -1;
    i++;
  }

  Instruction inst = {
    .op = OP_MOVE,
    .payload = payload,
  };

  push_instruction(array, inst);
  return i;
}

size_t
instruction_add(InstructionArray* array, const char* source, size_t i)
{

  uint64_t payload = 0;
  while (source[i] == '+' || source[i] == '-') {
    payload += (source[i] == '+') ? 1 : -1;
    i++;
  }

  Instruction inst = {
    .op = OP_ADD,
    .payload = payload,
  };

  push_instruction(array, inst);
  return i;
}

size_t
instruction_in(InstructionArray* array, size_t i)
{
  uint64_t payload = 0;

  Instruction inst = {
    .op = OP_IN,
    .payload = payload,
  };

  push_instruction(array, inst);
  return i + 1;
}

size_t
instruction_out(InstructionArray* array, size_t i)
{
  uint64_t payload = 0;

  Instruction inst = {
    .op = OP_OUT,
    .payload = payload,
  };

  push_instruction(array, inst);
  return i + 1;
}

size_t
instruction_begin_loop(InstructionArray* array, size_t i)
{
  uint64_t payload = 0;

  Instruction inst = {
    .op = OP_BEGIN_LOOP,
    .payload = payload,
  };

  push_instruction(array, inst);
  return i + 1;
}

size_t
instruction_end_loop(InstructionArray* array, size_t i)
{
  uint64_t payload = 0;

  Instruction inst = {
    .op = OP_END_LOOP,
    .payload = payload,
  };

  push_instruction(array, inst);
  return i + 1;
}

InstructionArray
scan(const char* source)
{
  InstructionArray array = new_instruction_array();
  for (size_t i = 0; i < strlen(source);) {
    switch (source[i]) {
      case '>':
      case '<':
        i = instruction_move(&array, source, i);
        break;
      case '+':
      case '-':
        i = instruction_add(&array, source, i);
        break;
      case '.':
        i = instruction_out(&array, i);
        break;
      case ',':
        i = instruction_in(&array, i);
        break;
      case '[':
        i = instruction_begin_loop(&array, i);
        break;
      case ']':
        i = instruction_end_loop(&array, i);
        break;
      default:
        i++;
        break;
    }
  }
  Instruction end_instruction = { .op = OP_END, .payload = 0 };
  push_instruction(&array, end_instruction);

  return array;
}

int
main(int argc, char const* argv[])
{

  if (argc == 1) {
    // repl();
    exit(0);
  } else if (argc == 2) {

    char* source = 0;
    long length;
    FILE* f = fopen(argv[1], "rb");

    if (f) {
      fseek(f, 0, SEEK_END);
      length = ftell(f);
      fseek(f, 0, SEEK_SET);
      source = malloc(length);
      if (source) {
        fread(source, 1, length, f);

        InstructionArray instructions = scan(source);
        interpret(instructions);
      }
      fclose(f);
    }
    free(source);
  }

  return 0;
}
