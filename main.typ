#import "wasm/lib.typ" as fol

#let formula = "not exists X. (forall Y. P(X, Y) => forall Y. P(Y, X))"

== Formula 

#eval(formula, mode: "math")

== Funciones
$"NNF" = #fol.nnf(formula)$

== Macro
$"CNF" = #fol.to-cnf(formula)$

== Proof Block
#fol.proof(
  formula,
  (
    fol.steps.impl,
    fol.steps.alpha,
    fol.steps.nnf,
    fol.steps.pnf,
    fol.steps.sko,
    fol.steps.dist,
    fol.steps.alpha,
    fol.steps.sets
  )
)