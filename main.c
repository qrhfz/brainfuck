#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 30000

typedef unsigned int Cell;
typedef struct
{
  Cell* array;
  size_t size;
  size_t current;
} MemoryArray;

MemoryArray
new_memory_array()
{
  int* array = calloc(INITIAL_SIZE, sizeof(int));
  MemoryArray mem = (MemoryArray){
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

int
get_current_value(MemoryArray* mem)
{
  int curr = mem->current;
  return mem->array[curr];
}

void
inc(MemoryArray* mem)
{
  mem->array[mem->current]++;
}

void
dec(MemoryArray* mem)
{
  mem->array[mem->current]--;
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
next(MemoryArray* mem)
{
  if (mem->current >= mem->size - 1) {
    grow_to_right(mem);
  }
  mem->current++;
}

void
prev(MemoryArray* mem)
{
  if (mem->current <= 0) {
    grow_to_left(mem);
  }
  mem->current--;
}

size_t
jump_forward(const char* source, size_t i)
{
  i++;

  int count = 0;
  for (;;) {
    if (source[i] == '\0') {
      break;
    }

    if (source[i] == ']' && count == 0) {
      i++;
      break;
    }

    if (source[i] == '[') {
      count++;
    } else if (source[i] == ']') {
      count--;
    }

    i++;
  }

  return i;
}

size_t
jump_back(const char* source, size_t i)
{
  i--;

  int count = 0;
  for (;;) {
    if (i == 0) {
      break;
    }

    if (source[i] == '[' && count == 0) {
      i++;
      break;
    }

    if (source[i] == ']') {
      count++;
    } else if (source[i] == '[') {
      count--;
    }

    i--;
  }

  return i;
}

void
interpret(const char* source, MemoryArray* mem)
{
  for (size_t i = 0; i < strlen(source);) {
    switch (source[i]) {
      case '>':
        next(mem);
        i++;
        break;
      case '<':
        prev(mem);
        i++;
        break;
      case '+':
        inc(mem);
        i++;
        break;
      case '-':
        dec(mem);
        i++;
        break;
      case '.':
        putchar(get_current_value(mem));
        i++;
        break;
      case ',':
        mem->array[mem->current] = getchar();
        i++;
        break;
      case '[':
        if (get_current_value(mem) == 0) {
          i = jump_forward(source, i);
          break;
        }
        i++;
        break;
      case ']':
        if (get_current_value(mem) != 0) {
          i = jump_back(source, i);
          break;
        }
        i++;
        break;
      default:
        i++;
        break;
    }
  }
}

void
repl()
{
  char source[1024];
  MemoryArray mem = new_memory_array();
  for (;;) {
    printf("> ");

    if (!fgets(source, sizeof(source), stdin)) {
      printf("\n");
      break;
    }

    if (strcmp(source, "q\n") == 0) {
      break;
    }
    interpret(source, &mem);
    printf("\n");
  }
  free_memory_array(&mem);
}

int
main(int argc, char const* argv[])
{

  if (argc == 1) {
    repl();
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
        MemoryArray mem = new_memory_array();
        interpret(source, &mem);
        free_memory_array(&mem);
      }
      fclose(f);
    }
    free(source);
  }

  return 0;
}
