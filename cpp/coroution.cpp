#include <functional>
#include <utility>
#include <variant>
#include <iostream>
#include <type_traits>
using namespace std;

//debug
template <class T>
constexpr
std::string_view
type_name()
{
  using namespace std;
#ifdef __clang__
  string_view p = __PRETTY_FUNCTION__;
  return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
  string_view p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
  return string_view(p.data() + 36, p.size() - 36 - 1);
#  else
  return string_view(p.data() + 49, p.find(';', 49) - 49);
#  endif
#elif defined(_MSC_VER)
  string_view p = __FUNCSIG__;
  return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
}

template <typename A>
using Continuation = function<monostate(A)>;

template <typename A, typename B>
class Coroution {
public:
  Continuation<pair<A, Coroution<B, A>*>> cont;
  Coroution(Continuation<pair<A, Coroution<B, A>*>> cont) {
    cout << "new a Coroution with type Continuation<" << type_name<decltype(cont)>() << ">" << endl;
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
  cout << "yield" << endl;
  // debug
  if(is_same<A,monostate>::value) cout << "reader yield ()" << endl;
  if(is_same<A,int>::value) cout << "counter yield int" << endl;
  std::cout << "call function with type: " << type_name<decltype(cr->cont)>() << endl;
  cr->cont(make_pair(a, new Coroution<B, A>(k_self)));
  return monostate();
}

template <typename A, typename B>
monostate coroutine(Continuation<Coroution<A, B>*> k_self,
               function<monostate(Continuation<Coroution<A, B>*>, pair<A, Coroution<B, A>*>)> f) {
  cout << "coroutine start" << endl;
  k_self(new Coroution<A, B>([&](auto x) {
    cout << "before call counter" << endl;
    return f(k_self, x);}));
  return monostate();
}

int generate(int x) {
  cout << "generate:" << x << endl;
  return x;
}

monostate counter (Continuation<Coroution<monostate, int>*> k_self,
              pair<monostate, Coroution<int, monostate>*> client) {
  auto self_0 = [&](pair<monostate, Coroution<int, monostate>*> p) {
    cout << "before counter end" << endl;
    return k_self(kill<int>());};
  auto self_1 = [&](pair<monostate, Coroution<int, monostate>*> p) {
    cout << "before counter generate 1" << endl;
    return yield<int, monostate>(self_0, p.second, generate(1));};
  cout << "before counter generate 0" << endl;
  return yield<int, monostate>(self_1, client.second, generate(0));
}

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

int main() {
  auto self = [](auto f) {
    cout << "before call reader" << endl;
    return reader([](auto x){
      cout << "end" << endl;
      return monostate ();}, f);
  };
  coroutine<monostate, int>(self, counter);
  return 0;
}
