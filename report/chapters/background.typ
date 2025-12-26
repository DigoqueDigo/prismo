== Background e Trabalho Relacionado

Este capítulo tem por objetivo apresentar os conceitos e trabalho relacionado que sejam relevantes para a compreensão do projeto, nesse sentido, inicialmente é apresentada uma breve descrição das técnicas de deduplicação e compressão, realçando as implementações e desafios associados.

De seguida, discute-se a importância de replicar traces obtidos em ambientes de produção, bem como o impacto da stack de #link(<io>)[*I/O*] sobre as operações, e de que modo algumas #link(<api>)[*APIs*] procuram reduzir ao máximo as limitações impostas.

Posteriormente são estudadas as últimas técnicas para geração de conteúdo realista e soluções de benchmark amplamente utilizadas pela comunidade, tentando perceber se os resultados apresentados por estas permitem a avaliação realista dos sistemas de armazenamento.

O capítulo termina com uma síntese da informação recolhida, procurando desvendar os impactos que isso terá na arquitetura da proposta de solução.

=== Background

Nesta secção dão-se a conhecer os conceitos de deduplicação e compressão, percebendo de que modo estes podem ser integrados em traces para mais tarde serem aplicados num benchmark que suporte diversas #link(<api>)[*APIs*], sem esquecer de realçar o ciclo de vida no interior da stack de #link(<io>)[*I/O*].

Convém mencionar que a proposta de solução funciona ao nível do bloco, portanto, e por motivos de simplicidade, os conceitos serão apresentados tendo isso em conta, embora a granularidade não lhes seja diretamente incutida.

==== Deduplicação

A deduplicação caracteriza-se por poupar espaço de armazenamento ao não escrever conteúdos redundantes, sendo aplicada numa grande variedade de contextos, que vão desde backup, archival e primary storage até à #link(<ram>)[*RAM*]. Uma visão geral do funcionamento deste processo está apresentada na @dedup.

#figure(
 image("../images/dedup.png", width: 80%),
 caption: [Visão geral do funcionamento da deduplicação]
) <dedup>

Sempre que um pedido de #link(<io>)[*I/O*] é submetido ao sistema, a stack de #link(<io>)[*I/O*] calcula a assinatura do bloco e consulta um índice responsável por estabelecer um mapeamento entre endereços físicos e lógicos, verificando assim a existência de duplicados. Caso a entrada já se encontre no índice, o bloco em questão é duplicado, como tal simplesmente será criado um apontador para a sua posição no disco, evitando uma escrita de conteúdo repetido. Consequentemente, nenhuma cópia será escrita, diminuindo os requisitos de armazenamento da aplicação.

Uma vez que este processo decorre na stack de #link(<io>)[*I/O*], a execução do processo de deduplicação é completamente transparente à aplicação, afinal a visão lógica apresenta os dados solicitados pela camada superior, e portanto contém duplicados, enquanto a visão física - onde os dados são realmente armazenados - não permite tal.

Embora existam diversas granularidades de deduplicação, esta dissertação apenas aborda a orientada ao bloco, deste modo os dados são divididos em blocos de tamanho fixo, sendo realizado um padding em caso de necessidade e armazenados somente os blocos únicos. Por norma, quanto menor for o tamanho do bloco, melhor será a utilização do espaço de armazenamento, pois à partida serão detetadas mais cópias, no entanto isso acarreta custos ao nível computacional, visto ser necessário lidar com mais blocos, neste sentido é preciso encontrar um tamanho razoável, sendo 4096 bytes um padrão aplicado em diversos sistemas de armazenamento.

Fora isso, a técnica em questão pode ser aplicada em diferentes alturas e com índices variados, das quais se destacam os seguintes:

===== Deduplicação Inline

Nesta alternativa, os blocos duplicados são identificados no caminho crítico de #link(<io>)[*I/O*], sempre que um pedido é realizado. Assim sendo, é calculada a assinatura do bloco e verificada na estrutura do índice a fim de determinar o endereço físico caso o bloco em questão já tenha sido registado. Não esquecer que o mapeamento e contador de referências devem ser atualizados antes do pedido ser dado como concluído.

Embora esta técnica consiga reduzir as operações de #link(<io>)[*I/O*] no disco subjacente e consequentemente aumentar o débito do sistema, a latência dos pedidos tende a aumentar devido às múltiplas repetições do processo anteriormente descrito. Daí que manter a performance e salvaguardar a latência seja um dos desafios na deduplicação inline.

===== Deduplicação Offline

Ao contrário da estratégia anterior, a deduplicação offline não interfere no caminho crítico de #link(<io>)[*I/O*], os dados são escritos diretamente no disco, salvaguardando assim baixa latência entre pedidos. Na verdade, a deduplicação é realizada mais tarde e em segundo plano, por exemplo, em alturas de menor demanda do sistema de armazenamento.

Após a operação de escrita, os blocos são colocados numa fila de espera, onde um processo em background irá calcular as assinaturas e consultar o índice, em caso de duplicados, os mapeamentos lógicos são atualizados juntamente com os contadores de referências, e o espaço duplicado é libertado.

Apesar desta estratégia diminuir a latência dos pedidos, o consumo de armazenamento aumenta temporariamente, e como não reduz os pedidos ao disco, os ganhos no débito são marginais. Por outro lado, o processo em background pode trazer implicações de desempenho e consistência se não for agendado para o momento certo.

===== Índice Completo

Este índice caracteriza-se por conter as assinaturas de todos os blocos únicos submetidos ao sistema, sendo impossível perder oportunidades para encontrar duplicados, no entanto a estrutura subjacente tende a crescer imenso e torna-se difícil de manter em #link(<ram>)[RAM], geralmente é transferida para o disco. Deste modo, workloads de backup e archival, que não exigem baixa latência, costumam adotar este índice.

===== Índice Parcial

Com o objetivo de tirar partido da localidade espacial e temporal, este índice armazena somente as informações relativas aos blocos mais recentes e populares, por conseguinte a estrutura de dados pode ser armazenada em #link(<ram>)[*RAM*], o que permite diminuir a latência dos pedidos. Por outro lado, uma vez que o índice não contém todos os blocos, é possível que blocos antigos ou pouco populares possam não ser identificados como duplicados e portanto existirão cópias no sistema de armazenamento.





==== Compressão

// explicar muito sucintamente a copmressão e as vantagens que podemos retirar daí, isto é importante para o benchmark porque os sistemas mais recentes també já abordam deduplicação

// começar por explicar o lz77 e depois passo para o codigo de huffman, no fim deizer que os dois podem ser combinardo, ou seja, os indices do lz77 podem ser otimizados com codigos de huffman, o que dá orignem ao deflate, o algoritmo tilizador pelo gzip, no entanto o algoritmo mais utilizado atualmente é o lz77

// explicar muito sucintamnte alguns algoritmos e dizer que o sztd é o standard atual

#figure(
  image("../images/lz77_sliding_window.png", width: 60%),
  caption: [Funcionamento da sliding window no algoritmo LZ77]
) <lz77>

#figure(
  image("../images/huffman.png", width: 30%),
  caption: [Árvore de Huffman]
) <huffman>

// dizer os codigos resultantes desta arvore
// como base no que aprendemos podemos arranjar uma forma de manipular a cmopressão, esplicar a tecnica do tudo random e tudo a zero

==== Traces

// explicar que os tracespodem ser recolhidos de ambiente em produção e portanto reflem as cargas de trabalho reiais,

// mencionar que os traces dispoinivies são muito antigos e pertencem ao FIU, apresentar a estrutura dos mesmos num pedaço de codigo tipo ASCN e procurar explicar cada campo desse trace







==== Stack de I/O

// apresentar o diagrama da stack de io
// dizer o monte de implicações de egurança e o impacto que isso tem na performance

==== Interfaces de I/O

// explicar a diversiadde de interfaces mas que eles se dividem entre sync e async

// explicar posix, uring e spdk, dizer que aio é muito parecido ao uring no sentido em que também funcoina atrávez de submissões e explicar que o spdk é um caso completamente à parte que solicita o controlo total do disco