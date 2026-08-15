#import "@preview/lilaq:0.6.0" as lq

// Paleta por ferramenta, constante em todo o capítulo. As cores diferem também em
// luminosidade, de modo a permanecerem distinguíveis em impressão a preto e branco.
#let tool-colors = (
  prismo: rgb("#1f4e79"),
  fio: rgb("#e08214"),
  vdbench: rgb("#8c8c8c"),
)

#let tool-labels = (prismo: [Prismo], fio: [FIO], vdbench: [Vdbench])

// Lê um CSV de figura com o formato
//   workload,prismo,fio,vdbench,prismo_std,fio_std,vdbench_std
// e devolve as séries prontas a desenhar, ignorando as ferramentas sem valores.
// O caminho é resolvido a partir deste ficheiro, de modo a não depender da raiz
// configurada no compilador.
#let read-series(name) = {
  let rows = csv("../data/" + name).slice(1)
  let tools = ("prismo", "fio", "vdbench")
  let present = tools.filter(t => {
    let i = tools.position(x => x == t) + 1
    rows.any(r => r.at(i) != "")
  })
  (
    labels: rows.map(r => r.at(0)),
    series: present.map(t => {
      let i = tools.position(x => x == t) + 1
      (
        name: t,
        values: rows.map(r => if r.at(i) == "" { 0.0 } else { float(r.at(i)) }),
        errors: rows.map(r => if r.at(i + 3) == "" { 0.0 } else { float(r.at(i + 3)) }),
      )
    }),
  )
}

// Linhas por ferramenta ao longo das workloads. Espera-se um CSV com o formato
// workload,prismo,fio,vdbench, sem colunas de desvio.
#let tool-lines(name, ylabel: none, xlabel: [Workload], width: 12.2cm, height: 5.0cm) = {
  let rows = csv("../data/" + name).slice(1)
  let tools = ("prismo", "fio", "vdbench")
  let xs = range(rows.len())
  let marks = ("o", "s", "d")

  lq.diagram(
    width: width,
    height: height,
    xlabel: xlabel,
    ylabel: ylabel,
    legend: (position: right + top),
    xaxis: (ticks: xs.zip(rows.map(r => [#r.at(0)])).map(((i, l)) => (i, l))),
    ..tools.enumerate().map(((i, t)) => lq.plot(
      xs,
      rows.map(r => float(r.at(i + 1))),
      color: tool-colors.at(t),
      mark: marks.at(i),
      label: tool-labels.at(t),
    )),
  )
}

// Repartição do tempo de CPU por estado, em barras empilhadas. Espera-se um CSV com o
// formato workload,usr,sys,wai. O tempo restante corresponde a idle e não é desenhado,
// sob pena de as restantes componentes se tornarem ilegíveis.
#let cpu-states = (
  usr: (rgb("#1f4e79"), [Utilizador]),
  sys: (rgb("#5b9bd5"), [Sistema]),
  wai: (rgb("#e08214"), [Espera por @io]),
)

#let cpu-stack(name, width: 12.2cm, height: 5.0cm, headroom: 1.45) = {
  let rows = csv("../data/" + name).slice(1)
  let xs = range(rows.len())
  let values = ("usr", "sys", "wai").enumerate().map(((i, key)) => (
    key: key,
    data: rows.map(r => float(r.at(i + 1))),
  ))
  // O lq.bar desenha de `base` até `y`, ambos absolutos, pelo que cada camada recebe
  // o topo acumulado e não a altura do próprio segmento.
  let bases = ()
  let tops = ()
  let running = rows.map(_ => 0.0)
  for layer in values {
    bases.push(running)
    running = running.zip(layer.data).map(((a, b)) => a + b)
    tops.push(running)
  }

  lq.diagram(
    width: width,
    height: height,
    xlabel: [Workload],
    ylabel: [Tempo de @cpu (%)],
    ylim: (0, calc.max(..running) * headroom),
    legend: (position: right + top),
    xaxis: (ticks: xs.zip(rows.map(r => [#r.at(0)])).map(((i, l)) => (i, l))),
    ..values.enumerate().map(((i, layer)) => lq.bar(
      xs,
      tops.at(i),
      base: bases.at(i),
      width: 0.62,
      fill: cpu-states.at(layer.key).at(0),
      label: cpu-states.at(layer.key).at(1),
    )),
  )
}

// Gráfico de barras agrupadas por ferramenta, com barras de erro correspondentes ao
// desvio padrão das execuções. As barras de erro só são desenhadas quando existe
// dispersão, o que evita marcas espúrias enquanto houver uma única execução.
#let tool-bars(
  name, ylabel: none, xlabel: [Workload],
  width: 12.2cm, height: 5.4cm, yscale: auto, legend: true,
) = {
  let data = read-series(name)
  let n = data.series.len()
  let bar-width = 0.8 / n
  let xs = range(data.labels.len())

  lq.diagram(
    width: width,
    height: height,
    xlabel: xlabel,
    ylabel: ylabel,
    yscale: yscale,
    legend: if legend { (position: right + top) } else { none },
    xaxis: (ticks: xs.zip(data.labels.map(l => [#l])).map(((i, l)) => (i, l))),
    ..data.series.enumerate().map(((k, s)) => lq.bar(
      xs.map(x => x + (k - (n - 1) / 2) * bar-width),
      s.values,
      width: bar-width,
      fill: tool-colors.at(s.name),
      label: tool-labels.at(s.name),
    )),
    ..data.series.enumerate().filter(((k, s)) => s.errors.any(e => e > 0)).map(((k, s)) => lq.plot(
      xs.map(x => x + (k - (n - 1) / 2) * bar-width),
      s.values,
      yerr: s.errors,
      color: black,
      stroke: none,
    )),
  )
}

// Painel de barras horizontais para uma categoria de componentes. As barras horizontais
// acomodam nomes de variantes longos sem sobreposição, e cada painel mantém a sua própria
// escala, dado que o débito difere em ordens de grandeza entre categorias.
#let component-bars(name, categoria, width: 5.7cm, height: 3.6cm) = {
  let rows = csv("../data/" + name).slice(1).filter(r => r.at(0) == categoria)
  let ys = range(rows.len())
  lq.diagram(
    width: width,
    height: height,
    title: categoria,
    xlabel: [Milhões de operações por segundo],
    yaxis: (ticks: ys.zip(rows.map(r => [#r.at(1)])).map(((i, l)) => (i, l))),
    lq.hbar(rows.map(r => float(r.at(2))), ys, width: 0.6, fill: tool-colors.prismo),
  )
}
