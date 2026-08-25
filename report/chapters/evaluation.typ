#import "../utils/charts.typ" : tool-bars, tool-lines, cpu-stack, component-bars, trace-lines
#import "../utils/functions.typ" : question_block, validation_point_block, doc_table, cell_yes, cell_no, cell_partial

== Avaliação Experimental <chapter4>

Após a descrição da arquitetura e dos mecanismos que sustentam a geração de workloads realistas, importa agora avaliar experimentalmente o Prismo, por um lado validando a sua correção e comparando o desempenho com as ferramentas de referência, e por outro demonstrando que a incorporação de propriedades de conteúdo nas workloads revela comportamentos dos sistemas de armazenamento que, de outro modo, permaneceriam invisíveis.

Neste sentido, são definidas sete perguntas de investigação e quatro pontos de validação que orientam as experiências realizadas:

#question_block[
/ Q1: As propriedades intrínsecas dos dados, nomeadamente a compressibilidade e a taxa de duplicados, influenciam o desempenho dos sistemas de armazenamento?
]

#question_block[
/ Q2: Diferentes distribuições de duplicados e compressibilidade resultam em comportamentos distintos por parte dos sistemas avaliados?
]

#question_block[
/ Q3: A escolha da interface de @io influencia significativamente o desempenho medido pelo benchmark, e em que medida essa influência varia consoante o tipo de workload?
]

#question_block[
/ Q4: As workloads baseadas em traces reproduzem fielmente as propriedades observadas nos dados originais, e as estratégias de extensão preservam essas características?
]

#question_block[
/ Q5: Distribuições de acesso com forte localidade revelam comportamentos relacionados com mecanismos de cache que não são exercitados por workloads aleatórias?
]

#question_block[
/ Q6: A escolha do sistema de armazenamento condiciona o benefício obtido a partir das propriedades dos dados, e qual o custo computacional associado às otimizações sensíveis ao conteúdo?
]

#question_block[
/ Q7: Em que medida a avaliação produzida pelo Prismo difere daquela obtida através dos benchmarks existentes, e que conclusões sobre os sistemas de armazenamento avaliados se tornam possíveis a partir dessa diferença?
]

#validation_point_block[
/ V1: O Prismo é capaz de gerar workloads de @io reprodutíveis que incorporam propriedades realistas de conteúdo, nomeadamente distribuições de duplicados e compressibilidade.
]

#validation_point_block[
/ V2: As medições produzidas são estáveis ao longo da execução e comparáveis entre ferramentas, garantindo que as variações observadas decorrem das propriedades das workloads e não de efeitos alheios.
]

#validation_point_block[
/ V3: O Prismo fornece todas as informações relevantes para a análise e avaliação do sistema de armazenamento, de modo a fundamentar as configurações dos sistemas para workloads específicas.
]

#validation_point_block[
/ V4: As conclusões alcançadas através do Prismo sobre os sistemas de armazenamento avaliados não são passíveis de obtenção com os benchmarks existentes, decorrendo essa diferença do realismo do conteúdo gerado e não da instrumentação da ferramenta.
]

Posto isto, o capítulo inicia-se pela descrição da metodologia experimental, avançando depois para a validação da ferramenta através da demonstração de equivalência com os benchmarks de referência em workloads genéricas. De seguida, são analisados cenários onde as funcionalidades exclusivas do Prismo se revelam determinantes, nomeadamente a geração de conteúdo com propriedades de deduplicação e compressão, a comparação entre interfaces de @io e a replicação de workloads baseadas em traces. Por fim, explora-se a localidade de acesso enquanto eixo complementar, antes de sintetizar as conclusões.

=== Metodologia <methodology>

A avaliação experimental assenta na execução de um conjunto alargado de workloads sobre a mesma máquina e sob condições controladas, sendo esta secção responsável por descrever o ambiente utilizado, as ferramentas comparadas, o procedimento seguido e as métricas recolhidas, de modo a que os resultados apresentados nas secções seguintes possam ser corretamente interpretados e reproduzidos.

==== Setup Experimental

Todas as experiências foram conduzidas numa única máquina, evitando assim que diferenças de hardware ou de configuração entre execuções se reflitam nas métricas recolhidas. As especificações do sistema encontram-se descritas na @hardware, sendo de realçar a capacidade de memória disponível, pois a condição de terminação de algumas workloads é calculada com base nesta.

#figure(
  doc_table(
    columns: (1fr, 1.6fr),
    header: ([Componente], [Especificação]),
    [Sistema operativo], [Ubuntu 20.04.6 LTS (Focal Fossa)],
    [Kernel], [Linux 5.4.0-216-generic],
    [Arquitetura], [x86_64],
    [Processador], [2 $times$ Intel Xeon Gold 6342],
    [Núcleos], [48 físicos (24 por processador), 96 threads],
    [Frequência], [800 MHz base, 2.80 GHz máxima],
    [Memória], [188.23 GiB, DDR4-3200],
    [Dispositivo de armazenamento], [Dell Enterprise @nvme P5600 MU U.2, 1.46 TiB],
  ),
  caption: [Especificações da máquina utilizada nas experiências]
) <hardware>

O sistema opera sobre Ubuntu 20.04.6 LTS com kernel Linux 5.4.0-216-generic, versão que condiciona as funcionalidades disponíveis nas interfaces assíncronas, em particular no io_uring, cuja implementação tem vindo a ser progressivamente otimizada desde a sua introdução @uring_kernel.

Convém realçar que todos os acessos são efetuados com a flag `O_DIRECT`, o que elimina a intervenção da page cache e garante que os pedidos atingem efetivamente o dispositivo, sendo esta uma condição indispensável para que as métricas reflitam o comportamento do sistema de armazenamento e não o da memória @didona2022 @ren2023. Esta garantia é integral quando o dispositivo é acedido diretamente, sem sistema de ficheiros interposto, no entanto os sistemas de ficheiros avaliados impõem-lhe restrições que serão detalhadas adiante.

==== Ferramentas Comparadas

A avaliação confronta o Prismo, na versão 1.0.0, com o @fio e o Vdbench, duas das ferramentas mais utilizadas na avaliação de sistemas de armazenamento, nas versões mais recentes de cada uma, respetivamente a 3.42 e a 5.04.07 @fio_docs @vdbench.

Estas ferramentas não partilham, no entanto, o mesmo âmbito de aplicação, pois enquanto o @fio suporta as mesmas interfaces de @io que o Prismo, apesar de o acesso ao @spdk ser conseguido através de um plugin externo, o Vdbench opera exclusivamente sobre POSIX síncrono, o que restringe a comparação entre as três ferramentas a esse cenário @fio_docs @vdbench.

#figure(
  doc_table(
    columns: (3fr, 1fr, 1fr, 1fr),
    header: ([Interface], [Prismo], [FIO], [Vdbench]),
    [POSIX], cell_yes, cell_yes, cell_yes,
    [libaio], cell_yes, cell_yes, cell_no,
    [io_uring], cell_yes, cell_yes, cell_no,
    [SPDK], cell_yes, cell_partial[Plugin], cell_no,
  ),
  caption: [Interfaces suportadas por cada ferramenta]
) <interfaces>

As interfaces comuns a mais do que uma ferramenta foram configuradas de forma equivalente, operando o io_uring e o libaio com 128 entradas na fila de submissão, sendo no primeiro caso ativado o polling do kernel através das flags `IORING_SETUP_SQPOLL` e `IORING_SETUP_SQ_AFF`, enquanto o @spdk recorre a uma máscara de quatro reactors e oito threads lógicas.

#figure(
  doc_table(
    columns: (3fr, 1fr, 1fr, 1fr),
    header: ([Funcionalidade], [Prismo], [FIO], [Vdbench]),
    [Distribuição de duplicados], cell_yes, cell_partial[Taxa global], cell_partial[Rácio global],
    [Distribuição de compressibilidade], cell_yes, cell_partial[Taxa global], cell_partial[Rácio global],
    [Replay de traces], cell_yes, cell_partial[Limitado], cell_no,
    [Extensão sintética de traces], cell_yes, cell_no, cell_no,
  ),
  caption: [Funcionalidades suportadas por cada ferramenta]
) <ferramentas>

Esta equivalência nem sempre é alcançável no que toca ao conteúdo, uma vez que o Vdbench não dispõe de barreiras de sincronização nem de geração de conteúdo constante, sendo a distribuição Zipfian aproximada através do parâmetro `hotband`, aproximações que devem ser tidas em conta na leitura dos resultados @vdbench.

==== Campanha Experimental

A campanha experimental é constituída por quinze workloads base, cada uma isolando exatamente uma dimensão relativamente à anterior, o que permite atribuir as diferenças observadas a um único fator. Estas workloads encontram-se descritas na @workloads-base, e posteriormente replicadas para as quatro interfaces de @io avaliadas, do que resulta um total de sessenta configurações.

#[
#show figure: set block(breakable: true)
#figure(
  doc_table(
    columns: (auto, 1.2fr, 1.6fr),
    header: ([\#], [Dimensão isolada], [Parâmetros principais]),
    fill: (x, y) => if x == 0 { gray.lighten(60%) },
    [*01*], [Débito sequencial de escrita], [Escrita, sequencial, conteúdo constante],
    [*02*], [Débito sequencial de leitura], [Leitura, sequencial, conteúdo constante],
    [*03*], [Tamanho do bloco], [Escrita, sequencial, blocos de 64 KiB],
    [*04*], [Acesso aleatório], [Leitura, aleatório],
    [*05*], [Contenção entre leituras e escritas], [50/50 leitura e escrita, aleatório],
    [*06*], [Localidade de acesso], [50/50 leitura e escrita, Zipf(0.9)],
    [*07*], [Rácio assimétrico], [90/10 escrita e leitura, sequencial],
    [*08*], [Custo da durabilidade], [Sequência de operações, `fsync` a cada 1024 escritas],
    [*09*], [Paralelismo], [50/50 leitura e escrita, aleatório, 3 jobs],
    [*10*], [Compressibilidade isolada], [Zipf(0.9), três níveis de compressão],
    [*11*], [Duplicados e compressibilidade], [Zipf(0.9), três níveis de duplicados e compressão],
    [*12*], [Localidade real e operações sintéticas], [Acessos do trace homes, 70/30 leitura e escrita],
    [*13*], [Operações reais e acessos sintéticos], [Operações do trace cheetah, Zipf(0.9)],
    [*14*], [Replay integral de um fileserver], [Trace homes, extensão por repetição],
    [*15*], [Replay integral de um webmail], [Trace webmail, extensão por amostragem e regressão],
  ),
  caption: [Workloads base da campanha experimental]
) <workloads-base>
]

A dimensão das workloads foi fixada em 752.91 GiB, valor que corresponde a quatro vezes a memória disponível, garantindo assim que o conjunto de dados manipulado não é passível de acomodação em cache e que os pedidos atingem efetivamente o dispositivo. Nas workloads mais demoradas, em particular aquelas assentes em barreiras de sincronização, alcançar este volume implicaria execuções incomportáveis, daí que nestes casos a condição de paragem seja de quinze minutos de execução, duração suficiente para que o sistema atinja um regime estacionário @traeger2008 @tarasov2011.

Cada configuração corresponde a uma única execução, sendo a estabilidade das medições aferida a partir das séries temporais recolhidas ao longo dessa execução, conforme descrito adiante @traeger2008 @tarasov2011.

===== Estabilização do Sistema entre Execuções

Antes de cada execução é aplicado um procedimento de limpeza que garante o isolamento entre medições, evitando que o estado deixado pela execução anterior contamine os resultados da seguinte. Este procedimento inicia-se com um `sync`, que força a escrita para o disco de todas as páginas ainda pendentes em memória, assegurando deste modo que nenhuma operação da execução anterior transita para a janela de medição seguinte.

De seguida, é escrito o valor 3 em `/proc/sys/vm/drop_caches`, invalidando não só a page cache mas também as estruturas de dentries e inodes mantidas pelo kernel. Esta invalidação assume particular importância nos sistemas de ficheiros avaliados, uma vez que a flag `O_DIRECT` não impede o recurso à cache quando as otimizações de conteúdo se encontram ativas.

Por fim, o procedimento aguarda cinco minutos antes de iniciar a execução seguinte, período durante o qual o dispositivo permanece em repouso e conclui as tarefas internas de manutenção, como o garbage collection e o esvaziamento dos buffers. Sem esta pausa, uma workload intensiva em escritas deixaria o dispositivo num estado degradado, penalizando artificialmente a execução subsequente @traeger2008 @tarasov2011.

==== Sistemas de Armazenamento Avaliados

As workloads são executadas sobre três sistemas de armazenamento distintos, sendo o primeiro o próprio dispositivo @nvme acedido sem qualquer sistema de ficheiros interposto, cenário que serve a comparação entre ferramentas e entre interfaces de @io, enquanto o Btrfs e o @zfs são avaliados por implementarem otimizações sensíveis às propriedades dos dados, nomeadamente compressão e deduplicação.

#figure(
  doc_table(
    columns: (1.3fr, 1fr, 1fr, 1fr),
    header: ([Sistema], [Versão], [Compressão], [Deduplicação]),
    [@nvme `/dev/nvme0n1`], [Kernel 5.4], [Não], [Não],
    [Btrfs], [btrfs-progs 5.4.1-2], [zstd, nível 3], [Offline, via bees 0.11],
    [@zfs], [2.4.0], [zstd, nível 3], [Inline],
  ),
  caption: [Sistemas de armazenamento avaliados e respetiva configuração]
) <sistemas>

Em ambos os sistemas de ficheiros a compressão recorre ao zstd no nível 3, valor que estabelece o compromisso entre a qualidade da compressão e a rapidez com que esta é alcançada, visto níveis superiores comprimirem mais, no entanto a um custo computacional que enviesaria a avaliação do sistema de armazenamento em detrimento da avaliação do próprio algoritmo @btrfs_docs.

No @zfs o recordsize foi ainda fixado em 4 KiB, de modo a coincidir com os blocos manipulados pela generalidade das workloads. Esta decisão revela-se indispensável, isto porque a deduplicação opera ao nível do record, daí que um valor superior implicasse que blocos duplicados de 4 KiB jamais originassem records idênticos, tornando a otimização inoperante face ao conteúdo gerado @zfs_docs.

As estratégias de deduplicação diferem igualmente, visto o @zfs atuar no caminho crítico de @io enquanto o Btrfs delega a tarefa no bees, um serviço que percorre o sistema de ficheiros em segundo plano recorrendo a um índice limitado a 1 GiB @bees. Tal diferença condiciona a leitura dos resultados, dado que no Btrfs a redução de espaço apenas se manifesta após a passagem do serviço, ao contrário do @zfs onde é imediata, embora ao custo de latência acrescida nas escritas @koller2010 @meyer2012.

Importa realçar que a garantia oferecida pela flag `O_DIRECT` deixa de ser absoluta quando estas otimizações se encontram ativas. No Btrfs, tanto as leituras de dados comprimidos como as escritas sobre inodes com checksums acabam por transitar pela page cache, enquanto no @zfs a deduplicação e as escritas diretas são mutuamente incompatíveis, o que obrigou a desativar a propriedade `direct` nos conjuntos de dados avaliados @btrfs_docs @zfs_docs.

Desta forma, os resultados obtidos sobre sistemas de ficheiros não são diretamente comparáveis com os do acesso direto ao dispositivo no que respeita ao efeito da page cache, limitação que decorre da natureza das otimizações avaliadas e não da metodologia adotada.

Em qualquer dos casos, o Prismo, tal como o @fio e o Vdbench, emite pedidos de leitura e escrita de tamanho fixo sobre um ficheiro previamente alocado, exercitando por isso o caminho de dados e não as operações de metadados. Assim sendo, não se trata de uma avaliação de sistemas de ficheiros, mas antes da forma como cada sistema reage às propriedades do conteúdo que lhe é submetido.

==== Métricas

As métricas recolhidas dividem-se entre aquelas reportadas pelas próprias ferramentas e as obtidas ao nível do sistema. Do primeiro grupo fazem parte o débito, os @iops e a latência, esta última caracterizada não apenas pelo valor médio mas também pelos percentis p50, p99 e p99.9, pois a média isoladamente esconde o comportamento da cauda da distribuição @traeger2008 @tarasov2011.

Já as métricas de sistema, nomeadamente a utilização de @cpu e de @ram, são recolhidas através do `pcp dstat` com uma frequência de amostragem de um segundo, que constitui a única fonte comum às três ferramentas e portanto a única que permite uma comparação justa do consumo de recursos.

Nas workloads que exercitam deduplicação e compressão é ainda registado o espaço efetivamente ocupado em disco, pois só através deste é possível confirmar que as otimizações do sistema de armazenamento foram de facto acionadas pelo conteúdo gerado.

===== Agregação de Métricas

Uma vez recolhidas, as métricas são apresentadas através do valor reportado pela ferramenta, acompanhado do desvio padrão calculado sobre a respetiva série temporal. Deste modo, as barras de erro presentes nas figuras adiante traduzem a oscilação da medição ao longo da execução, sendo descartadas as primeiras amostras de modo a excluir o período de arranque.

No entanto, os percentis de latência constituem um caso particular, dado que os relatórios registam apenas o valor já calculado sobre a totalidade da execução, não sendo por isso possível acompanhar a sua evolução temporal nem associar-lhes uma medida de dispersão.

=== Validação do Prismo <validation>

Previamente a utilizar o Prismo para avaliar sistemas de armazenamento, é necessário estabelecer confiança na ferramenta, daí que esta secção procure demonstrar que os resultados produzidos são fiáveis e comparáveis aos das ferramentas de referência em workloads genéricas, ao mesmo tempo que valida a correção dos mecanismos de geração de conteúdo.

==== Débito dos Componentes

Antes de confrontar o Prismo com as ferramentas de referência, importa determinar o débito máximo que os seus componentes conseguem sustentar, pois qualquer limite imposto pela própria ferramenta comprometeria a atribuição dos resultados ao sistema de armazenamento. Para o efeito, cada gerador foi exercitado isoladamente, sem submissão de pedidos, ao longo de 100 milhões de invocações.

#figure(
  grid(
    columns: 2, gutter: 6pt, row-gutter: 10pt,
    component-bars("componentes.csv", "Acesso"),
    component-bars("componentes.csv", "Conteúdo"),
    component-bars("componentes.csv", "Operação"),
    component-bars("componentes.csv", "Extensão"),
  ),
  caption: [Débito máximo de cada componente do Prismo]
) <componentes>

Os valores reunidos na @componentes revelam uma disparidade de duas ordens de grandeza entre variantes, sendo o gerador de operações constante o mais rápido, com 844.8 milhões de operações por segundo, enquanto o gerador de conteúdo com deduplicação se fica pelos 4.8 milhões, diferença que se explica pelo trabalho envolvido, dado que o primeiro devolve um valor fixo e o segundo constrói um bloco de 4096 bytes respeitando uma distribuição de duplicados e de compressibilidade.

Uma workload combina sempre um gerador de cada categoria, atuando estes em série na preparação de cada pedido, pelo que o débito da configuração resulta da soma dos respetivos tempos. Convém realçar que uma extensão de trace pode assumir o papel de qualquer um destes geradores, admitindo-se por isso configurações que recorrem a três extensões em simultâneo.

Nestes termos, a configuração menos favorável corresponde a três extensões por regressão, que consomem 213.7 nanossegundos cada e limitam o Prismo a 1.6 milhões de operações por segundo. Já a pior combinação entre geradores convencionais, reunindo acesso Zipfian, operações por percentagem e conteúdo com deduplicação, totaliza 294.0 nanossegundos e permite 3.4 milhões de operações por segundo.

Assim sendo, mesmo a configuração mais exigente mantém-se cerca de catorze vezes acima do débito máximo observado nas experiências, que foi de 113 015 @iops na workload 02 (demonstrado adiante), margem suficiente para acomodar dispositivos consideravelmente mais rápidos do que o utilizado. Deste modo, nenhuma combinação de geradores constitui o fator limitante da avaliação, e os resultados apresentados traduzem o comportamento do sistema de armazenamento.

==== Reprodutibilidade

A validação assenta nas workloads 01 a 09 executadas sobre o dispositivo @nvme através da interface POSIX, único cenário em que as três ferramentas operam sobre configurações muito semelhantes e podem por isso ser confrontadas diretamente.

O primeiro requisito a verificar é a estabilidade das medições, pois uma ferramenta cujos valores oscilem acentuadamente não permite distinguir o efeito da workload do simples ruído experimental.

Esta estabilidade é quantificada pelo coeficiente de variação das séries por segundo recolhidas ao longo de cada execução, apresentado na @reprodutibilidade.

#figure(
  tool-lines("validacao-cv.csv", ylabel: [Coeficiente de variação (%)]),
  caption: [Coeficiente de variação do débito de operações por workload]
) <reprodutibilidade>

Os valores obtidos pelo Prismo situam-se entre 0.50% e 2.71%, gama que confirma tratar-se de medições estáveis, sendo de realçar que em sete das nove workloads a oscilação é inferior à do Vdbench e em quatro delas inferior também à do @fio.

A workload 03 destaca-se por apresentar a maior dispersão nas três ferramentas, comportamento que não é imputável aos benchmarks mas ao dispositivo, pois trata-se da única workload com blocos de 64 KiB e o débito alcançado esgota periodicamente a cache interna do @nvme. Por outro lado, o Vdbench exibe oscilações acima de 9% em várias workloads, o que é consistente com as pausas introduzidas pela máquina virtual do Java.

Estes valores fixam o critério de leitura adotado no restante capítulo, dado que uma diferença entre duas medições só é considerada efetiva quando excede a oscilação que a própria medição apresenta. Na prática, e tomando o valor mais elevado registado pelo Prismo, diferenças inferiores a 3% são tratadas como equivalência e não como vantagem de uma ferramenta sobre a outra @traeger2008 @tarasov2011.

Convém realçar que esta análise caracteriza a estabilidade da medição e não a variabilidade entre execuções independentes, a qual exigiria a repetição integral da campanha e não foi avaliada, conforme se assinala nas limitações.

==== Equivalência em Workloads Genéricas

Aferida a estabilidade das medições, importa agora verificar se o Prismo produz valores comparáveis aos das ferramentas de referência quando submetido às mesmas condições, confronto que incide sobre o débito de operações, a latência e o consumo de recursos.

#figure(
  tool-bars("validacao-iops.csv", ylabel: [Milhares de @iops]),
  caption: [Débito de operações por workload em cada ferramenta]
) <validacao-iops>

O débito apresentado na @validacao-iops revela concordância entre as três ferramentas em praticamente todas as workloads, com desvios que se mantêm abaixo do limiar de dispersão nos cenários dominados pelo dispositivo, ou seja naqueles assentes em acessos aleatórios. Nestas condições o dispositivo satura muito antes de qualquer ferramenta se aproximar do seu limite, pelo que as três medem inevitavelmente a mesma realidade.

As workloads sequenciais afastam-se ligeiramente deste padrão, com uma vantagem para o Prismo na ordem dos cinco pontos percentuais, atribuível ao modelo produtor-consumidor descrito no @chapter3, uma vez que a preparação dos pedidos decorre numa thread distinta daquela que os submete e permite manter o dispositivo ocupado enquanto o bloco seguinte é construído.

A workload 07 constitui a única exceção relevante, com o Prismo a ficar cerca de um quinto abaixo do @fio. Ambas as ferramentas executaram exatamente o mesmo trabalho, com idêntico número de operações, volume escrito e repartição entre leituras e escritas, diferindo apenas na duração, pelo que a explicação tem de residir no custo por pedido e não na carga submetida.

Descartam-se desde logo duas hipóteses, dado que a geração de conteúdo é equivalente nas duas ferramentas, visto o parâmetro `buffer_compress_percentage` do @fio ativar automaticamente o `refill_buffers` @fio_docs, e dado que a @componentes demonstra que os geradores sustentam um débito duas ordens de grandeza superior ao exigido por esta workload.

Esta leitura é corroborada pela workload 03, que recorre igualmente à regeneração de conteúdo mas com blocos dezasseis vezes maiores, reduzindo na mesma proporção o número de pedidos, e onde o Prismo volta a apresentar vantagem. Deste modo, tudo indica tratar-se de um custo que se manifesta por pedido e não por byte transferido.

#figure(
  grid(
    columns: 2, gutter: 4pt,
    tool-bars("validacao-latencia.csv", ylabel: [Latência média (µs)],
              width: 6.0cm, height: 4.6cm, legend: false),
    tool-bars("validacao-p99.csv", ylabel: [Latência p99 (µs)],
              width: 6.0cm, height: 4.6cm),
  ),
  caption: [Latência média e percentil 99 por workload em cada ferramenta]
) <validacao-latencia>

A latência média confirma a equivalência estabelecida pelo débito, sendo o painel esquerdo da @validacao-latencia essencialmente uma imagem espelhada da figura anterior, o que demonstra medir a instrumentação do Prismo a mesma realidade que as ferramentas consagradas.

O percentil 99, no painel direito, revela porém um padrão que a média esconde, pois o Prismo apresenta uma cauda mais pesada nas workloads que combinam escritas com concorrência, chegando a duplicar o valor do @fio, enquanto nas sequenciais a situação se inverte a seu favor.

Este comportamento é atribuível ao mesmo modelo produtor-consumidor que explica a vantagem no débito, uma vez que a fila de pedidos impõe backpressure ao produtor sempre que a capacidade limite é atingida, penalizando os pedidos que nela aguardam sem afetar o débito agregado. Assim sendo, o Prismo privilegia a ocupação do dispositivo em detrimento da previsibilidade individual de cada pedido, compromisso que importa ter presente sempre que a cauda da distribuição seja o objeto de estudo.

#figure(
  grid(
    columns: 2, gutter: 4pt,
    tool-bars("validacao-cpu.csv", ylabel: [Utilização de @cpu (%)],
              width: 6.0cm, height: 4.4cm, legend: false),
    tool-bars("validacao-ram.csv", ylabel: [Memória utilizada (GiB)],
              width: 6.0cm, height: 4.4cm),
  ),
  caption: [Consumo de recursos por workload em cada ferramenta]
) <validacao-recursos>

Por fim, o consumo de recursos apresentado na @validacao-recursos demonstra que as decisões arquiteturais do Prismo não acarretam um custo desproporcional, sendo a utilização de @cpu indistinguível da do @fio, ao passo que o Vdbench se destaca por executar sobre a máquina virtual do Java @vdbench.

Ao nível da memória utilizada pela máquina a distância é mais nítida, embora modesta em termos absolutos, pois o Prismo ocupa cerca de meio gigabyte acima do @fio, diferença que decorre do pool de pacotes pré-alocado durante a inicialização do canal descrito na @chapter3, enquanto o Vdbench requer perto de três gigabytes adicionais. Tratando-se de uma máquina com 188 GiB, nenhum destes valores condiciona a avaliação.

==== Fator Limitante da Avaliação

O débito dos componentes, analisado no início desta secção, demonstrou que nenhum gerador limita a avaliação, no entanto essa medição incidiu sobre cada peça isoladamente e não sobre a execução completa. A repartição do tempo de @cpu recolhida pelo `pcp dstat` permite confirmar a conclusão em condições reais.


#figure(
  cpu-stack("validacao-cpu-tipos.csv"),
  caption: [Repartição do tempo de @cpu durante a execução do Prismo]
) <validacao-cpu-tipos>

A @validacao-cpu-tipos apresenta as componentes de utilizador, sistema e espera por @io, correspondendo o remanescente até 100% a tempo ocioso, que se situa acima de 95% em todas as workloads com um único job e em 91.6% na workload 09. Estes valores decorrem de a máquina disponibilizar 96 threads de execução enquanto o benchmark ocupa apenas uma, ou três no caso da workload 09.

Mais relevante é a proporção entre as componentes ativas, visto a espera por @io representar entre 29% e 48% do tempo não ocioso nas workloads com um único job, valor que ascende a 70% na workload 09. Por outras palavras, o processador passa mais tempo à espera do dispositivo do que a executar código do benchmark, seja em espaço de utilizador ou dentro do kernel.

Convém realçar que a componente de utilizador, onde reside a geração de conteúdo e a preparação dos pedidos, não ultrapassa 0.77% em qualquer workload, mantendo-se sempre abaixo da componente de sistema. Assim sendo, o custo do Prismo é dominado pelas chamadas ao sistema inerentes à interface POSIX e não pela lógica própria da ferramenta.

Em suma, os resultados apresentados ao longo desta secção medem a capacidade do sistema de armazenamento e não o limite das ferramentas utilizadas, conclusão que sustenta a interpretação de todas as comparações realizadas no restante capítulo @traeger2008 @didona2022.

==== Validação da Geração de Conteúdo

A equivalência demonstrada até aqui atesta a fiabilidade da instrumentação, no entanto nada diz sobre a propriedade que distingue o Prismo, ou seja, a capacidade de gerar conteúdo com distribuições de duplicados e compressibilidade controladas. Assim sendo, o #link("https://github.com/dsrhaslab/prismo/blob/main/tools/deltoide/README.md")[Deltoide] é aplicado sobre os dados efetivamente escritos, extraindo as distribuições presentes no dispositivo de armazenamento.

A workload 11 foi configurada com três grupos de duplicados, cada um com a sua própria repartição de compressibilidade, sendo os valores configurados e os medidos apresentados na @conteudo.

#figure(
  doc_table(
    columns: (0.5fr, 0.8fr, 0.7fr, 1.4fr, 1.4fr),
    header: ([Cópias], [Configurado], [Medido], [Redução configurada], [Redução medida]),
    [0], [50.0%], [51.1%], [50% sem redução \ 50% a 50% de redução], [49.7% sem redução \ 50.3% a 49.7% de redução],
    [1], [35.0%], [34.6%], [70% a 30% de redução \ 30% a 20% de redução], [71.2% a 30.2% de redução \ 28.8% a 19.8% de redução],
    [3], [15.0%], [14.3%], [100% sem redução], [100% sem redução],
  ),
  caption: [Distribuição de duplicados e compressibilidade configurada e medida]
) <conteudo>

Os desvios observados na @conteudo não ultrapassam 1.1% na repartição de duplicados nem 1.2% na de compressibilidade, valores que confirmam uma reprodução fiel a distribuição solicitada. Além disso, observa-se um comportamento particular do algoritmo de geração de duplicados: à medida que os blocos repetidos avançam através da janela, a percentagem medida para o último nível de cópias nunca pode exceder a percentagem solicitada, uma vez que este corresponde ao último estágio.

Estes resultados fundamentam o V1, dado que a distribuição medida sobre os dados escritos corresponde à configurada, e não a uma aproximação global como a praticada pelas ferramentas de referência, cujas configurações apenas admitem uma taxa única de duplicados e de compressibilidade @fio_docs @vdbench.

=== Impacto das Propriedades dos Dados no Desempenho <data-properties>

Estabelecida a credibilidade do Prismo enquanto instrumento de medição, esta secção explora o eixo que o distingue das ferramentas de referência, nomeadamente o impacto das propriedades intrínsecas dos dados no desempenho dos sistemas de armazenamento. Na prática, benchmarks que ignoram a compressibilidade e a taxa de duplicados tendem a produzir avaliações que não refletem o comportamento real dos sistemas sensíveis ao conteúdo.

Toda a análise decorre sobre o Btrfs e o @zfs, únicos sistemas avaliados que reagem ao conteúdo, começando pelo estabelecimento de uma linha de base com dados aleatórios, uma vez que só a comparação entre workloads que diferem exclusivamente nas propriedades do conteúdo, executadas sobre o mesmo sistema, permite isolar o contributo dessas propriedades.

==== Linha de Base por Sistema de Armazenamento

As workloads 04, 05 e 06 operam sobre conteúdo aleatório, logo incompressível e sem duplicados, pelo que o débito de operações obtido traduz aquilo que cada sistema de ficheiros consegue entregar quando as suas otimizações nada têm para explorar. Entre estas destaca-se a workload 06, que partilha com as duas seguintes (workloads 10 e 11) a distribuição Zipfian de acessos e constitui por isso a referência mais próxima.

#figure(
  tool-bars("impacto-baseline.csv", ylabel: [Milhares de @iops],
            xlabel: [Workload]),
  caption: [Débito de operações do Prismo com conteúdo aleatório em cada sistema de ficheiros]
) <impacto-baseline>

A @impacto-baseline evidencia desde logo uma diferença estrutural entre os dois sistemas, com o Btrfs a entregar entre três e quatro vezes o débito do @zfs em todas as workloads. Esta distância decorre do custo que o @zfs impõe a cada operação, resultante da combinação entre a semântica copy-on-write, a verificação de checksums e o recordsize de 4 KiB adotado, que multiplica o número de registos a gerir face ao valor por omissão.

Merece destaque o facto de a workload 04, composta exclusivamente por leituras, apresentar o débito mais baixo em ambos os sistemas, enquanto a workload 05, com metade das operações a serem escritas, mais do que duplica esse valor no Btrfs. Uma explicação plausível reside no encaminhamento das escritas através da cache, conforme exposto na metodologia, retornando estas sem aguardar o dispositivo.

Por outro lado, a workload 06 fica ligeiramente abaixo da workload 05 nos dois sistemas, apesar de a distribuição Zipfian concentrar os acessos numa fração reduzida do dispositivo, resultado que contraria a expectativa de a localidade favorecer o desempenho e que será retomado na secção dedicada aos efeitos de cache.

Convém realçar que a dispersão registada nestas workloads é bastante superior à observada sobre o dispositivo em acesso direto, situando-se entre 10% e 18% do valor médio, o que decorre de os sistemas de ficheiros introduzirem trabalho assíncrono que não acompanha o ritmo dos pedidos, oscilando por isso o débito instantâneo conforme essas tarefas são despachadas.

==== Compressão

A workload 10 escreve conteúdo com compressibilidade controlada segundo três níveis de redução, cenário que o @fio e o Vdbench apenas conseguem aproximar através de uma taxa global aplicada a todos os blocos, sendo o débito de operações obtido por cada ferramenta em cada sistema de ficheiros apresentado na @impacto-compressao.

#figure(
  tool-bars("impacto-compressao.csv", ylabel: [Milhares de @iops],
            xlabel: [Sistema de ficheiros]),
  caption: [Débito de operações na workload 10 em cada sistema de ficheiros]
) <impacto-compressao>

Confrontando a @impacto-compressao com a linha de base, verifica-se que o Prismo alcança mais 26% de débito no Btrfs e mais 32% no @zfs do que na workload 06, que partilha com esta a distribuição Zipfian de acessos. O mecanismo que sustenta este ganho parece claro, o conteúdo compressível permite ao sistema de ficheiros armazenar fisicamente menos dados do que aqueles que lhe são entregues, reduzindo na mesma medida o trabalho pedido ao dispositivo e libertando-o para aceitar mais operações no mesmo intervalo.

Convém realçar, no entanto, que esta comparação não isola o efeito do conteúdo, visto a workload 10 apresentar também uma proporção superior de escritas, as quais, conforme observado na própria linha de base, tendem a elevar o débito por retornarem sem aguardar o dispositivo. Assim sendo, ambas as diferenças atuam no mesmo sentido e o ganho não é integralmente imputável à compressibilidade.

É o confronto entre ferramentas que procura eliminar esta ambiguidade, dado que o Prismo e o @fio executam exatamente a mesma workload, com idêntico padrão de acessos e idêntica proporção de escritas, diferindo unicamente no conteúdo submetido. No @zfs o Prismo supera o @fio em cerca de 31%, diferença que atinge o dobro da dispersão registada e é por isso a única desta secção que o critério de leitura adotado permite declarar.

No entanto, merece particular destaque o facto de as duas configurações terem sido deliberadamente igualadas na compressibilidade média, pois a distribuição do Prismo, com metade dos blocos incompressíveis, 30% a reduzir metade e 20% a reduzir três quartos, produz exatamente os mesmos 30% que o @fio aplica de forma uniforme a todos os blocos.

Deste modo, a diferença observada não é imputável a uma carga globalmente mais compressível, mas ao modo como essa compressibilidade se distribui pelos blocos. Por outras palavras, o sistema de armazenamento responde à forma da distribuição e não apenas ao seu valor médio, propriedade que uma taxa única é por construção incapaz de exprimir.

No Btrfs a relação aparenta inverter-se, ficando o Prismo cerca de 16% abaixo do @fio, diferença que não deve porém ser interpretada como uma inversão efetiva, dado que a dispersão das medições ronda os 13% e os intervalos das duas ferramentas se sobrepõem numa extensão considerável, obrigando assim o critério estabelecido na secção anterior a tratá-las como equivalentes.

Convém realçar que a elevada dispersão do Btrfs decorre da sua própria arquitetura, dado que a compressão opera sobre extents de dimensão superior ao bloco de 4 KiB submetido, agrupando num mesmo extent blocos de compressibilidade distinta. Deste modo, a redução alcançada depende de quais os blocos que ficam agrupados, variando ao longo da execução de uma forma que o @zfs, ao comprimir cada record isoladamente, não apresenta.

Determinar se existe de facto uma diferença no Btrfs exigiria a medição do espaço ocupado em disco, única grandeza capaz de revelar quanto foi efetivamente reduzido em cada caso, e cuja ausência constitui a principal limitação desta subsecção @btrfs_docs.

==== Deduplicação

A workload 11 acrescenta à compressibilidade uma distribuição de duplicados repartida por três grupos, exercitando deste modo as duas otimizações em conjunto, sendo de recordar que a deduplicação opera de forma distinta nos dois sistemas de ficheiros, inline no @zfs e diferida no Btrfs, o que condiciona o momento em que os seus efeitos se tornam observáveis.

#figure(
  tool-bars("impacto-dedup.csv", ylabel: [Milhares de @iops],
            xlabel: [Sistema de ficheiros]),
  caption: [Débito de operações na workload 11 em cada sistema de ficheiros]
) <impacto-dedup>

A @impacto-dedup reproduz de forma quase exata o comportamento da workload anterior, mantendo-se as diferenças entre ambas abaixo de 2% nos dois sistemas, valor muito inferior à dispersão registada, pelo que a introdução de duplicados não produziu qualquer efeito mensurável no débito.

Este resultado admite duas leituras que os dados disponíveis não permitem separar, visto tanto poder significar que a deduplicação não chegou a ser acionada, como que foi acionada sem que daí resultasse ganho de desempenho.

No Btrfs a primeira hipótese é a mais provável, dado que o bees opera em segundo plano e a janela de medição termina antes de este percorrer os dados escritos, ao contrário da compressão, aplicada no caminho crítico.

Já no @zfs, onde a deduplicação atua no caminho crítico e a propriedade `direct` foi desativada precisamente para a manter operacional, a ausência de efeito admite duas explicações distintas. A primeira decorre da própria configuração das workloads, visto a workload 11 apresentar uma compressibilidade média de 22% contra os 30% da workload 10, pelo que o ganho trazido pelos duplicados pode estar a ser anulado por uma redução por compressão inferior em oito pontos percentuais.

A segunda aponta para o custo da tabela de deduplicação, uma vez que o @zfs consulta e atualiza esta estrutura a cada escrita, e o recordsize de 4 KiB adotado multiplica o número de entradas a manter. Nestas condições, o trabalho acrescido pode compensar as escritas evitadas, hipótese consistente com os quase sessenta gigabytes de memória ocupados neste sistema.

Convém realçar que a primeira explicação constitui igualmente uma limitação do desenho experimental, dado que as duas workloads não diferem apenas na presença de duplicados, o que impede o isolamento do contributo da deduplicação.

==== Custo Computacional das Otimizações

A redução do volume escrito não é gratuita, dado que a compressão e a deduplicação consomem processador e memória em troca das operações de @io poupadas, custo que a @impacto-recursos apresenta para a workload 11, onde ambas as otimizações se encontram ativas.

#figure(
  grid(
    columns: 2, gutter: 4pt,
    tool-bars("impacto-cpu.csv", ylabel: [Utilização de @cpu (%)],
              xlabel: [Sistema de ficheiros], width: 6.0cm, height: 4.4cm, legend: false),
    tool-bars("impacto-ram.csv", ylabel: [Memória utilizada (GiB)],
              xlabel: [Sistema de ficheiros], width: 6.0cm, height: 4.4cm),
  ),
  caption: [Consumo de recursos na workload 11 em cada sistema de ficheiros]
) <impacto-recursos>

O contraste apresentado na @impacto-recursos é acentuado, dado que o @zfs consome cerca de cinco vezes mais processador do que o Btrfs e ocupa perto de sessenta gigabytes de memória contra pouco mais de dez, custo que reflete a natureza inline das suas otimizações e o dimensionamento do ARC.

Particularmente esclarecedora é a comparação da memória entre ferramentas no @zfs, onde o Prismo ocupa cerca de oito gigabytes menos do que o @fio apesar de submeter exatamente o mesmo volume de dados lógicos. Uma vez que o ARC retém os blocos no estado em que são armazenados, ou seja já comprimidos, esta diferença só se explica se o conteúdo do Prismo estiver a ser efetivamente reduzido em maior grau.

No Btrfs a leitura mais informativa reside igualmente no processador, onde o @fio consome praticamente o dobro do Prismo para um débito que, conforme estabelecido, não é distinguível do ruído da medição. Esta assimetria é coerente com a forma das duas distribuições, visto metade dos blocos do Prismo serem incompressíveis e portanto descartados precocemente pelo algoritmo, enquanto os do @fio, uniformemente compressíveis a 30%, obrigam ao trabalho completo em cada bloco @btrfs_docs.

Deste modo, o custo de comprimir depende não apenas da quantidade de dados redutíveis mas do modo como essa redutibilidade se distribui, o que reforça a conclusão avançada na subsecção da compressão sobre a insuficiência de uma taxa única para caracterizar o conteúdo.

Em suma, um benchmark que ignore as propriedades do conteúdo subestima em cerca de um terço o débito que o @zfs entrega perante dados realistas, resultado que demonstra não ser a fidelidade do conteúdo um requisito acessório, mas antes condição para que a avaliação seja representativa. Estabelecido o efeito das propriedades dos dados, importa agora averiguar em que medida a interface de @io condiciona os valores medidos @koller2010 @meyer2012.

=== Comparação de Interfaces de I/O <io-interfaces>

O suporte a múltiplas interfaces de @io constitui uma das funcionalidades distintivas do Prismo, daí que faça todo o sentido avaliar o impacto da escolha da interface no desempenho observado. Embora o @fio suporte as mesmas interfaces, o acesso ao @spdk é conseguido através de um plugin cuja utilização é deveras complexa, não sendo por isso suportado nativamente. Por outro lado, o Vdbench opera exclusivamente sobre POSIX síncrono. Deste modo, a comparação entre as três ferramentas é possível para POSIX, enquanto para io_uring, libaio e @spdk a comparação é restrita ao Prismo e ao @fio.

Todas as execuções recorrem ao dispositivo acedido diretamente e partilham a parametrização descrita adiante, o que permite separar dois efeitos distintos. As diferenças entre interfaces medidas com a mesma ferramenta traduzem o custo do próprio mecanismo de submissão, ao passo que as diferenças entre ferramentas dentro da mesma interface só podem ser imputadas ao modo como cada uma a utiliza.

==== Configuração das Interfaces

A interface POSIX opera de forma síncrona através das chamadas `pread` e `pwrite`, mantendo por isso um único pedido em curso de cada vez, condição que a torna a referência natural contra a qual as restantes são confrontadas, afinal qualquer ganho observado traduz o benefício de sobrepor operações.

O libaio e o io_uring recebem em ambas as ferramentas uma profundidade de fila de 128 pedidos, sendo este o valor que fixa quantas operações podem aguardar conclusão ao mesmo tempo e portanto o parâmetro determinante deste confronto. No io_uring é adicionalmente ativado o polling do kernel, ficando a thread responsável fixada no primeiro core do processador @uring_kernel.

Já o @spdk dispensa por completo a intervenção do kernel, acedendo ao dispositivo a partir do espaço de utilizador através da abstração de @bdev. Além disso, o Prismo é configurado com uma máscara de quatro reactors e oito threads lógicas, enquanto o @fio recorre ao plugin `spdk_bdev` respeitando a profundidade de fila das restantes interfaces assíncronas @spdk_docs.

Convém realçar que apenas o libaio e o io_uring admitem uma comparação rigorosa entre ferramentas, dado que o modelo de reactors do Prismo não encontra correspondência direta nos parâmetros do plugin, pelo que os valores do @spdk devem ser lidos com a devida reserva.

==== Workloads Sequenciais

As workloads 01 e 02 percorrem o dispositivo de forma sequencial com blocos de 4 KiB, a primeira em escrita e a segunda em leitura, constituindo por isso o cenário mais favorável à submissão assíncrona, uma vez que o padrão de acessos é previsível e permite ao dispositivo antecipar os pedidos seguintes.

#figure(
  grid(
    columns: 2, gutter: 4pt,
    tool-bars("interfaces-wl01.csv", ylabel: [Milhares de @iops],
              xlabel: [Interface], width: 6.0cm, height: 4.4cm, legend: false),
    tool-bars("interfaces-wl02.csv", ylabel: [Milhares de @iops],
              xlabel: [Interface], width: 6.0cm, height: 4.4cm),
  ),
  caption: [Débito de operações nas workloads 01 e 02 em cada interface de @io]
) <interfaces-seq>

A @interfaces-seq revela um ganho considerável das interfaces assíncronas sobre o POSIX, que no Prismo ascende a cerca de cinco vezes na escrita e quatro na leitura, resultado esperado visto o POSIX síncrono bloquear a thread em cada operação e permitir por isso um único pedido em curso, enquanto as restantes mantêm dezenas em execução ao mesmo tempo.

Convém realçar que o Prismo iguala ou supera o @fio em todas as interfaces nestas duas workloads, com vantagem particularmente nítida no libaio, onde alcança perto de 20% acima. Assim sendo, a arquitetura produtor-consumidor mostra-se adequada a padrões previsíveis, nos quais o produtor consegue antecipar a preparação dos pedidos enquanto o consumidor aguarda as conclusões.

O @spdk apresenta, no entanto, um comportamento que contraria a expectativa, dado situar-se entre 10% e 14% abaixo do io_uring e do libaio no Prismo, quando seria de esperar que a eliminação do kernel do caminho crítico produzisse o débito mais elevado de todos.

Convém realçar que o mesmo padrão se observa no @fio, cujo @spdk fica igualmente abaixo das interfaces do kernel, afastando-se assim a hipótese de a causa residir na implementação de qualquer das ferramentas e apontando antes para o modo como esta interface lida com as características da workload.

Porém, no caso do Prismo, a explicação mais provável reside na configuração de reactors adotada, que reserva quatro núcleos e oito threads lógicas independentemente do perfil da workload, repartição que não é necessariamente a mais favorável a um padrão sequencial servido por um único produtor. Assim sendo, importa admitir que este resultado não fica cabalmente explicado pelos dados recolhidos.

==== Saturação da Largura de Banda

As workloads anteriores mantiveram o bloco em 4 KiB, dimensão que obriga a submeter um elevado número de pedidos para movimentar um volume modesto de dados e que coloca por isso o esforço do lado da submissão. A workload 03 altera exclusivamente este parâmetro, elevando-o para 64 KiB, reduzindo em 16 vezes o número de operações necessárias para transferir a mesma quantidade de dados.

Deste modo, o dispositivo deixa de ser solicitado pela cadência dos pedidos e passa a sê-lo pelo volume que deles resulta, permitindo assim averiguar se a vantagem das interfaces assíncronas se mantém quando o estrangulamento muda de natureza.

#figure(
  tool-bars("interfaces-wl03.csv", ylabel: [Milhares de @iops],
            xlabel: [Interface], width: 8.5cm, height: 4.6cm),
  caption: [Débito de operações na workload 03 em cada interface de @io]
) <interfaces-bloco>

Conforme se observa na @interfaces-bloco, as quatro interfaces produzem resultados indistinguíveis entre si e entre ferramentas, situando-se todas próximo das 28 mil operações por segundo. Uma vez que este valor corresponde a cerca de 1.8 GiB por segundo, conclui-se que o fator limitante deixou de ser a submissão de pedidos e passou a ser a largura de banda do próprio dispositivo.

Merece destaque o facto de esta convergência abranger igualmente o @spdk, cujo acesso em espaço de utilizador não lhe confere qualquer vantagem neste cenário, resultado que reforça a ideia de que a limitação se encontra no próprio dispositivo, e não no caminho de acesso utilizado.

Por fim, importa reter que a escolha da interface apenas é determinante enquanto o estrangulamento reside no número de operações submetidas, pois a partir do momento em que o volume de dados satura o dispositivo qualquer interface atinge o mesmo limite.

==== Workloads Aleatórias e Mistas

As workloads 04 e 05 substituem o acesso sequencial por acessos aleatórios distribuídos por toda a extensão do dispositivo, o que impede qualquer antecipação por parte deste e obriga cada pedido a suportar integralmente a latência do acesso. Nestas condições, o débito deixa de depender predominantemente da rapidez com que os dados são transferidos e passa a ser limitado pela capacidade de manter pedidos em curso, pelo que a profundidade da fila assume um papel determinante.

#figure(
  grid(
    columns: 2, gutter: 4pt,
    tool-bars("interfaces-wl04.csv", ylabel: [Milhares de @iops],
              xlabel: [Interface], width: 6.0cm, height: 4.4cm, legend: false),
    tool-bars("interfaces-wl05.csv", ylabel: [Milhares de @iops],
              xlabel: [Interface], width: 6.0cm, height: 4.4cm),
  ),
  caption: [Débito de operações nas workloads 04 e 05 em cada interface de @io]
) <interfaces-rand>

O ganho das interfaces assíncronas é aqui muito superior ao observado nas workloads sequenciais, atingindo no Prismo cerca de catorze vezes o débito do POSIX na leitura aleatória, amplificação que se explica pela latência de cada acesso, integralmente exposta à aplicação numa interface síncrona e sobreposta entre pedidos concorrentes nas restantes.

O confronto entre ferramentas revela, porém, uma diferença assinalável, com o @fio a alcançar aproximadamente o dobro do Prismo na workload 04 e cerca de 60% acima na workload 05, discrepância que constitui a maior registada em todo o capítulo e que não decorre da configuração, idêntica em ambas as ferramentas.

#figure(
  grid(
    columns: 2, gutter: 4pt,
    tool-bars("interfaces-lat-wl04.csv", ylabel: [Latência média (µs)],
              xlabel: [Interface], width: 6.0cm, height: 4.4cm, legend: false),
    tool-bars("interfaces-lat-wl05.csv", ylabel: [Latência média (µs)],
              xlabel: [Interface], width: 6.0cm, height: 4.4cm),
  ),
  caption: [Latência média nas workloads 04 e 05 em cada interface de @io]
) <interfaces-lat>

A @interfaces-lat esclarece a origem desta diferença, dado que a latência reportada pelo Prismo nas interfaces assíncronas duplica a do @fio, ao passo que em POSIX ambas coincidem ao décimo de microssegundo. Uma vez que o débito resulta do quociente entre os pedidos em curso e a latência de cada um, e sendo a profundidade configurada idêntica, o dobro da latência traduz-se necessariamente em metade do débito.

Convém realçar que a coincidência em POSIX é significativa, visto as duas ferramentas produzirem resultados equivalentes enquanto não existem pedidos pendentes, divergindo apenas quando estes surgem. Esta diferença é, portanto, atribuível à gestão dos pedidos pendentes, e não à instrumentação.

Merece particular destaque o facto de o Prismo obter praticamente o mesmo valor no io_uring e no libaio, duas interfaces que partilham apenas o carácter assíncrono e diferem por completo na implementação, o que localiza o estrangulamento num ponto anterior à interface e comum a ambas.

Três candidatos podem desde logo ser excluídos, dado que a @componentes demonstra sustentarem os geradores um débito duas ordens de grandeza superior ao exigido por esta workload, sendo além disso a profundidade configurada respeitada em ambas as ferramentas.

Também o canal entre produtor e consumidor fica afastado, visto medições realizadas sobre este isoladamente atingirem 10.2 de milhões operações por segundo na variante não bloqueante e 9.99 milhões na bloqueante, valores que excedem em mais de cinquenta vezes o débito aqui observado e que tornam a escolha entre as duas variantes irrelevante para o resultado.

Resta assim o ciclo do consumidor, que alterna entre submeter pedidos e recolher conclusões numa única thread, e onde o tempo despendido a recolher atrasa a reposição da fila. Convém realçar, no entanto, que esta explicação permanece por confirmar, pois a latência reportada não permite distinguir o tempo passado no dispositivo daquele que decorre dentro do próprio Prismo.

==== Concorrência

A workload 09 replica o padrão aleatório misto da workload 05, distribuindo-o por três jobs independentes, cada um com a sua fila e respetiva thread submissora, o que permite averiguar se cada interface acompanha o aumento do número de produtores ou se algum recurso partilhado impede essa escalabilidade.

#figure(
  tool-bars("interfaces-wl09.csv", ylabel: [Milhares de @iops],
            xlabel: [Interface], width: 8.5cm, height: 4.6cm),
  caption: [Débito de operações na workload 09 em cada interface de @io]
) <interfaces-conc>

A @interfaces-conc mostra que o libaio do Prismo escala com o número de jobs, passando de cerca de 215 mil operações por segundo com um único job para 326 mil com três, valor que iguala o do @fio e constitui o único cenário assíncrono em que as duas ferramentas convergem.

O io_uring, no entanto, não acompanha esta evolução no Prismo, mantendo-se próximo do valor obtido com um único job apesar de dispor do triplo das threads submissoras.

#figure(
  grid(
    columns: 2, gutter: 4pt,
    tool-bars("interfaces-cpu-1job.csv", ylabel: [Utilização de @cpu (%)],
              xlabel: [Interface], width: 6.0cm, height: 4.4cm, legend: false),
    tool-bars("interfaces-cpu-3jobs.csv", ylabel: [Utilização de @cpu (%)],
              xlabel: [Interface], width: 6.0cm, height: 4.4cm),
  ),
  caption: [Utilização de @cpu com um job e com três jobs em cada interface de @io]
) <interfaces-recursos>

A @interfaces-recursos, que confronta a workload 05 com a workload 09, oferece a explicação mais provável, dado que o io_uring do Prismo eleva o consumo de processador em mais de metade ao passar de um para três jobs sem qualquer ganho de débito, enquanto o libaio o aumenta apenas 12% e entrega em contrapartida mais 52%.

Esta penalização da performance decorre das threads de polling do kernel, que giram em espera ativa e que a configuração adotada fixa todas no mesmo core do processador, conforme descrito anteriormente, competindo portanto três instâncias por um único núcleo sem que o tempo assim despendido se traduza em pedidos submetidos.

O @spdk exibe neste cenário a degradação mais acentuada de todo o capítulo, caindo de 238 mil operações por segundo com um único job para 87 mil com três, ou seja pouco mais de um terço, enquanto o @fio mantém nesta interface o mesmo débito das restantes.

A origem desta degradação é, no entanto, a mesma que penaliza o io_uring, pois a máscara de reactors encontra-se definida ao nível da interface e não do job, sendo por isso replicada tal e qual pelas três instâncias.

Deste modo, os quatro núcleos que a máscara reserva passam a ser disputados pelos três jobs em simultâneo, sem que qualquer deles disponha de recursos exclusivos, o que explica a subida do consumo de processador sem retorno em débito.

Assim sendo, as duas interfaces que fixam afinidade ao processador exibem o mesmo comportamento, enquanto o libaio, por não fixar nenhuma, escala sem dificuldade. Convém realçar que a limitação não reside nas interfaces mas no modelo de configuração adotado, que exprime a afinidade uma única vez e não a reparte pelos jobs existentes.

Em suma, a interface de @io condiciona fortemente o débito medido, com ganhos que vão de nulos, quando o dispositivo satura, até catorze vezes nos acessos aleatórios, disparidade que fundamenta a necessidade de um benchmark capaz de as exercitar a todas. Convém realçar, contudo, que nenhuma interface se revela superior em todos os cenários, pois o @spdk vence nos acessos aleatórios com um único job mas fica atrás nas workloads sequenciais e degrada-se perante concorrência, sendo por isso desaconselhadas recomendações absolutas. Estabelecido este efeito, importa agora verificar se as workloads derivadas de traces reproduzem fielmente as propriedades dos dados originais.

=== Workloads Baseadas em Traces <trace-workloads>

De todas as funcionalidades do Prismo, a replicação de traces é aquela que mais o distingue das ferramentas de referência, afinal nenhuma delas consegue reproduzir em conjunto os padrões de acesso, o mix de operações e as propriedades do conteúdo registados numa execução real.

Infelizmente, replicar o trace não chega, uma vez que os registos disponíveis cobrem uma fração ínfima do tempo necessário para um dispositivo moderno atingir o regime estacionário. Nesta secção procura-se então perceber se a reprodução é fiel e se as estratégias de extensão preservam as características originais.

==== Traces Utilizados

Os traces utilizados provêm do repositório do @fiu e resultam da instrumentação de três servidores em produção, apresentando a estrutura já descrita no @chapter2. Não esquecer que cada registo conta com uma assinatura do conteúdo, algo indispensável ao presente trabalho, pois é através dela que se reconstitui a distribuição de duplicados sem sequer aceder aos dados originais @koller2010.

// Antes de serem consumidos, os ficheiros de texto originais são convertidos para o formato binário do Prismo através do Astroide, ferramenta que converte cada linha num registo de tamanho fixo e substitui a assinatura por um valor de 64 bits obtido através de uma função de hash.

#figure(
  doc_table(
    columns: (1fr, 0.7fr, 0.6fr, 0.6fr, 0.7fr, 0.7fr),
    align: (x, y) => if x == 0 or y == 0 { horizon + left } else { horizon + right },
    header: ([Trace], [Registos], [Volume], [Escritas], [Duplicados], [Workloads]),
    [homes], [2.0 M], [7.7 GiB], [92%], [36.5%], [12 e 14],
    [webmail], [0.7 M], [2.6 GiB], [96%], [37.7%], [15],
    [cheetah], [22.7 M], [86.6 GiB], [52%], [22.3%], [13],
  ),
  caption: [Traces utilizados, com as proporções medidas nos primeiros 100 mil pedidos]
) <traces-perfil>

O primeiro aspeto a reter da @traces-perfil é a dimensão destes ficheiros, pois mesmo o cheetah, de longe o mais extenso, cobre apenas 11.5% dos 752.91 GiB que uma workload da campanha movimenta, ficando os restantes dois abaixo de 1.1%. Sem extensão, portanto, o dispositivo mal chegaria a ser aquecido.

Merece igual atenção a distância entre servidores, com a proporção de escritas a ir dos 96% do webmail aos 52% do cheetah, único dedicado a servir páginas web e por isso o único onde as leituras pesam quase tanto quanto as escritas.

Convém realçar que qualquer destas percentagens é configurável no @fio e no Vdbench, dado tratar-se de rácios globais. O problema não está portanto no valor em si, mas no facto de cada percentagem da tabela resumir a execução inteira a um único número, escondendo tudo o que se passa entre o primeiro e o último pedido @fio_docs @vdbench.

==== Fidelidade do Replay

Uma workload baseada em traces atravessa dois momentos bem distintos, ou seja, enquanto o ficheiro tiver registos por consumir cada pedido corresponde ao que foi observado no servidor original, porém assim que este termina é a extensão que assume o comando. Posto isto, a fidelidade exigida difere, no primeiro trata-se de reprodução literal, enquanto no segundo de semelhança estatística.

As figuras seguintes acompanham a execução do trace homes com as três estratégias, assinalando uma linha vertical a transição ao fim dos primeiros 100 mil registos, aos quais a experiência foi limitada apesar de o ficheiro disponibilizar mais de dois milhões. Sendo o material anterior idêntico nas três, as séries sobrepõem-se necessariamente, e qualquer divergência que aí se observasse denunciaria distorção da ferramenta.

O eixo horizontal conta os pedidos submetidos, em milhares, sendo recolhida uma medição a cada mil. No caso dos acessos, o valor apresentado é o offset desse pedido em concreto, ao passo que as duas restantes grandezas são percentagens e portanto não existem ao nível de um pedido isolado, dado que este ou é escrita ou não é, ou repete um bloco anterior ou não.
// TODO: "ou é ou não é", não faz sentido utilizar este tipo de espressoes num texto academico, adapta

Daí que essas duas sejam medidas sobre os dois mil pedidos centrados em cada ponto, janela suficientemente estreita para revelar as oscilações e suficientemente larga para que a percentagem não salte entre extremos.

===== Padrões de Acesso

// TODO: acrescenta aqui um texto breve como introdução a esta subsubsecção

#figure(
  trace-lines("traces-offsets.csv", ylabel: [Offset acedido (GiB)]),
  caption: [Evolução dos acessos ao longo da execução do trace homes]
) <traces-offsets>

A @traces-offsets revela um percurso deveras irregular, com quase metade dos pontos a recair na vizinhança dos 328 GiB enquanto os restantes se espalham entre o primeiro gigabyte e os 431 GiB, padrão que as distribuições convencionais dificilmente produzem.

À primeira vista, as extensões por repetição e por amostragem devolvem nuvens que se confundem com a original, no entanto a semelhança engana. A extensão por repetição submete os offsets pela ordem observada, e por isso nada acrescenta ao segundo ciclo, ao passo que a extensão por amostragem os extrai do reservatório e os sorteia de novo, conservando o conjunto de endereços mas perdendo a ordem por que eram visitados.

Esta perda é relevante, pois blocos que o servidor acedia em instantes próximos passam a surgir dispersos por toda a execução, e é justamente essa proximidade que as caches e os índices de deduplicação exploram.

Já a extensão por regressão rompe com ambas, desenhando uma única reta que parte dos 357 GiB e recua 775.8 KiB a cada pedido, visto o offset ser extrapolado a partir do último valor do trace e do passo médio entre registos. Sendo esse passo negativo, a extensão percorre o dispositivo ao contrário, trocando o padrão irregular do servidor por um varrimento sequencial perfeito.

===== Mix de Operações

// TODO: acrescenta aqui um texto breve como introdução a esta subsubsecção

#figure(
  trace-lines("traces-operacoes.csv", ylabel: [Escritas na janela (%)]),
  caption: [Evolução do mix de operações ao longo da execução do trace homes]
) <traces-operacoes>

O mix de operações é a grandeza que melhor sobrevive à transição, oscilando na @traces-operacoes entre os 76% e os 100% de escritas durante o replay, oscilação que a extensão por repetição devolve intacta enquanto a extensão por amostragem a achata numa linha constante nos 91.5%. De facto, tal valor coincide com a média do trace, do que se infere serem as frequências marginais a única propriedade garantida pela alias table.

Quanto à extensão por regressão, o valor previsto para a operação percorre somente o intervalo entre 1.20 e 0.95 ao longo dos 100 mil registos sintéticos, daí que arredonde invariavelmente para escrita e a série se fixe nos 100%, deixando a workload de exercitar o caminho de leitura, precisamente aquele onde a deduplicação se traduz em ganho.
//TODO: falta uma esplicação do porque dos valores de escrita estarem no intervalo 1.2 e 0.95

// TODO: de que forma o intervalo 1.2 e 0.95 está relacionado com escrita, assim nao se percebe como o arrendondamento resulta numa escrita

// TODO:A afirmação "onde a deduplicação se traduz em ganho carece de explicação"

===== Duplicados de Conteúdo

// TODO: acrescenta aqui um texto breve como introdução a esta subsubsecção

#figure(
  trace-lines("traces-assinaturas.csv", ylabel: [Duplicados na janela (%)]),
  caption: [Evolução das assinaturas de conteúdo ao longo da execução do trace homes]
) <traces-assinaturas>

A @traces-assinaturas fecha o quadro com a oscilação mais expressiva de todas, variando a percentagem de blocos repetidos entre os 21% e os 46% ao longo do replay, do que se depreende concentrarem-se os duplicados em rajadas, algo que uma taxa global de 36.5% jamais conseguiria exprimir.
// TODO: a oscilação da evolução de acessos parece obsiclar bastante mais, qual a razao para teres dito que esta é a que oscila mais

// TODO: explica a questão de rajadas, isso é batch?

A extensão por repetição acompanha esta evolução sem a subida artificial que seria de recear, pois o ciclo abrange 100 mil registos enquanto a janela de medição fica-se pelos dois mil.
// TODO: a janela de medição não tem nada a ver com a estensao por repeticao, pois a estensao apenas repete os registo do trace original

Por outro lado, a extensão por amostragem apresenta o resultado mais curioso desta subsecção, descendo para os 9.9% na janela enquanto a taxa calculada sobre a totalidade da extensão sobe para 54.7%, bem acima dos 36.5% do próprio trace.
// TODO: não consegui perceber o que foi aqui dito

Na verdade, a contradição é somente aparente, visto o sorteio independente dispersar pela execução inteira os duplicados que o servidor produzia em rajadas, elevando a repetição global à custa da local, justamente aquela que o sistema de armazenamento consegue explorar.

Por fim, a extensão por regressão anula os duplicados por completo, visto o identificador de bloco decrescer cerca de $2.3 times 10^13$ a cada registo e jamais reincidir num valor já submetido.
// TODO: explica por que motivo isso acontece e quais as consequencia na avaliação do sistema de armazenamento que beneficia de duplicados

===== Comparação entre Estratégias

Confrontando com o previsto no @chapter3, as extensões por repetição e por amostragem comportam-se conforme antecipado, conservando aquela todas as correlações à custa da periodicidade e retendo esta apenas as distribuições marginais.
// TODO: texto pouco fluido e academico, "esta" e "aquela" não faz sentido utilizar

Já a extensão por regressão não confirma a expectativa de capturar as dependências entre dimensões, isto porque o identificador de bloco resulta de uma função de hash sem relação linear alguma com o offset, acabando a estratégia mais sofisticada por ser a menos variável das três.

==== Desempenho das Workloads Baseadas em Traces

Conhecido o grau de fidelidade alcançado, importa agora perceber que comportamento estas cargas produzem no sistema de armazenamento, confrontando as réplicas integrais com as workloads híbridas de modo a apurar se a troca de dimensões reais por sintéticas altera aquilo que é medido.

Estas últimas conservam somente uma dimensão real e geram sinteticamente as restantes, ou seja, a workload 12 extrai os acessos do trace homes enquanto combina operações sintéticas, e a 13 recolhe o mix de operações do cheetah sobre uma distribuição Zipfiana, algo que permite imputar a cada uma qualquer diferença observada.

Convém mencionar que a @traces-iops apresenta unicamente os valores do Prismo, visto nem o @fio nem o Vdbench conseguirem replicar traces desta natureza, o que retira qualquer termo de comparação entre ferramentas @fio_docs @vdbench.

#figure(
  tool-bars("traces-iops.csv", ylabel: [Milhares de @iops],
            xlabel: [Workload], width: 8.5cm, height: 4.6cm, legend: false),
  caption: [Débito de operações do Prismo nas workloads baseadas em traces]
) <traces-iops>

Os valores da @traces-iops resultam de execuções sobre o @zfs através da interface POSIX, pelo que devem ser confrontados com os da secção dedicada às propriedades dos dados e nunca com os das interfaces de @io.

As quatro workloads distribuem-se entre as 3485 e as 4169 operações por segundo, sendo a maior diferença entre duas delas inferior ao desvio padrão da workload 12, algo que pelo critério do @validation obriga a tratá-las como equivalentes.

As híbridas ladeiam aliás as réplicas integrais, ficando a 12 abaixo de ambas e a 13 acima, o que afasta qualquer efeito sistemático decorrente da substituição de dimensões reais por sintéticas. A 12 é ainda a mais dispersa e a única dominada por leituras, aproximando-se por aí da workload 04, a mais lenta da linha de base do @zfs.

Já o confronto com as sintéticas mostra as quatro a assentarem sobre a workload 05, que dista menos de 1% da média dos traces, ficando estes 25% acima da workload 06 e 85% acima da 04, do que se conclui não produzir nenhuma delas um débito que as sintéticas já não produzissem.

As réplicas integrais submetem conteúdo genuinamente duplicado, 36.5% no homes e 54.7% ao longo da extensão do webmail, e as híbridas blocos totalmente aleatórios, sem que daí resulte vantagem alguma para as primeiras, o que reforça a leitura avançada na subsecção da deduplicação.

Deste modo, o que distingue uma workload baseada em traces não é o débito que o @zfs entrega, mas as propriedades do conteúdo e a distribuição dos acessos documentadas na subsecção anterior.

==== Comparação de Capacidades

Independentemente dos valores obtidos, importa situar o Prismo face às ferramentas de referência quanto àquilo que cada uma consegue reproduzir a partir de um trace.

#figure(
  doc_table(
    columns: (2.2fr, auto, auto, auto),
    header: ([Capacidade], [Prismo], [FIO], [Vdbench]),
    [Replicação dos acessos], cell_yes, cell_partial[Limitado], cell_no,
    [Replicação das operações], cell_yes, cell_no, cell_no,
    [Replicação do conteúdo], cell_yes, cell_no, cell_no,
    [Combinação com geração sintética], cell_yes, cell_no, cell_no,
    [Extensão para lá da duração original], cell_yes, cell_no, cell_no,
  ),
  caption: [Capacidades de replicação de traces suportadas por cada ferramenta]
) <traces-capacidades>

Perante a @traces-capacidades, torna-se claro ser o Prismo a única das três ferramentas capaz de reproduzir as três dimensões de um trace, visto o @fio admitir apenas a repetição de um padrão de acessos previamente descrito, enquanto o Vdbench não oferece mecanismo algum equivalente @fio_docs @vdbench.

De destacar a combinação com geração sintética, que permite isolar o contributo de cada dimensão ao substituir as restantes por valores controlados, afinal sem ela jamais seria possível saber se um comportamento observado decorre do padrão de acessos ou das propriedades do conteúdo.

Estes resultados fundamentam o Q4 apenas em parte, dado que a fase de replay reproduz literalmente os registos originais, ao passo que na extensão somente a repetição conserva as três dimensões intactas, e fá-lo à custa de uma periodicidade que servidor algum exibe.

Já a amostragem retém as frequências mas dissolve a concentração temporal dos duplicados, enquanto a regressão anula por completo o conteúdo repetido.

Em suma, o Prismo replica um trace nas suas três dimensões e prolonga-o para lá da duração original, algo que nenhuma das ferramentas de referência oferece, embora nenhuma estratégia de extensão consiga estender a workload conservando ao mesmo tempo todas as propriedades do material de partida. Encerrado o eixo dos traces, importa agora averiguar de que modo a localidade dos acessos condiciona o desempenho observado.


















#pagebreak()


=== Efeitos de Localidade e Cache <locality>

Em ambientes de produção, as workloads exibem frequentemente padrões de acesso com forte localidade espacial e temporal, o que ativa mecanismos internos de cache e prefetching nos dispositivos e sistemas de ficheiros. No entanto, estes comportamentos não são exercitados por workloads puramente aleatórias, como tal esta secção procura avaliar em que medida diferentes distribuições de acesso influenciam o desempenho observado.

==== Zipfian vs Random vs Sequencial

// TODO: WL 06 (zipf 0.8) vs WL 05 (random) vs WL 01 (sequencial)
// TODO: séries temporais de latência: estabilização mais rápida com zipf vs instabilidade com random
// TODO: séries temporais de throughput ao longo da execução
// TODO: dstat.csv: disk ops/sec ao longo do tempo
// Evidência: report.json + dstat.csv das respetivas workloads

// TODO: conclusão: workloads com localidade realista são necessárias para exercitar mecanismos de cache
// TODO: comparação de latência p99 entre padrões de acesso
// TODO: fundamentação de Q5

=== Síntese e Discussão Geral <evaluation-synthesis>

// TODO: parágrafo introdutório da síntese

==== Respostas às Perguntas de Avaliação

A Q1 obtém resposta afirmativa na @data-properties, dado que o mesmo padrão de acessos executado sobre o mesmo sistema produz um débito superior em cerca de um terço quando o conteúdo é compressível, diferença que nenhuma outra característica da workload explica.

Já a Q2 admite uma resposta menos linear, pois as duas propriedades avaliadas não se comportam de igual modo, produzindo a compressibilidade um efeito imediato e mensurável enquanto a introdução de duplicados não alterou o débito em qualquer dos sistemas. Este contraste é atribuído na mesma secção tanto ao carácter diferido da deduplicação no Btrfs como ao custo da tabela mantida pelo @zfs.

// TODO: Q3: resposta concisa com referência cruzada à secção de interfaces de I/O
// TODO: Q4: resposta concisa com referência cruzada à secção de workloads baseadas em traces
// TODO: Q5: resposta concisa com referência cruzada à secção de localidade e cache

Quanto à Q6, verifica-se que a escolha do sistema condiciona fortemente o benefício obtido, embora de forma menos evidente do que os valores absolutos sugerem. O Btrfs entrega um débito três a quatro vezes superior, no entanto é no @zfs que o conteúdo realista produz maior ganho relativo, e é também aí que o custo computacional se revela mais elevado.

Deste modo, a recomendação depende do perfil da carga, beneficiando das otimizações do @zfs as cargas dominadas por dados compressíveis, enquanto as pouco redutíveis são melhor servidas por um sistema que não pague esse custo.

// TODO: Q7: resposta transversal — confronto com FIO e Vdbench ao longo de todo o capítulo,
//       quantificando o erro de avaliação de quem ignora conteúdo e enumerando as conclusões
//       sobre os sistemas avaliados que só o Prismo permite alcançar

==== Validação

// TODO: V1: confirmação com referência à secção de validação do Prismo (geração de conteúdo)
// TODO: V2: confirmação com referência à secção de reprodutibilidade
// TODO: V3: confirmação com referência às métricas e relatórios produzidos
// TODO: V4: confirmação com referência à equivalência demonstrada em workloads genéricas (a
//       diferença não vem da instrumentação) cruzada com a divergência observada em ZFS/Btrfs,
//       retomando o erro de avaliação de cerca de um terço quantificado na @data-properties

==== Limitações

// TODO: workloads não testadas, sistemas não avaliados
// TODO: condições experimentais (single machine, single device)
// TODO: cada configuração corresponde a uma única execução, pelo que a variabilidade entre
//       execuções independentes não foi caracterizada; a dispersão reportada traduz a
//       estabilidade da medição ao longo da execução
// TODO: limitações dos traces disponíveis
// TODO: a linha de base da secção sobre propriedades dos dados recorre à workload 06, que
//       partilha a distribuição de acessos com as workloads 10 e 11 mas não o mix de operações;
//       um controlo estrito exigiria uma variante da workload 10 com redução nula, que
//       diferisse das restantes apenas no conteúdo
// TODO: as workloads 10 e 11 diferem na presença de duplicados mas também na compressibilidade
//       média, 30% contra 22%, o que impede o isolamento do contributo da deduplicação; uma
//       variante da workload 11 com a mesma compressibilidade da 10 resolveria a ambiguidade
// TODO: o caminho de metadados dos sistemas de ficheiros não é exercitado, dado que o Prismo, tal
//       como o FIO e o Vdbench, opera sobre um ficheiro pré-alocado; uma avaliação completa exigiria
//       workloads intensivas em metadados, ficando essa análise fora do âmbito desta dissertação

==== Sumário

// TODO: contribuição principal: o Prismo iguala as ferramentas existentes em workloads genéricas,
//       diferencia-se na geração de conteúdo realista, e é exclusivo no suporte a traces com
//       propriedades de dados, progressão dos 3 patamares (equivalência → diferenciação → exclusividade)
