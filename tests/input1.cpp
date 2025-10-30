extern "C" int add(int a, int b) {
  // koristimo samo 'a'; 'b' je mrtav argument
  return a;
}

extern "C" int main() {
  return add(1, 2);
}
