#import "../utils/functions.typ" : question_block, validation_point_block

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
/ V2: Os resultados obtidos são estáveis e comparáveis entre execuções, garantindo que as variações observadas decorrem das propriedades das workloads e não de efeitos alheios.
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

Todas as experiências foram conduzidas numa única máquina, evitando assim que diferenças de hardware ou de configuração entre execuções se reflitam nas métricas recolhidas. As especificações do sistema encontram-se descritas na @hardware, sendo de realçar a capacidade de memória disponível, pois a condição de terminação de grande parte das workloads é calculada com base nesta.

#figure(
  table(
    columns: (1fr, 1.6fr),
    inset: 6pt,
    align: horizon + left,
    fill: (x, y) => if y == 0 { gray.lighten(60%) },
    table.header([*Componente*], [*Especificação*]),
    [Sistema operativo], [Ubuntu 20.04.6 LTS (Focal Fossa)],
    [Kernel], [Linux 5.4.0-216-generic],
    [Arquitetura], [x86_64],
    [Processador], [2 $times$ Intel Xeon Gold 6342],
    [Núcleos], [48 físicos (24 por processador), 96 threads],
    [Frequência], [800 MHz base, 2.80 GHz máxima],
    [Cache L2], [72 MiB],
    [Memória], [188.23 GiB],
    [Dispositivo], [Dell Enterprise @nvme P5600 MU U.2, 1.46 TiB],
  ),
  caption: [Especificações da máquina utilizada nas experiências]
) <hardware>

O sistema opera sobre Ubuntu 20.04.6 LTS com kernel Linux 5.4.0-216-generic, cabendo referir que a versão do kernel condiciona as funcionalidades disponíveis nas interfaces assíncronas, em particular no io_uring, cuja implementação tem vindo a ser progressivamente otimizada desde a sua introdução @uring_kernel.

Convém realçar que todos os acessos são efetuados com a flag `O_DIRECT`, o que elimina a intervenção da page cache e garante que os pedidos atingem efetivamente o dispositivo, sendo esta uma condição indispensável para que as métricas reflitam o comportamento do sistema de armazenamento e não o da memória @didona2022 @ren2023. Esta garantia é integral sobre o dispositivo em bruto, no entanto os sistemas de ficheiros avaliados impõem-lhe restrições que serão detalhadas adiante.

==== Ferramentas Comparadas

A avaliação confronta o Prismo, na versão 1.0.0, com o @fio e o Vdbench, duas das ferramentas mais utilizadas na avaliação de sistemas de armazenamento, sendo consideradas as versões mais recentes de cada uma, respetivamente a 3.42 e a 5.04.07 @fio_docs @vdbench.

Estas ferramentas não partilham, no entanto, o mesmo âmbito de aplicação, pois enquanto o @fio suporta as mesmas interfaces de @io que o Prismo, ainda que o acesso ao @spdk seja conseguido através de um plugin externo, o Vdbench opera exclusivamente sobre POSIX síncrono, o que restringe a comparação entre as três ferramentas a esse cenário @fio_docs @vdbench.

#figure(
  table(
    columns: (1.8fr, auto, auto, auto),
    inset: 6pt,
    align: horizon + left,
    fill: (x, y) => if y == 0 { gray.lighten(60%) },
    table.header([*Funcionalidade*], [*Prismo*], [*FIO*], [*Vdbench*]),
    [POSIX], [Sim], [Sim], [Sim],
    [libaio], [Sim], [Sim], [Não],
    [io_uring], [Sim], [Sim], [Não],
    [SPDK], [Sim], [Plugin], [Não],
    [Distribuição de duplicados], [Sim], [Taxa global], [Rácio global],
    [Distribuição de compressibilidade], [Sim], [Taxa global], [Rácio global],
    [Replay de traces], [Sim], [Limitado], [Não],
    [Extensão sintética de traces], [Sim], [Não], [Não],
  ),
  caption: [Funcionalidades suportadas por cada ferramenta]
) <ferramentas>

Sempre que possível, as configurações foram replicadas entre ferramentas de modo a garantir equivalência semântica, no entanto o Vdbench não dispõe de barreiras de sincronização nem de geração de conteúdo constante, sendo a distribuição Zipfiana aproximada através do parâmetro `hotband`, aproximações que devem ser tidas em conta na leitura dos resultados @vdbench.

==== Campanha Experimental

A campanha experimental é constituída por quinze workloads base, cada uma isolando exatamente uma dimensão relativamente à anterior, o que permite atribuir as diferenças observadas a um único fator. Estas workloads encontram-se descritas na @workloads-base, sendo posteriormente replicadas para as quatro interfaces de @io avaliadas, do que resulta um total de sessenta configurações.

#[
#show figure: set block(breakable: true)
#figure(
  table(
    columns: (auto, 1.2fr, 1.6fr),
    inset: 6pt,
    align: horizon + left,
    fill: (x, y) => if y == 0 or x == 0 { gray.lighten(60%) },
    table.header([*\#*], [*Dimensão isolada*], [*Parâmetros principais*]),
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

A dimensão das workloads foi fixada em 752.91 GiB, valor que corresponde a quatro vezes a memória disponível, garantindo assim que o conjunto de dados manipulado não é passível de acomodação em cache e que os pedidos atingem efetivamente o dispositivo. Nas workloads mais demoradas, nomeadamente aquelas assentes em acessos aleatórios, alcançar este volume implicaria execuções incomportáveis, daí que nestes casos a condição de paragem seja de quinze minutos de execução, duração suficiente para que o sistema atinja um regime estacionário @traeger2008 @tarasov2011.

No que respeita às interfaces, o io_uring e o libaio operam com 128 entradas na fila de submissão, sendo no primeiro caso ativado o polling do kernel através das flags `IORING_SETUP_SQPOLL` e `IORING_SETUP_SQ_AFF`, enquanto o @spdk é configurado com uma máscara de quatro reactors e uma única thread lógica.

Antes de cada execução, o conteúdo em memória é sincronizado com o disco e as caches do sistema são invalidadas, seguindo-se um período de espera de cinco minutos que permite ao dispositivo estabilizar após a carga anterior. Só então tem início a recolha de métricas, sendo o benchmark lançado um segundo depois de forma a garantir que o período inicial fica devidamente registado.

// TODO: número de repetições por configuração e tratamento estatístico (Cardoide com --repetitions),
//       incluindo a forma como a média e o desvio padrão são calculados sobre as execuções

==== Sistemas de Armazenamento Avaliados

As workloads são executadas sobre três sistemas de armazenamento distintos, sendo o dispositivo @nvme utilizado em bruto como linha de base agnóstica ao conteúdo, enquanto o Btrfs e o @zfs são avaliados por implementarem otimizações sensíveis às propriedades dos dados, nomeadamente compressão e deduplicação.

#figure(
  table(
    columns: (1fr, auto, auto, 1.6fr),
    inset: 6pt,
    align: horizon + left,
    fill: (x, y) => if y == 0 { gray.lighten(60%) },
    table.header([*Sistema*], [*Compressão*], [*Deduplicação*], [*Papel na avaliação*]),
    [@nvme em bruto], [Não], [Não], [Linha de base agnóstica ao conteúdo],
    [Btrfs], [zstd, nível 3], [Offline, via bees], [Compressão no caminho crítico e deduplicação diferida],
    [@zfs], [zstd, nível 3], [Inline], [Compressão e deduplicação no caminho crítico],
  ),
  caption: [Sistemas de armazenamento avaliados]
) <sistemas>

Os sistemas foram utilizados em versões compatíveis com o kernel instalado, nomeadamente o @zfs 2.4.0, que suporta kernels desde a versão 4.18 até à 6.18, e o bees 0.11, cuja documentação recomenda expressamente a versão 5.4 @zfs_docs @bees. Já o Btrfs, sendo implementado no interior do kernel, corresponde à implementação disponibilizada pelo 5.4.0-216-generic, cabendo às ferramentas de espaço de utilizador a versão 5.2.1 distribuída pelo Ubuntu 20.04.

Ambos os sistemas de ficheiros recorrem ao zstd para comprimir os blocos escritos, sendo em qualquer deles utilizado o nível 3. Esta escolha resulta do compromisso que tal nível estabelece entre a qualidade da compressão e a rapidez com que esta é alcançada, pois níveis superiores comprimem mais, no entanto acarretam um custo computacional que se refletiria nas métricas recolhidas, enviesando a avaliação do sistema de armazenamento em detrimento da avaliação do próprio algoritmo @btrfs_docs.

Já a deduplicação encontra-se ativa em ambos, ainda que segundo estratégias distintas, pois enquanto o @zfs deduplica no caminho crítico de @io, o Btrfs delega essa tarefa no bees, um serviço que percorre o sistema de ficheiros em segundo plano recorrendo a um índice limitado a 1 GiB @bees.

No @zfs, o recordsize foi fixado em 4 KiB de modo a coincidir com o tamanho dos blocos manipulados pela generalidade das workloads, à exceção da workload 03 que recorre a blocos de 64 KiB. Esta decisão revela-se indispensável, pois a deduplicação opera ao nível do record, daí que um valor superior implicasse que blocos duplicados de 4 KiB jamais originassem records idênticos, tornando a otimização inoperante perante o conteúdo gerado @zfs_docs.

Esta diferença tem implicações diretas na leitura dos resultados, dado que no Btrfs a redução de espaço apenas se manifesta após a passagem do bees, ao contrário do @zfs onde esta é imediata, ainda que ao custo de latência acrescida nos pedidos de escrita @koller2010 @meyer2012.

Importa ainda esclarecer que a garantia oferecida pela flag `O_DIRECT` deixa de ser absoluta quando as otimizações de conteúdo se encontram ativas. No Btrfs, as leituras de dados comprimidos recorrem sempre ao caminho tradicional, sendo as escritas igualmente redirecionadas para esse caminho quando o inode possui checksums, o que corresponde à configuração por omissão @btrfs_docs.

No @zfs, por sua vez, as escritas apenas são efetuadas de forma direta caso o offset e a dimensão do pedido se encontrem alinhados com o recordsize, condição que o alinhamento adotado satisfaz. Sucede, no entanto, que a deduplicação e as escritas diretas são mutuamente incompatíveis, uma vez que os pedidos submetidos por esta via não são verificados quanto à existência de duplicados @zfs_docs.

Perante esta incompatibilidade, e dado que a deduplicação constitui precisamente um dos objetos de avaliação, a propriedade `direct` foi desativada nos conjuntos de dados envolvidos, o que encaminha as escritas através do ARC e assegura que os duplicados são efetivamente detetados, ainda que ao custo de a page cache deixar de ser contornada.

Desta forma, os resultados obtidos sobre sistemas de ficheiros não são diretamente comparáveis com os do dispositivo em bruto no que respeita ao efeito da page cache, sendo esta uma limitação que decorre da natureza das otimizações avaliadas e não da metodologia adotada.

Importa realçar que o Prismo, tal como o @fio e o Vdbench, emite pedidos de leitura e escrita de tamanho fixo sobre um ficheiro previamente alocado, exercitando por isso o caminho de dados e não as operações de metadados. Assim sendo, não se trata de uma avaliação de sistemas de ficheiros, mas antes da forma como cada sistema reage às propriedades do conteúdo que lhe é submetido.

==== Métricas

As métricas recolhidas dividem-se entre aquelas reportadas pelas próprias ferramentas e as obtidas ao nível do sistema. Do primeiro grupo fazem parte o débito, os @iops e a latência, esta última caracterizada não apenas pelo valor médio mas também pelos percentis p50, p99 e p99.9, pois a média isoladamente esconde o comportamento da cauda da distribuição @traeger2008 @tarasov2011.

Já as métricas de sistema, nomeadamente a utilização de @cpu e de @ram, são recolhidas através do `pcp dstat` com uma frequência de amostragem de um segundo, sendo esta a única fonte comum às três ferramentas e portanto a única que permite uma comparação justa do consumo de recursos.

Por fim, nas workloads que exercitam deduplicação e compressão é ainda registado o espaço efetivamente ocupado em disco, pois só através deste é possível confirmar que as otimizações do sistema de armazenamento foram de facto acionadas pelo conteúdo gerado.

=== Validação do Prismo <validation>

Antes de utilizar o Prismo para avaliar sistemas de armazenamento, é necessário estabelecer confiança na ferramenta, daí que esta secção procure demonstrar que os resultados produzidos são fiáveis e comparáveis aos das ferramentas de referência em workloads genéricas, ao mesmo tempo que valida a correção dos mecanismos de geração de conteúdo.

==== Reprodutibilidade

// TODO: mesma workload executada N vezes (via Cardoide com --repetitions)
// TODO: reportar desvio padrão e coeficiente de variação sobre métricas dos report.json
// TODO: workloads selecionadas: WL 01 (seq_write), WL 05 (rw_rand_mixed)
// Evidência: report.json de múltiplas execuções da mesma workload

==== Equivalência em Workloads Genéricas

// TODO: comparação Prismo vs FIO vs Vdbench em WL 01-09 (dados aleatórios, POSIX, raw NVMe)
// TODO: gráficos de barras: throughput (MB/s) e IOPS por ferramenta
// TODO: latência média: Prismo (report.json) vs FIO (clat.mean) vs Vdbench (resp time)
// TODO: overhead de recursos: dstat.csv, usr%+sys% e mem used entre as 3 ferramentas
// Evidência: report.json e dstat.csv de prismo_posix_1_9, fio_posix_1_9, vdbench_posix_1_9

==== Validação da Geração de Conteúdo

// TODO: Deltoide aplicado sobre dados escritos pelo Prismo → JSON com distribuições de dedup e compressão
// TODO: comparar distribuição configurada vs distribuição medida pelo Deltoide
// TODO: Deltoide aplicado sobre dados escritos por FIO e Vdbench → confirmar ~0% duplicados e compressão negligenciável
// TODO: gráfico ou tabela: distribuição configurada vs medida (Prismo) vs medida (FIO/Vdbench)
// Evidência: output JSON do Deltoide sobre datasets gerados por cada ferramenta

==== Discussão

// TODO: síntese dos 3 subpontos anteriores
// TODO: conclusão: o Prismo é tão fiável quanto FIO/Vdbench em workloads genéricas
// TODO: destaque: a validação do conteúdo confirma que o Prismo gera dados realistas que os outros não conseguem
// TODO: fundamentação de V1, V2, V3

=== Impacto das Propriedades dos Dados no Desempenho <data-properties>

Uma vez demonstrada a equivalência do Prismo em workloads genéricas, esta secção explora o eixo de diferenciação fundamental, nomeadamente o impacto das propriedades intrínsecas dos dados no desempenho dos sistemas de armazenamento. Na prática, benchmarks que ignoram a compressibilidade e a taxa de duplicados dos dados tendem a produzir avaliações que não refletem o comportamento real dos sistemas, em particular daqueles que implementam otimizações sensíveis ao conteúdo, como é o caso do @zfs e do Btrfs.

Convém realçar que a análise se inicia pelo estabelecimento de uma linha de base com conteúdo aleatório, sem a qual seria impossível distinguir o custo intrínseco de cada sistema de armazenamento do efeito atribuível às propriedades dos dados, afinal apenas a comparação entre ambos os cenários permite isolar o contributo do conteúdo.

==== Linha de Base por Sistema de Armazenamento

// TODO: WL 01, 04, 05 (conteúdo aleatório, logo incompressível e sem duplicados) em raw NVMe vs
//       Btrfs vs ZFS, engine POSIX para comparação justa
// TODO: objetivo: estabelecer o custo de cada sistema independentemente das propriedades do
//       conteúdo, servindo de controlo às subsecções seguintes
// TODO: declarar o desalinhamento entre block_size de 4K e o recordsize do ZFS como potencial
//       confundidor (read-modify-write), e verificar a configuração efetivamente usada
// TODO: gráfico: débito e latência por workload × sistema de armazenamento
// Evidência: report.json de prismo_posix_1_9_odirect_dev_nvme vs _btrfs vs _zfs

==== Compressão

// TODO: WL 10 (compress_zipf): Prismo (dados compressíveis) vs FIO (dados aleatórios)
// TODO: targets: ZFS (compressão nativa), Btrfs, raw NVMe
// TODO: ZFS otimiza dados compressíveis do Prismo → throughput superior; FIO gera aleatórios → ZFS não beneficia
// TODO: gráfico: throughput por ferramenta × target
// Evidência: report.json de prismo_*_10_11 vs fio_*_10_11 em ZFS, Btrfs, NVMe

==== Deduplicação

// TODO: WL 11 (dedup_zipf): mesmo esquema de comparação
// TODO: impacto em ZFS com deduplicação ativa
// TODO: Deltoide confirma que WL 11 do Prismo contém duplicados reais vs FIO (~0%)
// TODO: gráfico: throughput e espaço utilizado por ferramenta × target
// Evidência: report.json + Deltoide sobre dados escritos

==== Trade-offs: Redução de I/O vs Overhead Computacional

// TODO: dstat.csv WL 10-11: séries temporais de usr%+sys% e mem used
// TODO: comparar ZFS-dedup vs raw NVMe: CPU e RAM adicionais consumidos
// TODO: análise: quando o overhead computacional da dedup/compressão supera o benefício da redução de I/O
// TODO: gráfico: CPU% e RAM ao longo do tempo por target
// Evidência: dstat.csv de execuções em ZFS-dedup vs raw NVMe

==== Discussão

// TODO: quantificar a diferença entre o débito reportado por FIO vs Prismo no mesmo sistema
// TODO: esta diferença representa o erro de avaliação introduzido por benchmarks que ignoram conteúdo
// TODO: conclusão: benchmarks que não modelam propriedades de conteúdo produzem avaliações enganadoras
// TODO: contrastar o ganho obtido em cada sistema com a respetiva linha de base, de modo a atribuir
//       a diferença ao conteúdo e não ao sistema de armazenamento
// TODO: recomendações sobre a escolha do sistema para diferentes perfis de workload
// TODO: fundamentação de Q1, Q2, Q6

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

==== Discussão

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

==== Discussão

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

==== Discussão

// TODO: conclusão: workloads com localidade realista são necessárias para exercitar mecanismos de cache
// TODO: comparação de latência p99 entre padrões de acesso
// TODO: fundamentação de Q5

=== Síntese e Discussão Geral <evaluation-synthesis>

// TODO: parágrafo introdutório da síntese

==== Respostas às Perguntas de Avaliação

// TODO: Q1: resposta concisa com referência cruzada à secção de impacto das propriedades dos dados
// TODO: Q2: resposta concisa com referência cruzada à mesma secção
// TODO: Q3: resposta concisa com referência cruzada à secção de interfaces de I/O
// TODO: Q4: resposta concisa com referência cruzada à secção de workloads baseadas em traces
// TODO: Q5: resposta concisa com referência cruzada à secção de localidade e cache
// TODO: Q6: resposta concisa com referência cruzada à linha de base e aos trade-offs da secção de
//       impacto das propriedades dos dados
// TODO: Q7: resposta transversal — confronto com FIO e Vdbench ao longo de todo o capítulo,
//       quantificando o erro de avaliação de quem ignora conteúdo e enumerando as conclusões
//       sobre os sistemas avaliados que só o Prismo permite alcançar

==== Validação

// TODO: V1: confirmação com referência à secção de validação do Prismo (geração de conteúdo)
// TODO: V2: confirmação com referência à secção de reprodutibilidade
// TODO: V3: confirmação com referência às métricas e relatórios produzidos
// TODO: V4: confirmação com referência à equivalência demonstrada em workloads genéricas (a
//       diferença não vem da instrumentação) cruzada com a divergência observada em ZFS/Btrfs

==== Limitações

// TODO: workloads não testadas, sistemas não avaliados
// TODO: condições experimentais (single machine, single device)
// TODO: limitações dos traces disponíveis
// TODO: o caminho de metadados dos sistemas de ficheiros não é exercitado, dado que o Prismo, tal
//       como o FIO e o Vdbench, opera sobre um ficheiro pré-alocado; uma avaliação completa exigiria
//       workloads intensivas em metadados, ficando essa análise fora do âmbito desta dissertação

==== Sumário

// TODO: contribuição principal: o Prismo iguala as ferramentas existentes em workloads genéricas,
//       diferencia-se na geração de conteúdo realista, e é exclusivo no suporte a traces com
//       propriedades de dados, progressão dos 3 patamares (equivalência → diferenciação → exclusividade)
