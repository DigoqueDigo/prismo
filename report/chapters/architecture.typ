#import "../utils/functions.typ" : raw_code_block

== Abordagem e Planeamento

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


#figure(
    image("../images/block.png", width: 60%),
    caption: [Hierarquia da interface de geração de blocos]
)

====== Geração de Duplicados e Compressão

// apresentar a estrutura do fxeito de confiuração para cada um dos casos
// espetar o json como code block

// explicar o algoritmo de sliding window
// dizer como é realizada a seleção da taxa de compressão

#grid(
    columns: 2,
    gutter: 10pt,
    figure(
        image("../images/deduplication.png", width: 100%),
        caption: [Mapa de duplicados]
    ),
    figure(
        image("../images/deduplication.png", width: 100%),
        caption: [Mapa das taxas de compressão]
    ),
)

// dizer que esta estratégia é bastante eficiente e o único problema seria eventualmente a escolha de um número aleatório, no entanto isso é atenuado pelo buffer que utiliza o shishua






==== Integração de Interfaces de I/O

// dar uma brave introdução da dificulta de combinar interfaces sincronas e assincronas
// explicar por bullet points os metodos disponibilizados


#figure(
    image("../images/consumer.png", width: 60%),
    caption: [Interação do consumidor com a interface de engine]
)

// explicar muito brevemente o diagrama

===== POSIX

// apresentar a estrutura do ficheiro de configuração e realçar os parametros mais relevantes

#figure(
    image("../images/flow_posix.png", width: 60%),
    caption: [Funcionamento interno da POSIX Engine]
)

// expicar o diagrama

===== Uring


#figure(
    image("../images/flow_uring.png", width: 60%),
    caption: [Funcionamento interno da Uring Engine]
)

===== SPDK


#figure(
    image("../images/flow_uring.png", width: 60%),
    caption: [Funcionamento interno da SPDK Engine]
)


==== Flow de Execução

// explicar novamente a questão do producer consumer e dizer que isto pode ser estendido para multiplica consumer caso a geração de conteudo esteja muito avança em relação que consumer

// como é uma queue é realizado buffering dos pedidos e ao mesmo tempo backpressure para não saturar o consumidor



=== Resultados Permilinares

=== Próximas Etapas
