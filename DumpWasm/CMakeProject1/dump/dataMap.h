//
// Created by admin on 2025/4/29.
//

#ifndef CMAKEPROJECT1_DATAMAP_H
#define CMAKEPROJECT1_DATAMAP_H

#include "dumpContext.h"
#include <map>
#include "Pattern.h"
#include <iostream>
#include "../3rd/PS.h"
#include "../include/json.hpp"
#include "../NT/NTHeader.h"

class dataMap{
public:
    inline  static bool dump(dumpContext ctx,std::map<std::string, std::map<std::string, uint64_t>>& output){
        struct DataMap
        {
            uint64_t m_dataDesc;
            uint32_t m_dataNumFields;
            uint32_t _pad0;
            uint64_t m_dataClassName;
            uint64_t _pad1;
            uint64_t _pad2;
            uint64_t m_baseMap;
        };

        struct DataTypeDesc
        {
            int32_t  m_fieldType;
            uint32_t _pad0;
            uint64_t m_fieldName;
            uint32_t m_fieldOffset[2];
            uint64_t m_externalName;
            uint64_t _pad1[5];
            uint64_t m_td;
            int32_t  m_fieldSizeInBytes;
            uint64_t _pad2[5];
        };

        auto fieldTypeName = [](int32_t t) -> const char* {
            switch (t) {
                case 0: return "Void";
                case 1: return "Float";
                case 2: return "String";
                case 3: return "Vector";
                case 4: return "Quaternion";
                case 5: return "Int";
                case 6: return "Bool";
                case 7: return "Short";
                case 8: return "Char";
                case 9: return "Color32";
                case 10: return "Embedded";
                case 11: return "Custom";
                case 12: return "ClassPtr";
                case 13: return "EHANDLE";
                case 14: return "edic";
                case 15: return "PositionVector";
                case 16: return "Time";
                case 17: return "Tick";
                case 18: return "ModelName";
                case 19: return "SoundName";
                case 20: return "FIELD_INPUT";
                case 21: return "FIELD_FUNCTION";
                case 22: return "FIELD_VMATRIX";
                case 23: return "FIELD_VMATRIX_WORLDSPACE";
                case 24: return "FIELD_MATRIX3X4_WORLDSPACE";
                case 25: return "FIELD_INTERVAL";
                case 26: return "FIELD_MODELINDEX";
                case 27: return "FIELD_MATERIALINDEX";
                case 28: return "FIELD_VECTOR2D";
                default: return "FIELD_UNKNOWN";
            }
        };

        auto matches = PS::SearchInSectionMultiple(ctx.data.data(), ".text", "\x48\x8D\x05\x00\x00\x00\x00\xC3", "xxx????x");

        if (!matches.size()){
            std::cout << "DataMap matches Null" << std::endl;
        }

        // std::cout << "matches.size:" << matches.size() << std::endl;

        for (auto i = size_t(); i < matches.size(); i++)
        {
            //auto addr = RVAEx(matches[i], data.c_str(), 7);
            auto addr = matches[i];
            addr += 3;
            addr += *reinterpret_cast<const std::int32_t*>(ctx.data.data() + addr);
            addr += 4;

            //LogE("addr %p", addr);

            if (addr < 0x13d2000 && PS::In(ctx.baseAddress,ctx.data.size(),(uint64_t)(ctx.data.data() + addr),8)  ) {
                continue;
            }

            //std::cout << "addr " << std::hex << addr << std::endl;

            auto maps = reinterpret_cast<const DataMap*>(ctx.data.data() + addr);

            //LogE("maps %p", maps);
            if (!maps->m_dataNumFields || maps->m_dataNumFields < 0 || maps->m_dataNumFields > 0x1000) continue;

            // std::cout << "maps->m_dataClassName " << std::hex << maps->m_dataClassName << std::endl;

            //auto cstr = data.c_str() + (maps->m_dataClassName - GlobalData::baseAddress);

            //LogE("cstr offset %p addr :%p", maps->m_dataClassName - GlobalData::baseAddress, maps->m_dataClassName);
            if (!PS::In(ctx.baseAddress,ctx.data.size(),(uint64_t)( maps->m_dataClassName),8)) {
                continue;
            }
            auto className = (char*)(maps->m_dataClassName - ctx.baseAddress + ctx.data.data());
            // std::cout << "className " << className << std::endl;
            if (!std::strlen(className) || !PS::isAsciiOnly(className)) continue;

            //LogE("className %s", className);
            if(!PS::In(ctx.baseAddress,ctx.data.size(),(uint64_t)( maps->m_dataDesc),8)){
                continue;
            }
            auto desc = reinterpret_cast<const DataTypeDesc*>(maps->m_dataDesc - ctx.baseAddress + ctx.data.data());

            for (auto j = 0u; j < maps->m_dataNumFields; j++)
            {
                //if (!info->Is(desc[j].m_fieldName)) continue;
                if (!PS::In(ctx.baseAddress,ctx.data.size(),(uint64_t)( desc[j].m_fieldName),8)) {
                    continue;
                }

                auto name = reinterpret_cast<char*>(desc[j].m_fieldName  - ctx.baseAddress + ctx.data.data());
                if (!std::strlen(name) || !PS::isAsciiOnly(name)) continue;

                output[className][name] = desc[j].m_fieldOffset[0];
                // std::cout << "[DataMap] " << className << "::" << name
                //           << " type " << fieldTypeName(desc[j].m_fieldType)
                //           << "(" << desc[j].m_fieldType << ")"
                //           << " offset 0x" << std::hex << desc[j].m_fieldOffset[0]
                //           << std::dec << std::endl;
            }
        }
        return true;
    }
};

#endif //CMAKEPROJECT1_DATAMAP_H
