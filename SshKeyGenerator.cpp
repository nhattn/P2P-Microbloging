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

int modular(int c, int d, int n);
std::string sha_encrypt(const std::string, const struct public_key);
std::string sha_decrypt(const std::string, const struct private_key);

int main(int argc, char **argv) {
    //struct RSA rs = rsa_generator();
    struct RSA rs = {
        .p = 13,
        .q = 7,
        .phi = 72,
        .m = 91,
        .r = 5,
        .s = 5
    };
    if (!rsa_valid(rs)) {
        int gc = gcd(rs.r, rs.phi);
        std::cout << "Not OK" << std::endl;
        std::cout << "{m:"<< rs.m << ", r: " << rs.r << ",phi: " << rs.phi << ",gcd: " << gc << "}" << std::endl;
    }
    std::cout << "(P, Q): (" << rs.p << "," << rs.q << ")" << std::endl;
    std::cout << "Modulus is: " << rs.m << std::endl;
    std::cout << "Phi(n) is: " << rs.phi << std::endl;
    std::cout << "Toitient is: " << rs.r << std::endl;
    std::cout << "Public key is: (" << rs.m << "," << rs.r << ")" << std::endl;
    std::cout << "Private key is: " << rs.s << std::endl;
    
    std::cout << "struct RSA rs = {" << std::endl;
    std::cout << "    .p = " << rs.p << "," << std::endl;
    std::cout << "    .q = " << rs.q << "," << std::endl;
    std::cout << "    .phi = " << rs.phi << "," << std::endl;
    std::cout << "    .m = " << rs.m << "," << std::endl;
    std::cout << "    .r = " << rs.r << "," << std::endl;
    std::cout << "    .s = " << rs.s << "" << std::endl;
    std::cout << "};" << std::endl;

    struct public_key pub = rsa_public_key(rs);
    struct private_key pri = rsa_private_key(rs);

    std::string text = "I am using template function and integer constraint on it.";
    std::cout << "Text is: \n" << text << std::endl;
    std::string encrypt = sha_encrypt(text, pub);
    std::cout << "Encrypted is:\n[" << encrypt << "]" << std::endl;
    std::cout << "Decrypted is:\n[" << sha_decrypt(encrypt, pri) << "]" << std::endl;
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
    for (int i = 2; i <= numb / 2; i++) {
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
    int j = rand() % t.size();
    int r = t[j];
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
int modular(int c, int d, int n) {
    int v = 1;
    while( d > 0 ) {
        v *= c;
        v %= n;
        d--;
    }
    return v;
}
std::string sha_encrypt(const std::string s, const struct public_key rs) {
    std::vector<uint8_t> ret;
    for (unsigned int i = 0; i < s.size(); i++) {
        int e = modular(s.at(i) - 97, rs.r, rs.m);
        ret.push_back((char)e);
    }
    return std::string(ret.begin(), ret.end());
}
std::string sha_decrypt(const std::string s, const struct private_key rs) {
    std::vector<uint8_t> ret;
    int m = rs.p * rs.q;
    for (unsigned int i = 0; i < s.size(); i++) {
        int d = 97 + modular(s.at(i), rs.s, m);
        ret.push_back((char)d);
    }
    return std::string(ret.begin(), ret.end());
}
