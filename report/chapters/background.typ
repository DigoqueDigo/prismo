#import "../utils/functions.typ" : raw_code_block

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

==== Compreensão

Os sistemas de armazenamento modernos aplicam compressão aos blocos únicos identificados pelo processo de deduplicação, assim a informação é codificada de modo mais eficiente, reduzindo a quantidade de bits necessários para representar os mesmos dados. Daqui obtém-se mais aproveitamento do espaço de armazenamento, o que diminui custos e aumenta a rapidez da transferência entre sistemas.

===== Entropia

A fim de conhecer o limite de compressão, a entropia consiste numa medida que reflete a incerteza ou aleatoriedade associada à informação, como tal baixa entropia implica a existência de padrões e uma oportunidade para comprimir, enquanto entropia elevada resulta da aleatoriedade dos dados, havendo por isso pouca margem de compressão.

$
  H = - sum_(i=0)^k p_i log_2(p_i)
$

A partir da fórmula da entropia, ficamos a conhecer o número médio de bits necessários para representar cada símbolo de forma ideal, onde $p_i$ estabelece a probabilidade do símbolo $i$, enquanto $log_2(p_i)$ a informação associada a esse mesmo.

Com o objetivo de esclarecer a fórmula, apresentam-se de seguida os cálculos relativos à string `banana`. Uma vez que o símbolo `a` repete-se três vezes, a sua probabilidade ($p_i$) equivale a $3/6 = 1/2$, raciocínio que aplicamos aos restantes símbolos.

$
  H = - (frac(log_2(1/6), 6) + frac(log_2(1/2), 2) + frac(log_2(1/3), 3)) = 1.46 #text("bits/símbolo")
$

Tendo em conta que a string é constituída por seis caracteres, $6 dot 1.46 = 8.76 #text("bits")$ corresponde ao limite teórico mínimo para codificar `banana` de forma ideal através de codificação ótima como Huffman ou Shannon-Fano.

===== Huffman Coding

A fórmula da entropia nada diz sobre a codificação dos símbolos, para isso é necessário recorrer a um algoritmo de codificação, neste caso abordamos o Huffman Coding, que permitem gerar códigos binários de tamanho variável para uma compressão sem perdas, nela os símbolos mais frequentes recebem códigos mais curtos enquanto os símbolos menos frequentes códigos mais longos.

#figure(
  image("../images/huffman.png", width: 60%),
  caption: [Árvore de Huffman]
) <huffman>

O funcionamento do algoritmo é bastante simples, inicialmente os símbolos são ordenados conforme a sua frequência, de seguida os dois primeiros da lista são agrupados numa árvore cuja raiz tem valor de frequência igual ao somatório, sendo esta colocada de novo na lista conforme o seu valor de frequência.

Ao repetir este processo, obtemos uma árvore com as frequências dos símbolos, deste modo os mais populares estão posicionados perto da raiz e portanto necessitam de menos bits para serem representados. Tendo em consideração a @huffman, a codificação de cada símbolo obtém-se ao atravessar a árvore, onde um salto para a esquerda corresponde a `0`, e para a direita `1`. Por conseguinte, o símbolo `a` possui o código `010`, enquanto `x` corresponde a `10010`.

===== LZ77

Huffman provou que o seu código é a forma mais eficiente de associar uns e zeros a caracteres individuais, é matematicamente impossível superar isso. Porém os algoritmos de compressão procuram identificar padrões que aumentem o tamanho dos símbolos e assim alcançar melhores taxas de compressão.

#figure(
  image("../images/lz77_sliding_window.png", width: 60%),
  caption: [Sliding window no algoritmo LZ77]
) <lz77>

Na grande maioria dos algoritmos, incluído o LZ77, a identificação de padrões ocorre dentro de uma sliding window, assim sempre que um padrão é quebrado, a codificação dos símbolos anteriores é dada por um tuplo com o deslocamento, comprimento, e novo símbolo.

Deste modo o índice $(3,3,a)$ indica que é necessário deslocar três posições para trás e repetir os três símbolos seguintes, sendo no final adicionado um $a$ que originou a quebra do padrão dentro da sliding window.

Quanto maior for a sliding window, maior será a probabilidade de encontrar padrões, no entanto isso acarreta custos computacionais, daí que 65536 bytes seja um standard adotado em vários algoritmos. Por outro lado, reparamos que a codificação dos próprios índices através de Huffman Coding pode trazer melhorias de desempenho ao LZ77, de facto os algoritmos mais recentes, como o `DEFLATE` utilizado no `gzip`, aplicam este princípio.

Tendo por base estes conceitos, a geração de conteúdo que comprime X% torna-se deveras simples, bastando para isso fixar X% dos símbolos da string, enquanto os restantes devem ser completamente aleatórios e sem qualquer padrão possível de exploração. No fundo, procuramos o mínimo de entropia em X% da string, e o máximo de aleatoriedade entre os demais símbolos.

==== Traces

A melhor forma de simular workloads realistas é saber exatamente em que consistem essas workloads, por conseguinte um trace oferece uma visão detalhada de todas as operações que ocorrem no sistema, permitindo conhecer os momentos em que as aplicações e processos interagiram com o sistema de armazenamento.

Idealmente os traces são obtidos em ambiente de produção, dado que somente aí observamos o sistema sob condições reais de uso, portanto faz todo o sentido que o benchmark consiga replicar esse ambiente para termos uma noção do desempenho esperado.

Infelizmente existem pouquíssimos traces disponíveis, e os do #link(<fiu>)[*FIU*] já contam com imensos anos, não sendo a sua replicação viável em máquinas modernas, isto por terem sido obtidos em dispositivos obsoletos aos dias de hoje.

#raw_code_block[
```
<timestamp> <file_id> <process> <offset> <size> <op> <version> <0> <hash>
89967404265337 4253 nfsd 508516672 8 W 6 0 88b93b628d84082186026d9da044f173
89967404311353 4253 nfsd 508516680 8 W 6 0 b5e9f4e5ab62a4fff5313a606b0ad4e3
89967404359328 4253 nfsd 508516688 8 W 6 0 e6434714a2358bc5f55005d6c5502d80
89968195447404 20782 gzip 283193112 8 R 6 0 ef58ea75660587908a49b83a338bff34
89968195487477 20782 gzip 283193120 8 R 6 0 980f03b2810fd0267bea07bc4f0c78fa
89968195487477 20782 gzip 283193120 8 R 6 0 980f03b2810fd0267bea07bc4f0c78fa
```
]

A estrutura do trace é descritiva das operações efetuadas, sendo para cada uma identificado o timestamp, processo responsável e dados da operação de #link(<io>)[*I/O*], como offset, tamanho e tipo de operação. Por fim, cada registo conta com uma assinatura, pois sendo este um trace de deduplicação, é necessário conhecer o bloco alvo da operação, o que permite posteriormente identificar duplicados.







==== Stack de I/O

// apresentar o diagrama da stack de io
// dizer o monte de implicações de egurança e o impacto que isso tem na performance

==== Interfaces de I/O

// explicar a diversiadde de interfaces mas que eles se dividem entre sync e async

// explicar posix, uring e spdk, dizer que aio é muito parecido ao uring no sentido em que também funcoina atrávez de submissões e explicar que o spdk é um caso completamente à parte que solicita o controlo total do disco