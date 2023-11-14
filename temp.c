#include<stdio.h>
#include<string.h>

int main(int argc, char const *argv[])
{
    /* code */
    int count=0;
    char data[1024];
    //fgets till data is full i.e 100 characters

    while(1)
    {
        fgets(data,100,stdin);
        if(strcmp(data,"\n")==0)
        {
            break;
        }

        printf("%s",data);
    }
    return 0;
}
