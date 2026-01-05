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

// explicar que aqui dentro ainda podemos fazer outra divisão, no caso dos offset, conteudo, operação

#figure(
    image("../images/producer.png", width: 60%),
    caption: [Interação do produtor com a interface de geração de conteúdo]
)

// explicar muito bravemente o diagrama

===== Acesso

// explicar por abstrato em que consiste a interface e o metodo disponibilizado
// explicar por bullet points cada uma das implementações contretas

#figure(
    image("../images/access.png", width: 60%),
    caption: [Hierarquia da interface de acessos]
)

===== Operação

#figure(
    image("../images/operation.png", width: 60%),
    caption: [Hierarquia da interface de operações]
)

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
