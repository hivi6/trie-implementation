#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct node_t {
  struct node_t *children[26];
  char is_end;
};

struct trie_t {
  struct node_t *root;
};

struct suggestions {
  int count;
  int capacity;
  char **items;
};

struct node_t *node_init() {
  return calloc(1, sizeof(struct node_t));
}

void node_delete(struct node_t *node) {
  if (node == NULL) return;
  for (int i = 0; i < 26; i++) {
    node_delete(node->children[i]);
  }
  free(node);
}

struct trie_t *trie_init() {
  struct trie_t *trie = malloc(sizeof(struct trie_t));
  trie->root = node_init();
  return trie;
}

void trie_delete(struct trie_t *trie) {
  node_delete(trie->root);
  free(trie);
}

void trie_insert(struct trie_t *trie, const char *word) {
  struct node_t *current = trie->root;
  for (int i = 0; word[i]; i++) {
    int index = word[i] - 'a';
    if (current->children[index] == NULL) {
      current->children[index] = node_init();
    }
    current = current->children[index];
  }
  current->is_end = 1;
}

void node_find_words(struct node_t *node, const char *prefix, struct suggestions *s) {
  if (node->is_end) {
    int len = strlen(prefix);
    char *res = malloc(len + 1);
    strcpy(res, prefix);
    res[len] = 0;
    
    if (s->count >= s->capacity) {
      s->capacity += 8;
      char **temp = malloc(s->capacity * sizeof(char *));
      for (int i = 0; i < s->count; i++) {
        temp[i] = s->items[i];
      }
      free(s->items);
      s->items = temp;
    }
    s->items[s->count] = res;
    s->count++;
  }

  for (int i = 0; i < 26; i++) {
    if (node->children[i] != NULL) {
      char *new_prefix = malloc(strlen(prefix) + 2);
      strcpy(new_prefix, prefix);
      new_prefix[strlen(prefix)] = i + 'a';
      new_prefix[strlen(prefix) + 1] = 0;
      node_find_words(node->children[i], new_prefix, s);
      free(new_prefix);
    }
  }
}

void trie_suggest(struct trie_t *trie, const char *prefix, struct suggestions *s) {
  struct node_t *node = trie->root;
  for (int i = 0; prefix[i]; i++) {
    int index = prefix[i] - 'a';
    if (node->children[index] == NULL) {
      return;
    }
    node = node->children[index];
  }
  node_find_words(node, prefix, s);
}

void trie_from_file(struct trie_t *trie, const char *fname) {
  FILE *fd = fopen(fname, "r");

  char buffer[256];

  while (fgets(buffer, sizeof(buffer), fd)) {
    int len = strlen(buffer);

    // remove newline
    if (buffer[len - 1] == '\n') {
      buffer[len - 1] = 0;
    }
    
    // convert to lowercase
    for (int i = 0; i < len - 1; i++) {
      buffer[i] = tolower(buffer[i]);
    }

    trie_insert(trie, buffer);
  }

  fclose(fd);
}

int main() {
  char buffer[256];
  printf("Enter prefix: ");
  scanf("%s", buffer);

  struct trie_t *trie = trie_init();

  trie_from_file(trie, "words.txt");

  struct suggestions *s = calloc(1, sizeof(struct suggestions));
  
  trie_suggest(trie, buffer, s);

  printf("Do you mean:\n");
  for (int i = 0; i < s->count; i++) {
    printf("%s\n", s->items[i]);
  }

  for (int i = 0; i < s->count; i++) {
    free(s->items[i]);
  }
  free(s->items);
  free(s);

  trie_delete(trie);

  return 0;
}
