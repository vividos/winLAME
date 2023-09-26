#pragma once

namespace APE
{

#pragma pack(push, 1)

/**************************************************************************************************
CSmartPtr - a simple smart pointer class that can automatically initialize and free memory
    note: (doesn't do garbage collection / reference counting because of the many pitfalls)
**************************************************************************************************/
template <class TYPE> class CSmartPtr
{
public:
    TYPE * m_pObject;
    bool m_bArray;
    bool m_bDelete;

    __forceinline CSmartPtr()
    {
        m_bDelete = true;
        m_pObject = APE_NULL;
        m_bArray = false;
    }
    __forceinline CSmartPtr(TYPE * pObject, bool bArray = false, bool bDelete = true)
    {
        m_bDelete = true;
        m_pObject = APE_NULL;
        m_bArray = false;
        Assign(pObject, bArray, bDelete);
    }

    __forceinline ~CSmartPtr()
    {
        Delete();
    }

    __forceinline void Assign(TYPE * pObject, bool bArray = false, bool bDelete = true)
    {
        Delete();

        m_bDelete = bDelete;
        m_bArray = bArray;
        m_pObject = pObject;
    }

    __forceinline void Delete()
    {
        if (m_bDelete && m_pObject)
        {
            TYPE * pObject = m_pObject;
            m_pObject = APE_NULL;

            if (m_bArray)
                delete [] pObject;
            else
                delete pObject;
        }
    }

    void SetDelete(const bool bDelete)
    {
        m_bDelete = bDelete;
    }

    __forceinline TYPE * GetPtr() const
    {
        return m_pObject;
    }

    __forceinline operator TYPE * () const
    {
        return m_pObject;
    }

    __forceinline TYPE * operator ->() const
    {
        return m_pObject;
    }

    // declare assignment, but don't implement (compiler error if we try to use)
    // that way we can't carelessly mix smart pointers and regular pointers
    __forceinline void * operator =(void *) const;
};

#pragma pack(pop)

}
