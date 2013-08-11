/* Generated from /home/cmaxwell/chopstix/src/lib/libchopstix/chopstix.asn1 */
/* Do not edit */

#ifndef __chopstix_asn1_h__
#define __chopstix_asn1_h__

#include <stddef.h>
#include <time.h>

time_t timegm (struct tm*);

#ifndef __asn1_common_definitions__
#define __asn1_common_definitions__

typedef struct heim_octet_string {
  size_t length;
  void *data;
} heim_octet_string;

typedef char *heim_general_string;

typedef char *heim_utf8_string;

typedef struct heim_oid {
  size_t length;
  unsigned *components;
} heim_oid;

#define ASN1_MALLOC_ENCODE(T, B, BL, S, L, R)                  \
  do {                                                         \
    (BL) = length_##T((S));                                    \
    (B) = malloc((BL));                                        \
    if((B) == NULL) {                                          \
      (R) = ENOMEM;                                            \
    } else {                                                   \
      (R) = encode_##T(((unsigned char*)(B)) + (BL) - 1, (BL), \
                       (S), (L));                              \
      if((R) != 0) {                                           \
        free((B));                                             \
        (B) = NULL;                                            \
      }                                                        \
    }                                                          \
  } while (0)

#endif

/*
CHOPSTIX-PAYMENTTYPE ::= INTEGER
*/

typedef enum CHOPSTIX_PAYMENTTYPE {
  PAYMENT_NONE = 0,
  PAYMENT_CASH = 1,
  PAYMENT_CREDIT = 2,
  PAYMENT_VOID = 3,
  PAYMENT_DEBIT = 4,
  PAYMENT_CHEQUE = 5,
  PAYMENT_OTHER = 6
} CHOPSTIX_PAYMENTTYPE;

int    encode_CHOPSTIX_PAYMENTTYPE(unsigned char *, size_t, const CHOPSTIX_PAYMENTTYPE *, size_t *);
int    decode_CHOPSTIX_PAYMENTTYPE(const unsigned char *, size_t, CHOPSTIX_PAYMENTTYPE *, size_t *);
void   free_CHOPSTIX_PAYMENTTYPE  (CHOPSTIX_PAYMENTTYPE *);
size_t length_CHOPSTIX_PAYMENTTYPE(const CHOPSTIX_PAYMENTTYPE *);
int    copy_CHOPSTIX_PAYMENTTYPE  (const CHOPSTIX_PAYMENTTYPE *, CHOPSTIX_PAYMENTTYPE *);


/*
CHOPSTIX-ORDERTYPE ::= INTEGER
*/

typedef enum CHOPSTIX_ORDERTYPE {
  ORDER_NONE = 0,
  ORDER_PICKUP = 1,
  ORDER_DELIVERY = 2,
  ORDER_WALKIN = 3,
  ORDER_VOID = 4
} CHOPSTIX_ORDERTYPE;

int    encode_CHOPSTIX_ORDERTYPE(unsigned char *, size_t, const CHOPSTIX_ORDERTYPE *, size_t *);
int    decode_CHOPSTIX_ORDERTYPE(const unsigned char *, size_t, CHOPSTIX_ORDERTYPE *, size_t *);
void   free_CHOPSTIX_ORDERTYPE  (CHOPSTIX_ORDERTYPE *);
size_t length_CHOPSTIX_ORDERTYPE(const CHOPSTIX_ORDERTYPE *);
int    copy_CHOPSTIX_ORDERTYPE  (const CHOPSTIX_ORDERTYPE *, CHOPSTIX_ORDERTYPE *);


/*
CHOPSTIX-CARDTYPE ::= INTEGER
*/

typedef enum CHOPSTIX_CARDTYPE {
  CARD_OTHER = 0,
  CARD_VISA = 1,
  CARD_MASTERCARD = 2
} CHOPSTIX_CARDTYPE;

int    encode_CHOPSTIX_CARDTYPE(unsigned char *, size_t, const CHOPSTIX_CARDTYPE *, size_t *);
int    decode_CHOPSTIX_CARDTYPE(const unsigned char *, size_t, CHOPSTIX_CARDTYPE *, size_t *);
void   free_CHOPSTIX_CARDTYPE  (CHOPSTIX_CARDTYPE *);
size_t length_CHOPSTIX_CARDTYPE(const CHOPSTIX_CARDTYPE *);
int    copy_CHOPSTIX_CARDTYPE  (const CHOPSTIX_CARDTYPE *, CHOPSTIX_CARDTYPE *);


/*
CHOPSTIX-ITEMTYPE ::= INTEGER
*/

typedef enum CHOPSTIX_ITEMTYPE {
  ITEM_NORMAL = 0,
  ITEM_SUBITEM = 1,
  ITEM_EXTRA = 2,
  ITEM_RULE = 3
} CHOPSTIX_ITEMTYPE;

int    encode_CHOPSTIX_ITEMTYPE(unsigned char *, size_t, const CHOPSTIX_ITEMTYPE *, size_t *);
int    decode_CHOPSTIX_ITEMTYPE(const unsigned char *, size_t, CHOPSTIX_ITEMTYPE *, size_t *);
void   free_CHOPSTIX_ITEMTYPE  (CHOPSTIX_ITEMTYPE *);
size_t length_CHOPSTIX_ITEMTYPE(const CHOPSTIX_ITEMTYPE *);
int    copy_CHOPSTIX_ITEMTYPE  (const CHOPSTIX_ITEMTYPE *, CHOPSTIX_ITEMTYPE *);


/*
CHOPSTIX-DISCOUNTTYPE ::= INTEGER
*/

typedef enum CHOPSTIX_DISCOUNTTYPE {
  DISCOUNT_NONE = 0,
  DISCOUNT_DOLLAR = 1,
  DISCOUNT_PERCENT = 2,
  DISCOUNT_RULE = 3
} CHOPSTIX_DISCOUNTTYPE;

int    encode_CHOPSTIX_DISCOUNTTYPE(unsigned char *, size_t, const CHOPSTIX_DISCOUNTTYPE *, size_t *);
int    decode_CHOPSTIX_DISCOUNTTYPE(const unsigned char *, size_t, CHOPSTIX_DISCOUNTTYPE *, size_t *);
void   free_CHOPSTIX_DISCOUNTTYPE  (CHOPSTIX_DISCOUNTTYPE *);
size_t length_CHOPSTIX_DISCOUNTTYPE(const CHOPSTIX_DISCOUNTTYPE *);
int    copy_CHOPSTIX_DISCOUNTTYPE  (const CHOPSTIX_DISCOUNTTYPE *, CHOPSTIX_DISCOUNTTYPE *);


/*
ChopstixFlags ::= BIT STRING {
  deleted(0)
}
*/

typedef struct ChopstixFlags {
  unsigned int deleted:1;
} ChopstixFlags;


int    encode_ChopstixFlags(unsigned char *, size_t, const ChopstixFlags *, size_t *);
int    decode_ChopstixFlags(const unsigned char *, size_t, ChopstixFlags *, size_t *);
void   free_ChopstixFlags  (ChopstixFlags *);
size_t length_ChopstixFlags(const ChopstixFlags *);
int    copy_ChopstixFlags  (const ChopstixFlags *, ChopstixFlags *);
unsigned ChopstixFlags2int(ChopstixFlags);
ChopstixFlags int2ChopstixFlags(unsigned);
const struct units * asn1_ChopstixFlags_units(void);

/*
ChopstixTime ::= GeneralizedTime
*/

typedef time_t ChopstixTime;

int    encode_ChopstixTime(unsigned char *, size_t, const ChopstixTime *, size_t *);
int    decode_ChopstixTime(const unsigned char *, size_t, ChopstixTime *, size_t *);
void   free_ChopstixTime  (ChopstixTime *);
size_t length_ChopstixTime(const ChopstixTime *);
int    copy_ChopstixTime  (const ChopstixTime *, ChopstixTime *);


/*
ChopstixStreet ::= GeneralString
*/

typedef heim_general_string ChopstixStreet;

int    encode_ChopstixStreet(unsigned char *, size_t, const ChopstixStreet *, size_t *);
int    decode_ChopstixStreet(const unsigned char *, size_t, ChopstixStreet *, size_t *);
void   free_ChopstixStreet  (ChopstixStreet *);
size_t length_ChopstixStreet(const ChopstixStreet *);
int    copy_ChopstixStreet  (const ChopstixStreet *, ChopstixStreet *);


/*
ChopstixName ::= GeneralString
*/

typedef heim_general_string ChopstixName;

int    encode_ChopstixName(unsigned char *, size_t, const ChopstixName *, size_t *);
int    decode_ChopstixName(const unsigned char *, size_t, ChopstixName *, size_t *);
void   free_ChopstixName  (ChopstixName *);
size_t length_ChopstixName(const ChopstixName *);
int    copy_ChopstixName  (const ChopstixName *, ChopstixName *);


/*
ChopstixSpecial ::= GeneralString
*/

typedef heim_general_string ChopstixSpecial;

int    encode_ChopstixSpecial(unsigned char *, size_t, const ChopstixSpecial *, size_t *);
int    decode_ChopstixSpecial(const unsigned char *, size_t, ChopstixSpecial *, size_t *);
void   free_ChopstixSpecial  (ChopstixSpecial *);
size_t length_ChopstixSpecial(const ChopstixSpecial *);
int    copy_ChopstixSpecial  (const ChopstixSpecial *, ChopstixSpecial *);


/*
ChopstixPhone ::= SEQUENCE {
  npa[0]          INTEGER,
  nxx[1]          INTEGER,
  num[2]          INTEGER,
  ext[3]          INTEGER OPTIONAL
}
*/

typedef struct ChopstixPhone {
  int npa;
  int nxx;
  int num;
  int *ext;
} ChopstixPhone;

int    encode_ChopstixPhone(unsigned char *, size_t, const ChopstixPhone *, size_t *);
int    decode_ChopstixPhone(const unsigned char *, size_t, ChopstixPhone *, size_t *);
void   free_ChopstixPhone  (ChopstixPhone *);
size_t length_ChopstixPhone(const ChopstixPhone *);
int    copy_ChopstixPhone  (const ChopstixPhone *, ChopstixPhone *);


/*
ChopstixAddress ::= SEQUENCE {
  addr[0]         GeneralString,
  apt[1]          GeneralString,
  entry[2]        GeneralString
}
*/

typedef struct ChopstixAddress {
  heim_general_string addr;
  heim_general_string apt;
  heim_general_string entry;
} ChopstixAddress;

int    encode_ChopstixAddress(unsigned char *, size_t, const ChopstixAddress *, size_t *);
int    decode_ChopstixAddress(const unsigned char *, size_t, ChopstixAddress *, size_t *);
void   free_ChopstixAddress  (ChopstixAddress *);
size_t length_ChopstixAddress(const ChopstixAddress *);
int    copy_ChopstixAddress  (const ChopstixAddress *, ChopstixAddress *);


/*
ChopstixIntersect ::= SEQUENCE {
  cross[0]        GeneralString
}
*/

typedef struct ChopstixIntersect {
  heim_general_string cross;
} ChopstixIntersect;

int    encode_ChopstixIntersect(unsigned char *, size_t, const ChopstixIntersect *, size_t *);
int    decode_ChopstixIntersect(const unsigned char *, size_t, ChopstixIntersect *, size_t *);
void   free_ChopstixIntersect  (ChopstixIntersect *);
size_t length_ChopstixIntersect(const ChopstixIntersect *);
int    copy_ChopstixIntersect  (const ChopstixIntersect *, ChopstixIntersect *);


/*
ChopstixCredit ::= SEQUENCE {
  credit[0]       INTEGER,
  remain[1]       INTEGER,
  reason[2]       GeneralString
}
*/

typedef struct ChopstixCredit {
  int credit;
  int remain;
  heim_general_string reason;
} ChopstixCredit;

int    encode_ChopstixCredit(unsigned char *, size_t, const ChopstixCredit *, size_t *);
int    decode_ChopstixCredit(const unsigned char *, size_t, ChopstixCredit *, size_t *);
void   free_ChopstixCredit  (ChopstixCredit *);
size_t length_ChopstixCredit(const ChopstixCredit *);
int    copy_ChopstixCredit  (const ChopstixCredit *, ChopstixCredit *);


/*
ChopstixCredits ::= SEQUENCE OF ChopstixCredit
*/

typedef struct ChopstixCredits {
  unsigned int len;
  ChopstixCredit *val;
} ChopstixCredits;

int    encode_ChopstixCredits(unsigned char *, size_t, const ChopstixCredits *, size_t *);
int    decode_ChopstixCredits(const unsigned char *, size_t, ChopstixCredits *, size_t *);
void   free_ChopstixCredits  (ChopstixCredits *);
size_t length_ChopstixCredits(const ChopstixCredits *);
int    copy_ChopstixCredits  (const ChopstixCredits *, ChopstixCredits *);


/*
ChopstixCustomer ::= SEQUENCE {
  phone[0]        ChopstixPhone,
  name[1]         ChopstixName,
  addr[2]         ChopstixAddress,
  isect[3]        ChopstixIntersect,
  reps[4]         INTEGER,
  special[5]      ChopstixSpecial OPTIONAL,
  credit[6]       ChopstixCredits OPTIONAL
}
*/

typedef struct ChopstixCustomer {
  ChopstixPhone phone;
  ChopstixName name;
  ChopstixAddress addr;
  ChopstixIntersect isect;
  int reps;
  ChopstixSpecial *special;
  ChopstixCredits *credit;
} ChopstixCustomer;

int    encode_ChopstixCustomer(unsigned char *, size_t, const ChopstixCustomer *, size_t *);
int    decode_ChopstixCustomer(const unsigned char *, size_t, ChopstixCustomer *, size_t *);
void   free_ChopstixCustomer  (ChopstixCustomer *);
size_t length_ChopstixCustomer(const ChopstixCustomer *);
int    copy_ChopstixCustomer  (const ChopstixCustomer *, ChopstixCustomer *);


/*
ChopstixCreditNumber ::= GeneralString
*/

typedef heim_general_string ChopstixCreditNumber;

int    encode_ChopstixCreditNumber(unsigned char *, size_t, const ChopstixCreditNumber *, size_t *);
int    decode_ChopstixCreditNumber(const unsigned char *, size_t, ChopstixCreditNumber *, size_t *);
void   free_ChopstixCreditNumber  (ChopstixCreditNumber *);
size_t length_ChopstixCreditNumber(const ChopstixCreditNumber *);
int    copy_ChopstixCreditNumber  (const ChopstixCreditNumber *, ChopstixCreditNumber *);


/*
ChopstixCreditExpiry ::= GeneralString
*/

typedef heim_general_string ChopstixCreditExpiry;

int    encode_ChopstixCreditExpiry(unsigned char *, size_t, const ChopstixCreditExpiry *, size_t *);
int    decode_ChopstixCreditExpiry(const unsigned char *, size_t, ChopstixCreditExpiry *, size_t *);
void   free_ChopstixCreditExpiry  (ChopstixCreditExpiry *);
size_t length_ChopstixCreditExpiry(const ChopstixCreditExpiry *);
int    copy_ChopstixCreditExpiry  (const ChopstixCreditExpiry *, ChopstixCreditExpiry *);


/*
ChopstixCreditInfo ::= SEQUENCE {
  type[0]         CHOPSTIX-CARDTYPE,
  number[1]       ChopstixCreditNumber,
  expiry[2]       ChopstixCreditExpiry
}
*/

typedef struct ChopstixCreditInfo {
  CHOPSTIX_CARDTYPE type;
  ChopstixCreditNumber number;
  ChopstixCreditExpiry expiry;
} ChopstixCreditInfo;

int    encode_ChopstixCreditInfo(unsigned char *, size_t, const ChopstixCreditInfo *, size_t *);
int    decode_ChopstixCreditInfo(const unsigned char *, size_t, ChopstixCreditInfo *, size_t *);
void   free_ChopstixCreditInfo  (ChopstixCreditInfo *);
size_t length_ChopstixCreditInfo(const ChopstixCreditInfo *);
int    copy_ChopstixCreditInfo  (const ChopstixCreditInfo *, ChopstixCreditInfo *);


/*
ChopstixPayment ::= SEQUENCE {
  type[0]         CHOPSTIX-PAYMENTTYPE,
  ccinfo[1]       ChopstixCreditInfo OPTIONAL
}
*/

typedef struct ChopstixPayment {
  CHOPSTIX_PAYMENTTYPE type;
  ChopstixCreditInfo *ccinfo;
} ChopstixPayment;

int    encode_ChopstixPayment(unsigned char *, size_t, const ChopstixPayment *, size_t *);
int    decode_ChopstixPayment(const unsigned char *, size_t, ChopstixPayment *, size_t *);
void   free_ChopstixPayment  (ChopstixPayment *);
size_t length_ChopstixPayment(const ChopstixPayment *);
int    copy_ChopstixPayment  (const ChopstixPayment *, ChopstixPayment *);


/*
ChopstixTotal ::= SEQUENCE {
  subtotal[0]     INTEGER,
  discount[1]     INTEGER,
  disctype[2]     CHOPSTIX-DISCOUNTTYPE,
  delivery[3]     INTEGER,
  delitype[4]     CHOPSTIX-DISCOUNTTYPE,
  credit[5]       INTEGER,
  tax1[6]         INTEGER,
  tax2[7]         INTEGER,
  total[8]        INTEGER
}
*/

typedef struct ChopstixTotal {
  int subtotal;
  int discount;
  CHOPSTIX_DISCOUNTTYPE disctype;
  int delivery;
  CHOPSTIX_DISCOUNTTYPE delitype;
  int credit;
  int tax1;
  int tax2;
  int total;
} ChopstixTotal;

int    encode_ChopstixTotal(unsigned char *, size_t, const ChopstixTotal *, size_t *);
int    decode_ChopstixTotal(const unsigned char *, size_t, ChopstixTotal *, size_t *);
void   free_ChopstixTotal  (ChopstixTotal *);
size_t length_ChopstixTotal(const ChopstixTotal *);
int    copy_ChopstixTotal  (const ChopstixTotal *, ChopstixTotal *);


/*
ChopstixItemCode ::= GeneralString
*/

typedef heim_general_string ChopstixItemCode;

int    encode_ChopstixItemCode(unsigned char *, size_t, const ChopstixItemCode *, size_t *);
int    decode_ChopstixItemCode(const unsigned char *, size_t, ChopstixItemCode *, size_t *);
void   free_ChopstixItemCode  (ChopstixItemCode *);
size_t length_ChopstixItemCode(const ChopstixItemCode *);
int    copy_ChopstixItemCode  (const ChopstixItemCode *, ChopstixItemCode *);


/*
ChopstixItemStyle ::= SEQUENCE {
  num[0]          INTEGER,
  name[1]         GeneralString
}
*/

typedef struct ChopstixItemStyle {
  int num;
  heim_general_string name;
} ChopstixItemStyle;

int    encode_ChopstixItemStyle(unsigned char *, size_t, const ChopstixItemStyle *, size_t *);
int    decode_ChopstixItemStyle(const unsigned char *, size_t, ChopstixItemStyle *, size_t *);
void   free_ChopstixItemStyle  (ChopstixItemStyle *);
size_t length_ChopstixItemStyle(const ChopstixItemStyle *);
int    copy_ChopstixItemStyle  (const ChopstixItemStyle *, ChopstixItemStyle *);


/*
ChopstixItemStyles ::= SEQUENCE OF ChopstixItemStyle
*/

typedef struct ChopstixItemStyles {
  unsigned int len;
  ChopstixItemStyle *val;
} ChopstixItemStyles;

int    encode_ChopstixItemStyles(unsigned char *, size_t, const ChopstixItemStyles *, size_t *);
int    decode_ChopstixItemStyles(const unsigned char *, size_t, ChopstixItemStyles *, size_t *);
void   free_ChopstixItemStyles  (ChopstixItemStyles *);
size_t length_ChopstixItemStyles(const ChopstixItemStyles *);
int    copy_ChopstixItemStyles  (const ChopstixItemStyles *, ChopstixItemStyles *);


/*
ChopstixItemExtra ::= SEQUENCE {
  qty[0]          INTEGER,
  code[1]         ChopstixItemCode
}
*/

typedef struct ChopstixItemExtra {
  int qty;
  ChopstixItemCode code;
} ChopstixItemExtra;

int    encode_ChopstixItemExtra(unsigned char *, size_t, const ChopstixItemExtra *, size_t *);
int    decode_ChopstixItemExtra(const unsigned char *, size_t, ChopstixItemExtra *, size_t *);
void   free_ChopstixItemExtra  (ChopstixItemExtra *);
size_t length_ChopstixItemExtra(const ChopstixItemExtra *);
int    copy_ChopstixItemExtra  (const ChopstixItemExtra *, ChopstixItemExtra *);


/*
ChopstixItemExtras ::= SEQUENCE OF ChopstixItemExtra
*/

typedef struct ChopstixItemExtras {
  unsigned int len;
  ChopstixItemExtra *val;
} ChopstixItemExtras;

int    encode_ChopstixItemExtras(unsigned char *, size_t, const ChopstixItemExtras *, size_t *);
int    decode_ChopstixItemExtras(const unsigned char *, size_t, ChopstixItemExtras *, size_t *);
void   free_ChopstixItemExtras  (ChopstixItemExtras *);
size_t length_ChopstixItemExtras(const ChopstixItemExtras *);
int    copy_ChopstixItemExtras  (const ChopstixItemExtras *, ChopstixItemExtras *);


/*
ChopstixSubItem ::= SEQUENCE {
  qty[0]          INTEGER,
  code[1]         ChopstixItemCode,
  style[2]        INTEGER,
  pricedelta[3]   INTEGER,
  special[4]      ChopstixSpecial OPTIONAL
}
*/

typedef struct ChopstixSubItem {
  int qty;
  ChopstixItemCode code;
  int style;
  int pricedelta;
  ChopstixSpecial *special;
} ChopstixSubItem;

int    encode_ChopstixSubItem(unsigned char *, size_t, const ChopstixSubItem *, size_t *);
int    decode_ChopstixSubItem(const unsigned char *, size_t, ChopstixSubItem *, size_t *);
void   free_ChopstixSubItem  (ChopstixSubItem *);
size_t length_ChopstixSubItem(const ChopstixSubItem *);
int    copy_ChopstixSubItem  (const ChopstixSubItem *, ChopstixSubItem *);


/*
ChopstixSubItems ::= SEQUENCE OF ChopstixSubItem
*/

typedef struct ChopstixSubItems {
  unsigned int len;
  ChopstixSubItem *val;
} ChopstixSubItems;

int    encode_ChopstixSubItems(unsigned char *, size_t, const ChopstixSubItems *, size_t *);
int    decode_ChopstixSubItems(const unsigned char *, size_t, ChopstixSubItems *, size_t *);
void   free_ChopstixSubItems  (ChopstixSubItems *);
size_t length_ChopstixSubItems(const ChopstixSubItems *);
int    copy_ChopstixSubItems  (const ChopstixSubItems *, ChopstixSubItems *);


/*
ChopstixMenuitem ::= SEQUENCE {
  gen[0]          INTEGER,
  code[1]         ChopstixItemCode,
  name[2]         GeneralString,
  price[4]        INTEGER,
  styles[5]       ChopstixItemStyles,
  extras[6]       ChopstixItemExtras,
  subitems[7]     ChopstixItemExtras OPTIONAL,
  flags[8]        ChopstixFlags
}
*/

typedef struct ChopstixMenuitem {
  int gen;
  ChopstixItemCode code;
  heim_general_string name;
  int price;
  ChopstixItemStyles styles;
  ChopstixItemExtras extras;
  ChopstixItemExtras *subitems;
  ChopstixFlags flags;
} ChopstixMenuitem;

int    encode_ChopstixMenuitem(unsigned char *, size_t, const ChopstixMenuitem *, size_t *);
int    decode_ChopstixMenuitem(const unsigned char *, size_t, ChopstixMenuitem *, size_t *);
void   free_ChopstixMenuitem  (ChopstixMenuitem *);
size_t length_ChopstixMenuitem(const ChopstixMenuitem *);
int    copy_ChopstixMenuitem  (const ChopstixMenuitem *, ChopstixMenuitem *);


/*
ChopstixMenu ::= SEQUENCE OF ChopstixMenuitem
*/

typedef struct ChopstixMenu {
  unsigned int len;
  ChopstixMenuitem *val;
} ChopstixMenu;

int    encode_ChopstixMenu(unsigned char *, size_t, const ChopstixMenu *, size_t *);
int    decode_ChopstixMenu(const unsigned char *, size_t, ChopstixMenu *, size_t *);
void   free_ChopstixMenu  (ChopstixMenu *);
size_t length_ChopstixMenu(const ChopstixMenu *);
int    copy_ChopstixMenu  (const ChopstixMenu *, ChopstixMenu *);


/*
ChopstixHeader ::= SEQUENCE {
  company[0]      ChopstixName,
  addr[1]         ChopstixAddress,
  phone[2]        ChopstixPhone,
  city[3]         GeneralString,
  province[4]     GeneralString,
  country[5]      GeneralString
}
*/

typedef struct ChopstixHeader {
  ChopstixName company;
  ChopstixAddress addr;
  ChopstixPhone phone;
  heim_general_string city;
  heim_general_string province;
  heim_general_string country;
} ChopstixHeader;

int    encode_ChopstixHeader(unsigned char *, size_t, const ChopstixHeader *, size_t *);
int    decode_ChopstixHeader(const unsigned char *, size_t, ChopstixHeader *, size_t *);
void   free_ChopstixHeader  (ChopstixHeader *);
size_t length_ChopstixHeader(const ChopstixHeader *);
int    copy_ChopstixHeader  (const ChopstixHeader *, ChopstixHeader *);


/*
ChopstixOrderItem ::= SEQUENCE {
  type[0]         CHOPSTIX-ITEMTYPE,
  qty[1]          INTEGER,
  code[2]         ChopstixItemCode,
  style[3]        INTEGER,
  special[4]      ChopstixSpecial OPTIONAL,
  subitems[5]     ChopstixSubItems OPTIONAL
}
*/

typedef struct ChopstixOrderItem {
  CHOPSTIX_ITEMTYPE type;
  int qty;
  ChopstixItemCode code;
  int style;
  ChopstixSpecial *special;
  ChopstixSubItems *subitems;
} ChopstixOrderItem;

int    encode_ChopstixOrderItem(unsigned char *, size_t, const ChopstixOrderItem *, size_t *);
int    decode_ChopstixOrderItem(const unsigned char *, size_t, ChopstixOrderItem *, size_t *);
void   free_ChopstixOrderItem  (ChopstixOrderItem *);
size_t length_ChopstixOrderItem(const ChopstixOrderItem *);
int    copy_ChopstixOrderItem  (const ChopstixOrderItem *, ChopstixOrderItem *);


/*
ChopstixOrderItems ::= SEQUENCE OF ChopstixOrderItem
*/

typedef struct ChopstixOrderItems {
  unsigned int len;
  ChopstixOrderItem *val;
} ChopstixOrderItems;

int    encode_ChopstixOrderItems(unsigned char *, size_t, const ChopstixOrderItems *, size_t *);
int    decode_ChopstixOrderItems(const unsigned char *, size_t, ChopstixOrderItems *, size_t *);
void   free_ChopstixOrderItems  (ChopstixOrderItems *);
size_t length_ChopstixOrderItems(const ChopstixOrderItems *);
int    copy_ChopstixOrderItems  (const ChopstixOrderItems *, ChopstixOrderItems *);


/*
ChopstixOrderKey ::= INTEGER
*/

typedef int ChopstixOrderKey;

int    encode_ChopstixOrderKey(unsigned char *, size_t, const ChopstixOrderKey *, size_t *);
int    decode_ChopstixOrderKey(const unsigned char *, size_t, ChopstixOrderKey *, size_t *);
void   free_ChopstixOrderKey  (ChopstixOrderKey *);
size_t length_ChopstixOrderKey(const ChopstixOrderKey *);
int    copy_ChopstixOrderKey  (const ChopstixOrderKey *, ChopstixOrderKey *);


/*
ChopstixOrder ::= [APPLICATION 1] SEQUENCE {
  key[0]          ChopstixOrderKey,
  date[1]         ChopstixTime,
  customer[2]     ChopstixCustomer,
  type[3]         CHOPSTIX-ORDERTYPE,
  payment[4]      ChopstixPayment,
  items[5]        ChopstixOrderItems,
  special[6]      ChopstixSpecial,
  ruleitems[7]    ChopstixOrderItems,
  total[8]        ChopstixTotal
}
*/

typedef struct  {
  ChopstixOrderKey key;
  ChopstixTime date;
  ChopstixCustomer customer;
  CHOPSTIX_ORDERTYPE type;
  ChopstixPayment payment;
  ChopstixOrderItems items;
  ChopstixSpecial special;
  ChopstixOrderItems ruleitems;
  ChopstixTotal total;
} ChopstixOrder;

int    encode_ChopstixOrder(unsigned char *, size_t, const ChopstixOrder *, size_t *);
int    decode_ChopstixOrder(const unsigned char *, size_t, ChopstixOrder *, size_t *);
void   free_ChopstixOrder  (ChopstixOrder *);
size_t length_ChopstixOrder(const ChopstixOrder *);
int    copy_ChopstixOrder  (const ChopstixOrder *, ChopstixOrder *);


/*
ChopstixOrders ::= SEQUENCE OF ChopstixOrder
*/

typedef struct ChopstixOrders {
  unsigned int len;
  ChopstixOrder *val;
} ChopstixOrders;

int    encode_ChopstixOrders(unsigned char *, size_t, const ChopstixOrders *, size_t *);
int    decode_ChopstixOrders(const unsigned char *, size_t, ChopstixOrders *, size_t *);
void   free_ChopstixOrders  (ChopstixOrders *);
size_t length_ChopstixOrders(const ChopstixOrders *);
int    copy_ChopstixOrders  (const ChopstixOrders *, ChopstixOrders *);


#endif /* __chopstix_asn1_h__ */
