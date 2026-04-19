#let logic-plugin = plugin("plugin.wasm")

#str(logic-plugin.run_cnf_transform(bytes("forall X. Q(f(f(f(x))),y) and O")))