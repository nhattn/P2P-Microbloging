/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Timothy Ko https://github.com/tko22
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

#ifndef _BLOCKCHAIN_H
#define _BLOCKCHAIN_H

#include <cstdint>
#include <vector>
#include <json.h>
#include <utility>
#include <memory>
#include <block.h>
#include <cstddef>
#include <type_traits>
/* https://stackoverflow.com/a/17902439 */
namespace std {
    template<class T> struct _Unique_if {
        typedef unique_ptr<T> _Single_object;
    };

    template<class T> struct _Unique_if<T[]> {
        typedef unique_ptr<T[]> _Unknown_bound;
    };

    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };

    template<class T, class... Args>
        typename _Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
            return unique_ptr<T>(new T(std::forward<Args>(args)...));
        }

    template<class T>
        typename _Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
            typedef typename remove_extent<T>::type U;
            return unique_ptr<T>(new U[n]());
        }

    template<class T, class... Args>
        typename _Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;
}

class BlockChain {
    public:
        BlockChain(int);
        Block getBlock(std::string);
        Block getBlock(int);
        int getNumOfBlocks(void);
        int addBlock(int, std::string, std::string, std::string, std::string);
        std::string getLatestBlockHash(void);
        std::string toJSON(void);
        std::string getHash(void);
        int replaceChain(json::JSON);
    private:
        std::vector<std::unique_ptr<Block> > blockchain;
};

std::pair<std::string,std::string> findHash(int, std::string, std::string);

#endif // _BLOCKCHAIN_H