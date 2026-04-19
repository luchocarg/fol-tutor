#let logic-plugin = plugin("plugin.wasm")

#let res = logic-plugin.run_cnf_transform(bytes("forall X_23. Q(f(X_23, Y_23, Z_23),y) and O"))

$forall X_23. Q(f(x),y) and O$

#str(res)