#include <functional>
#include <utility>
#include <variant>
#include <iostream>
#include <type_traits>
using namespace std;

template <typename A>
using Continuation = function<monostate(A)>;

template <typename A, typename B>
class Coroution {
public:
  Continuation<pair<A, Coroution<B, A>*>> cont;
  Coroution(Continuation<pair<A, Coroution<B, A>*>> cont) {
    cont = cont;}
};

template <typename A>
Coroution<monostate, A>* kill() {
  Continuation<pair<monostate, Coroution<A, monostate>*>> cont = [](auto x) {return monostate();};
  return new Coroution<monostate, A>(cont);
}

template <typename A, typename B>
monostate yield(Continuation<pair<B, Coroution<A, B>*>> k_self,
           Coroution<A, B>* cr, A a) {
  cr->cont(make_pair(a, new Coroution<B, A>(k_self)));
  return monostate();
}

template <typename A, typename B>
monostate coroutine(Continuation<Coroution<A, B>*> k_self,
               function<monostate(Continuation<Coroution<A, B>*>, pair<A, Coroution<B, A>*>)> f) {
  k_self(new Coroution<A, B>([&](auto x) {
    return f(k_self, x);}));
  return monostate();
}

int generate(int x) {
  return x;
}

// void counter(client) {
//   yield(client.second, generate(0));
//   yield(client.second, generate(1));
//   return kill();
// }
monostate counter (Continuation<Coroution<monostate, int>*> k_self,
              pair<monostate, Coroution<int, monostate>*> client) {
  auto self_0 = [&](pair<monostate, Coroution<int, monostate>*> p) {
    return k_self(kill<int>());};
  auto self_1 = [&](pair<monostate, Coroution<int, monostate>*> p) {
    return yield<int, monostate>(self_0, p.second, generate(1));};
  return yield<int, monostate>(self_1, client.second, generate(0));
}

// void read(f) {
//   auto p = yield(f, _);
//   cout << "counter " << p.first << endl;
//   auto p = yield(f, _);
//   cout << "counter " << p.first << endl;
//   return;
// }
monostate reader (Continuation<monostate> k_self,
                  Coroution<monostate, int>*f) {
  auto self_0 = [&](pair<int, Coroution<monostate, int>*> p) {
    cout << "counter " << p.first << endl;
    return monostate();};
  auto self_1 = [&](pair<int, Coroution<monostate, int>*> p) {
    yield<monostate, int>(self_0, p.second, monostate());
    return monostate();};
  auto self_2 = [&](pair<int, Coroution<monostate, int>*> p) {
    cout << "counter " << p.first << endl;
    return self_1(p);};
  return yield<monostate, int>(self_2, f, monostate());
}

// reader(coroutine(counter));
// return 0
int main() {
  auto self = [](auto f) {
    return reader([](auto x){
      return monostate ();}, f);
  };
  coroutine<monostate, int>(self, counter);
  return 0;
}
