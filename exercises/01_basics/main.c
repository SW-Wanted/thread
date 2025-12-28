#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void *professor_1();
void *professor_2();

int main()
{
    pthread_t thread1;

    pthread_create(&thread1, NULL, professor_1, NULL);
    professor_2();
    pthread_join(thread1, NULL);
    return 0;
}

void *professor_1()
{
    printf("Professor 1: Ola turma, antes da aula começar eu vou só ao quarto de banho rápido.\n");
    sleep(8);
    printf("Professor 1: Voltei, podemos começar...\n");
    sleep(1);
    printf("Professor 1: Bla bla bla...\n");
    sleep(2);
    printf("Professor 1: Tchau pessoal, fiquem bem!\n");
}

void *professor_2()
{
    sleep(1);
    printf("Professor 2: Boa tarde turma, tem novo Lab!\n");
    sleep(2);
    printf("Professor 2: Vim apenas vos explicar como devem fazer.\n");
    sleep(2);
    printf("Professor 2: Bla bla bla...\n");
    sleep(2);
    printf("Professor 2: Opah, Tcháu! Até no último tempo.\n");
}