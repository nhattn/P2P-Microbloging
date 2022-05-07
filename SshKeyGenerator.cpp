#include <iostream>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

struct public_key {
    int r;
    int m;
};

struct private_key {
    int s;
    int p;
    int q;
};

struct RSA {
    int p;
    int q;
    int phi;
    int m;
    int r;
    int s;
};

int gcd(int a, int b);
int mod_inverse(int a, int b);
int lcm(int a, int b);
int is_prime(int numb);
int calc_phi(int a, int b);

struct RSA rsa_generator(void);
struct private_key rsa_private_key(const struct RSA);
struct public_key rsa_public_key(const struct RSA);

int rsa_valid(const struct RSA);

int main(int argc, char **argv) {
    struct RSA rs = rsa_generator();
    if (!rsa_valid(rs)) {
        int gc = gcd(rs.r, rs.phi);
        std::cout << "Not OK" << std::endl;
        std::cout << "{m:"<< rs.m << ", r: " << rs.r << ",phi: " << rs.phi << ",gcd: " << gc << "}" << std::endl;
    }
    std::cout << "Modulus is: " << rs.m << std::endl;
    std::cout << "Phi(n) is: " << rs.phi << std::endl;
    std::cout << "Toitient is: " << rs.r << std::endl;
    std::cout << "Public key is: (" << rs.m << "," << rs.r << ")" << std::endl;
    std::cout << "Private key is: " << rs.s << std::endl;
    return 0;
}

int gcd(int a, int b) {
    if (a == 0 || b == 0) {
        return 0;
    }
    if (a == b) return a;
    if (a > b) return gcd(a - b, b);
    return gcd(a, b - a);
}
int mod_inverse(int a, int m) {
    int m0 = m;
    int y = 0, x = 1;
    if (m == 1) return 0;
    while (a > 1) {
        int q = a / m;
        int t = m;
        m = a % m, a = t;
        t = y;
        y = x - q * y;
        x = t;
    }
    if (x < 0) x += m0;
    return x;
}
int lcm(int a, int b) {
    return (a * b) / gcd(a, b);
}
struct private_key rsa_private_key(const struct RSA rs) {
    private_key pri = {
        .s = rs.s,
        .p = rs.p,
        .q = rs.q
    };
    return pri;
}
struct public_key rsa_public_key(const struct RSA rs) {
    public_key pub = {
        .r = rs.r,
        .m = rs.m
    };
    return pub;
}
int is_prime(int numb) {
    for (int i = 2; i <= sqrt(numb); i++) {
        if (numb % i == 0) return 0;
    }
    return 1;
}
int calc_phi(int a, int b) {
    if(!is_prime(a) || !is_prime(b)) {
        return 0;
    }
    return (a - 1) * (b - 1);
}
struct RSA rsa_generator(void) {
    int P, Q;
    int i = 0;
    srand(time(NULL));
    int rnd;
    while (i < 2) {
        rnd = rand() % 40 + 3;
        if(is_prime(rnd)) {
            if (i == 0) {
                P = rnd;
                i++;
            } else {
                if (rnd != P) {
                    Q = rnd;
                    i++;
                }
            }
        }
    }
    int l = lcm(P - 1, Q - 1);
    std::vector<int> t;
    for(i = 3; i < l; i++) {
        if(gcd(i, l) == 1) {
            t.push_back(i);
        }
    }
    rnd = rand() % t.size();
    int r = t[rnd];
    int s = mod_inverse(r, l);
    struct RSA rs = {
        .p = P,
        .q = Q,
        .phi = calc_phi( P, Q ),
        .m = P * Q,
        .r = r,
        .s = s
    };
    return rs;
}
int rsa_valid(const struct RSA rs) {
    if ((rs.r < rs.m) && (rs.r > 1) && gcd(rs.r, rs.phi) != 1) {
        return 0;
    }
    return 1;
}
