/*
 * Copyright (C) 2016+     AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license: http://github.com/azerothcore/azerothcore-wotlk/LICENSE-GPL2
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
 */

#include "vmapexport.h"
#include "wdtfile.h"
#include "adtfile.h"
#include <cstdio>

char* wdtGetPlainName(char* FileName)
{
    char* szTemp;

    if ((szTemp = strrchr(FileName, '\\')) != nullptr)
        FileName = szTemp + 1;
    return FileName;
}

WDTFile::WDTFile(char* file_name, char* file_name1) : WDT(file_name), gWmoInstansName(nullptr), gnWMO(0)
{
    filename.append(file_name1, strlen(file_name1));
}

bool WDTFile::init(char* /*map_id*/, unsigned int mapID)
{
    if (WDT.isEof())
    {
        //printf("Can't find WDT file.\n");
        return false;
    }

    char fourcc[5];
    uint32 size;

    std::string dirname = std::string(szWorkDirWmo) + "/dir_bin";
    FILE* dirfile;
    dirfile = fopen(dirname.c_str(), "ab");
    if (!dirfile)
    {
        printf("Can't open dirfile!'%s'\n", dirname.c_str());
        return false;
    }

    while (!WDT.isEof())
    {
        WDT.read(fourcc, 4);
        WDT.read(&size, 4);

        flipcc(fourcc);
        fourcc[4] = 0;

        size_t nextpos = WDT.getPos() + size;

        if (!strcmp(fourcc, "MAIN"))
        {
        }
        if (!strcmp(fourcc, "MWMO"))
        {
            // global map objects
            if (size)
            {
                char* buf = new char[size];
                WDT.read(buf, size);
                char* p = buf;
                int q = 0;
                gWmoInstansName = new string[size];
                while (p < buf + size)
                {
                    char* s = wdtGetPlainName(p);
                    fixnamen(s, strlen(s));
                    p = p + strlen(p) + 1;
                    gWmoInstansName[q++] = s;
                }
                delete[] buf;
            }
        }
        else if (!strcmp(fourcc, "MODF"))
        {
            // global wmo instance data
            if (size)
            {
                gnWMO = (int)size / 64;

                for (int i = 0; i < gnWMO; ++i)
                {
                    int id;
                    WDT.read(&id, 4);
                    WMOInstance inst(WDT, gWmoInstansName[id].c_str(), mapID, 65, 65, dirfile);
                }

                delete[] gWmoInstansName;
            }
        }
        WDT.seek((int)nextpos);
    }

    WDT.close();
    fclose(dirfile);
    return true;
}

WDTFile::~WDTFile()
{
    WDT.close();
}

ADTFile* WDTFile::GetMap(int x, int z)
{
    if (!(x >= 0 && z >= 0 && x < 64 && z < 64))
        return nullptr;

    char name[512];

    snprintf(name, sizeof(name), R"(World\Maps\%s\%s_%d_%d.adt)", filename.c_str(), filename.c_str(), x, z);
    return new ADTFile(name);
}
