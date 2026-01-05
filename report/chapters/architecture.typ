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

// meter exepmlos de codigo para motrar a interface em confionamento, ou seja,
// Access sequentialaccess = new SequentialAcces
// Acess randomaccess = new RandomAccess
//
// explicar por bullet points cada uma das implementações contretas
//
// // DIAGRAMA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

===== Acesso

===== Operação

===== Conteúdo




==== Integração de Interfaces de I/O

// dizer que também seguem uma interface

// explicar cada um dos metodos e dizer que o reap leaft completions não faz nada no caso das interfaces sincronas

// DIAGRAMA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

===== POSIX

// para cada uma, meter um diagrama que explica o que acontece em backgrounf
// DIAGRAMA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

===== Uring

// DIAGRAMA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

===== SPDK

// DIAGRAMA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


==== Flow de Execução

// explicar a questão do producer consumer e meter um diagrama

// DIAGRAMA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
