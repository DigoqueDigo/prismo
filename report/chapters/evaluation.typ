#import "../utils/functions.typ" : question_block, validation_point_block

== Avaliação Experimental <chapter4>

Após a descrição da arquitetura e dos mecanismos que sustentam a geração de workloads realistas, importa agora avaliar experimentalmente o Prismo, por um lado validando a sua correção e comparando o desempenho com as ferramentas de referência, e por outro demonstrando que a incorporação de propriedades de conteúdo nas workloads revela comportamentos dos sistemas de armazenamento que, de outro modo, permaneceriam invisíveis.

Neste sentido, são definidas seis perguntas de investigação e três pontos de validação que orientam as experiências realizadas:

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
/ Q6: A execução da mesma workload sobre um block device e sobre um sistema de ficheiros resulta na obtenção de métricas de desempenho distintas?
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

Posto isto, o capítulo inicia-se pela descrição da metodologia experimental, avançando depois para a validação da ferramenta através da demonstração de equivalência com os benchmarks de referência em workloads genéricas. De seguida, são analisados cenários onde as funcionalidades exclusivas do Prismo se revelam determinantes, nomeadamente a geração de conteúdo com propriedades de deduplicação e compressão, a comparação entre interfaces de @io e a replicação de workloads baseadas em traces. Por fim, exploram-se eixos complementares, localidade de acesso e diferenças entre block devices e sistemas de ficheiros, antes de sintetizar as conclusões.

=== Metodologia <methodology>

// TODO: parágrafo introdutório da metodologia

==== Setup Experimental

// TODO: descrição do hardware (Cloudinha140 / Alibaba)
// TODO: sistema operativo, kernel, configurações relevantes (scheduler, O_DIRECT)
// TODO: tabela com especificações de hardware

==== Ferramentas Comparadas

// TODO: Prismo, versão, configuração base
// TODO: FIO, versão, equivalência de configurações de workload
// TODO: Vdbench, versão, limitações (apenas POSIX síncrono)
// TODO: tabela comparativa de funcionalidades (engines suportadas, geração de conteúdo, traces, etc.)

==== Campanha Experimental

// TODO: tabela-resumo das 15 workloads com dimensões (operação × acesso × conteúdo)
// TODO: procedimento: duração/tamanho das runs, warm-up, cleanup entre execuções
// TODO: número de repetições e tratamento estatístico (Cardoide com --repetitions)

==== Sistemas de Armazenamento Avaliados

// TODO: block devices: raw NVMe (/dev/nvme0n1)
// TODO: sistemas de ficheiros: Btrfs, ZFS
// TODO: tabela com propriedades de cada sistema (dedup nativo, compressão nativa, stack)

==== Métricas

// TODO: throughput (MB/s), IOPS, latência (p50, p99)
// TODO: CPU%, RAM (recolha via dstat/pidstat)
// TODO: espaço em disco (para workloads com dedup/compressão)
// TODO: ferramentas de recolha e frequência de amostragem

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

// TODO: quantificar a diferença entre throughput reportado por FIO vs Prismo no mesmo sistema
// TODO: esta diferença representa o erro de avaliação introduzido por benchmarks que ignoram conteúdo
// TODO: conclusão: benchmarks que não modelam propriedades de conteúdo produzem avaliações enganadoras
// TODO: fundamentação de Q1, Q2

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

=== Block Devices vs Sistemas de Ficheiros <bdev-vs-fs>

Ao executar workloads diretamente sobre um block device, a camada de abstração do sistema de ficheiros é eliminada, o que à partida deverá resultar em perfis de desempenho distintos. Deste modo, esta secção procura quantificar essas diferenças e analisar o impacto das funcionalidades nativas dos sistemas de ficheiros nos resultados obtidos.

==== Overhead do Filesystem

// TODO: WL 01, 04, 05 em raw NVMe vs Btrfs vs ZFS (engine POSIX, para comparação justa)
// TODO: quantificar overhead introduzido pelo filesystem em throughput e latência
// TODO: gráfico: throughput por workload × target
// Evidência: report.json de prismo_posix_1_9_odirect_dev_nvme vs _btrfs vs _zfs

==== Deduplicação e Compressão Nativa

// TODO: WL 10-11 em ZFS (dedup+compress) vs Btrfs (compress) vs raw NVMe (sem)
// TODO: benefícios vs custos: throughput, CPU, RAM, espaço em disco
// TODO: gráfico: throughput e espaço utilizado por target
// Evidência: report.json + dstat.csv de prismo_*_10_11_odirect_{zfs,btrfs}

==== Discussão

// TODO: o ranking de engines pode inverter-se consoante o target
// TODO: recomendações sobre escolha de sistema para diferentes perfis de workload
// TODO: fundamentação de Q6

=== Síntese e Discussão Geral <evaluation-synthesis>

// TODO: parágrafo introdutório da síntese

==== Respostas às Perguntas de Avaliação

// TODO: Q1: resposta concisa com referência cruzada à secção de impacto das propriedades dos dados
// TODO: Q2: resposta concisa com referência cruzada à mesma secção
// TODO: Q3: resposta concisa com referência cruzada à secção de interfaces de I/O
// TODO: Q4: resposta concisa com referência cruzada à secção de workloads baseadas em traces
// TODO: Q5: resposta concisa com referência cruzada à secção de localidade e cache
// TODO: Q6: resposta concisa com referência cruzada à secção de bdev vs FS

==== Validação

// TODO: V1: confirmação com referência à secção de validação do Prismo (geração de conteúdo)
// TODO: V2: confirmação com referência à secção de reprodutibilidade
// TODO: V3: confirmação com referência às métricas e relatórios produzidos

==== Limitações

// TODO: workloads não testadas, sistemas não avaliados
// TODO: condições experimentais (single machine, single device)
// TODO: limitações dos traces disponíveis

==== Sumário

// TODO: contribuição principal: o Prismo iguala as ferramentas existentes em workloads genéricas,
//       diferencia-se na geração de conteúdo realista, e é exclusivo no suporte a traces com
//       propriedades de dados, progressão dos 3 patamares (equivalência → diferenciação → exclusividade)
