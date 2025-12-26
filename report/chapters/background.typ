== Background e Trabalho Relacionado

Este capítulo tem por objetivo apresentar os conceitos e trabalho relacionado que sejam relevantes para a compreensão do projeto, nesse sentido, inicialmente é apresentada uma breve descrição das técnicas de deduplicação e compressão, realçando as implementações e desafios associados.

De seguida, discute-se a importância de replicar traces obtidos em ambientes de produção, bem como o impacto da stack de #link(<io>)[*I/O*] sobre as operações, e de que modo algumas #link(<api>)[*APIs*] procuram reduzir ao máximo as limitações impostas.

Posteriormente são estudadas as últimas técnicas para geração de conteúdo realista e soluções de benchmark amplamente utilizadas pela comunidade, tentando perceber se os resultados apresentados por estas permitem a avaliação realista dos sistemas de armazenamento.

O capítulo termina com uma síntese da informação recolhida, procurando desvendar os impactos que isso terá na arquitetura da proposta de solução.

=== Background

Nesta secção dão-se a conhecer os conceitos de deduplicação e compressão, percebendo de que modo estes podem ser integrados em traces para mais tarde serem aplicados num benchmark que suporte diversas #link(<api>)[*APIs*], sem esquecer de realçar o ciclo de vida no interior da stack de #link(<io>)[*I/O*].

Convém mencionar que a proposta de solução funciona ao nível do bloco, portanto, e por motivos de simplicidade, os conceitos serão apresentados tendo isso em conta, embora a granularidade não lhes seja diretamente incutida.

==== Deduplicação


// esplicar muito sucintamente em que consiste a deduplicação

// esplicar o flow que é realizado a través da stack de io, falar também de como funciona o indice

#figure(
  image("../images/dedup.png", width: 80%),
  caption: [Visão geral do funcionamento da deduplicação]
)

// esplicar a diferença entre as visões e a trasnparaencia que isso oferece ao programador pois assim não se precisa de preocupar com os detalhes fisicos,

// apesar de esistirem diversas extrategias de deduplicação, apenas vamos abordar a direcionada ao bloco, ou seja, divide os dados em conjuntos de blocs de tamamnho fixo, sendo o ultiom bloco preenchido com padding,

// fora isso a deduplicação também pode ser aplicada em diferentes alturas (bulet points)

// Inline (in-band): deduplicação no caminho crítico da escrita (maior overhead de latência)

// Offline (post-processamento): deduplicação em segundo plano (menor impacto na latência, maior uso temporário de espaço)

// Já em relação à indexação também existem diferentes abordagem, não pode ser com bulet point porque quero explicar as vantagens e desvangens de cada uma

// Indexação
// O índice parcial guarda apenas uma parte dos blocos únicos.
// tipos de indexação: parcial e copmlata, posso dizer que o parcial tem em conta a localidade espacial e temporal


==== Compressão

// explicar muito sucintamente a copmressão e as vantagens que podemos retirar daí, isto é importante para o benchmark porque os sistemas mais recentes també já abordam deduplicação

// começar por explicar o lz77 e depois passo para o codigo de huffman, no fim deizer que os dois podem ser combinardo, ou seja, os indices do lz77 podem ser otimizados com codigos de huffman, o que dá orignem ao deflate, o algoritmo tilizador pelo gzip, no entanto o algoritmo mais utilizado atualmente é o lz77

// explicar muito sucintamnte alguns algoritmos e dizer que o sztd é o standard atual

#figure(
  image("../images/huffman.png", width: 30%),
  caption: [Árvore de Huffman]
)

// dizer os codigos resultantes desta arvore
// como base no que aprendemos podemos arranjar uma forma de manipular a cmopressão, esplicar a tecnica do tudo random e tudo a zero

==== Traces

// explicar que os tracespodem ser recolhidos de ambiente em produção e portanto reflem as cargas de trabalho reiais,

// mencionar que os traces dispoinivies são muito antigos e pertencem ao FIU, apresentar a estrutura dos mesmos num pedaço de codigo tipo ASCN e procurar explicar cada campo desse trace

==== Stack de I/O

==== Interfaces de I/O