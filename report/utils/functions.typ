#let raw_code_block(body, width: 100%) = {
  set text(size: 11pt)
  block(
    fill: luma(230),
    inset: 8pt,
    radius: 5pt,
    stroke: 1pt,
    width: width,
    align(left)[#body]
  )
}

#let my_block(body, color: color, width: 100%) = {
  block(
    fill: color,
    stroke: 1pt,
    radius: 5pt,
    inset: 8pt,
    width: width,
    align(left)[#body]
  )
}

#let question_block(body) = {
  my_block(body, color: gray.lighten(60%), width: 100%)
}

#let validation_point_block(body) = {
  my_block(body, color: navy.lighten(80%), width: 100%)
}

#let doc_table(columns: (), header: (), fill: none, align: horizon + left, ..rows) = {
  table(
    columns: columns,
    inset: 6pt,
    align: align,
    fill: (x, y) => {
      if y == 0 { gray.lighten(60%) } else if fill != none { fill(x, y) }
    },
    table.header(..header.map(strong)),
    ..rows,
  )
}

#let cell_yes = table.cell(fill: green.lighten(60%))[Sim]
#let cell_no = table.cell(fill: red.lighten(60%))[Não]
#let cell_partial(body) = table.cell(fill: yellow.lighten(60%), body)
