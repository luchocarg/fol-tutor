#let logic = plugin("plugin.wasm")

#let test = "not (exists X. (forall Y. P(X,Y) => forall Y. P(Y,X)))"
#eval("$" + test + "$")

#let test = str(logic.run_remove_implications(bytes(test)))
#eval("$" + test + "$")

#let test = str(logic.run_alpha(bytes(test)))
#eval("$" + test + "$")

#let test = str(logic.run_nnf(bytes(test)))
#eval("$" + test + "$")

#let test = str(logic.run_pnf(bytes(test)))
#eval("$" + test + "$")

#let test = str(logic.run_skolem(bytes(test)))
#eval("$" + test + "$")

#let test = str(logic.run_distribute(bytes(test)))
#eval("$" + test + "$")

#let test = str(logic.run_push_universals(bytes(test)))
#eval("$" + test + "$")

#let test = str(logic.run_alpha(bytes(test)))
#eval("$" + test + "$")

#let test = str(logic.run_to_sets(bytes(test)))
#eval("$" + test + "$")