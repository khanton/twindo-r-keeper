#pragma once

#include <windows.h>

#ifdef TWINDODLL_EXPORTS
#define FARCARDS_API __declspec(dllexport)
#else
#define FARCARDS_API __declspec(dllimport)
#endif

typedef unsigned __int16 Word;
//typedef unsigned __int32 DWORD;
typedef __int64 Int64;
typedef unsigned char Byte;


#pragma pack(push,1)
typedef struct {
    Word len;   // 1164
    Byte card_deleted;
    Byte card_must_removed;
    Byte card_out_of_time;
    Byte card_not_active;
    Byte can_manager_confirm;
    Byte card_blocked;
    char block_resone[256];
    char owner_name[40];
    Int64 owner_id;
    DWORD account_number;
    DWORD unpayer_type;
    Word bonus_number;
    Word discount_number;
    Int64 maximum_amount;
    Int64 avail_for_account;
    Int64 card_account_2;
    Int64 card_account_3;
    Int64 card_account_4;
    Int64 card_account_5;
    Int64 card_account_6;
    Int64 card_account_7;
    Int64 card_account_8;
    char card_info[256];
    char card_screen_info[256];
    char card_printer_info[256];
} card_info_t;

extern  "C" FARCARDS_API int GetCardInfoEx(
    Int64 card,
    DWORD restaurant,
    DWORD unit_no,
    card_info_t *info,
    char *inp_buf,
    DWORD inp_len,
    Word inp_kind,
    char *out_buf,
    DWORD *out_len,
    Word *out_kind
);

typedef struct {
    Word len; // 122
    Int64 card;
    Int64 owner;
    DWORD account;
    Byte payment_type;
    Int64 summa;
    Word restaurant;
    DWORD date_cassa;
    Byte cassa;
    DWORD check_num;

    Int64 nalog_a_sum;
    Word nalog_a_perc;

    Int64 nalog_b_sum;
    Word nalog_b_perc;

    Int64 nalog_c_sum;
    Word nalog_c_perc;

    Int64 nalog_d_sum;
    Word nalog_d_perc;

    Int64 nalog_e_sum;
    Word nalog_e_perc;

    Int64 nalog_f_sum;
    Word nalog_f_perc;

    Int64 nalog_g_sum;
    Word nalog_g_perc;

    Int64 nalog_h_sum;
    Word nalog_h_perc;

} transaction_t;

const int cassa_info_type_none = 0;
const int cassa_info_type_xml = 1;

extern "C" FARCARDS_API int TransactionsEx(
    Word count,
    transaction_t * list[],
    char* inp_buf,
    DWORD inp_len,
    Word inp_kind,
    char* out_buf,
    DWORD * out_len,
    Word * out_kind
);

extern  "C" FARCARDS_API void Init();
extern  "C" FARCARDS_API void Done();

#pragma pack(pop)
