#pragma once


enum class RenderPrimitiveType : unsigned char
{
    FLOAT = 0, INT = 1, UNSIGNED_INT = 2, CHAR = 3, UNSIGNED_CHAR = 4, UNKNOWN = 5
};

enum class RenderQueueTypes : unsigned char
{
    DirectQueue = 0, ComputeQueue = 1, CopyQueue = 2
};
