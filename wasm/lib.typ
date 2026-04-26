#let _wasm = plugin("plugin.wasm")

#let _run(func, formula-str) = {
  return str(func(bytes(formula-str)))
}

#let apply-impl(f)       = _run(_wasm.run_remove_implications, f)
#let apply-alpha(f)      = _run(_wasm.run_alpha, f)
#let apply-nnf(f)        = _run(_wasm.run_nnf, f)
#let apply-pnf(f)        = _run(_wasm.run_pnf, f)
#let apply-skolem(f)     = _run(_wasm.run_skolem, f)
#let apply-distribute(f) = _run(_wasm.run_distribute, f)
#let apply-push(f)       = _run(_wasm.run_push_universals, f)
#let apply-to-sets(f)    = _run(_wasm.run_to_sets, f)
#let auto-resolve(f)     = _run(_wasm.run_auto_resolve, f)
#let mgu-trace(f)        = _run(_wasm.run_calculate_mgu_trace, f)
#let run_to_json(f)      = _run(_wasm.run_to_json, f)

#let config = (
  colors: (
    rgb("#e6194B"), rgb("#3cb44b"), rgb("#4363d8"), rgb("#f58231"),
    rgb("#911eb4"), rgb("#42d4f4"), rgb("#f032e6"), rgb("#bfef45"),
    rgb("#469990"), rgb("#dcbeff"), rgb("#9A6324"), rgb("#800000"),
    rgb("#808000"), rgb("#000075"), rgb("#fabed4"), rgb("#aaffc3")
  ),
  c-map: (
    "a":0,"b":1,"c":2,"d":3,"e":4,"f":5,"g":6,"h":7,"i":8,"j":9,"k":10,"l":11,"m":12,
    "n":13,"o":14,"p":15,"q":0,"r":1,"s":2,"t":3,"u":4,"v":5,"w":6,"x":7,"y":8,"z":9,
    "A":0,"B":1,"C":2,"D":3,"E":4,"F":5,"G":6,"H":7,"I":8,"J":9,"K":10,"L":11,"M":12,
    "N":13,"O":14,"P":15,"Q":0,"R":1,"S":2,"T":3,"U":4,"V":5,"W":6,"X":7,"Y":8,"Z":9
  ),
  skip-words: (
    "brace", "l", "r", "auto",
    "forall", "exists",
    "not", "and", "or", "bot"
  )
)

#let _render-var(var-name) = {
  let clean-name = var-name.replace("?","")
  let first-char = clean-name.clusters().first()
  let hash = config.c-map.at(first-char, default: 0)
  let c = config.colors.at(calc.rem(hash,config.colors.len()))
  return  text(fill: c)[$#clean-name$]
}

#let _render-node(node) = {
  if type(node) != dictionary { return $#node$ }

  let t = node.at("type", default: "unknown")

  if t == "quantifier" {
    let sym = if node.op == "forall" { $forall$ } else { $exists$ }
    let var = _render-var(node.var)
    return $#sym #var . #_render-node(node.body)$
  }

  if t == "binary" {
    let sym = if node.op == "and" { $and$ }
              else if node.op == "or" { $or$ }
              else if node.op == "implies" { $=>$ }
              else { [#node.op] }
    return $(#_render-node(node.left) #sym #_render-node(node.right))$
  }

  if t == "unary" {
    let sym = if node.op == "not" { $not$ } else { [#node.op] }
    return $#sym #_render-node(node.body)$
  }

  if t == "atom" {
    let name = math.op(node.name) 
    
    if "terms" not in node or node.terms.len() == 0 {
    return $name$ 
  }
    
    let terms-rendered = node.terms.map(_render-node).join([, ])
    return $#name (#terms-rendered)$
  }

  if t == "var" {
    return _render-var(node.name)
  }

  if t == "const" {
    return $#node.name$
  }

  if t == "func" {
    let name = node.name 
    let args = node.args.map(_render-node).join([, ])
    return [#name (#args)]
  }

  return text(fill: red)[Error: unknown node]
}

#let _run_json(formula-str) = {
  let json-data = str(_wasm.run_to_json(bytes(formula-str)))
  return json(bytes(json-data))
}

#let humanize(formula-str) = {
  let ast = _run_json(formula-str)
  return _render-node(ast)
}