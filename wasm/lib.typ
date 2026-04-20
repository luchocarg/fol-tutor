#let logic = plugin("plugin.wasm")

#let test = "forall X. forall X. exists Z. (P(X) and (F(f(X)) or (K and M(X))))"
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

#let test = str(logic.run_skolem(bytes("∀X.∃Y.P(X,Y)")))
#eval("$" + test + "$")