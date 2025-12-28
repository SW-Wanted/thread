# Thread
## O que é uma Thread?
Uma **thread de execução** é uma sequência lógica de instruções dentro de um processo, gerenciada automaticamente pelo kernel do sistema operacional. Um programa sequencial comum possui apenas uma thread, mas sistemas operacionais modernos permitem criar várias threads no mesmo programa, que podem ser executadas em **paralelo**.

Cada thread de um processo possui seu **próprio contexto**: seu próprio ID, sua própria pilha (stack), seu próprio ponteiro de instruções e seus próprios registradores do processador. No entanto, como todas as threads pertencem ao mesmo processo, elas **compartilham o mesmo espaço de endereçamento virtual**: o mesmo código, o mesmo heap, as mesmas bibliotecas compartilhadas e os mesmos descritores de arquivos abertos.

O contexto de uma thread consome **menos recursos** do que o contexto de um processo. Por isso, criar uma thread é muito mais rápido para o sistema do que criar um processo. Da mesma forma, alternar a execução entre threads é mais rápido do que alternar entre processos.

Threads não possuem a hierarquia rígida de pai e filho que existe entre processos. Em vez disso, elas formam um grupo de **pares**, independentemente de qual thread criou outra. A única diferença da thread “principal” (main thread) é que ela é a primeira a existir quando o processo inicia. Isso significa que, dentro de um mesmo processo, **qualquer thread pode esperar a finalização de outra thread ou encerrar outra thread**.

Além disso, qualquer thread pode **ler e escrever na mesma memória virtual**, o que torna a comunicação entre threads muito mais simples do que a comunicação entre processos. Mais adiante, veremos os problemas que podem surgir justamente por causa desse compartilhamento de memória.

## Criando uma Thread
A interface padrão em C para manipular threads é a POSIX, por meio da biblioteca `<pthread.h>`.
Podemos criar uma nova thread a partir de qualquer outra thread do programa usando a função `pthread_create`. Seu protótipo é:
```c
int pthread_create(pthread_t *restrict thread,
                   const pthread_attr_t *restrict attr,
                   void *(*start_routine)(void *),
                   void *restrict arg);
```
Vamos analisar cada argumento:
- **thread**: ponteiro para uma variável do tipo `pthread_t`, onde será armazenado o ID da thread que será criada.
- **attr**: permite modificar os atributos padrão da nova thread. Isso foge do escopo aqui e, na prática, passar `NULL` costuma ser suficiente.
- **start_routine**: função onde a thread começará sua execução. Essa função deve ter o protótipo `void *nome_da_funcao(void *arg);`. Quando a thread chega ao fim dessa função, ela encerra sua execução.
- **arg**: ponteiro para o argumento que será passado à função `start_routine`. Se for necessário passar vários parâmetros, deve-se usar um ponteiro para uma estrutura de dados.

Quando a função `pthread_create` termina, a variável `thread` conterá o **ID da thread** recém-criada. A função retorna **0** se a criação foi bem-sucedida ou um **código de erro** caso contrário.

## Juntando ou Destacando Threads
Para bloquear a execução de uma thread até que outra thread termine, podemos usar a função `pthread_join`:
```c
int pthread_join(pthread_t thread, void **retval);
```
Seus parâmetros são os seguintes:
- **thread**: o ID da thread que esta thread deve esperar. A thread especificada deve ser _joinable_ (ou seja, não destacada — veja abaixo).
- **retval**: um ponteiro para uma variável que pode conter o valor de retorno da função de rotina da thread (a função `start_routine` que fornecemos na sua criação). Aqui, não precisaremos desse valor; um simples `NULL` será suficiente.

A função `pthread_join` retorna **0** em caso de sucesso ou um **código de erro** em caso de falha.

Vale notar que só podemos esperar a terminação de uma thread específica. Não há como esperar pela primeira thread que terminar sem especificar um ID, como a função `wait` faz para processos filhos.

Mas, em alguns casos, é possível e até preferível **não esperar o fim de certas threads**. Nesse caso, podemos **destacar a thread** para informar ao sistema operacional que ele pode liberar os recursos da thread assim que ela terminar sua execução. Para isso, usamos a função `pthread_detach` (geralmente logo após a criação da thread):

```c
int pthread_detach(pthread_t thread);
```

Aqui, tudo o que precisamos fornecer é o ID da thread. A função retorna **0** se a operação foi bem-sucedida ou um **valor diferente de zero** se houve erro. Depois de destacar a thread, outras threads não poderão terminar ou esperar por essa thread usando `pthread_join`.

## Memória Compartilhada das Threads
Uma das maiores vantagens das threads é que todas compartilham a memória do seu processo. Cada thread possui sua própria stack, mas as outras threads podem acessar essa memória facilmente com um ponteiro. Além disso, o heap e quaisquer descritores de arquivos abertos são totalmente compartilhados entre as threads.

Essa memória compartilhada e a facilidade com que uma thread pode acessar a memória de outra também trazem perigos: podem causar erros de sincronização complicados.

## O Perigo das Data Races (Condições de Corrida)
Considere um programa que executa duas threads e cada uma delas tem a responsabilidade de incrementar uma variável `count`  com um número de vezes `TIMES_TO_COUNT`. O esperado é que no final do programa `count` passe a valer `2 * TIMES_TO_COUNT`, uma vez que a `thread1`-  contou de `0` até `TIMES_TO_COUNT` e a `thread2` simplesmente executou a contagem novamente e `count` passou a valer de `TIMES_TO_COUNT` até `2 * TIMES_TO_COUNT`.

Se analisarmos os resultados, vemos que a contagem final estará correta somente se a **primeira thread terminar de contar antes da segunda começar**. Sempre que as execuções se sobrepõem, o resultado está errado e é sempre menor que o esperado.

O problema é que ambas as threads frequentemente acessam a mesma área de memória ao mesmo tempo. Suponha que a contagem atual seja 10. A Thread 1 lê o valor 10 — mais precisamente, copia esse valor para seu registrador para manipulá-lo. Em seguida, adiciona 1, obtendo 11. Mas antes de salvar esse resultado na memória apontada pela variável count, a Thread 2 lê o valor 10. A Thread 2 também incrementa para 11. Ambas as threads salvam seu resultado e pronto! Ao invés de incrementar a contagem uma vez por thread, acabamos incrementando apenas uma vez no total… Por isso perdemos contagens e o resultado final fica totalmente errado.

Essa situação é chamada **data race** (_condição de corrida_). Ela ocorre quando um programa depende da sequência ou do tempo de eventos que não podemos controlar. É impossível prever se o sistema operacional escolherá a ordem correta para nossas threads.

Na prática, se compilarmos o programa com as opções -fsanitize=thread e -g e depois executarmos assim:
```bash
gcc -fsanitize=thread -g threads.c && ./a.out
```

Receberemos um alerta: `"WARNING: ThreadSanitizer: data race"`.

Então, existe uma forma de impedir que uma thread leia um valor enquanto outra o modifica? Sim, graças aos **mutexes**!

## O que é um Mutex?
Um **mutex** (abreviação de **mut**ual **ex**clusion, exclusão mútua) é um **primitivo de sincronização**. Basicamente, é um trava que nos permite controlar o acesso a dados e impedir que recursos compartilhados sejam usados ao mesmo tempo.

Podemos pensar em um mutex como a **trava da porta de um banheiro**. Uma thread tranca a porta para indicar que o banheiro está ocupado. As outras threads terão que esperar pacientemente na fila até que a porta seja destrancada para então usarem o banheiro.

## Declarando um Mutex
Graças ao cabeçalho `<pthread.h>`, podemos declarar uma variável do tipo mutex assim:
```c
pthread_mutex_t mutex;
```
Antes de usá-lo, precisamos **inicializá-lo** com a função `pthread_mutex_init`, que tem o seguinte protótipo:
```c
int pthread_mutex_init(pthread_mutex_t *mutex,
                       const pthread_mutexattr_t *mutexattr);
```
Parâmetros:

- **mutex**: ponteiro para uma variável do tipo `pthread_mutex_t`, o mutex que queremos inicializar.
- **mutexattr**: ponteiro para atributos específicos do mutex. Aqui podemos simplesmente passar `NULL`.

A função `pthread_mutex_init` **sempre retorna 0**.

## Trancando e Destrancando um Mutex

Para trancar e destrancar o mutex, usamos duas outras funções:

```c
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

- Se o mutex estiver destrancado, `pthread_mutex_lock` o tranca e a thread que chamou a função se torna sua **proprietária**. Nesse caso, a função termina imediatamente.
- Se o mutex já estiver trancado por outra thread, `pthread_mutex_lock` suspende a execução da thread que chamou a função até que o mutex seja destrancado.

A função `pthread_mutex_unlock` destranca o mutex. O mutex a ser destrancado **deve estar trancado pela thread que chamou a função**, e a função simplesmente o libera. É importante notar que **essa função não verifica se o mutex realmente estava trancado** nem se a thread que chamou é a dona do mutex. Portanto, é possível que uma thread destranque um mutex que não trancou antes.

Devemos tomar cuidado ao organizar `pthread_mutex_lock` e `pthread_mutex_unlock` no código, caso contrário podemos receber erros de **“lock order violation”**.

Ambas as funções retornam **0** em caso de sucesso ou um **código de erro** caso contrário.

## Destruindo um Mutex
Quando não precisarmos mais de um mutex, devemos destruí-lo usando:

```c
int pthread_mutex_destroy(pthread_mutex_t *mutex);
```

Essa função destrói um mutex destrancado, liberando quaisquer recursos que ele possa estar usando. Na implementação LinuxThreads das threads POSIX, **não há recursos associados aos mutexes**, então `pthread_mutex_destroy` basicamente apenas verifica se o mutex não está trancado.

## Deadlock
No entanto, mutexes podem frequentemente **provocar deadlocks**. Isso acontece quando cada thread espera por um recurso que está sendo segurado por outra thread.

Por exemplo: a thread **T1** adquiriu o mutex **M1** e está esperando pelo mutex **M2**. Enquanto isso, a thread **T2** adquiriu o mutex **M2** e está esperando pelo mutex **M1**. Nessa situação, o programa fica **perpetuamente travado** e precisa ser encerrado.

Um deadlock também pode ocorrer **quando uma thread espera por um mutex que ela mesma já possui**!

## Lidando com Deadlocks
Existem várias formas de lidar com deadlocks como esses. Entre elas, podemos:

- **Ignorá-los**, mas apenas se pudermos provar que eles nunca acontecerão. Por exemplo, quando os intervalos de tempo entre pedidos de recursos são muito longos e espaçados.

- **Corrigi-los quando acontecerem**, matando uma thread ou redistribuindo recursos, por exemplo.

- **Prevenir e corrigir antes que aconteçam**.

- **Evitá-los impondo uma ordem estrita na aquisição de recursos**.

- **Evitá-los forçando uma thread a liberar um recurso** antes de solicitar novos ou antes de renovar seu pedido.

Não existe uma “melhor” solução que resolva todos os casos de deadlock. O melhor método depende da situação específica.

## Dicas para Testar Threads de um Programa
O mais importante ao testar qualquer programa que use threads é **testar a mesma coisa várias vezes seguidas**. Muitas vezes, erros de sincronização não aparecem na primeira, segunda ou até terceira execução. Isso depende da ordem que o sistema operacional escolhe para a execução de cada thread. Rodando o mesmo teste repetidamente, podemos ver grandes variações nos resultados.

Existem algumas ferramentas que ajudam a detectar erros relacionados a threads, como possíveis **data races, deadlocks e erros de ordem de lock**:

- A flag `-fsanitize=thread -g` que podemos adicionar na compilação. A opção `-g` mostra os arquivos e linhas específicas envolvidas.
- A ferramenta de detecção de erros em threads **Helgrind**, que podemos usar assim:
```bash
valgrind --tool=helgrind ./main <args>
```
- **DRD**, outra ferramenta de detecção de erros em threads, que podemos usar assim:
```bash
valgrind --tool=drd ./main <args>
```

**Atenção**: usar `-fsanitize=thread` e **Valgrind** juntos não funciona bem.

E, como sempre, não podemos esquecer de verificar vazamentos de memória usando `-fsanitize=address` e **Valgrind**.