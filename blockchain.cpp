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

#include <blockchain.h>
#include <sha256.h>
#include <memory>
#include <stdexcept>
#include <cipher.h>
#include <base64.h>

std::pair<std::string,std::string> findHash(int index, std::string parent, std::string data) {
    std::string header = std::to_string(index) + parent + sha256(data);
    unsigned int nonce;
    for (nonce = 0; nonce < 100000; nonce++ ) {
        std::string blockHash = sha256(header + std::to_string(nonce));
        if (blockHash.substr(0,2) == "00"){
            return std::make_pair(blockHash,std::to_string(nonce));
        }
    }
    return std::make_pair("fail","fail");
}

BlockChain::BlockChain(int genesis = 1){
    if (genesis == 0) {
        std::string _data = "";
        auto hash_nonce_pair = findHash(0,std::string("00000000000000"), _data);
        blockchain.push_back(std::make_unique<Block>(0,std::string("00000000000000"),hash_nonce_pair.first,hash_nonce_pair.second,_data));
    }
}
Block BlockChain::getBlock(std::string hash) {
    if (hash.size() != 14 && hash.size() != 64) throw std::invalid_argument("Hash does not exist.");
    for ( size_t i = 0; i < blockchain.size(); i++ ){
        std::string _hash = blockchain[i]->getHash();
        if(hash.size() == 14) {
            _hash = _hash.substr(0, 14);
        }
        if (_hash == hash) {
            return *(blockchain[i]);
        }
    }
    throw std::invalid_argument("Hash does not exist.");
}
Block BlockChain::getBlock(int index) {
    for ( size_t i = 0; i < blockchain.size(); i++ ){
        if (blockchain[i]->getIndex() == index) {
            return *(blockchain[i]);
        }
    }
    throw std::invalid_argument("Index does not exist.");
}
int BlockChain::getNumOfBlocks(void) {
    return (int)blockchain.size();
}
int BlockChain::addBlock(int index, std::string parent, std::string hash, std::string nonce, std::string data) {
    std::string header = std::to_string(index) + parent + sha256(data) + nonce;
    if ( (!sha256(header).compare(hash)) && (hash.substr(0,2) == "00" ) && (index == (int)blockchain.size())) {
        blockchain.push_back(std::make_unique<Block>(index,parent,hash,nonce,data));
        return 1;
    }
    return 0;
}
std::string BlockChain::getLatestBlockHash(void) {
    if (blockchain.size() == 0) throw std::invalid_argument("Blockchain is empty.");
    return blockchain[blockchain.size()-1]->getHash();
}
std::string BlockChain::toJSON() {
    json::JSON j;
    j["length"] = blockchain.size();
    for (size_t i = 0; i < blockchain.size(); i++){
        j["data"][blockchain[i]->getIndex()] = blockchain[i]->toJSON();
    }
    return j.dump();
}
int BlockChain::replaceChain(json::JSON chain) {
    while (blockchain.size() > 1){
        blockchain.pop_back();
    }
    for (int a = 1; a < chain["length"].ToInt(); a++ ){
        auto block = chain["data"][a];
        std::string raw = cipher_encrypt(block["data"].stringify(), SALT_CIPHER);
        addBlock(block["index"].ToInt(), block["parent"].ToString(), block["hash"].ToString(), block["nonce"].ToString(), base64_url_encode(raw));
    }
    return 1;
}
