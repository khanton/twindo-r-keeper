#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <sstream>

#include "farcards.h"
#include "TwindoProto.h"
#include "HttpProto.h"

TwindoProto proto;

extern "C" FARCARDS_API int TransactionsEx(
    Word count,
    transaction_t * list[],
    char* inp_buf,
    DWORD inp_len,
    Word inp_kind,
    char* out_buf,
    DWORD * out_len,
    Word * out_kind
) {
    proto.trace(std::string{ inp_buf, inp_len });

    Int64 card = 0;
    unsigned long long tr_sum = 0;

    for (Word i = 0; i < count; i++) {
        transaction_t *tr = list[i];

        if (tr->payment_type != 0) {
            continue;
        }

        if (card == 0) {
            card = tr->card;
            tr_sum += -tr->summa;
        }
        else if (card == tr->card) {
            tr_sum += -tr->summa;
        }
        else {
            proto.error("Invalid card num in transaction!");
        }
    }

    return 0;
}

extern "C" FARCARDS_API void Init() {
    proto.init();
}

extern "C" FARCARDS_API void Done() {
}

void convertToMB(const std::wstring& from, const size_t len, char* buffer) {

    int buf_len = WideCharToMultiByte(1251, 0, from.c_str(), -1, NULL, 0, NULL, NULL);
    
    char* inside = new char[buf_len];
    memset(inside, '\x0', buf_len);
    
    WideCharToMultiByte(1251, 0, from.c_str(), -1, inside, buf_len, NULL, NULL);

    strncpy_s(buffer, len, inside, _TRUNCATE);

    delete[] inside;
}   

extern "C" FARCARDS_API int GetCardInfoEx(
    Int64 card,
    DWORD restaurant,
    DWORD unit_no,
    card_info_t * info,
    char* inp_buf,
    DWORD inp_len,
    Word inp_kind,
    char* out_buf,
    DWORD * out_len,
    Word * out_kind
) {
    
    if (info->len != sizeof(card_info_t))
    {
        proto.error("Invalid info struct size");
        return 1;
    }

    double check_sum = 0;
    if (inp_len > 0) {

        proto.trace(std::string{ inp_buf, inp_len });

        namespace pt = boost::property_tree;
        std::istringstream istr(std::string{ inp_buf, inp_len });
        try {
            pt::ptree tree;
            pt::read_xml(istr, tree);
            BOOST_FOREACH(pt::ptree::value_type & v, tree.get_child("CHECK.CHECKDATA.CHECKLINES")) {
                if (v.first == "LINE") {
                    check_sum += boost::lexical_cast<double>(v.second.get<std::string>("<xmlattr>.sum"));
                }
            }
        }
        catch (const boost::property_tree::xml_parser::xml_parser_error) {
            proto.error("Invalid check XML");
            return 1;
        }
        catch (const boost::property_tree::ptree_error) {
            // if check empty
        };
    }

    if (check_sum == 0) {
        proto.error("Check sum zero!");
    }

    std::string code;
    std::string name;

    if (!proto.setUser(card, code)) {
        return 1;
    }
  
    if (!proto.get(code, name)) {
        return 1;
    }

    unsigned long balance = 0;
    unsigned long allowed = 0;
    unsigned long allowed_in_currency = 0;

    if (check_sum > 0) {
        if (!proto.balance(code, check_sum, balance, allowed, allowed_in_currency)) {
            return 1;
        }
    }

    convertToMB(HttpProto::utf8_to_wstring(name), sizeof(info->owner_name), info->owner_name);

    info->avail_for_account = allowed_in_currency * 100;

    std::wstringstream screen_info;
    screen_info << L"Для оплаты:" << allowed_in_currency << L" Всего:" << balance << L" лайков";
    convertToMB(screen_info.str(), sizeof(info->card_screen_info), info->card_screen_info);

/*    std::wstringstream card_info;
    screen_info << L"Оплата по счету " << balance << L" лайков";
    convertToMB(card_info.str(), sizeof(info->card_info), info->card_info);

    std::wstringstream print_info;
    screen_info << L"Печать " << balance << L" лайков";
    convertToMB(print_info.str(), sizeof(info->card_printer_info), info->card_printer_info);
*/

//    info->account_number = card;

    return 0;
};
