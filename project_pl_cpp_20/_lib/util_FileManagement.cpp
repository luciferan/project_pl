#include "./util_FileManagement.h"
#include "../_external/md5.h"

#include <filesystem>

//
int GetFileList(string strDirectoryPath, list<string>& fileList)
{
    try {
        for (const auto& entry : filesystem::directory_iterator(strDirectoryPath)) {
            fileList.emplace_back(entry.path().string());
        }
    }
    catch (const filesystem::filesystem_error& e) {
        fileList.emplace_back(e.what());
        return -1;
    }

    return (int)fileList.size();
}

int MakeMD5(wstring wstrFilePath, wstring& wstrChecksum)
{
    FILE* pFile{nullptr};

    md5_byte_t cDigest[16];
    memset((void*)&cDigest, 0, sizeof(cDigest));
    md5_state_s md5struct;
    memset((void*)&md5struct, 0, sizeof(md5struct));

    auto nSize{0};
    BYTE byBuffer[1024]{0,};

    auto error = _wfopen_s(&pFile, wstrFilePath.c_str(), L"rb");
    if (0 != error) {
        return error;
    }

    //
    md5_init(&md5struct);
    while (0 != (nSize = (int)fread_s(byBuffer, 1024, 1024, 1, pFile))) {
        md5_append(&md5struct, (const md5_byte_t*)byBuffer, nSize);
    }
    md5_finish(&md5struct, cDigest);

    fclose(pFile);
    pFile = nullptr;

    //
    wstrChecksum.resize(16 * 2);
    for (auto idx = 0; idx < 16; ++idx) {
        swprintf_s(&wstrChecksum[idx * 2], 2, L"%02x", cDigest[idx]);
    }

    //
    return 0;
}

bool CheckMD5(wstring wstrFilePath, wstring wstrChecksum)
{
    wstring wstrMakeChecksum = {};
    if (0 != MakeMD5(wstrFilePath, wstrMakeChecksum)) {
        return false;
    }

    if (0 != wstrMakeChecksum.compare(wstrChecksum)) {
        return false;
    }

    return true;
}
