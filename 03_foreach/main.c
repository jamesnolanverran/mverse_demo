#include <stddef.h>
#include <stdio.h>

typedef struct IntArray {
    size_t len;
    int *items;
} IntArray;

@def(foreach(type, arr, item_name=item, index_name=__mv_i)){{
  size_t __mv_len   = ($arr) ? ($arr)->len : 0;
  $type *__mv_items = ($arr) ? ($arr)->items : NULL;
  if(!__mv_items) __mv_len = 0;
  for (size_t $index_name = 0; $index_name < __mv_len; ++$index_name) {
      $type *$item_name = &__mv_items[$index_name];
      $body
  }
}}

int main(void) {
    int values[] = {10, 20, 30};
    IntArray numbers = {3, values};

    @foreach(int, &numbers, item_name=value, index_name=i) {
        printf("numbers[%zu] = %d\n", i, *value);
    }

    @foreach(int, &numbers) {
        printf("again: %d\n", *item);
    }

    return 0;
}
