#let plugin = plugin("plugin.wasm")

#let mgu(lit1, lit2) = {
  let input = lit1 + "∧" + lit2
  let result-bytes = plugin.run_calculate_mgu(bytes(input))
  return eval("$"+str(result-bytes)+"$")
}

#mgu("P(X, Y)","P(f(Z), Z)")