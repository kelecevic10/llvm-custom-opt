extern "C" int C(int x, int y) {
  // y se nigde ne koristi
  return x + 10;
}

extern "C" int B(int a, int b, int c) {
  // c se prosleđuje dalje u C, ali se ni tamo ne koristi
  return C(a, c) * 2;
}

extern "C" int A(int p, int q, int r, int s) {
  // r i s se prosleđuju u B, ali su mrtvi
  return B(p, q, r);
}

extern "C" int main() {
  return A(1, 2, 3, 4);
}
