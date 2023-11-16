#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//tries in c for all ascii characters

typedef struct node
{
    int is_end;     //also the same as server_num
    struct node *next[128];
}trie;

//initialise trie

trie* init()
{
    trie* root = (trie*)malloc(sizeof(trie));
    root->is_end = 0;
    for (int i = 0; i < 128; i++)
    {
        root->next[i] = NULL;
    }
    return root;
}

//insert into trie

void insert(trie* root, char* str, int server_num)
{
    trie* p = root;
    for (int i = 0; i < strlen(str); i++)
    {
        if (p->next[str[i]] == NULL)
        {
            trie* temp = (trie*)malloc(sizeof(trie));
            temp->is_end = 0;
            for (int j = 0; j < 128; j++)
            {
                temp->next[j] = NULL;
            }
            p->next[str[i]] = temp;
        }
        p = p->next[str[i]];
    }
    p->is_end = server_num;
}

//search in trie

int search(trie* root, char* str)
{
    trie* p = root;
    for (int i = 0; i < strlen(str); i++)
    {
        if (p->next[str[i]] == NULL)
        {
            return 0;
        }
        p = p->next[str[i]];
    }
    if (p->is_end > 0)
    {
        return 1;
    }
    return 0;
}

//delete from trie

void delete(trie* root, char* str)
{
    trie* p = root;
    for (int i = 0; i < strlen(str); i++)
    {
        if (p->next[str[i]] == NULL)
        {
            return;
        }
        p = p->next[str[i]];
    }
    p->is_end = 0;
}

//print all strings in trie in lexicographical order
void print_trie(trie* root, char* prefix)
{
    for(int i=0;i<128;i++)
    {
        if(root->next[i]!=NULL)
        {
            char* temp = (char*)malloc(sizeof(char)*(strlen(prefix)+2));
            strcpy(temp,prefix);
            temp[strlen(prefix)] = i;
            temp[strlen(prefix)+1] = '\0';
            if(root->next[i]->is_end>0)
            {
                printf("%s\n",temp);
            }
            print_trie(root->next[i],temp);
        }
    }
}

void print_all_strings_in_trie(trie* root)
{
    print_trie(root,"");
}

int main()
{
    int n;
    scanf("%d",&n);
    trie* root = init();
    for (int i = 0; i < n; i++)
    {
        char str[100];
        scanf("%s",str);
        insert(root,str,i+1);
    }
    print_all_strings_in_trie(root);
    return 0;
}