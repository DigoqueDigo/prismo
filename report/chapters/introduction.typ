== Introdução

Com o aumento das aplicações de inteligência artificial, a necessidade de processar e armazenar grandes quantidades de dados tornou-se cada vez mais relevante, consequentemente os sistemas de armazenamento evoluíram no sentido de oferecer uma maior eficiência de acessos e densidade dos dados.

Sistemas de armazenamento modernos, como o #link(<zfs>)[*ZFS*], disponibilizam uma série de recursos que procuram melhorar a performance das aplicações. Em particular, destaca-se a deduplicação - geralmente abreviada para dedup - que procura reduzir o espaço de armazenamento utilizado ao não reescrever dados que já existam. Por outro lado, a compressão também exerce um papel relevante neste sistema, permitindo aumentar a entropia dos dados ao eliminar aqueles que de algum modo podem ser obtidos através de uma amostra menor.

Neste caso em particular, estamos interessados num sistema orientado ao bloco, portanto a técnica de deduplicação tem como unidade básica um bloco de bytes, geralmente 4096 bytes, mas este valor pode variar conforme o sistema em questão. Da mesma forma, as técnicas de compressão são aplicadas ao nível do bloco (intrabloco), no entanto é possível que alguns sistemas realizem uma análise de entropia entre blocos (interbloco) para obter mais densidade.

Por outro lado, e com o objetivo de ultrapassar as limitações impostas pela stack de #link(<io>)[*I/O*] do kernel, nomeadamente as recorrentes mudanças de contexto, interrupções e cópias entre user e kernel space que tornam evidentes o gargalo de desempenho, surgiram várias #link(<api>)[*APIs*] que visam resolver esses mesmos problemas e geralmente funcionam sobre runtimes assíncronos.

Tendo em consideração a diversidade de técnicas que podemos encontrar num sistema de armazenamento e interfaces de #link(<io>)[*I/O*] existentes para interagir com o disco, torna-se difícil avaliar um qualquer sistema nos seus pontos específicos de funcionamento, isto porque nem todos os sistemas foram desenvolvidos para servir o mesmo fim, devendo assim procurar ajustar o benchmark e workload àquilo que o sistema oferece, pois somente assim será alcançada uma análise justa e correta do seu funcionamento.


=== Definição do Problema e Desafios

Devido ao baixo nível de manipulação em termos de deduplicação e compressão, os benchmarks atuais não proporcionam uma avaliação correta dos sistemas de armazenamento orientados a estas técnicas, contribuindo para análises incorretas dos mesmos. Entre os fatores que justificam esta deficiencia convém destacar os seguintes:

1. A geração de conteúdo deduplicado deve seguir uma determinada distribuição, o que por sua acarreta custos aos nível da seleção dos índices e transferencia de dados entre buffers, consequentemente obtemos um débito menor, e nor pior dos casos não somos capazes de saturar o disco.

2. Os sistemas de armazenamento comportam-se de modo diferente conforme a distribuição de deduplicados e compressão, no entanto os padrões oferecidos pelo #link(<fio>)[*FIO*] são bastante simplistas por realizarem deduplicação sobre o mesmo bloco, o que aumenta a localidade temporal e espacial, benficiando indevidamente o sistema de armazenamento.

3. Embora o #link(<fio>)[*FIO*] seja um benchmark relativamente ofereça suporte a várias interfaces de #link(<io>)[*I/O*] sincronas e assincronas, os restantes oferecem uma gama muito limitada de #link(<api>)[*APIs*], tornando-se a utilização do #link(<fio>)[*FIO*] praticamente obrigatoria, no entanto este não oferece de forma direta suporte para #link(<spdk>)[*SPDK*] e portanto dificulta a avaliação de sistemas que funcionem diretamente sobre um #link(<nvme>)[*NVMe*], ou seja, com bypass da stack de #link(<io>)[*I/O*].

4. Os sistemas de armazenamento evoluiram imenso nos ultimos anos, consequentemente a replicação de traces antigos praticamente instantáneo e como tal não poermite uma avalização correta do sistema, podem não existe um metodo conhecido para extender o trace à medida que o benchmark é executado, sendo que a extensão deverão salvaguar as propiedades de deduplicação e compressão do trace original. No fundo isto resume-se a seguir dados reais enquanto for possível, e depois gerar sinteticamente para manter a avaliação durante o tempo que for necessário.

5. Perante a impossibilidade de acesso a dados reais, o benchmark é obrigado a seguir uma distribuição que defina padrões de acessos e operações a realizar, no entanto a maior parte das soluções é demasiado simplista e não permite testar pardrões do género: READ, WRITE, WRITE, READ.

Posto isto, nenhum benchmark é versátil o suficente para permitir ao uitlizador definir as suas ditribuições de padrões de acesso ou taxa de deduplicados e assim avaliar corretamente as caracteristicas alvo do sistema de armazenamento.

=== Objetivos e Contribuições


// o meu algortimo de sliding window para obter deduplicado de modo eficiente

=== Estrutura do Documento