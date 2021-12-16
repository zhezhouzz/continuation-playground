type 'a t = 'a -> unit
type ('a, 'b) coroution = C of ('a * ('b, 'a) coroution) t

let x : (unit, 'a) coroution = C (fun _ -> ())

let yield (self: ('b * ('a, 'b) coroution) t) (cr: ('a, 'b) coroution) (v: 'a): unit =
  match cr with
  | C k -> k (v, C self)

let coroutine (self: (('a, 'b) coroution) t) (f: ((unit, int) coroution) t -> 'a * ('b, 'a) coroution -> unit) : unit =
  self (C (fun x -> f self x))

let counter (self: ((unit, int) coroution) t) (client: unit * (int, unit) coroution): unit =
  let self = fun _ -> (self x) in
  let self = fun (_, client) -> yield self client 1 in
  yield self (snd client) 0

let reader (self: unit t) (f: (unit, int) coroution): unit =
  let self = fun (n, _) -> Printf.printf "counter %i\n" n in
  let self = fun (_, f) -> yield self f () in
  let self = fun (n, f) -> Printf.printf "counter %i\n" n; self (n, f) in
  yield self f ()
;;

let self = fun f -> reader (fun () -> ()) f in
coroutine self counter;;
