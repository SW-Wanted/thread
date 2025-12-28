#include <stdio.h>
#include <unistd.h>

void professor_1();
void professor_2();

int main() 
{
    professor_1();
    sleep(2);
    professor_2();
    return 0;
}

void professor_1()
{
    printf("Professor 1: Ola turma, antes da aula começar eu vou só ao quarto de banho rápido.\n");
    sleep(3);
    printf("Professor 1: Voltei, podemos começar...\n");
    sleep(1);
    printf("Professor 1: Bla bla bla...\n");
    sleep(2);
    printf("Professor 1: Tchau pessoal, fiquem bem!\n");
}

void professor_2()
{
    printf("Professor 2: Boa tarde turma, tem novo Lab!\n");
    sleep(2);
    printf("Professor 2: Vim apenas vos explicar como devem fazer.\n");
    sleep(2);
    printf("Professor 2: Bla bla bla...\n");
    sleep(2);
    printf("Professor 2: Adeus!\n");
}