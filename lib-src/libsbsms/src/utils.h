// -*- mode: c++ -*-
#ifndef UTILS_H
#define UTILS_H

#include "real.h"
#include "config.h"
#include "sbsms.h"
#if defined(ENABLE_SSE) && !defined(APPLE_PPC) && !defined(_M_ARM64)
#include <emmintrin.h>
#endif

namespace _sbsms_ {

#define ONEOVERTWOPI 0.15915494309189533576888376337251f
#define PI 3.1415926535897932384626433832795f
#define TWOPI 6.28318530717958647692528676655900576f

#if defined(ENABLE_SSE) && !defined(APPLE_PPC) && !defined(_M_ARM64)
// SSE-Versionen von c2even/c2odd: zwei komplexe Werte pro Schritt.
// eo[N-k] laeuft rueckwaerts, darum werden eo[Nk-1], eo[Nk] geladen und die
// Paare getauscht. Ergebnis bitgleich zur skalaren Fassung: a - b wird als
// a + (-b) gerechnet (Vorzeichenbit per xor), das ist in IEEE 754 exakt gleich.
// Voraussetzung: eo und even/odd ueberlappen nicht (in sbsms immer erfuellt:
// grain::x und die SMS-Arrays x00/x01/x10/x11/x2 sind eigene Speicherbloecke).
inline void c2even(audio *eo, audio *even, int N)
{
  int Nover2 = N/2;
  even[0][0] = eo[0][0];
  even[0][1] = 0.0f;
  const __m128 half = _mm_set1_ps(0.5f);
  const __m128 negIm = _mm_castsi128_ps(_mm_set_epi32(0x80000000, 0, 0x80000000, 0));
  int k = 1;
  for(; k+1<=Nover2; k+=2) {
    int Nk = N-k;
    __m128 a = _mm_loadu_ps(&eo[k][0]);                 // re k, im k, re k+1, im k+1
    __m128 b = _mm_loadu_ps(&eo[Nk-1][0]);              // re Nk-1, im Nk-1, re Nk, im Nk
    b = _mm_shuffle_ps(b, b, _MM_SHUFFLE(1,0,3,2));     // re Nk, im Nk, re Nk-1, im Nk-1
    _mm_storeu_ps(&even[k][0], _mm_mul_ps(half, _mm_add_ps(a, _mm_xor_ps(b, negIm))));
  }
  for(; k<=Nover2; k++) {
    int Nk = N-k;
    even[k][0] = 0.5f*(eo[k][0] + eo[Nk][0]);
    even[k][1] = 0.5f*(eo[k][1] - eo[Nk][1]);
  }
}

inline void c2odd(audio *eo, audio *odd, int N)
{
  int Nover2 = N/2;
  odd[0][0] = eo[0][1];
  odd[0][1] = 0.0f;
  const __m128 half = _mm_set1_ps(0.5f);
  const __m128 negRe = _mm_castsi128_ps(_mm_set_epi32(0x80000000, 0, 0x80000000, 0));
  int k = 1;
  for(; k+1<=Nover2; k+=2) {
    int Nk = N-k;
    __m128 a = _mm_loadu_ps(&eo[k][0]);
    a = _mm_shuffle_ps(a, a, _MM_SHUFFLE(2,3,0,1));     // im k, re k, im k+1, re k+1
    __m128 b = _mm_loadu_ps(&eo[Nk-1][0]);
    b = _mm_shuffle_ps(b, b, _MM_SHUFFLE(0,1,2,3));     // im Nk, re Nk, im Nk-1, re Nk-1
    // [im k + im Nk, re Nk - re k] = b + a mit negiertem re k
    _mm_storeu_ps(&odd[k][0], _mm_mul_ps(half, _mm_add_ps(_mm_xor_ps(a, negRe), b)));
  }
  for(; k<=Nover2; k++) {
    int Nk = N-k;
    odd[k][0] = 0.5f*(eo[k][1] + eo[Nk][1]);
    odd[k][1] = 0.5f*(eo[Nk][0] - eo[k][0]);
  }
}
#else
inline void c2even(audio *eo, audio *even, int N)
{
  int Nover2 = N/2;
  even[0][0] = eo[0][0];
  even[0][1] = 0.0f;
  for(int k=1;k<=Nover2;k++) {
    int Nk = N-k;
    even[k][0] = 0.5f*(eo[k][0] + eo[Nk][0]);
    even[k][1] = 0.5f*(eo[k][1] - eo[Nk][1]);
  }
}

inline void c2odd(audio *eo, audio *odd, int N)
{
  int Nover2 = N/2;
  odd[0][0] = eo[0][1];
  odd[0][1] = 0.0f;
  for(int k=1;k<=Nover2;k++) {
    int Nk = N-k;
    odd[k][0] = 0.5f*(eo[k][1] + eo[Nk][1]);
    odd[k][1] = 0.5f*(eo[Nk][0] - eo[k][0]);
  }
}

#endif

inline float canonPI(float ph) 
{
  ph -= TWOPI * lrintf(ph * ONEOVERTWOPI);
  if(ph < -PI) ph += TWOPI;
  else if(ph >= PI) ph -= TWOPI;
  return ph;
}

inline float canon2PI(float ph) 
{
  ph -= TWOPI * lrintf(ph * ONEOVERTWOPI);
  if(ph < 0.0f) ph += TWOPI;
  if(ph >= TWOPI) ph -= TWOPI;
  return ph;
}

inline float square(float x)
{ 
  return x*x;
}

inline float norm2(t_fft x)
{
  return square(x[0]) + square(x[1]);
}

}

#endif
