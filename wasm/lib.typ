#let _wasm = plugin("plugin.wasm")

#let _palette = (rgb("#1f77b4"), rgb("#d62728"), rgb("#2ca02c"), rgb("#ff7f0e"), rgb("#9467bd"))
#let _alphabet = ("X", "Y", "Z", "U", "V", "W", "S", "T", "P", "Q")

#let _fmt_var(id_str) = {
  let id = int(id_str)
  let letter = _alphabet.at(calc.rem(id - 1, _alphabet.len()))
  let col = _palette.at(calc.rem(id - 1, _palette.len()))
  let primes = "'" * calc.floor((id - 1) / _alphabet.len())
  
  text(fill: col, weight: "bold", math.italic(letter + primes))
}

#let _humanize(formula-str) = {
  if type(formula-str) != str { return formula-str }
  
  let processed = formula-str.replace(regex("([A-Z])_(\d+)"), match => {
    " fmt(\"" + match.captures.at(1) + "\") "
  })
  
  return eval(processed, mode: "math", scope: (fmt: _fmt_var))
}

// Wrapper
#let _run(func, formula-str) = {
  return str(func(bytes(formula-str)))
}

// Step by step

#let remove-implications(f) = _humanize(_run(_wasm.run_remove_implications, f))
#let alpha(f)               = _humanize(_run(_wasm.run_alpha, f))
#let nnf(f)                 = _humanize(_run(_wasm.run_nnf, f))
#let pnf(f)                 = _humanize(_run(_wasm.run_pnf, f))
#let skolemize(f)           = _humanize(_run(_wasm.run_skolem, f))
#let distribute(f)          = _humanize(_run(_wasm.run_distribute, f))
#let push-universals(f)     = _humanize(_run(_wasm.run_push_universals, f))
#let to-sets(f)             = _humanize(_run(_wasm.run_to_sets, f))

// MACRO

#let to-cnf(formula) = {
  let current = formula
  current = _run(_wasm.run_remove_implications, current)
  current = _run(_wasm.run_alpha, current)
  current = _run(_wasm.run_nnf, current)
  current = _run(_wasm.run_pnf, current)
  current = _run(_wasm.run_skolem, current)
  current = _run(_wasm.run_distribute, current)
  current = _run(_wasm.run_push_universals, current)
  current = _run(_wasm.run_alpha, current)
  current = _run(_wasm.run_to_sets, current)
  
  return _humanize(current)
}

// Demos API

#let steps = (
  impl:  (name: "Eliminar Implicaciones", func: (b) => _wasm.run_remove_implications(b)),
  alpha: (name: "Rectificación",  func: (b) => _wasm.run_alpha(b)),
  nnf:   (name: "Forma Normal Negada",  func: (b) => _wasm.run_nnf(b)),
  pnf:   (name: "Forma Normal Prenexa",   func: (b) => _wasm.run_pnf(b)),
  sko:   (name: "Skolemización",          func: (b) => _wasm.run_skolem(b)),
  dist:  (name: "Distribución",     func: (b) => _wasm.run_distribute(b)),
  push:  (name: "Miniscoping",            func: (b) => _wasm.run_push_universals(b)),
  sets:  (name: "Conjuntos de Cláusulas", func: (b) => _wasm.run_to_sets(b)),
)

#let proof(formula, pipeline) = block(
  fill: luma(250), inset: 12pt, radius: 4pt, stroke: 1pt + luma(200),
  {
    set align(left)
    text(weight: "bold", fill: rgb("#800000"))[]
    v(4pt)
    [ *Input:* #_humanize(formula) ]
    v(8pt)
    
    let current = formula
    for step in pipeline {
      current = str((step.func)(bytes(current)))
      grid(
        columns: (15pt, auto),
        gutter: 8pt,
        align: (right, left),
        [$equiv$], [#text(style: "italic", size: 0.85em, luma(100))[#step.name:] \ #_humanize(current)]
      )
      v(4pt)
    }
  }
)