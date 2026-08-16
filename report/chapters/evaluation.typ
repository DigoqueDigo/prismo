#import "../utils/charts.typ" : tool-bars, tool-lines, cpu-stack, component-bars
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

Por outro lado, a workload 06 fica ligeiramente abaixo da workload 05 nos dois sistemas, apesar de a distribuição Zipfian concentrar os acessos numa fração reduzida do dispositivo. Este resultado contraria a expectativa de que a localidade favoreça o desempenho, matéria que será retomada na secção dedicada aos efeitos de cache.

Convém realçar que a dispersão registada nestas workloads é bastante superior à observada sobre o dispositivo em acesso direto, situando-se entre 10% e 18% do valor médio, o que decorre de os sistemas de ficheiros introduzirem trabalho assíncrono que não acompanha o ritmo dos pedidos, oscilando por isso o débito instantâneo conforme essas tarefas são despachadas.

==== Compressão

A workload 10 escreve conteúdo com compressibilidade controlada segundo três níveis de redução, cenário que o @fio e o Vdbench apenas conseguem aproximar através de uma taxa global aplicada a todos os blocos, sendo o débito de operações obtido por cada ferramenta em cada sistema de ficheiros apresentado na @impacto-compressao.

#figure(
  tool-bars("impacto-compressao.csv", ylabel: [Milhares de @iops],
            xlabel: [Sistema de ficheiros]),
  caption: [Débito de operações na workload 10 em cada sistema de ficheiros]
) <impacto-compressao>

Confrontando a @impacto-compressao com a linha de base, verifica-se que o Prismo alcança mais 26% de débito no Btrfs e mais 32% no @zfs do que na workload 06, embora esta partilhe a mesma distribuição de acessos e a workload 10 contenha uma proporção superior de escritas, pelo que o ganho é atribuível ao zstd, única dimensão favorável que as distingue.

O mecanismo subjacente é direto, dado que conteúdo compressível permite ao sistema de ficheiros armazenar fisicamente menos dados do que aqueles que lhe são entregues, reduzindo na mesma medida o trabalho pedido ao dispositivo e libertando-o para aceitar mais operações no mesmo intervalo.

Mais revelador é o confronto entre ferramentas no @zfs, onde o Prismo supera o @fio em cerca de 31%, diferença que atinge o dobro da dispersão registada e é por isso a única desta secção que o critério de leitura adotado permite declarar. Convém realçar que as duas configurações foram deliberadamente igualadas na compressibilidade média, pois a distribuição do Prismo, com metade dos blocos incompressíveis, 30% a reduzir metade e 20% a reduzir três quartos, produz exatamente os mesmos 30% que o @fio aplica de forma uniforme a todos os blocos.

Assim sendo, a diferença observada não é imputável a uma carga globalmente mais compressível, mas ao modo como essa compressibilidade se distribui pelos blocos. Por outras palavras, o sistema de armazenamento responde à forma da distribuição e não apenas ao seu valor médio, propriedade que uma taxa única é por construção incapaz de exprimir.

No Btrfs a relação aparenta inverter-se, ficando o Prismo cerca de 16% abaixo do @fio. Esta diferença não deve porém ser interpretada como uma inversão efetiva, dado que a dispersão das medições ronda os 13% e os intervalos das duas ferramentas se sobrepõem numa extensão considerável, pelo que o critério estabelecido na secção anterior obriga a tratá-las como equivalentes.

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

Importa notar que também o @fio gera duplicados nesta workload, embora através de uma percentagem global, pelo que a comparação incide novamente sobre a forma da distribuição e não sobre a sua presença.

No Btrfs a primeira hipótese é a mais provável, dado que o bees opera em segundo plano e a janela de medição termina antes de este percorrer os dados escritos, ao contrário da compressão, aplicada no caminho crítico.

Já no @zfs, onde a deduplicação atua no caminho crítico e a propriedade `direct` foi desativada precisamente para a manter operacional, a ausência de efeito admite duas explicações distintas. A primeira decorre da própria configuração das workloads, visto a workload 11 apresentar uma compressibilidade média de 22% contra os 30% da workload 10, pelo que o ganho trazido pelos duplicados pode estar a ser anulado por uma redução por compressão inferior em oito pontos percentuais.

A segunda aponta para o custo da tabela de deduplicação, uma vez que o @zfs consulta e atualiza esta estrutura a cada escrita, e o recordsize de 4 KiB adotado multiplica o número de entradas a manter. Nestas condições, o trabalho acrescido pode compensar as escritas evitadas, hipótese consistente com os quase sessenta gigabytes de memória ocupados neste sistema.

Convém realçar que a primeira explicação constitui igualmente uma limitação do desenho experimental, dado que as duas workloads não diferem apenas na presença de duplicados, o que impede o isolamento do contributo da deduplicação.

==== Custo Computacional das Otimizações

A redução do volume escrito não é gratuita, uma vez que a compressão e a deduplicação consomem processador e memória em troca das operações de @io poupadas. Este custo é apresentado na @impacto-recursos para a workload 11, onde ambas as otimizações se encontram ativas.

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

Trata-se de uma confirmação independente do ganho de débito discutido na subsecção da compressão, visto provir de uma grandeza distinta e apontar no mesmo sentido, atestando que a vantagem observada resulta de redução real de dados e não de qualquer particularidade da instrumentação.

No Btrfs a leitura mais informativa reside igualmente no processador, onde o @fio consome praticamente o dobro do Prismo para um débito que, conforme estabelecido, não é distinguível do ruído da medição. Esta assimetria é coerente com a forma das duas distribuições, visto metade dos blocos do Prismo serem incompressíveis e portanto descartados precocemente pelo algoritmo, enquanto os do @fio, uniformemente compressíveis a 30%, obrigam ao trabalho completo em cada bloco @btrfs_docs.

Deste modo, o custo de comprimir depende não apenas da quantidade de dados redutíveis mas do modo como essa redutibilidade se distribui, o que reforça a conclusão avançada na subsecção da compressão sobre a insuficiência de uma taxa única para caracterizar o conteúdo.

Em suma, um benchmark que ignore as propriedades do conteúdo subestima em cerca de um terço o débito que o @zfs entrega perante dados realistas, resultado que demonstra não ser a fidelidade do conteúdo um requisito acessório, mas antes condição para que a avaliação seja representativa. Estabelecido o efeito das propriedades dos dados, importa agora averiguar em que medida a interface de @io condiciona os valores medidos @koller2010 @meyer2012.

#pagebreak()














=== Comparação de Interfaces de I/O <io-interfaces>

O suporte a múltiplas interfaces de @io constitui uma das funcionalidades distintivas do Prismo, daí que faça todo o sentido avaliar o impacto da escolha da interface no desempenho observado. Embora o @fio suporte as mesmas interfaces, o acesso ao @spdk é conseguido através de um plugin cuja utilização é deveras complexa, não sendo por isso suportado nativamente. Por outro lado, o Vdbench opera exclusivamente sobre POSIX síncrono. Deste modo, a comparação entre as três ferramentas é possível para POSIX, enquanto para io_uring, libaio e @spdk a comparação é restrita ao Prismo e ao @fio.

==== Workloads Sequenciais

// TODO: POSIX vs io_uring vs libaio vs SPDK em WL 01, 02, 03 no raw NVMe
// TODO: para POSIX: comparação Prismo vs FIO vs Vdbench
// TODO: para io_uring, libaio, SPDK: comparação Prismo vs FIO
// TODO: gráfico de barras: throughput (MB/s) por engine × workload × ferramenta
// Evidência: report.json de prismo_posix_1_9, prismo_uring_1_9, prismo_aio_1_9,
//            fio_posix_1_9, fio_uring_1_9, fio_aio_1_9, fio_spdk_1_9, vdbench_posix_1_9

==== Workloads Aleatórias e Mistas

// TODO: WL 04 (rand_read), WL 05 (rw_rand_mixed)
// TODO: para POSIX: Prismo vs FIO vs Vdbench; restantes: Prismo vs FIO
// TODO: vantagem das interfaces assíncronas em random I/O
// TODO: gráfico: IOPS por engine × ferramenta
// Evidência: report.json das mesmas campanhas

==== Concorrência

// TODO: WL 09 (rw_rand_multijob): escalabilidade com múltiplos jobs por engine
// TODO: para POSIX: Prismo vs FIO vs Vdbench; restantes: Prismo vs FIO
// TODO: throughput total e per-job
// TODO: dstat.csv: CPU por core
// Evidência: report.json + dstat.csv

// TODO: tabela-síntese: ranking de interfaces por tipo de workload × ferramenta
// TODO: verificar se o ranking de interfaces se inverte consoante o sistema de armazenamento
// TODO: trade-off entre complexidade de configuração e ganho de desempenho
// TODO: diferenças entre Prismo e FIO nas interfaces assíncronas
// TODO: recomendações práticas para utilizadores do benchmark
// TODO: fundamentação de Q3

=== Workloads Baseadas em Traces <trace-workloads>

A capacidade de replicar workloads baseadas em traces de produção constitui uma funcionalidade exclusiva do Prismo no panorama dos benchmarks avaliados. Embora o @fio ofereça suporte limitado ao replay de padrões de acesso, não modela o conteúdo dos dados associado ao trace, enquanto o Vdbench não dispõe de qualquer suporte para traces. Tendo isto em mente, esta secção avalia a fidelidade do replay e a eficácia das estratégias de extensão de traces.

==== Replay de Traces

// TODO: WL 14-15 (traces FIU: homes, webmail)
// TODO: Deltoide aplicado ao trace original vs dados escritos pelo Prismo → distribuições devem coincidir
// TODO: métricas do replay: padrão de acesso, mix de operações
// Evidência: output Deltoide + report.json de workloads 14-15

==== Workloads Híbridas e Estratégias de Extensão

// TODO: WL 12-13: extensão por repetição vs sampling vs regressão
// TODO: Deltoide compara distribuições geradas por cada estratégia de extensão
// TODO: comparação de throughput e latência entre estratégias
// TODO: gráfico: distribuição de dedup/compressão por estratégia de extensão
// Evidência: output Deltoide + report.json de workloads 12-13

// TODO: o Prismo é o único benchmark a combinar replay de traces com geração de conteúdo realista
// TODO: tabela comparativa: Prismo vs FIO vs Vdbench em cada capacidade de trace (✓/✗/parcial)
// TODO: utilidade prática das workloads híbridas para avaliação de sistemas modernos
// TODO: fundamentação de Q4

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
