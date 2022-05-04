#include <iostream>
#include <json.h>
#include <blockchain.h>

std::string tweet(std::string, std::string, std::string);

int main(void) {
    BlockChain bc(0);
    std::string data;
    data = tweet("Trần Ngọc Nhật", "nhattn", "Ngày hôm qua, mưa có tiếng tí tách của luyến thương và tiếc nuối, có mơ hồ cô gái xa xôi, ta ngồi lắng nghe đôi ba câu chuyện bâng quơ của chốn thị thành, xem vài bức hình của một góc quán yên bình nào đó...");
    auto pair = findHash(bc.getNumOfBlocks(),bc.getLatestBlockHash(),data);
    bc.addBlock(bc.getNumOfBlocks(),bc.getLatestBlockHash(),pair.first,pair.second, data);
    data = tweet("Nguyễn Hoàng Phương Thuy", "thuyhp", "Giữa đời hối hả với muôn ngàn những mối quan hệ, muôn nỗi lo âu. Em là ai? Anh không biết! Cuộc đời này quá rộng cho ta có thể gặp nhau, hoặc, có thể ta đã bên nhau từ những phút chia lìa ngày ấy.");
    pair = findHash(bc.getNumOfBlocks(),bc.getLatestBlockHash(),data);
    bc.addBlock(bc.getNumOfBlocks(),bc.getLatestBlockHash(),pair.first,pair.second, data);
    data = tweet("Nguyễn Ngọc Uyển Nhi", "uyennhi", "Chủ nhật ngày mưa, chẳng có gì thú hơn là được ngồi trong căn phòng quen thuộc với ly chè xanh nóng hổi, nhâm nhi vài điếu richmond thơm lừng và thưởng thức phin cà phê đương hòa quyện theo những giai điệu tình ca ngọt ngào của Như Quỳnh.");
    pair = findHash(bc.getNumOfBlocks(),bc.getLatestBlockHash(),data);
    bc.addBlock(bc.getNumOfBlocks(),bc.getLatestBlockHash(),pair.first,pair.second, data);

    std::cout << bc.toJSON() << std::endl;

    return 0;
}

std::string tweet(std::string fullname, std::string username, std::string content) {
    auto obj = json::JSON();
    obj["author"]["fullname"] = fullname;
    obj["author"]["username"] = username;
    obj["content"] = content;
    return obj.stringify();
}
