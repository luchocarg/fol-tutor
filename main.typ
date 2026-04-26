#import "wasm/lib.typ" as fol

#let f = "forall ?X. (P(?X) implies Q(s,f_1(s),sigma))"

#fol.apply-impl(f) 

#fol.humanize(fol.apply-impl(f))