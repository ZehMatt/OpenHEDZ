#include "../core/diagnostics/logging.hpp"
#include "../core/interop/interop.hpp"
#include "../core/memory.hpp"
#include "../globals.hpp"

#include <array>
#include <varargs.h>

namespace openhedz
{
    namespace logging = diagnostics::logging;

    struct DecodeInfo
    {
        uint32_t numEntries;
        uint32_t entryTableSizeInBytes;
        uint32_t uncompressedSize;
        uint32_t gap;
        const uint8_t* pDataStart;
    };

    struct DecodeTableNode
    {
        uint8_t byte;
        DecodeTableNode* nodeLeft;
        DecodeTableNode* nodeRight;
    };

    // 0x004D7A94
    static uint8_t gCurrentDecodeBitIndex{};
    // 0x004D7A98
    static uint32_t gCurrentDecodeBufIndex{};
    // 005E448C
    static DecodeTableNode* gNodeEntry{};

    // 0x00424B80
    static DecodeTableNode** allocTableNode()
    {
        DecodeTableNode** res = memory::alloc<DecodeTableNode*>();
        *res = memory::alloc<DecodeTableNode>();

        return res;
    }

    // 0x00424BC0
    static void initTableEntry(DecodeTableNode** entry, const uint8_t* buf)
    {
        DecodeTableNode* cur = *entry;

        uint8_t count = 0;
        uint8_t entries = buf[5];
        if (entries)
        {
            uint32_t m = *(uint32_t*)buf;
            do
            {
                const uint32_t m2 = 1u << (entries - count - 1);
                if ((m2 & m) != 0)
                {
                    if (!cur->nodeLeft)
                    {
                        cur->nodeLeft = memory::alloc<DecodeTableNode>();
                    }
                    cur = cur->nodeLeft;
                }
                else
                {
                    if (!cur->nodeRight)
                    {
                        cur->nodeRight = memory::alloc<DecodeTableNode>();
                    }
                    cur = cur->nodeRight;
                }
                entries = buf[5];
                ++count;
            } while (count < entries);
        }
        cur->byte = buf[4];
    }

    // 0x00424B40
    static DecodeTableNode** buildDecodeTree(const uint8_t* buf, uint16_t numEntries)
    {
        DecodeTableNode** data = allocTableNode();
        if (numEntries)
        {
            int count = numEntries;
            do
            {
                initTableEntry(data, buf);
                buf += 6;
                --count;
            } while (count);
        }
        return data;
    }

    // 0x00424C50
    static int decodeTable(DecodeTableNode** entryNode, const uint8_t* buf, uint8_t* pOutByte)
    {
        char bitIndex;
        DecodeTableNode* v4;
        int v5;
        DecodeTableNode* nodeLeft;
        int result;
        uint8_t curByte;

        bitIndex = gCurrentDecodeBitIndex;
        curByte = buf[gCurrentDecodeBufIndex];
        v4 = gNodeEntry;
        v5 = 0;

        do
        {
            uint8_t m = 1u;
            m <<= (7u - bitIndex);

            if ((m & curByte) != 0)
            {
                nodeLeft = v4->nodeLeft;
                if (nodeLeft)
                {
                    goto LABEL_7;
                }
                v5 = 1;
            }
            else
            {
                nodeLeft = v4->nodeRight;
                if (nodeLeft)
                {
                LABEL_7:
                    v4 = nodeLeft;
                    ++bitIndex;
                    gNodeEntry = nodeLeft;
                    gCurrentDecodeBitIndex = bitIndex;
                    continue;
                }
                v5 = 1;
            }
        } while (!v5 && bitIndex != 8);

        if (v5 == 1)
        {
            *pOutByte = v4->byte;
            if (gCurrentDecodeBitIndex == 8)
            {
                gCurrentDecodeBitIndex = 0;
                ++gCurrentDecodeBufIndex;
            }
            result = 1;
            gNodeEntry = *entryNode;
        }
        else if (v4->nodeRight || v4->nodeLeft)
        {
            ++gCurrentDecodeBufIndex;
            gCurrentDecodeBitIndex = 0;
            return 0;
        }
        else
        {
            *pOutByte = v4->byte;
            gCurrentDecodeBitIndex = 0;
            ++gCurrentDecodeBufIndex;
            gNodeEntry = *entryNode;
            return 1;
        }
        return result;
    }

    // 0x004249A0
    static void destroyNodeRecursive(DecodeTableNode* pParent, char isRight, DecodeTableNode* pNode)
    {
        DecodeTableNode* v3; // eax

        if (pNode)
        {
            if (pNode->nodeLeft || pNode->nodeRight)
            {
                destroyNodeRecursive(pNode, 0, pNode->nodeLeft);
                destroyNodeRecursive(pNode, 1, pNode->nodeRight);

                memory::dealloc(pNode);
                v3 = pParent;

                if (!pParent)
                {
                    return;
                }
                if (!isRight)
                {
                    pParent->nodeLeft = nullptr;
                    return;
                }
            }
            else
            {
                memory::dealloc(pNode);
                v3 = pParent;

                if (!isRight && pParent)
                {
                    pParent->nodeLeft = nullptr;
                    return;
                }
            }
            v3->nodeRight = nullptr;
        }
    }

    // 00424970
    static void destroyNodes(DecodeTableNode** pNodes)
    {
        destroyNodeRecursive(0, 0, *pNodes);
        if (pNodes)
        {
            memory::dealloc(pNodes);
        }
    }

    // 0x00424A20
    uint8_t* decompressText(const uint8_t* buf, uint32_t* outTotalSize)
    {
        DecodeInfo info{};

        WaitForSingleObject(gMutex02, 0xFFFFFFFF);

        info.numEntries = *(uint16_t*)buf;
        info.uncompressedSize = *(uint32_t*)(buf + 2);
        info.entryTableSizeInBytes = 6 * info.numEntries;

        const uint8_t* pDataStart = &buf[info.entryTableSizeInBytes + 6];
        info.pDataStart = pDataStart;

        DecodeTableNode** entryNode = buildDecodeTree(buf + 6, info.numEntries);
        gNodeEntry = *entryNode;

        *outTotalSize = info.uncompressedSize;

        uint8_t* outputBuffer = memory::alloc<uint8_t>(info.uncompressedSize);
        if (outputBuffer == nullptr)
            return nullptr;

        uint8_t* curPos = outputBuffer;

        gCurrentDecodeBitIndex = 0;
        gCurrentDecodeBufIndex = 0;

        int curCount = 0;
        do
        {
            uint8_t outByte{};
            while (decodeTable(entryNode, info.pDataStart, &outByte) != 1)
            {
                ;
            }
            *curPos++ = outByte;
            ++curCount;
        } while (curCount != info.uncompressedSize);

        destroyNodes(entryNode);

        ReleaseMutex(gMutex02);

        return outputBuffer;
    }
    HOOK_FUNCTION(0x00424A20, decompressText);

} // namespace openhedz