#let raw_code_block(body) = {
  set text(size: 11pt)
  block(
    fill: luma(230),
    inset: 8pt,
    radius: 5pt,
    stroke: 1pt,
    width: 100%,
    [#body],
  )
}