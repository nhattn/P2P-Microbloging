#include <iostream>
#include <string>
#include <sha256.h>

int main(void) {
    std::string str = "The above copyright notice and this permission notice shall be included";
    std::string hash_1 = sha256(str);
    const char *s = "The above copyright notice and this permission notice shall be included";
    std::string hash_2 = sha256(s);
    std::cout << "String: " << str << std::endl;
    std::cout << "Hash: " << hash_1 << std::endl;
    std::cout << "String: " << s << std::endl;
    std::cout << "Hash: " << hash_2 << std::endl;
    return 0;
}
