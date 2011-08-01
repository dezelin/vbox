/* $Id$ */
/** @file
 *
 * IO helper for IGuest COM class implementations.
 */

/*
 * Copyright (C) 2011 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

/******************************************************************************
 *   Header Files                                                             *
 ******************************************************************************/
#include "GuestCtrlImplPrivate.h"


/******************************************************************************
 *   Structures and Typedefs                                                  *
 ******************************************************************************/

/** @todo *NOT* thread safe yet! */
/** @todo Add exception handling for STL stuff! */

GuestProcessStreamBlock::GuestProcessStreamBlock()
{

}

GuestProcessStreamBlock::~GuestProcessStreamBlock()
{
    Clear();
}

/**
 * Adds a key (if not existing yet).
 *
 * @return  IPRT status code.
 * @param   pszKey              Key name to add.
 */
int GuestProcessStreamBlock::AddKey(const char *pszKey)
{
    AssertPtrReturn(pszKey, VERR_INVALID_POINTER);
    /** @todo Add check for already existing keys! (VERR_ALREADY_EXISTS). */
    m_mapPairs[Utf8Str(pszKey)].pszValue = NULL;

    return VINF_SUCCESS;
}

/**
 * Destroys the currently stored stream pairs.
 *
 * @return  IPRT status code.
 */
void GuestProcessStreamBlock::Clear()
{
    for (GuestCtrlStreamPairsIter it = m_mapPairs.begin(); it != m_mapPairs.end(); it++)
    {
        if (it->second.pszValue)
            RTMemFree(it->second.pszValue);
    }

    m_mapPairs.clear();
}

/**
 * Returns a 64-bit signed integer of a specified key.
 *
 * @return  IPRT status code. VERR_NOT_FOUND if key was not found.
 * @param  pszKey               Name of key to get the value for.
 * @param  piVal                Pointer to value to return.
 */
int GuestProcessStreamBlock::GetInt64Ex(const char *pszKey, int64_t *piVal)
{
    AssertPtrReturn(pszKey, VERR_INVALID_POINTER);
    AssertPtrReturn(piVal, VERR_INVALID_POINTER);
    const char *pszValue = GetString(pszKey);
    if (pszValue)
    {
        *piVal = RTStrToInt64(pszValue);
        return VINF_SUCCESS;
    }
    return VERR_NOT_FOUND;
}

/**
 * Returns a 64-bit integer of a specified key.
 *
 * @return  int64_t             Value to return, 0 if not found / on failure.
 * @param   pszKey              Name of key to get the value for.
 */
int64_t GuestProcessStreamBlock::GetInt64(const char *pszKey)
{
    int64_t iVal;
    if (RT_SUCCESS(GetInt64Ex(pszKey, &iVal)))
        return iVal;
    return 0;
}

/**
 * Returns the current number of stream pairs.
 *
 * @return  uint32_t            Current number of stream pairs.
 */
size_t GuestProcessStreamBlock::GetCount()
{
    return m_mapPairs.size();
}


/**
 * Returns a string value of a specified key.
 *
 * @return  uint32_t            Pointer to string to return, NULL if not found / on failure.
 * @param   pszKey              Name of key to get the value for.
 */
const char* GuestProcessStreamBlock::GetString(const char *pszKey)
{
    AssertPtrReturn(pszKey, NULL);

    try
    {
        GuestCtrlStreamPairsIterConst itPairs = m_mapPairs.find(Utf8Str(pszKey));
        if (itPairs != m_mapPairs.end())
            return itPairs->second.pszValue;
    }
    catch (const std::exception &ex)
    {
        NOREF(ex);
    }
    return NULL;
}

/**
 * Returns a 32-bit unsigned integer of a specified key.
 *
 * @return  IPRT status code. VERR_NOT_FOUND if key was not found.
 * @param  pszKey               Name of key to get the value for.
 * @param  puVal                Pointer to value to return.
 */
int GuestProcessStreamBlock::GetUInt32Ex(const char *pszKey, uint32_t *puVal)
{
    AssertPtrReturn(pszKey, VERR_INVALID_POINTER);
    AssertPtrReturn(puVal, VERR_INVALID_POINTER);
    const char *pszValue = GetString(pszKey);
    if (pszValue)
    {
        *puVal = RTStrToUInt32(pszValue);
        return VINF_SUCCESS;
    }
    return VERR_NOT_FOUND;
}

/**
 * Returns a 32-bit unsigned integer of a specified key.
 *
 * @return  uint32_t            Value to return, 0 if not found / on failure.
 * @param   pszKey              Name of key to get the value for.
 */
uint32_t GuestProcessStreamBlock::GetUInt32(const char *pszKey)
{
    uint32_t uVal;
    if (RT_SUCCESS(GetUInt32Ex(pszKey, &uVal)))
        return uVal;
    return 0;
}

/**
 * Sets a value to a key or deletes a key by setting a NULL value.
 *
 * @return  IPRT status code.
 * @param   pszKey              Key name to process.
 * @param   pszValue            Value to set. Set NULL for deleting the key.
 */
int GuestProcessStreamBlock::SetValue(const char *pszKey, const char *pszValue)
{
    AssertPtrReturn(pszKey, VERR_INVALID_POINTER);

    int rc = VINF_SUCCESS;
    if (pszValue)
    {
        char *pszVal = RTStrDup(pszValue);
        if (pszVal)
            m_mapPairs[pszKey].pszValue = pszVal;
        else
            rc = VERR_NO_MEMORY;
    }
    else
    {
        GuestCtrlStreamPairsIter it = m_mapPairs.find(pszKey);
        if (it != m_mapPairs.end())
        {
            if (it->second.pszValue)
            {
                RTMemFree(it->second.pszValue);
                it->second.pszValue = NULL;
            }
            m_mapPairs.erase(it);
        }
    }
    return rc;
}

///////////////////////////////////////////////////////////////////////////////

GuestProcessStream::GuestProcessStream()
    : m_cbAllocated(0),
      m_cbSize(0),
      m_cbOffset(0),
      m_pbBuffer(NULL)
{

}

GuestProcessStream::~GuestProcessStream()
{
    Destroy();
}

/**
 * Adds data to the internal parser buffer. Useful if there
 * are multiple rounds of adding data needed.
 *
 * @return  IPRT status code.
 * @param   pbData              Pointer to data to add.
 * @param   cbData              Size (in bytes) of data to add.
 */
int GuestProcessStream::AddData(const BYTE *pbData, size_t cbData)
{
    AssertPtrReturn(pbData, VERR_INVALID_POINTER);
    AssertReturn(cbData, VERR_INVALID_PARAMETER);

    int rc = VINF_SUCCESS;

    /* Rewind the buffer if it's empty. */
    size_t     cbInBuf   = m_cbSize - m_cbOffset;
    bool const fAddToSet = cbInBuf == 0;
    if (fAddToSet)
        m_cbSize = m_cbOffset = 0;

    /* Try and see if we can simply append the data. */
    if (cbData + m_cbSize <= m_cbAllocated)
    {
        memcpy(&m_pbBuffer[m_cbSize], pbData, cbData);
        m_cbSize += cbData;
    }
    else
    {
        /* Move any buffered data to the front. */
        cbInBuf = m_cbSize - m_cbOffset;
        if (cbInBuf == 0)
            m_cbSize = m_cbOffset = 0;
        else if (m_cbOffset) /* Do we have something to move? */
        {
            memmove(m_pbBuffer, &m_pbBuffer[m_cbOffset], cbInBuf);
            m_cbSize = cbInBuf;
            m_cbOffset = 0;
        }

        /* Do we need to grow the buffer? */
        if (cbData + m_cbSize > m_cbAllocated)
        {
            size_t cbAlloc = m_cbSize + cbData;
            cbAlloc = RT_ALIGN_Z(cbAlloc, _64K);
            void *pvNew = RTMemRealloc(m_pbBuffer, cbAlloc);
            if (pvNew)
            {
                m_pbBuffer = (uint8_t *)pvNew;
                m_cbAllocated = cbAlloc;
            }
            else
                rc = VERR_NO_MEMORY;
        }

        /* Finally, copy the data. */
        if (RT_SUCCESS(rc))
        {
            if (cbData + m_cbSize <= m_cbAllocated)
            {
                memcpy(&m_pbBuffer[m_cbSize], pbData, cbData);
                m_cbSize += cbData;
            }
            else
                rc = VERR_BUFFER_OVERFLOW;
        }
    }

    return rc;
}

/**
 * Destroys the the internal data buffer.
 */
void GuestProcessStream::Destroy()
{
    if (m_pbBuffer)
    {
        RTMemFree(m_pbBuffer);
        m_pbBuffer = NULL;
    }

    m_cbAllocated = 0;
    m_cbSize = 0;
    m_cbOffset = 0;
}

/**
 * Returns the current offset of the parser within
 * the internal data buffer.
 *
 * @return  uint32_t            Parser offset.
 */
uint32_t GuestProcessStream::GetOffset()
{
    return m_cbOffset;
}

/**
 * Try to parse the next upcoming pair block within the internal
 * buffer. Old pairs from a previously parsed block will be removed first!
 *
 * @return  IPRT status code.
 */
int GuestProcessStream::ParseBlock(GuestProcessStreamBlock &streamBlock)
{
    AssertPtrReturn(m_pbBuffer, VINF_SUCCESS);
    AssertReturn(m_cbSize, VINF_SUCCESS);
    AssertReturn(m_cbOffset <= m_cbSize, VERR_INVALID_PARAMETER);

    int rc = VINF_SUCCESS;

    size_t uCur = m_cbOffset;
    for (;uCur < m_cbSize;)
    {
        const char *pszStart = (char*)&m_pbBuffer[uCur];
        const char *pszEnd = pszStart;

        /* Search end of current pair (key=value\0). */
        while (uCur++ < m_cbSize)
        {
            if (*pszEnd == '\0')
                break;
            pszEnd++;
        }

        size_t uPairLen = pszEnd - pszStart;
        if (uPairLen)
        {
            const char *pszSep = pszStart;
            while (   *pszSep != '='
                   &&  pszSep != pszEnd)
            {
                pszSep++;
            }

            /* No separator found (or incomplete key=value pair)? */
            if (   pszSep == pszStart
                || pszSep == pszEnd)
            {
                m_cbOffset =  uCur - uPairLen - 1;
                rc = VERR_MORE_DATA;
            }

            if (RT_FAILURE(rc))
                break;

            size_t uKeyLen = pszSep - pszStart;
            size_t uValLen = pszEnd - (pszSep + 1);

            /* Get key (if present). */
            if (uKeyLen)
            {
                Assert(pszSep > pszStart);
                char *pszKey = (char*)RTMemAllocZ(uKeyLen + 1);
                if (!pszKey)
                {
                    rc = VERR_NO_MEMORY;
                    break;
                }
                memcpy(pszKey, pszStart, uKeyLen);

                streamBlock.AddKey(pszKey);

                /* Get value (if present). */
                if (uValLen)
                {
                    Assert(pszEnd > pszSep);
                    char *pszVal = (char*)RTMemAllocZ(uValLen + 1);
                    if (!pszVal)
                    {
                        rc = VERR_NO_MEMORY;
                        break;
                    }
                    memcpy(pszVal, pszSep + 1, uValLen);

                    streamBlock.SetValue(pszKey, pszVal);
                    RTMemFree(pszVal);
                }

                RTMemFree(pszKey);

                m_cbOffset += uCur - m_cbOffset;
            }
        }
        else /* No pair detected, check for a new block. */
        {
            do
            {
                if (*pszEnd == '\0')
                {
                    m_cbOffset = uCur;
                    rc = VERR_MORE_DATA;
                    break;
                }
                pszEnd++;
            } while (++uCur < m_cbSize);
        }

        if (RT_FAILURE(rc))
            break;
    }

    RT_CLAMP(m_cbOffset, 0, m_cbSize);

    return rc;
}

