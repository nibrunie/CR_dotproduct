#include <cmath>
#include <cstdint>

#include <iostream>
#include <cstdio>


/** Generic implementation of 2-sum algorithm following
 *  description in "Handbook of Floating-point Arithmetic, 2nd edition", Muller et al.
 *  Algorithm 4.4, Section 4.3.2 p 108
 */
template<typename T> void TwoSum(T &s, T& t, T a, T b)
{
    s = a + b;
    T ap = s - b;
    T bp = s - ap;
    T delta_a = a - ap;
    T delta_b = b - bp;
    t = delta_a + delta_b;
}

typedef union {
    uint64_t u;
    double d;
} ud_t;


/** Double precision addition rounded to odd */
double fp64_add_ro(double x, double y)
{
    ud_t s, t;
    TwoSum<double>(s.d, t.d, x, y);
    if (t.d != 0 && !(s.u & 0x1ull)) {
        // rounding occured and s's mantissa is even
        if (copysign(1.0, t.d) != copysign(1.0, s.d)) {
            // t and s signs differ
            s.u -= 0x1ull;
        } else {
            // t and s signs match
            // forcing s's lsb to 1
            s.u |= 0x1ull;
        }
    }
    return s.d;
}


/** 4-element single precision vector */
typedef float v4sf[4];

/** Addition of 2 vectors of 4 floating-point element
 *  using round-to-odd */
double fp64_v4_add_ro(v4sf vx, v4sf vy)
{
    double acc[4] = {0};
    double result = 0.0;

    // product computation
    for (int i = 0; i < 4; ++i) {
        acc[i] = vx[i] * (double) vy[i];
    }

    // sorting: dummy quadratic sort
    for (int i = 0; i < 4; ++i) {
        unsigned min_index = 0;
        for (int j = i + 1; j < 4; j++) {
            if (acc[j] < acc[min_index]) min_index = j;
        }
        double tmp = acc[min_index];
        acc[min_index] = acc[i];
        acc[i] = tmp;
    }

    // accumulation
    for (int i = 1; i < 4; ++i) {
        double lhs = 0.0, rhs = 0.0;
        // if predicate is not verified, appliying 2-sum
        if (fabs(acc[i]) < 2 * fabs(result)) {
            TwoSum<double>(lhs, rhs, result, acc[i]);
        } else {
            lhs = acc[i];
            rhs = result;
        }
        result = fp64_add_ro(lhs, rhs);
    }

    return result;
}


/** Test vectors for fp64_add_ro generated using
 *  python3 cr_fp_dot_emulation.py`
 */
double fp64_add_ro_test[][3] = {
    {-0x1.c072b4d172d26p693, -0x1.b123968039f7ep-227, -0x1.c072b4d172d27p693},
    {0x1.596f646f30a06p498, -0x1.1062e38c42cap-394, 0x1.596f646f30a05p498},
    {0x1.c25906b6f8658p809, -0x1.d08fe98d73a95p-842, 0x1.c25906b6f8657p809},
    {-0x1.6af2199dd8dbp-766, -0x1.d4dce14c24d03p928, -0x1.d4dce14c24d03p928},
    {0x1.a4fcfcec9876ap771, 0x1.aa2551b12ee06p-492, 0x1.a4fcfcec9876bp771},
    {0x1.5341dba487704p-989, -0x1.c319c0c64fb43p-555, -0x1.c319c0c64fb43p-555},
    {0x1.ab8b8d0302c68p-606, -0x1.63c5ed135dda6p259, -0x1.63c5ed135dda5p259},
    {0x1.f4c2eee43ab4ap972, 0x1.9dce20bfdb5b3p-191, 0x1.f4c2eee43ab4bp972},
    {0x1.2903f8119575ep10, 0x1.ad2bd567a4d5ep-466, 0x1.2903f8119575fp10},
    {-0x1.54b5c1c219636p880, -0x1.3d04556f80401p200, -0x1.54b5c1c219637p880},
    {0x1.f3295ad809757p0, -0x1.71fb160fa5296p-1, 0x1.3a2bcfd036e0cp0},
    {-0x1.83f96dd35f983p2, 0x1.4e37f1a1fb24dp-1, -0x1.5a326f9f20339p2},
    {0x1.77b85c2e904f8p2, 0x1.1d7c6c8441a35p-2, 0x1.899022f6d469bp2},
    {-0x1.68adfd98e1869p-1, 0x1.4009f18cf2fafp1, 0x1.cbbce44d75329p0},
    {-0x1.ccf1af22a61a1p0, -0x1.290c8bdc977c1p0, -0x1.7aff1d7f9ecb1p1},
    {-0x1.1c13ea7e15c1ep1, 0x1.e3c1db8f1a512p1, 0x1.8f5be222091e8p0},
    {0x1.0fcc3fa076876p-1, -0x1.e960e96a76eb4p1, -0x1.a56dd98259497p1},
    {-0x1.5408db57d107p-1, -0x1.0de23aaa6bc4ap-1, -0x1.30f58b011e65dp0},
    {-0x1.b2981f3f7b97p-2, -0x1.c3d4c018c3f3ap0, -0x1.183d63f4516cbp1},
    {-0x1.bb29cba065c67p1, 0x1.969ca5f56915cp-1, -0x1.5582a2230b81p1},
};
int main(void) {
	unsigned error_count = 0;
	int i;
	std::cout << "testing fp64_add_ro" << std::endl;
	for (i = 0; i < sizeof(fp64_add_ro_test) / (sizeof(double) * 3); ++i) {
		double lhs = fp64_add_ro_test[i][0];	
		double rhs = fp64_add_ro_test[i][1];	
		double expected = fp64_add_ro_test[i][2];	
		double result = fp64_add_ro(lhs, rhs);
		if (result != expected) {
			printf("error encountered %a + %a =[RO] %a vs %a expected.\n", lhs, rhs, result, expected);
			error_count++;
		}
	}
	if (error_count) {
		std::cerr << "test failed ! (" << error_count << "/" << i << " error(s))" << std::endl;
		return -1;
	}

	std::cout << "test succeed !" << std::endl;
	return 0;
}
