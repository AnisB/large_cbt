// Project includes
#include "cbt/cbt_utility.h"
#include "cbt/OCBT_128k.h"
#include "cbt/OCBT_256k.h"
#include "cbt/OCBT_512k.h"
#include "cbt/OCBT_1m.h"

const char* g_CBTTypesNames[] = { "OCBT_128K", "OCBT_256K", "OCBT_512K", "OCBT_1M"};

const char* cbt_type_to_string(CBTType type)
{
    switch (type)
    {
    case CBTType::OCBT_128K:
        return "OCBT_128K";

    case CBTType::OCBT_256K:
        return "OCBT_256K";

    case CBTType::OCBT_512K:
        return "OCBT_512K";

    case CBTType::OCBT_1M:
        return "OCBT_1M";
    }
    return "";
}

uint32_t cbt_num_elements(CBTType type)
{
    switch (type)
    {
        case CBTType::OCBT_128K:
            return OCBT_128K_NUM_ELEMENTS;

        case CBTType::OCBT_256K:
            return OCBT_256K_NUM_ELEMENTS;

        case CBTType::OCBT_512K:
            return OCBT_512K_NUM_ELEMENTS;

        case CBTType::OCBT_1M:
            return OCBT_1M_NUM_ELEMENTS;
    }
    return 0;
}

CBT* create_cbt(CBTType type)
{
    switch (type)
    {
        case CBTType::OCBT_128K:
            return new UPCBT_128k();

        case CBTType::OCBT_256K:
            return new UPCBT_256k();

        case CBTType::OCBT_512K:
            return new UPCBT_512k();

        case CBTType::OCBT_1M:
            return new UPCBT_1M();
    }
    return nullptr;
}
