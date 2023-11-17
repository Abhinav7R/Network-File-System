#ifndef TRIES_H
#define TRIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//tries in c for all ascii characters

typedef struct node
{
    int is_end;     //also the same as server_num
    struct node *next[128];
}trie;

trie* init();
void insert(trie* root, char* str, int server_num);
int search(trie* root, char* str);
void print_trie(trie* root, char* prefix);
void print_all_strings_in_trie(trie* root);
void delete_node(trie* root, char* str);

#endif