#let plugin = plugin("plugin.wasm")

#let mgu(lit1, lit2) = {
  let input = lit1 + "∧" + lit2
  let result-bytes = plugin.run_calculate_mgu(bytes(input))
  return eval("$"+str(result-bytes)+"$")
}



#let resolver(c1, c2) = {
  let input = "(" + c1 + ") ∧ (" + c2 + ")"
  let res = plugin.run_resolve(bytes(input))
  eval("$" + str(res) + "$")
}

#let formula = "not exists X. (forall Y.P(X,Y) => forall Y.P(Y,X))"

#let transform = (
  plugin.run_remove_implications,
  plugin.run_alpha,
  plugin.run_nnf,
  plugin.run_pnf,
  plugin.run_skolem,
  plugin.run_distribute,
  plugin.run_push_universals,
  plugin.run_alpha,
  plugin.run_to_sets,
)

$ 
& #eval(formula, mode: "math") \

#for step in transform {
  formula = str(step(bytes(formula)))
  $ &equiv #eval(formula, mode: "math")\ $
}
$








/*
(#mgu("P(X)","P(f(Z))"))  \
#let test = resolver("P(X_1,X_2)", "¬P(X_4,X_3)")
test
/*