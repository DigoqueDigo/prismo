=== Trabalho Relacionado

A fim de explorar ferramentas que solucionem problemas semelhantes aos abordados na dissertação, esta secção procura explicar algumas das técnicas utilizadas para obter workloads mais realistas e assim avaliar com maior critério os sistemas de armazenamento.

Na verdade, o problema em questão não é completamente resolvido pelas ferramentas que se apresentam de seguida, cada uma sofre de limitações ao nível da geração de conteúdo, replicação de traces, ou suporte a diversas #link(<api>)[*APIs*] de #link(<io>)[*I/O*]. No entanto, convém perceber essas mesmas limitações para concluir em que medida a solução proposta se destaca das existentes.

==== DEDISbench

Tratando-se de um micro-benchmark de #link(<io>)[*I/O*] para sistemas de deduplicação orientados ao bloco, o DEDISbench gera dados com padrões de deduplicação semelhantes aos encontrados em ambientes reais, para isso serve-se do DEDISgen, que após analisar um dataset, resume a informação numa grelha que indica a quantidade de blocos com X cópias.

#figure(
 table(
   columns: (1fr, 1fr, 1fr, 1fr),
   inset: 6pt,
   align: horizon + left,
   fill: (x, y) => if y == 0 { gray.lighten(60%) },
   table.header(
     [*Nº de Cópias*], [*Nº de Blocos*],
     [*Blocos Únicos (%)*], [*Blocos Totais (%)*],
   ),
   [0], [2000], [50.00%], [22.73%],
   [1], [1000], [25.00%], [22.73%],
   [3], [600],  [15.00%], [27.27%],
   [5], [400],  [10.00%], [27.27%],
 ),
 caption: [Associação entre distribuição de duplicados e blocos totais]
) <dedisbenchdedup>

As duas primeiras colunas da @dedisbenchdedup representam a informação da distribuição de duplicados, sendo esta definida mediante a quantidade de cópias e blocos, neste caso em concreto, 2000 blocos não possuem qualquer cópia e portanto são únicos. Por outro lado, 400 blocos únicos apresentam cada um cinco cópias, resultando num total de 2400 blocos.

De facto, a forma como a distribuição é especificada dificulta a perceção da amostra total, apesar de 50% dos blocos únicos estarem contidos no mesmo grupo, esses representam somente um quarto da população total, pois à medida que a quantidade de cópias aumenta, o peso do grupo cresce proporcionalmente.

Neste sentido, o DEDISgen é responsável por fornecer a distribuição de duplicados num formato passível de interpretação ao DEDISbench, algo que mais tarde será replicado numa workload com padrões de acesso e débito definidos pelo utilizador.

- *Acesso Sequencial:* as operações de #link(<io>)[*I/O*] são realizadas de modo sequencial, o que beneficia a localidade espacial, visto os blocos encontrarem-se fisicamente próximos no sistema de armazenamento.

- *Acesso Uniforme:* os offsets são obtidos através de uma distribuição uniforme, daí que todas as zonas do disco sejam alvo da mesma carga, não havendo por isso benefícios obtidos através da localidade espacial ou temporal.

- *Acesso TPC-C:* uma distribuição com hotspots é responsável por estabelecer os padrões de acesso, consequentemente um subconjunto reduzido de blocos é alvo da maioria dos acessos, contribuindo assim para um bom uso da cache e reflexão do comportamento de partilha de blocos duplicados.

Em sentido semelhante, o DEDISbench permite manipular outros parâmetros da workload, dos quais se destacam a distribuição das operações de #link(<io>)[*I/O*] e o débito a que estas são realizadas, podendo ser nominal ou stress quando pretendemos extrair o máximo das capacidades do sistema de armazenamento.

Apesar desta versatilidade, a impossibilidade de simular traces e falta de suporte para interfaces assíncronas tornam a ferramenta pouco apetecível face aos dispositivos atuais que beneficiam as estratégias de polling face a interrupções, consequentemente não é possível atingir o máximo de performance.

Por outro lado, a própria distribuição de duplicados resulta numa limitação das workloads, uma vez que esta é definida em termos absolutos, ao ser atingido o limite de blocos únicos e respetivas cópias torna-se impossível continuar a escrever mais blocos, afinal não conseguimos depreender o grupo a que estes pertenceriam. Além disso, uma workload inferior àquela estabelecida pela distribuição resulta numa infração da taxa de duplicados, daí que a única forma de respeitar os limites seja não estender nem diminuir a distribuição.




==== DEDISbench++

// mencionar o que o dedisbench++ apresenta como melhorias ao dedisbench, mostras a nova distribuição atraves de uma tabela

// mencionar muito brevemente como são calculadas as copmressões

// espeficicar que isso só funciona se souber ẁ partida o numero total de blocos que vou utilizar durante o benchmark, o que se torna muito complicado nas workloads onde isso não é especificado

==== FIO

// mencionar que o fio é o estado da arte relativamente a benchmark para sistemas de armazenamento

// suporte a vaŕias interfaces, no entanto o spdk tem a desvantagem de ser através de um plugin cuja utilização não é completamente trivial

// dizer que permite várias opções para manipular as workloads, no entanto os requisitos para um benchmark de deduplicação são muito superiores aqueles que o fio permite

// a taxa de copmressão e deduplicação é obtida de modo muito simplesta

// é possivel seguir traces, no entanto não existe uma distinção clara entre processos, algo que os traces do FIU especificam claramente como visto nos exemplos anteirores

=== Discussão

// após experimentar todas as ferramentas mencionadas anteirormente, podemos concluir que os requisitos para um benchmark de deduplicação são cumpridos parcilamente, faltando alguns pontos que o meu benchmark tentará melhorar

// a integração direta com o spdk

// controlo maior sobre os parametros das as workloads

// seguir traces com a especificação de processos

// combinar traces realista com sisnteticos quando o trace não apresenta todos os dados necessários à realização da operação de io