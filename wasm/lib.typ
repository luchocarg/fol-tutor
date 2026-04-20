#let my-plugin = plugin("plugin.wasm")

#let cnf-transform(formula) = {
  let result = my-plugin.run_cnf_transform(bytes(formula))
  
  str(result)
}

#let original = "forall X. (forall X. (forall X. (P(X) => X) and X) and K(X))"
#let transformed = cnf-transform(original)

Original: #eval("$" + original + "$") \
S-expression: #raw(transformed)