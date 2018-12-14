/*************************************************************************
	> File Name: te.c
	> Author: 
	> Mail: 
	> Created Time: 2018年12月13日 星期四 21时07分08秒
 ************************************************************************/

#include<stdio.h>

int swap(int **a , int **b)
{
    int q;
    q = **a;
    **a = **b;
    **b = q;
    printf("%d %d",**a,**b);
}

int main ()
{
    int a=3, b=8;

    int *p = &a,*q = &b;
    swap(&p , &q);

    printf("%d %d", a ,b);

}
