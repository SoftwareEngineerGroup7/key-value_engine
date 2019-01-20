/*************************************************************************
	> File Name: test.c
	> Author: 
	> Mail: 
	> Created Time: 2019年01月20日 星期日 21时02分49秒
 ************************************************************************/

#include<stdio.h>
int main(){
    struct bs
    {
        unsigned a:1;
        unsigned b:3;
        unsigned c:4;
    } bit,*pbit;
    
    bit.a=1;
    bit.b=7;
    bit.c=15;
    printf("%d,%d,%d\n",bit.a,bit.b,bit.c);
    pbit=&bit;
    pbit->a=0;
    pbit->b &= 3;
    pbit->c|=1;
    printf("%d,%d,%d\n",pbit->a,pbit->b,pbit->c);
} 
