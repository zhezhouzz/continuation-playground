open MLton
open Cont
datatype ('a, 'b) coroution = C of ('a * ('b, 'a) coroution)  t

fun kill () = C ( isolate (fn _ => ()))

fun yield (C k: ('a, 'b) coroution) (v: 'a) : 'b * ('a, 'b) coroution =
    callcc (fn self => throw (k, (v, C self)))

fun coroutine (f: 'a * ('b, 'a) coroution -> ('a, 'b) coroution) : ('a, 'b) coroution =
    callcc
        (fn (exit: ('a, 'b) coroution  t) =>
            f ( callcc (fn (k: ('a * ('b, 'a) coroution)  t) =>
                           throw (exit, C k)
                       )
              )
        )

(* Real Imp *)

fun counter (client: (int, unit) coroution): (unit, int) coroution =
    let
        val (_, client) = yield client 0
        val (_, client) = yield client 1
    in
        kill ()
    end

fun reader (f: (unit, int) coroution): unit =
    let
        val (n, f) = yield f ()
        val _ = print("counter " ^ (Int.toString n ^ "\n"))
        val (n, f) = yield f ()
        val _ = print("counter " ^ (Int.toString n ^ "\n"))
    in
        ()
    end
val _ = reader (coroutine (fn (_, k) => counter k));;
