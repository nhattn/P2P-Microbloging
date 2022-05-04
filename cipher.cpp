/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Trần Ngọc Nhật <https://github.com/nhattn>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cipher.h>

bool is_cipher(std::string message, std::string key) {
    if(key.empty()) return false;
    char ch = message[0] - key[0];
    if (ch == key[0]) return true;
    return false;
}

std::string cipher_encrypt(std::string message, std::string key) {
    if (key.length() <= 0) return message;
    std::string mcipher = key.substr(0,1) + message;
    int msgLen = mcipher.length(), keyLen = key.length(), i, j = 0;
    for(i = 0; i < msgLen; ++i) {
        if (j >= keyLen) j = 0;
        mcipher[i] = (mcipher[i] + key[j]);
    }
    return mcipher;
}

std::string cipher_decrypt(std::string message, std::string key) {
    if (key.length() <= 0) return message;
    if(!is_cipher(message, key)) return message;
    message = message.substr(1);
    int msgLen = message.length(), keyLen = key.length(), i, j = 0;
    for(i = 0; i < msgLen; ++i) {
        if (j >= keyLen) j = 0;
        message[i] = message[i] - key[j];
    }
    return message;
}
