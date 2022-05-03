/*
 * Copyright 2022 Trần Ngọc Nhật <https://github.com/nhattn>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <ctime>
#include <json.h>
#include <blockchain.h>

int main(int argc, char **argv)
{
    Blockchain bChain = Blockchain();

    std::cout << "Mining block 1..." << std::endl;

    auto first_block = json::Object();
    first_block["author"] = json::Object();
    first_block["author"]["fullname"] = "Trần Ngọc Nhật";
    first_block["author"]["username"] = "nhattn";
    first_block["content"] = "Ngày hôm qua, mưa có tiếng tí tách của luyến thương và tiếc nuối, có mơ hồ cô gái xa xôi, ta ngồi lắng nghe đôi ba câu chuyện bâng quơ của chốn thị thành, xem vài bức hình của một góc quán yên bình nào đó...";
    first_block["created"] = std::time(NULL);

    bChain.AddBlock(Block(1, first_block.stringify()));

    std::cout << "Mining block 2..." << std::endl;
    
    auto second_block = json::Object();
    second_block["author"] = json::Object();
    second_block["author"]["fullname"] = "Nguyễn Hoàng Phương Thuy";
    second_block["author"]["username"] = "thuyhp";
    second_block["content"] = "Giữa đời hối hả với muôn ngàn những mối quan hệ, muôn nỗi lo âu. Em là ai? Anh không biết! Cuộc đời này quá rộng cho ta có thể gặp nhau, hoặc, có thể ta đã bên nhau từ những phút chia lìa ngày ấy.";
    second_block["created"] = std::time(NULL);
    
    bChain.AddBlock(Block(2, second_block.stringify()));

    std::cout << "Mining block 3..." << std::endl;
    
    auto third_block = json::Object();
    third_block["author"] = json::Object();
    third_block["author"]["fullname"] = "Nguyễn Ngọc Uyển Nhi";
    third_block["author"]["username"] = "uyennhi";
    third_block["content"] = "Chủ nhật ngày mưa, chẳng có gì thú hơn là được ngồi trong căn phòng quen thuộc với ly chè xanh nóng hổi, nhâm nhi vài điếu richmond thơm lừng và thưởng thức phin cà phê đương hòa quyện theo những giai điệu tình ca ngọt ngào của Như Quỳnh.";
    third_block["created"] = std::time(NULL);
    
    bChain.AddBlock(Block(3, third_block.stringify()));

    return 0;
}
