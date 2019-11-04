# Implemeting a correcting rounded multi-dimensionnal dot product
# using upper precision and rounding-to-odd

import sollya
import random
import sys

S2 = sollya.SollyaObject(2)
sollya.settings.display = sollya.hexadecimal
sollya.settings.prec = 500

def round_to_odd(v, p):
    """ round the SollyaObject @p v to precision @p p using
        rounding-to-odd mode

        :param v: value to round
        :type v: SollyaObject (numerical value)
        :param p: targe precision
        :type p: SollyaObject (precision)
        :return: rounded value (using round-to-odd)
        :rtype: SollyaObject
    """
    r_up = sollya.round(v, p, sollya.RU)
    r_down = sollya.round(v, p, sollya.RD)
    # determining parity of r_up
    # if r_up can be expressed exactly on (p-1) bits it means
    # r_up's LSB is 0 => even mantissa
    r_up_even = (r_up == sollya.round(v, p - 1, sollya.RU))
    if r_up_even:
        return r_down
    else:
        return r_up


def dp4_cr(a_v, b_v):
    """ Correctly rounded 4D dot product

        :param a_v: left-hand-side vector
        :type a_v: list(SollyaObject)
        :param b_v: right-hand-side vector
        :type b_v: list(SollyaObject)
        :return: correctly rounded 4D dot-product
        :rtype: SollyaObject
    """
    prod_v = [a * b for a, b in zip(a_v, b_v)]
    prod_v.sort()
    acc = 0
    for p in prod_v:
        acc = round_to_odd(p + acc, 53)
    return sollya.round(acc, sollya.binary32, sollya.RN)

def exact_dp4(a_v, b_v):
    """ Exact version (formal) of 4D dot-product

        :param a_v: left-hand-side vector
        :type a_v: list(SollyaObject)
        :param b_v: right-hand-side vector
        :type b_v: list(SollyaObject)
        :return: exact 4D dot-product
        :rtype: SollyaObject (value or expression)
    """
    prod_v = [a * b for a, b in zip(a_v, b_v)]
    acc = 0
    for p in prod_v:
        acc = p + acc
    return acc

def random_fp32_value():
    """ Dummy generation for random binary32/fp32 value

        :return: random fp32 number
        :rtype: SollyaObject
    """
    real_value = random.randrange(-1, 2, 2) * S2**random.randrange(-126, 127) * random.random()
    return sollya.round(real_value, sollya.binary32, sollya.RN)

def random_4d_vector():
    """ Random 4D vector

        :return: random 4D vector
        :rtype: list(SollyaObject)
    """
    vector = [random_fp32_value() for i in range(4)]
    return vector

TEST_NUM = 10000000

for i in range(TEST_NUM):
    a_v, b_v = random_4d_vector(), random_4d_vector()
    exact_result = exact_dp4(a_v, b_v)
    cr_result = sollya.round(exact_result, sollya.binary32, sollya.RN)
    new_method_result = dp4_cr(a_v, b_v)
    if cr_result != new_method_result:
        print("{} vs {} mismatch".format(cr_result, new_method_result))
        sys.exit(1)

