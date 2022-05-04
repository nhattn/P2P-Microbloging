/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Dave Nash https://github.com/knasher
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

#include <ctime>
#include <block.h>
#include <sha256.h>

Block::Block(int index, const std::string parent, const std::string hash, const std::string nonce, const std::string data) {
    _index = index;
    _parent = parent;
    _hash = hash;
    _nonce = nonce;
    _data = data;
    _time = std::time(nullptr);
}

std::string Block::previousHash(void) {
    return _parent;
}
std::string Block::getHash(void) {
    return _hash;
}
int Block::getIndex(void) {
    return _index;
}
std::string Block::getData(void) {
    return _data;
}
void Block::toString(void) {
    try {
        auto data = json::JSON::Load(_data);
        std::string raw = data.dump();
        printf("Block: %d\nHash: %s\nParent: %s\n%s", _index, _hash.c_str(), _parent.c_str(), raw.c_str());
    } catch(...) {
        printf("Invalid block");
    }
}
json::JSON Block::toJSON(void) {
    try {
        json::JSON j;
        j["index"] = _index;
        j["hash"] = _hash;
        j["parent"] = _parent;
        j["nonce"] = _nonce;
        if (_data.size() > 0) {
            j["data"] = json::JSON::Load(_data);
        } else {
            j["data"] = json::JSON();
        }
        j["created"] = _time;
        return j;
    } catch(...) {
        return json::JSON();
    }
}