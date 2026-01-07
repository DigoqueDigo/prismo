#import "@preview/wrap-it:0.1.1": wrap-content
#import "../utils/functions.typ" : raw_code_block

== Abordagem e Planeamento <chapter3>

Depois de esclarecido o problema da avaliação realista dos sistemas de armazenamento e compreendidos os conceitos em seu redor, este capítulo visa abordar a arquitetura do protótipo de benchmark, passando pela identificação dos respetivos componentes, estratégias adotadas para geração de conteúdo e integração com #link(<api>)[*APIs*] de #link(<io>)[*I/O*] cuja natureza é bastante diversa, isto fundamentalmente porque algumas são síncronas e outras assíncronas.

De seguida, serão apresentados resultados preliminares da performance do benchmark, procurando explicar as diferenças obtidas conforme as configurações e ambiente de teste, sendo isto fundamental para perceber se o overhead associado à geração de conteúdo impossibilita a saturação do sistema de armazenamento.

Por fim, e uma vez que somente o protótipo foi implementado, serão estabelecidas as próximas etapas do desenvolvimento do benchmark, algo que inevitavelmente passará pela integração com traces e extensão dos mesmos, no então outras configurações relativamente simples como speed up ou slow down seriam interessantes e contribuiriam para uma maior flexibilidade em termos de configuração.

=== Arquitetura

Numa primeira abordagem ao problema, percebemos que a geração de conteúdo é facilmente dissociável das operações solicitadas ao sistema de armazenamento, sendo estas realizadas por meio das #link(<api>)[*APIs*] de #link(<io>)[*I/O*]. Deste modo, a arquitetura pode ser dividida em dois grandes componentes que estabelecem cada um interfaces para manipulação da conduta.

Posto isto, a interface para geração de conteúdo abstrai as implementações concretas, daí que a sua utilização não implique desvios de padrão caso o utilizador escolha usufruir de dados sintéticos ou reais obtidos através de traces, do mesmo modo esta lógica é aplicável para a interface de abstração do disco.

Com o estabelecimento destes componentes, um produtor é responsável para invocar os métodos da interface de geração de conteúdo e encapsular os resultados num pedido de #link(<io>)[*I/O*], sendo este colocado numa blocking queue como forma de solicitação de execução.

Do outro lado, um consumidor está constantemente à escuta na queue com o objetivo de receber pedidos, mal isto ocorra, é realizada uma submissão na interface de #link(<io>)[*I/O*], sendo mais tarde a estrutura do pedido libertada e transmitida ao produtor para nova utilização.

==== Geração de Conteúdo Sintético

Na generalidade das interfaces, os pedidos de #link(<io>)[*I/O*] são caracterizados pelo tipo de operação, conteúdo e posição do disco onde o pedido será satisfeito, consequentemente o gerador de conteúdo sintético pode ser desacoplado nestas três funcionalidades, dando origem a interfaces que visam fornecer os parâmetros dos pedidos.

Como fruto desta abordagem, e uma vez que os geradores são definidos ao nível dos parâmetros, a combinação entre geradores sintéticos e reais torna-se bastante simples, isto porque o produtor apenas conhece uma interface que é independente da implementação concreta, assim podemos ter acessos reais e operações sintéticas, sendo o contrário igualmente válido.

#figure(
   image("../images/producer.png", width: 60%),
   caption: [Interação do produtor com a interface de geração de conteúdo]
)

Enquanto medida para reutilização de memória, produtor e consumidor partilham duas queues, uma direcionada ao envio de pedidos (produtor para consumidor) e outra responsável por identificar as structs cujo pedido já foi concluído (consumidor para produtor), e como tal podem ser reutilizadas pelo produtor.

Ao recolher uma struct, através da operação de dequeue, o produtor invoca os métodos disponibilizados por cada uma das interfaces, de relembrar que o conteúdo do bloco apenas é gerado quando a operação solicitada for um `WRITE`. De seguida, e tendo os parâmetros devidamente identificados, o mesmos são encapsulados num pedido que é inserido na queue para futura execução por parte do consumidor.

Uma vez que as queues apresentam capacidade limitada, e tendo em consideração que à partida o produtor será mais performante que o consumidor, isto permite alcançar buffering e backpressure em simultâneo, pois quando a capacidade limite for atingida, o produtor irá bloquear e portanto o consumidor jamais será sobrecarregado com uma quantidade infindável de pedidos, o que contribui para um uso eficiente da memória disponível.

===== Acesso

Os pedidos de `READ` e `WRITE` necessitam de ser identificados pela zona do disco onde a operação irá ocorrer, neste sentido a interface `AccessGenerator` disponibiliza o método `nextAccess` que devolve o offset da próxima operação a realizar, sendo de realçar que nem todas as implementações concretas apresentam a mesma performance, pois algumas seguem distribuições enquanto outras utilizam aritmética simples.

#figure(
   image("../images/access.png", width: 60%),
   caption: [Hierarquia da interface de acessos]
)

Dado que os acessos são realizados ao nível do bloco, todas as implementações devem conhecer o tamanho do bloco e o limite da zona do disco até onde é permitido ler ou escrever, deste modo os offsets devolvidos serão inferiores ou iguais ao limite e acima de tudo múltiplos do tamanho do bloco.

#grid(
   columns: 3,
   gutter: 5pt,
   raw_code_block[
        ```yaml
        type: sequential
        blocksize: 4096
        limit: 65536
        ```
   ],
   raw_code_block[
        ```yaml
        type: random
        blocksize: 4096
        limit: 65536
        ```
   ],
   raw_code_block[
        ```yaml
        type: zipfian
        blocksize: 4096
        limit: 65536
        skew: 0.99
        ```
   ],
)

A implementação do tipo sequencial é responsável por devolver os offsets num padrão contínuo, sendo que o alcance do limite implica o reposicionamento no offset zero, esta estratégia beneficia claramente a localidade espacial, pois as zonas do disco são acedidas num padrão favorável.

Por outro lado, os acessos totalmente aleatórios não favorecem quaisquer propriedades de localidade, daí que sejam especialmente úteis para evitar uma utilização eficiente da cache. Por fim, os acessos zipfian seguem uma distribuição cuja skew pode ser manipulada pelo utilizador, neste sentido cargas de trabalho com hotspots são facilmente replicáveis por esta implementação.

===== Operação

Os sistemas de armazenamento suportam uma infinidade de operações, no entanto o gerador de operações apenas disponibiliza `READ`, `WRITE`, `FSYNC`, `FDATASYNC` e `NOP` por serem as mais comuns e portanto adotadas pela maioria das #link(<api>)[*APIs*] de #link(<io>)[*I/O*]. Embora a operação `NOP` não faça rigorosamente nada, a mesma é útil para testar a performance do benchmark independente da capacidade do disco, permitindo identificar o débito máximo que o sistema de armazenamento pode almejar.

#figure(
   image("../images/operation.png", width: 60%),
   caption: [Hierarquia da interface de operações]
)

A implementação do tipo constante é a mais simples, isto porque devolve sempre a mesma operação que foi definida previamente pelo utilizador. Em contrapartida, as operações percentuais são obtidas à custa de uma distribuição cujo somatório das probabilidade deve resultar em 100, exemplificando com a configuração abaixo, metade das operações serão `READs` e as restantes `WRITES`.

#grid(
   columns: 3,
   gutter: 5pt,
   raw_code_block[
        ```yaml
        type: constant
        operation: write
        ```
   ],
   raw_code_block[
        ```yaml
        type: percentage
        percentages:
            read: 50
            write: 50
        ```
   ],
   raw_code_block[
        ```yaml
        type: sequence
        operations:
            - write
            - fsync
        ```
   ],
)

Por fim, a replicação de padrões é obtida com recurso à implementação de sequência, sendo o utilizador responsável por definir uma lista de operações que mais tarde será repetidamente devolvida, neste caso em concreto, se o método `nextOperation` fosse invocado cinco vezes, as operações seriam devolvidas pela ordem: `WRITE`, `FSYNC`, `WRITE`, `FSYNC`, `WRITE`.

===== Geração de Blocos

A geração de blocos é sem dúvida a operação mais custosa, no entanto apenas torna-se necessária quando a operação selecionada for um `WRITE`, nesse sentido a interface de `BlockGenerator` disponibiliza o método `nextBlock` que preenche um buffer passado como argumento.

Embora a implementação principal desta interface seja aquela que combina duplicados e compressão, existem outras mais rudimentares que servem para testar cenários específicos com maior eficiência, isto porque o gerador de duplicados é capaz de simular os blocos dos outros geradores, mas com uma performance significativamente menor.

#figure(
   image("../images/block.png", width: 60%),
   caption: [Hierarquia da interface de geração de blocos]
)

Tal como seria expectável, os geradores necessitam de conhecer o tamanho do bloco, deste modo podem garantir que os limites dos buffers jamais serão violados. A implementação mais simplista deste gerador corresponde ao constante, que devolve sempre o mesmo buffer, resultando numa deduplicação e compressibilidade interbloco máximas. Por outro lado, o aleatório tem exatamente o comportamento oposto, pois ao devolver buffers diferentes não existem duplicados e a entropia é elevada.

#grid(
  columns: 3,
  gutter: 5pt,
  raw_code_block[
       ```yaml
       type: constant
       blocksize: 4096
       ```
  ],
  raw_code_block[
       ```yaml
       type: random
       blocksize: 4096
       ```
  ],
  raw_code_block[
       ```yaml
       type: dedup
       blocksize: 4096
       refill_buffers: false
       ```
  ],
)

Por fim, o gerador de duplicados e compressão procura seguir uma distribuição de duplicados definida pelo utilizador, esta estabelece a percentagem de blocos que terão X cópias, sendo que cada grupo de cópias tem associada uma distribuição de compressão, indicando que Y% dos blocos reduz cerca de Z%.

Além disso, a opção `refill_buffers` permite a partilha do buffer base entre blocos, deste modo quando os mesmos são criados a zona de entropia máxima é obtida a partir do buffer, consequentemente todos os blocos partilham a mesma informação e portanto a compressibilidade interbloco atinge o limite.

====== Geração de Duplicados e Compressão

Para que o utilizador manipule a distribuição de duplicados e compressão, o benchmark oferece um ficheiro de configuração sobre o qual as informações são retiradas, bastando seguir o formato indicado.

#grid(
   columns: 3,
   gutter: 5pt,
   raw_code_block[
       ```yaml
       - percentage: 50
           repeats: 1
           compression:
           - percentage: 50
             reduction: 10
           - percentage: 20
             reduction: 20
           - percentage: 10
             reduction: 30
           - percentage: 15
             reduction: 25
           - percentage: 5
             reduction: 5
       ```
   ],
   raw_code_block[
       ```yaml
       - percentage: 30
           repeats: 2
           compression:
           - percentage: 20
             reduction: 20
           - percentage: 40
             reduction: 10
           - percentage: 40
             reduction: 0
       ```
   ],
   raw_code_block[
       ```yaml
       - percentage: 20
           repeats: 3
           compression:
           - percentage: 40
             reduction: 30
           - percentage: 60
             reduction: 0
       ```
   ],
)

A distribuição de duplicados e compressão é definida de modo particular, inicialmente é realizada uma associação entre o número de cópias e a respetiva probabilidade, sendo mais tarde definidas as taxas de compressão dentro de cada grupo.

#grid(
   columns: 2,
   gutter: 5pt,
   [
       #figure(
           image("../images/compression.png", width: 100%),
           caption: [Mapa das taxas de compressão],
       ) <compression-map>
   ],
   [
       #figure(
           image("../images/deduplication.png", width: 100%),
           caption: [Mapa dos duplicados],
       ) <dedup-map>
   ],
)

A @compression-map representa a estrutura sobre a qual as taxas de compressão são armazenadas para cada grupo, sendo basicamente um mapa que associa o número de repetições a uma lista formada por tuplos de percentagem cumulativa e respetiva redução.

Por outro lado, a @dedup-map é responsável por gerir os blocos duplicados e tem um funcionamento semelhante ao de uma sliding window, onde os tuplos da lista são constituídos pelo identificador de bloco e cópias que faltam efetuar.

O funcionamento do algoritmo é bastante simples, inicialmente uma entrada do mapa é selecionada conforme as probabilidades do ficheiro de configuração, a partir daí, caso a lista não tenha atingido o limite de elementos, um novo é adicionado com o número de cópias igual ao de repetições.

Na situação em que a lista encontra-se completa, um dos elementos é selecionado aleatoriamente e o valor das cópias em falta é decrementado uma unidade, ao ser atingido o valor zero a entrada é definitivamente retirada da lista, pois o bloco já foi repetido as vezes necessárias.

Por fim, depois de selecionado o identificador do bloco, volta a ser sorteado um número aleatório para descobrir a taxa de compressão a aplicar, de relembrar que a distribuição é obtida pela entrada do mapa selecionada inicialmente.

Apesar de bastante eficiente, esta abordagem acarreta o problema da geração pseudo aleatória, algo que tende a ser bastante custoso relativamente às restantes operações, no entanto esta implementação faz uso de um buffer gerido pelo SHISHUA, deste modo gerações massivas são realizadas periodicamente enquanto a aplicação limita-se a recolher dados do buffer.

==== Integração de Interfaces de I/O

Sabendo que o consumidor está à escuta de pedidos enviados pelo produtor, quando os mesmos são recebidos procede-se de imediato ao desencapsulamento para compreender o tipo de operação em questão e assim facilitar o acesso aos restantes parâmetros, como offset e conteúdo.

A interface `Engine` disponibiliza o método `submit` que aceita operações de qualquer tipo, assim o consumidor não é responsável por definir as alterações de comportamento associadas. Mal o pedido seja dado por concluído, a struct é devolvida pela interface, permitindo ao consumidor fazer dequeue para que a zona de memória seja reutilizada pelo produtor.

#figure(
   image("../images/consumer.png", width: 60%),
   caption: [Interação do consumidor com a interface de engine]
)

Um pedido obtido a partir da queue pode ser de três tipos distintos, onde as structs de abertura e fecho são caracterizadas pelos argumentos encontrados nas syscalls de `open` e `close`, importa realçar que tais estruturas não fazem sentido para a engine de #link(<spdk>)[*SPDK*], visto esta funcionar diretamente sobre o dispositivo de armazenamento e portanto não existir uma abstração do sistema de ficheiros.

#grid(
 columns: 3,
 gutter: 5pt,
 raw_code_block[
   ```c
   struct CloseRequest {
       int fd;
   };
   ```
 ],
 raw_code_block[
   ```c
   struct OpenRequest {
     int flags;
     mode_t mode;
     char* filename;
   };
   ```
 ],
 raw_code_block[
   ```c
   struct CommonRequest {
       int fd;
       size_t size;
       uint64_t offset;
       uint8_t* buffer;
       Metadata metadata;
       OperationType op;
   };
   ```
 ]
)



Perante a combinação de interfaces síncronas e assíncronas, o método `submit` nem sempre devolve uma struct para reutilização, pois, no caso das interfaces assíncronas nunca sabemos exatamente quando o pedido será dado por concluído e além disso não é possível esperar até que tal aconteça, caso contrário estaria a ser dado comportamento síncrono e as vantagens de paralelismo seriam perdidas.

Tendo isto em mente, o método `reap_left_completions` possibilita a espera forçosa dos  pedidos pendentes, algo que deve ser utilizado entre a última submissão e a operação de `close`.



===== POSIX





#let posix_config = raw_code_block(width: auto)[
  ```yaml
  engine:
    type: posix
    openflags:
      - O_CREAT
      - O_TRUNC
      - O_RDONLY
      - O_DIRECT
  ```
]

#let posix_body = [#lorem(100)]

#wrap-content(
  posix_config,
  posix_body,
  align: top + right,
)


#figure(
  image("../images/flow_posix.png", width: 65%),
  caption: [Funcionamento interno da POSIX Engine]
)

// apresentar a estrutura do ficheiro de configuração e realçar os parametros mais relevantes


// expicar o diagrama

===== Uring

#let uring_config = raw_code_block(width: auto)[
  ```yaml
  engine:
    type: uring
    openflags:
      - O_CREAT
      - O_RDWR
    entries: 128
    params:
      cq_entries: 128
      sq_thread_cpu: 0
      sq_thread_idle: 0
      flags:
        - IORING_SETUP_SQPOLL
        - IORING_SETUP_IOPOLL
        - IORING_SETUP_SQ_AFF
  ```
]

#let uring_body = [#lorem(100)]

#wrap-content(
  uring_config,
  uring_body,
  align: top + right
)



#figure(
    image("../images/flow_uring.png", width: 75%),
    caption: [Funcionamento interno da Uring Engine]
)

===== SPDK

#let spdk_config = raw_code_block(width: auto)[
  ```yaml
  engine:
    type: spdk
    spdk_threads: 1
    bdev_name: Malloc0
    reactor_mask: "0xF"
    json_config_file: spdk_bdev.json
  ```
]

#let spdk_body = [#lorem(100)]

#wrap-content(
  spdk_config,
  spdk_body,
  align: top + right
)

#figure(
    image("../images/flow_spdk.png", width: 85%),
    caption: [Funcionamento interno da SPDK Engine]
)


==== Flow de Execução

// explicar novamente a questão do producer consumer e dizer que isto pode ser estendido para multiplica consumer caso a geração de conteudo esteja muito avança em relação que consumer

// explicar a questão do speed up e slow down que é basicamente a cadencia com que o conteudo é colocado na queue (produtor -> consumidor)



=== Resultados Permilinares

=== Próximas Etapas
