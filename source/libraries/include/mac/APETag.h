#pragma once

namespace APE
{

class CIO;

/**************************************************************************************************
APETag version history / supported formats

1.0 (1000) - Original APE tag spec.  Fully supported by this code.
2.0 (2000) - Refined APE tag spec (better streaming support, UTF encoding). Fully supported by this code.

Notes:
    - also supports reading of ID3v1.1 tags
    - all saving done in the APE Tag format using CURRENT_APE_TAG_VERSION
**************************************************************************************************/

/**************************************************************************************************
APETag layout

1) Header - APE_TAG_FOOTER (optional) (32 bytes)
2) Fields (array):
        Value Size (4 bytes)
        Flags (4 bytes)
        Field Name (? ANSI bytes -- requires NULL terminator -- in range of 0x20 (space) to 0x7E (tilde))
        Value ([Value Size] bytes)
3) Footer - APE_TAG_FOOTER (32 bytes)
**************************************************************************************************/

/**************************************************************************************************
Notes

When saving images, store the filename (no directory -- i.e. Cover.jpg) in UTF-8 followed
by a null terminator, followed by the image data.

What saving text lists, delimit the values with a NULL terminator.
**************************************************************************************************/

/**************************************************************************************************
The version of the APE tag
**************************************************************************************************/
#define CURRENT_APE_TAG_VERSION                 2000

/**************************************************************************************************
"Standard" APE tag fields
**************************************************************************************************/
#define APE_TAG_FIELD_TITLE                     L"Title"
#define APE_TAG_FIELD_ARTIST                    L"Artist"
#define APE_TAG_FIELD_ALBUM                     L"Album"
#define APE_TAG_FIELD_ALBUM_ARTIST              L"Album Artist"
#define APE_TAG_FIELD_COMMENT                   L"Comment"
#define APE_TAG_FIELD_YEAR                      L"Year"
#define APE_TAG_FIELD_TRACK                     L"Track"
#define APE_TAG_FIELD_DISC                      L"Disc"
#define APE_TAG_FIELD_GENRE                     L"Genre"
#define APE_TAG_FIELD_COVER_ART_FRONT           L"Cover Art (front)"
#define APE_TAG_FIELD_NOTES                     L"Notes"
#define APE_TAG_FIELD_LYRICS                    L"Lyrics"
#define APE_TAG_FIELD_COPYRIGHT                 L"Copyright"
#define APE_TAG_FIELD_BUY_URL                   L"Buy URL"
#define APE_TAG_FIELD_ARTIST_URL                L"Artist URL"
#define APE_TAG_FIELD_PUBLISHER_URL             L"Publisher URL"
#define APE_TAG_FIELD_FILE_URL                  L"File URL"
#define APE_TAG_FIELD_COPYRIGHT_URL             L"Copyright URL"
#define APE_TAG_FIELD_TOOL_NAME                 L"Tool Name"
#define APE_TAG_FIELD_TOOL_VERSION              L"Tool Version"
#define APE_TAG_FIELD_PEAK_LEVEL                L"Peak Level"
#define APE_TAG_FIELD_REPLAY_GAIN_RADIO         L"Replay Gain (radio)"
#define APE_TAG_FIELD_REPLAY_GAIN_ALBUM         L"Replay Gain (album)"
#define APE_TAG_FIELD_COMPOSER                  L"Composer"
#define APE_TAG_FIELD_KEYWORDS                  L"Keywords"
#define APE_TAG_FIELD_RATING                    L"Rating"
#define APE_TAG_FIELD_PUBLISHER                 L"Publisher"
#define APE_TAG_FIELD_BPM                       L"BPM"

/**************************************************************************************************
Standard APE tag field values
**************************************************************************************************/
#define APE_TAG_GENRE_UNDEFINED                 L"Undefined"

/**************************************************************************************************
ID3 v1.1 tag
**************************************************************************************************/
#define ID3_TAG_BYTES 128 // equal to sizeof(ID3_TAG)
#pragma pack(push, 1)
struct ID3_TAG
{
    char Header[3];             // should equal 'TAG'
    char Title[30];             // title
    char Artist[30];            // artist
    char Album[30];             // album
    char Year[4];               // year
    char Comment[29];           // comment
    unsigned char Track;        // track
    unsigned char Genre;        // genre
};
#pragma pack(pop)

/**************************************************************************************************
Footer (and header) flags
**************************************************************************************************/
#define APE_TAG_FLAG_CONTAINS_HEADER            ((unsigned int) 1 << 31)
#define APE_TAG_FLAG_CONTAINS_FOOTER            (1 << 30)
#define APE_TAG_FLAG_IS_HEADER                  (1 << 29)

#define APE_TAG_FLAGS_DEFAULT                   (APE_TAG_FLAG_CONTAINS_FOOTER)

/**************************************************************************************************
Tag field flags
**************************************************************************************************/
#define TAG_FIELD_FLAG_READ_ONLY                (1 << 0)

#define TAG_FIELD_FLAG_DATA_TYPE_MASK           (6)
#define TAG_FIELD_FLAG_DATA_TYPE_TEXT_UTF8      (0 << 1)
#define TAG_FIELD_FLAG_DATA_TYPE_BINARY         (1 << 1)
#define TAG_FIELD_FLAG_DATA_TYPE_EXTERNAL_INFO  (2 << 1)
#define TAG_FIELD_FLAG_DATA_TYPE_RESERVED       (3 << 1)

#pragma pack(push, 1)

/**************************************************************************************************
The footer at the end of APE tagged files (can also optionally be at the front of the tag)
**************************************************************************************************/
#ifdef APE_ENABLE_TAG_FOOTER
#define APE_TAG_FOOTER_BYTES    32
class APE_TAG_FOOTER
{
protected:
    char m_cID[8];              // should equal 'APETAGEX'
    int m_nVersion;             // equals CURRENT_APE_TAG_VERSION
    int m_nSize;                // the complete size of the tag, including this footer (excludes header)
    int m_nFields;              // the number of fields in the tag
    int m_nFlags;               // the tag flags
    char m_cReserved[8];        // reserved for later use (must be zero)

public:
    APE_TAG_FOOTER(int nFields = 0, int nFieldBytes = 0)
    {
        memcpy(m_cID, "APETAGEX", 8);
        APE_CLEAR(m_cReserved);
        m_nFields = nFields;
        m_nFlags = APE_TAG_FLAGS_DEFAULT;
        m_nSize = nFieldBytes + APE_TAG_FOOTER_BYTES;
        m_nVersion = CURRENT_APE_TAG_VERSION;
    }

    __forceinline int GetTotalTagBytes() { return m_nSize + (GetHasHeader() ? APE_TAG_FOOTER_BYTES : 0); }
    __forceinline int GetFieldBytes() { return m_nSize - APE_TAG_FOOTER_BYTES; }
    __forceinline int GetFieldsOffset() { return GetHasHeader() ? APE_TAG_FOOTER_BYTES : 0; }
    __forceinline int GetNumberFields() { return m_nFields; }
    __forceinline bool GetHasHeader() { return (m_nFlags & APE_TAG_FLAG_CONTAINS_HEADER) ? true : false; }
    __forceinline bool GetIsHeader() { return (m_nFlags & APE_TAG_FLAG_IS_HEADER) ? true : false; }
    __forceinline int GetVersion() { return m_nVersion; }
    __forceinline void Empty() { APE_CLEAR(m_cID); }
    __forceinline bool GetIsValid(bool bAllowHeader)
    {
        bool bValid = (strncmp(m_cID, "APETAGEX", 8) == 0) &&
            (m_nVersion <= CURRENT_APE_TAG_VERSION) &&
            (m_nFields <= 65536) &&
            (m_nSize >= APE_TAG_FOOTER_BYTES) &&
            (GetFieldBytes() <= (APE_BYTES_IN_MEGABYTE * 256));

        if (bValid && !bAllowHeader && GetIsHeader())
            bValid = false;

        return bValid ? true : false;
    }
};
#endif

/**************************************************************************************************
CAPETagField class (an APE tag is an array of these)
**************************************************************************************************/
class CAPETagField
{
public:
    // create a tag field (use nFieldBytes = -1 for null-terminated strings)
    CAPETagField(const str_utfn * pFieldName, const void * pFieldValue, int nFieldBytes = -1, int nFlags = 0);

    // destruction
    virtual ~CAPETagField();

    // gets the size of the entire field in bytes (name, value, and metadata)
    int GetFieldSize() const;

    // get the name of the field
    const str_utfn * GetFieldName() const;

    // get the value of the field
    const char * GetFieldValue();

    // get the size of the value (in bytes)
    int GetFieldValueSize();

    // get any special flags
    int GetFieldFlags();

    // output the entire field to a buffer (GetFieldSize() bytes)
    int SaveField(char * pBuffer, int nBytes);

    // checks to see if the field is read-only
    bool GetIsReadOnly();
    bool GetIsUTF8Text();

    // set helpers (use with EXTREME caution)
    void SetFieldFlags(int nFlags);

private:
    // helpers
    void Save32(char* pBuffer, int nValue);

    // data
    CSmartPtr<str_utfn> m_spFieldNameUTF16;
    CSmartPtr<char> m_spFieldValue;
    int m_nFieldFlags;
    int m_nFieldValueBytes;
};

/**************************************************************************************************
IAPETag interface
**************************************************************************************************/
class IAPETag
{
public:

    virtual ~IAPETag() { }

    // save the tag to the I/O source (bUseOldID3 forces it to save as an ID3v1.1 tag instead of an APE tag)
    virtual int Save(bool bUseOldID3 = false) = 0;

    // removes any tags from the file (bUpdate determines whether is should re-analyze after removing the tag)
    virtual int Remove(bool bUpdate = true) = 0;

    // sets the value of a field (use nFieldBytes = -1 for null terminated strings)
    // note: using NULL or "" for a string type will remove the field
    virtual int SetFieldString(const str_utfn * pFieldName, const str_utfn * pFieldValue, const str_utfn * pListDelimiter = APE_NULL) = 0;
    virtual int SetFieldString(const str_utfn * pFieldName, const char * pFieldValue, bool bAlreadyUTF8Encoded, const str_utfn * pListDelimiter = APE_NULL) = 0;
    virtual int SetFieldBinary(const str_utfn * pFieldName, const void * pFieldValue, intn nFieldBytes, int nFieldFlags) = 0;

    // gets the value of a field (returns -1 and an empty buffer if the field doesn't exist)
    virtual int GetFieldBinary(const str_utfn * pFieldName, void * pBuffer, int * pBufferBytes) = 0;
    virtual int GetFieldString(const str_utfn * pFieldName, str_utfn * pBuffer, int * pBufferCharacters, const str_utfn * pListDelimiter = L"; ") = 0;
    virtual int GetFieldString(const str_utfn * pFieldName, str_ansi * pBuffer, int * pBufferCharacters, bool bUTF8Encode = false) = 0;

    // remove a specific field
    virtual int RemoveField(const str_utfn * pFieldName) = 0;
    virtual int RemoveField(int nIndex) = 0;

    // clear all the fields
    virtual int ClearFields() = 0;

    // see if we've been analyzed (we do lazy analysis)
    virtual bool GetAnalyzed() = 0;

    // get the total tag bytes in the file from the last analyze
    // need to call Save() then Analyze() to update any changes
    virtual int GetTagBytes() = 0;

    // fills in an ID3_TAG using the current fields (useful for quickly converting the tag)
    virtual int CreateID3Tag(ID3_TAG * pID3Tag) = 0;

    // see whether the file has an ID3 or APE tag
    virtual bool GetHasID3Tag() = 0;
    virtual bool GetHasAPETag() = 0;
    virtual int GetAPETagVersion() = 0;

    // gets a desired tag field (returns NULL if not found)
    // again, be careful, because this a pointer to the actual field in this class
    virtual CAPETagField* GetTagField(const str_utfn * pFieldName) = 0;
    virtual CAPETagField* GetTagField(int nIndex) = 0;

    // options
    virtual void SetIgnoreReadOnly(bool bIgnoreReadOnly) = 0;

};

/**************************************************************************************************
CAPETag class
**************************************************************************************************/
class CAPETag : public IAPETag
{
public:
    // create an APE tag
    // bAnalyze determines whether it will analyze immediately or on the first request
    // be careful with multiple threads / file pointer movement if you don't analyze immediately
    CAPETag(CIO * pIO, bool bAnalyze = true, bool bCheckForID3v1 = true);
    CAPETag(const str_utfn * pFilename, bool bAnalyze = true);

    // destructor
    virtual ~CAPETag();

    // save the tag to the I/O source (bUseOldID3 forces it to save as an ID3v1.1 tag instead of an APE tag)
    int Save(bool bUseOldID3 = false);

    // removes any tags from the file (bUpdate determines whether is should re-analyze after removing the tag)
    int Remove(bool bUpdate = true);

    // sets the value of a field (use nFieldBytes = -1 for null terminated strings)
    // note: using NULL or "" for a string type will remove the field
    int SetFieldString(const str_utfn * pFieldName, const str_utfn * pFieldValue, const str_utfn * pListDelimiter = APE_NULL);
    int SetFieldString(const str_utfn * pFieldName, const char * pFieldValue, bool bAlreadyUTF8Encoded, const str_utfn * pListDelimiter = APE_NULL);
    int SetFieldBinary(const str_utfn * pFieldName, const void * pFieldValue, intn nFieldBytes, int nFieldFlags);

    // gets the value of a field (returns -1 and an empty buffer if the field doesn't exist)
    int GetFieldBinary(const str_utfn * pFieldName, void * pBuffer, int * pBufferBytes);
    int GetFieldString(const str_utfn * pFieldName, str_utfn * pBuffer, int * pBufferCharacters, const str_utfn * pListDelimiter = L"; ");
    int GetFieldString(const str_utfn * pFieldName, str_ansi * pBuffer, int * pBufferCharacters, bool bUTF8Encode = false);

    // remove a specific field
    int RemoveField(const str_utfn * pFieldName);
    int RemoveField(int nIndex);

    // clear all the fields
    int ClearFields();

    // see if we've been analyzed (we do lazy analysis)
    bool GetAnalyzed();

    // get the total tag bytes in the file from the last analyze
    // need to call Save() then Analyze() to update any changes
    int GetTagBytes();

    // fills in an ID3_TAG using the current fields (useful for quickly converting the tag)
    int CreateID3Tag(ID3_TAG * pID3Tag);

    // see whether the file has an ID3 or APE tag
    bool GetHasID3Tag();
    bool GetHasAPETag();
    int GetAPETagVersion();

    // gets a desired tag field (returns NULL if not found)
    // again, be careful, because this a pointer to the actual field in this class
    CAPETagField * GetTagField(const str_utfn * pFieldName);
    CAPETagField * GetTagField(int nIndex);

    // options
    void SetIgnoreReadOnly(bool bIgnoreReadOnly);

    // statics
    static const int s_nID3GenreUndefined = 255;
    static const int s_nID3GenreCount = 148;
    static const wchar_t * s_aryID3GenreNames[s_nID3GenreCount];

private:
    // private functions
    int Analyze();
    int GetTagFieldIndex(const str_utfn * pFieldName);
    int WriteBufferToEndOfIO(void * pBuffer, int nBytes);
    int LoadField(const char * pBuffer, int nMaximumBytes, int * pBytes);
    int SortFields();
    static int CompareFields(const void * pA, const void * pB);

    // helper set / get field functions
    int SetFieldID3String(const str_utfn * pFieldName, const char * pFieldValue, int nBytes);
    int GetFieldID3String(const str_utfn * pFieldName, char * pBuffer, int nBytes);

    // private data
    CSmartPtr<CIO> m_spIO;
    int m_nTagBytes;
    int m_nFields;
    int m_nAllocatedFields;
    int m_nAPETagVersion;
    CAPETagField ** m_aryFields;
    bool m_bHasAPETag;
    bool m_bAnalyzed;
    bool m_bHasID3Tag;
    bool m_bIgnoreReadOnly;
    bool m_bCheckForID3v1;
};

#pragma pack(pop)

}
