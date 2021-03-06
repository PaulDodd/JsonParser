//
//  json_wrapper.h
//  Created by Paul M Dodd
//  05/06/2014
//

#ifndef JSON_WRAPPER_H
#define JSON_WRAPPER_H

// the only include for json. the following link shows how to install
// https://github.com/akheron/jansson/blob/2.6/doc/gettingstarted.rst
#include <jansson.h>

// Standard library headers
#include <cmath>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <map>
#include <stdarg.h>
#include <memory>

#if __cplusplus >= 201103L
// Needed for the std::tuple class.
#ifndef c_plus_plus_11
    #define c_plus_plus_11
#endif

#include <tuple>
#include <type_traits>
#include <utility>

#endif


namespace json {
using namespace std;

template<class T>
struct memory_ptr
{
    #ifdef c_plus_plus_11
        typedef std::unique_ptr<T>  value_type;
    #else
        typedef std::auto_ptr<T>    value_type;
    #endif
};




// TODOs:
// 1.   make general numerical class with template. and that all numerical classes are compatible.
//      this will make certian use cases possible and code more flexible. (check)
//
// 2.   jansson manages memory through reference counts.  Need to double check that all are
//      being handeled correctly.
//
// 3.   Parser class to have a owned pointer and a unowned pointer
//
// 4.   Parser update json functionality.
//
// 5.   Write unit tests to test all the different classes.
//
// 6.   JSON Tuple class generalized (check)
//
// 7.   Think if there is a good way to combine the pointer classes and the smart pointer classes. ( std::enable_if ...)
//
// 8.   CJSONValue permissions: read permission, write permission, update permission to help support legacy file formats
//

#define JSON_OBJECT_TRACK_MISSING_VALUES_DEFAULT true

template<class DerivedClass> class CJSONValueObject;


class CJSONValue
{
    public:
        CJSONValue(json_type type, const std::string& name)
        {
            m_type = type;
            m_name = name;
            m_pJValue = NULL;
        }
        virtual ~CJSONValue()
        {
            ClearJValue();
        }

    // Abstract methods
        virtual bool Parse (const json_t* pVal) = 0;
        virtual bool Dump (json_t*& pRet) = 0;

        virtual void Setup(size_t argc, ...) { cout << "passed in "<< argc << " arguments." << endl; } // to make virtual abstract?
    // Class Method
        void ClearJValue()
        {
            json_decref( m_pJValue );
            m_pJValue = NULL;
        }
    // Accessor Methods
        const std::string& GetName() const { return m_name; }
        void SetName(std::string name ) { m_name = name; }

        bool IsInt()    { return m_type == JSON_INTEGER; }
        bool IsFloat()  { return m_type == JSON_REAL; }
        bool IsString() { return m_type == JSON_STRING; }
        bool IsArray()  { return m_type == JSON_ARRAY; }
        bool IsObject() { return m_type == JSON_OBJECT; }
        bool IsBool()   { return m_type == JSON_TRUE; }
        bool IsNull()   { return m_type == JSON_NULL; }

        std::string TypeToString()
        {
            std::string type;
            if(IsInt())
            {
                type = "integer";
            }
            else if(IsFloat())
            {
                type = "float";
            }
            else if(IsString())
            {
                type = "string";
            }
            else if(IsArray())
            {
                type = "array";
            }
            else if(IsObject())
            {
                type = "object";
            }
            else if(IsBool())
            {
                type = "bool";
            }
            else if(IsNull())
            {
                type = "null";
            }

            return type;

        }

    protected:
        json_t*         m_pJValue;  // buffer to hold values for dump on create in the dump command.
        json_type       m_type;
        std::string     m_name;
};

template< class NVal, json_type _type_ >
class CJSONValueNumber : public CJSONValue // may need an unsigned version of this class.
{
    public:
        typedef NVal type;

        CJSONValueNumber(const std::string& name, NVal * pval, const NVal& defaultVal = 0) : CJSONValue(_type_, name), m_DefaultValue(defaultVal), m_pValue(pval)  {}
        ~CJSONValueNumber() {}

    // Overloaded Methods
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;

            if(json_is_number(pVal))
            {
                bParseSuccess = true;
                *m_pValue = NVal(json_number_value(pVal)); // Always casts to a double so we have to cast it back.
            }
            else{
                fprintf(stderr, "ERROR: %s is not an number (%s) as expected. \n", m_name.c_str(), TypeToString().c_str());
            }
            return bParseSuccess;
        }

        bool Dump (json_t*& pRet)
        {
            ClearJValue();
            if( std::isnan(*m_pValue) || std::isinf(*m_pValue))
            {
                cout << "Warning! trying to dump nan/inf value to " << m_name << endl;
            }
            if(IsInt())
            {
                pRet = json_integer(*m_pValue);
            }
            else
            {
                pRet = json_real(*m_pValue);
            }

            m_pJValue = pRet;
            return pRet != NULL;
        }

    // Accessor Methods
        const NVal& GetValue() const { return *m_pValue; }
        const NVal& GetDefaultValue() const { return m_DefaultValue; }

    private:
        NVal  m_DefaultValue;
        NVal* m_pValue;
};

typedef CJSONValueNumber<int, JSON_INTEGER>         CJSONValueInt;
typedef CJSONValueNumber<size_t, JSON_INTEGER>      CJSONValueUInt;
typedef CJSONValueNumber<float, JSON_REAL>          CJSONValueFloat;
typedef CJSONValueNumber<double, JSON_REAL>         CJSONValueDouble;

class CJSONValueString : public CJSONValue
{
    public:
        typedef std::string type;

        CJSONValueString(const std::string& name, std::string* pval, const std::string& defaultVal = "") : CJSONValue(JSON_STRING, name),  m_DefaultValue(defaultVal), m_pValue(pval) {}
        ~CJSONValueString() {}

        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_string(pVal))
            {
                bParseSuccess = true;
                *m_pValue = json_string_value(pVal);
            }
            else{
                fprintf(stderr, "ERROR: %s is not an std::string as expected. \n", m_name.c_str());
            }
            return bParseSuccess;
        }

        bool Dump (json_t*& pRet)
        {
            ClearJValue();
            pRet = json_string(m_pValue->c_str());
            if(!pRet) std::cout << "Error could not dump std::string " << m_name << std::endl;
            m_pJValue = pRet;
            return pRet != NULL;
        }

        const std::string& GetValue() const { return *m_pValue; }
        const std::string& GetDefaultValue() const { return m_DefaultValue; }

    private:
        std::string  m_DefaultValue;
        std::string* m_pValue;
};

class CJSONValueBool : public CJSONValue
{
    public:
        typedef bool type;

        CJSONValueBool(const std::string& name, bool * pval, const bool& defaultVal = false) : CJSONValue(JSON_TRUE, name), m_DefaultValue(defaultVal), m_pValue(pval)  {}
        ~CJSONValueBool() {}

        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_boolean(pVal))
            {
                bParseSuccess = true;
                *m_pValue = json_is_true(pVal);
            }
            else{
                fprintf(stderr, "ERROR: %s is not a boolean as expected. \n", m_name.c_str());
            }
            return bParseSuccess;
        }

        bool Dump (json_t*& pRet)
        {
            ClearJValue();
            pRet = (*m_pValue) ? json_true() : json_false();
            m_pJValue = pRet;
            return pRet != NULL;
        }

        const bool& GetValue() const { return *m_pValue; }
        const bool& GetDefaultValue() const { return m_DefaultValue; }

    private:
        bool  m_DefaultValue;
        bool* m_pValue;
};


/*
NOTE: the class below will be deprecated and the array class at the bottom will be a more generalized form for all array types.
*/

// Will have to think about how to make this work for fixed array types of containers.
// Implemeted to work with the std::vector class for ease.
// ?? Could/Should make a CJSONValueFixedArray<> class and change the name of this class to be CJSONValueDynamicArray or CJSONValueVector ??
// TVal is object then we will need to call the SetupObjectClass.

template <class TVal, class JVal>
class CJSONValueArray : public CJSONValue
{
    public:
        typedef std::vector<TVal> type;

        CJSONValueArray(const std::string& name, std::vector<TVal>* pval, const std::vector<TVal>& defaultVal = std::vector<TVal>()) : CJSONValue(JSON_ARRAY, name), m_pValue(pval), m_DefaultValue(defaultVal)
        {
            m_DefaultArrayValue = JVal("", NULL).GetDefaultValue();
        }

        CJSONValueArray(const CJSONValueArray& src ) : CJSONValue(JSON_ARRAY, src.GetName())
        {
            CopyFrom(src);
        }

        ~CJSONValueArray() {}

        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_array(pVal))
            {
                bParseSuccess = true;
                size_t n = json_array_size(pVal);
                json_t* data;

                for (size_t i = 0; i < n; i++)
                {
                    TVal temp = m_DefaultArrayValue;

                    char array_number[30]; // should be enough space.
                    sprintf(&array_number[0], "-%zu", i);
                    JVal tjson((m_name + std::string(array_number)), &temp);
                    data = json_array_get(pVal, i);
                    bParseSuccess = tjson.Parse(data) && bParseSuccess;

                    m_pValue->push_back(temp);
                }
            }
            else{
                fprintf(stderr, "ERROR: %s is not an array as expected. \n", m_name.c_str());
            }

            return bParseSuccess;
        }

        bool Dump (json_t*& pRet)
        {
            ClearJValue();
            bool bDumpSuccess = true;
            pRet = json_array();
            if(pRet)
            {
                for( size_t i = 0; i < m_pValue->size(); i++)
                {
                    json_t* pVal = NULL;
                    TVal temp = m_pValue->at(i);

                    JVal tjson("", &temp);

                    bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;

                    if(pVal)
                        bDumpSuccess = (json_array_append(pRet, pVal) != -1) && bDumpSuccess;
                    else
                        std::cout << "Error could not dump array element. "<< m_name << "-"<< i << std::endl;
                }
            }
            else
            {
                bDumpSuccess = false;
                std::cout << "Error! Could not dump array. "<< m_name << std::endl;
            }
            m_pJValue = pRet;
            return bDumpSuccess;
        }


        const CJSONValueArray& CopyFrom( const CJSONValueArray& src )
        {
           m_pValue = &src.GetValue();
           m_DefaultValue = src.GetDefaultValue();
        }

    // Accessor Methods
        const TVal& GetValue() const { return *m_pValue; }
        const std::vector<TVal>& GetDefaultValue() const { return m_DefaultValue; }
    private:
        std::vector<TVal>*  m_pValue;
        std::vector<TVal>   m_DefaultValue;
        TVal                m_DefaultArrayValue;
};

template <class TVal>
class CJSONValueArray<TVal, CJSONValueObject<TVal> > : public CJSONValue
{
    public:
        typedef std::vector<TVal> type;

        CJSONValueArray(const std::string& name, std::vector<TVal>* pval, const std::vector<TVal>& defaultVal = std::vector<TVal>()) : CJSONValue(JSON_ARRAY, name), m_pValue(pval), m_DefaultValue(defaultVal)
        {
        }

        CJSONValueArray(const CJSONValueArray& src ) : CJSONValue(JSON_ARRAY, src.GetName())
        {
            CopyFrom(src);
        }

        ~CJSONValueArray() {}

        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_array(pVal))
            {
                bParseSuccess = true;
                size_t n = json_array_size(pVal);
                json_t* data = NULL;

                for (size_t i = 0; i < n; i++)
                {
                    TVal temp;
                    temp.SetupJSONObject();

                    char array_number[30]; // should be enough space.
                    sprintf(&array_number[0], "-%zu", i);
                    temp.SetName(m_name + "-" + std::string(array_number));
                    data = json_array_get(pVal, i);
                    bParseSuccess = temp.Parse(data) && bParseSuccess;
                    m_pValue->push_back(temp);
                }
            }
            else{
                fprintf(stderr, "ERROR: %s is not an array as expected. \n", m_name.c_str());
            }
            return bParseSuccess;
        }

        bool Dump (json_t*& pRet)
        {
            ClearJValue();
            bool bDumpSuccess = true;
            pRet = json_array();
            if(pRet)
            {
                for( size_t i = 0; i < m_pValue->size(); i++)
                {
                    json_t* pVal = NULL;
                    m_pValue->at(i).SetupJSONObject();
                    bDumpSuccess = m_pValue->at(i).Dump(pVal) && bDumpSuccess;

                    if(pVal)
                        bDumpSuccess = (json_array_append(pRet, pVal) != -1) && bDumpSuccess;
                    else
                        std::cout << "Error could not dump array element. "<< m_name << "-"<< i << std::endl;
                }
            }
            else
            {
                bDumpSuccess = false;
                std::cout << "Error! Could not dump array. "<< m_name << std::endl;
            }
            m_pJValue = pRet;
            return bDumpSuccess;
        }


        const CJSONValueArray& CopyFrom( const CJSONValueArray& src )
        {
           m_pValue = src.GetValue();
           m_DefaultValue = src.GetDefaultValue();
        }

    // Accessor Methods
        const std::vector<TVal>* GetValue() const { return m_pValue; }
        const std::vector<TVal>& GetDefaultValue() const { return m_DefaultValue; }
    private:
        std::vector<TVal>*  m_pValue;
        std::vector<TVal>   m_DefaultValue;
};


/*
End Note
*/


// This requires c++11.
#ifdef c_plus_plus_11

template< typename CVal, typename... TVals >
class CJSONValueTuple : public CJSONValue
{
    public:
        typedef CVal type;

        CJSONValueTuple(const std::string& name, CVal* pval, const CVal&& defaultVal = CVal()) : CJSONValue(JSON_ARRAY, name), m_pValue(pval), m_DefaultValue(defaultVal)
        {
        }

        ~CJSONValueTuple() {}
    // Overloaded Methods
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_array(pVal))
            {
                bParseSuccess = ParseTupleElements(pVal);
            }
            else{
                fprintf(stderr, "ERROR: %s is not an array as expected. \n", m_name.c_str());
            }

            return bParseSuccess;
        }

        bool Dump (json_t*& pRet)
        {
            ClearJValue();
            bool bDumpSuccess = true;
            pRet = json_array();
            if(pRet)
            {
                bDumpSuccess = DumpTupleElements(pRet);
            }
            else
            {
                bDumpSuccess = false;
                std::cout << "Error! Could not dump array. "<< m_name << std::endl;
            }
            m_pJValue = pRet;
            return bDumpSuccess;
        }


        const CVal& GetDefaultValue() { return m_DefaultValue; }

    private:
    // std::tuple utility functions.
        template<size_t I = 0>
        typename  std::enable_if<I == sizeof...(TVals), bool >::type ParseTupleElements(const json_t*) { return true; } // All values have been parsed...

        template<size_t I = 0>
        typename  std::enable_if< I < sizeof...(TVals), bool >::type ParseTupleElements(const json_t* pVal)
        {
            bool bParseSuccess = false;

            json_t* data;

            char array_number[30]; // should be enough space.
            sprintf(&array_number[0], "-%zu", I);
            std::string elemName(m_name + std::string(array_number));

            data = json_array_get(pVal, I);
            typename std::tuple_element<I, CVal>::type* pElem = &std::get<I>(*m_pValue);
            std::unique_ptr<CJSONValue> pJson = CreateJSONValue<typename std::tuple_element<I, CVal>::type, TVals...>(I, elemName, pElem);
            bParseSuccess = pJson->Parse(data);

            return ParseTupleElements<I+1>(pVal) && bParseSuccess;
        }

        template<size_t I = 0>
        typename  std::enable_if<I == sizeof...(TVals), bool >::type DumpTupleElements(json_t*&) { return true; } // All values have been dumped...

        template<size_t I = 0>
        typename  std::enable_if< I < sizeof...(TVals), bool >::type DumpTupleElements(json_t*& pRet)
        {
            bool bDumpSuccess = true;
            char array_number[30]; // should be enough space.
            sprintf(&array_number[0], "-%zu", I);
            std::string elemName(m_name + std::string(array_number));

            json_t* pVal = NULL;
            auto pElem = &std::get<I>(*m_pValue);

            std::unique_ptr<CJSONValue> pJson = CreateJSONValue<typename std::tuple_element<I, CVal>::type, TVals...>(I, elemName, pElem);
            pJson->Dump(pVal);

            if(pVal)
                bDumpSuccess = (json_array_append(pRet, pVal) != -1) && bDumpSuccess;
            else
                std::cout << "Error could not dump array element. " << m_name << "[" << I << "]" << std::endl;

            return DumpTupleElements<I+1>(pRet) && bDumpSuccess;
        }

        template<class Type1, class Type2, class JType>
        typename  std::enable_if< !std::is_same<Type1, Type2>::value, std::unique_ptr<CJSONValue> >::type create(const std::string&, Type1*)
        {
            return std::unique_ptr<CJSONValue>(nullptr);
        }

        template<class Type1, class Type2, class JType>
        typename  std::enable_if< std::is_same<Type1, Type2>::value, std::unique_ptr<CJSONValue> >::type create(const std::string& name, Type1* pVal)
        {
            return std::unique_ptr<CJSONValue>(new JType(name, pVal));
        }

        template<typename DataType, typename Type, typename... Others>
        typename  std::enable_if< 0 == sizeof...(Others), std::unique_ptr<CJSONValue> >::type CreateJSONValue(const size_t& Index, const std::string& name, DataType* pVal, size_t counter = 0)
        {
            if(Index == counter){
                return create<DataType, typename Type::type, Type>(name, pVal);
            }
            return std::unique_ptr<CJSONValue>(nullptr);
        }

        template<typename DataType, typename Type, typename... Others>
        typename  std::enable_if< 0 < sizeof...(Others), std::unique_ptr<CJSONValue> >::type CreateJSONValue(const size_t& Index, const std::string& name, DataType* pVal, size_t counter = 0)
        {
            if(Index == counter){
                return create<DataType, typename Type::type, Type>(name, pVal);
            }
            return CreateJSONValue<DataType, Others...>(Index, name, pVal, ++counter);
        }

    private:
        CVal*       m_pValue;
        CVal        m_DefaultValue;
};

#endif // end c++11 specialization

template <class DerivedClass>
class CJSONValueObject : public CJSONValue
{
    public:
        typedef DerivedClass type;

        CJSONValueObject(const std::string& name, DerivedClass* pval) : CJSONValue(JSON_OBJECT, name), m_pDerived(pval), m_bUpdate(JSON_OBJECT_TRACK_MISSING_VALUES_DEFAULT) {  }

    /* Want to delete any way of copying this object -- is there any other way? */
    #ifdef c_plus_plus_11
        CJSONValueObject( const CJSONValueObject<DerivedClass>& src) = delete;
        CJSONValueObject<DerivedClass>& operator=(const CJSONValueObject<DerivedClass>& src) = delete;
    #else
    private:
        CJSONValueObject(const CJSONValueObject<DerivedClass>& src);
        CJSONValueObject<DerivedClass>& operator=(const CJSONValueObject<DerivedClass>& src);
    public:
    #endif
    /****************************************************************************/
        ~CJSONValueObject() { Destroy(); }

        void Destroy()
        {

            size_t ct = 0;
            for(std::map<std::string, CJSONValue* >::iterator iter = m_Map.begin(); iter != m_Map.end(); iter++)
            {
                if(!iter->second->IsObject()) // the memory was created by this class for all but objects.
                {
                    if(iter->second)
                        delete iter->second;
                    iter->second = NULL;
                    ct++;
                }
            }
            m_Map.clear();
            for(std::map<std::string, json_t* >::iterator iter = m_MissingValues.begin(); iter != m_MissingValues.end(); iter++)
            {
                json_decref(iter->second);
            }
            m_MissingValues.clear();
        }

        void ClearBuffer()
        {
            ClearJValue();
            std::map<std::string, CJSONValue* >::iterator iter;
            for(iter = m_Map.begin(); iter != m_Map.end(); iter++)
            {
                iter->second->ClearJValue();
            }
        }

    // Inherited Abstract Methods.
        virtual bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_object(pVal))
            {
//                bParseSuccess = true;
//                json_t* data;
//                std::map<std::string, CJSONValue* >::iterator iter;
//                for(iter = m_Map.begin(); iter != m_Map.end(); iter++)
//                {
//                    data = json_object_get(pVal, iter->first.c_str());
//                    if(data)
//                        bParseSuccess = iter->second->Parse(data) && bParseSuccess;
//                    else{
//                        //std::cout << "Key (" << iter->first << ") was not found in file." << std::endl;
//                    }
//                }

                bParseSuccess = true;
                const char * key;
                json_t* val;
                json_object_foreach((json_t*)pVal, key, val)
                {
                    string name(key);
                    std::map<std::string, CJSONValue* >::iterator elem;
                    elem = m_Map.find(name);
                    if(elem != m_Map.end())
                    {
                        bParseSuccess = elem->second->Parse(val) && bParseSuccess;
                    }
                    else if(m_bUpdate)
                    {
                        json_incref(val); // keep the data around.
                        m_MissingValues.insert(pair<string, json_t*>(name, val));
                    }
                }
            }
            else
            {
                fprintf(stderr, "ERROR: %s is not an object as expected. \n", m_name.c_str());
            }

            return bParseSuccess;
        }

        virtual bool Dump (json_t*& pRet)
        {
            ClearJValue();
            bool bDumpSuccess = true;

            pRet = json_object();

            if(pRet)
            {
                for(std::map<std::string, CJSONValue* >::iterator iter = m_Map.begin(); iter != m_Map.end(); iter++)
                {
                    json_t* value = NULL;
                    if ( iter->second->Dump(value) )
                    {
                        if ( json_object_set(pRet, iter->first.c_str(), value) == -1)
                        {
                            bDumpSuccess = false;
                            std::cout << "Error! Could not add " << iter->first << " to object of size "<< json_object_size(pRet) << std::endl;
                        }
                    }
                    else
                    {
                        bDumpSuccess = false;
                        std::cout << "Error! Could not dump " << iter->first << std::endl;
                    }
                }

                if(bDumpSuccess && m_bUpdate) // add the missing data
                {
                    for(std::map<std::string, json_t* >::iterator iter = m_MissingValues.begin(); iter != m_MissingValues.end(); iter++)
                    {
                        if ( json_object_set(pRet, iter->first.c_str(), iter->second) == -1)
                        {
                            bDumpSuccess = false;
                            std::cout << "Error! Could not add " << iter->first << " to object of size "<< json_object_size(pRet) << std::endl;
                        }
                    }
                }
            }
            else
            {
                std::cout << "Error! could not create object!" << std::endl;
                bDumpSuccess = false;
            }
            m_pJValue = pRet;
            return bDumpSuccess;
        }

    // Abstract methods
        virtual void SetupJSONObject() = 0;

        virtual bool LoadFromFile( const std::string& Path ); // not abstract. implementation below.

        virtual bool SaveToFile( const std::string& Path );   // not abstract. implementation below.

    // Class methods
        template<class TVal, class JVal>
        void AddNameValuePair(const std::string& name, TVal* pval)
        {
            std::map < std::string, CJSONValue* >::iterator iter = m_Map.find(name);
            if(iter == m_Map.end())
            {
                m_Map.insert( std::pair< std::string, CJSONValue* >(name, new JVal(name, pval)));
            }
            else
            {
                //std::cout << "Already a key named "<< name << " is in the object. No action taken."<< std::endl;
            }
        }

        // Could Remove the following becasuse of the encompassing method above.
        void AddIntegerValue(const std::string& name, int* pval)
        {
            AddNameValuePair<int, CJSONValueInt>(name, pval);
        }
        void AddUIntegerValue(const std::string& name, size_t* pval)
        {
            AddNameValuePair<size_t, CJSONValueUInt>(name, pval);
        }
        void AddBoolValue(const std::string& name, bool* pval)
        {
            AddNameValuePair<bool, CJSONValueBool>(name, pval);
        }
        void AddFloatingPointValue(const std::string& name, double* pval)
        {
            AddNameValuePair<double, CJSONValueDouble>(name, pval);
        }
        void AddStringValue(const std::string& name, std::string* pval)
        {
            AddNameValuePair<std::string, CJSONValueString>(name, pval);
        }
        void AddStringArrayValue(const std::string& name, std::vector<std::string>* pval)
        {
            AddNameValuePair<std::vector<std::string>, CJSONValueArray<std::string, CJSONValueString> >(name, pval);
        }

        template<class TVal>
        void AddObjectValue(const std::string& name, CJSONValueObject<TVal>* pval)
        {
            std::map < std::string, CJSONValue* >::iterator iter = m_Map.find(name);
            if(iter == m_Map.end())
            {
                m_Map.insert(std::pair<std::string, CJSONValue* >(name,  pval));
            }
            else
            {
                //std::cout << "Already a key named "<< name << " is in the object. No action taken."<< std::endl;
            }
        }

        DerivedClass* GetDerived()  { return m_pDerived; }
        const DerivedClass& GetDefaultValue()   { return *m_pDerived; }

    private:
        DerivedClass*                               m_pDerived;
        std::map < std::string, CJSONValue* >       m_Map;              // map for each element in the object at this level. How to access data?
        bool                                        m_bUpdate;
        map<string, json_t*>                        m_MissingValues;
};


//                  CJSONValuePointer
//********************************************************************//
// TVal is a data type that can be parsed by the corresponding
// json class JVal. It is intended to allow for flexibility in
// parsing and different coding styles. Some special handling
// of memory will be required by the user so that all objects
// are created and destroyed properly. This class will allocate
// the memory if needed but will not destroy it.  That responsiblity
// rests on the calling object.
//
// It is important to set *m_pValue to NULL if memory is needed to
// be allocated.  Otherwise no memory will be allocated.
//********************************************************************//

template< typename TVal, typename JVal>
class CJSONValuePointer : public CJSONValue
{
    public:
        typedef TVal type;

        CJSONValuePointer(const std::string& name, TVal** pval, TVal* defaultVal = NULL) : CJSONValue(JSON_NULL, name), m_pValue(pval), m_pJson(NULL), m_DefaultValue(defaultVal)
        {
            if(m_pValue)
            {
                if(!*m_pValue)
                {
                    //********** Note **********//
                    // Caller must delete this
                    // memory
                    //**************************//

                    (*m_pValue) = new TVal; // must have default constructor. delete in the parent object.
                }

                m_pJson = new JVal(name, (*m_pValue)); // delete in the desctructor.
            }
        }

    // Destructor.
        ~CJSONValuePointer()
        {
            if(m_pJson && !m_pJson->IsObject()) // do not delete json object.
                delete m_pJson;
            m_pJson = NULL;
        }

    // Overloaded Methods
        bool Parse (const json_t* pVal)
        {
            return m_pJson->Parse(pVal);
        }

        bool Dump (json_t*& pRet)
        {
            return m_pJson->Dump(pRet);
        }

    // Accessor Methods
        const TVal* GetValue() const { return *m_pValue; }
        TVal* GetDefaultValue() const { return m_DefaultValue; }
    private:
        TVal**      m_pValue;
        TVal*       m_DefaultValue;
        JVal*       m_pJson;
};

// Specialization for objects.
template< typename TVal>
class CJSONValuePointer<TVal, CJSONValueObject<TVal> > : public CJSONValue
{
    public:
        typedef TVal type;

        CJSONValuePointer(const std::string& name, TVal** pval, TVal* defaultVal = NULL) : CJSONValue(JSON_NULL, name), m_pValue(pval), m_DefaultValue(defaultVal), m_pJson(NULL)
        {
            if(m_pValue)
            {
                if(!(*m_pValue))
                {
                    //********** Note **********//
                    // Caller must delete this
                    // memory
                    //**************************//

                    (*m_pValue) = new TVal; // must have default constructor.
                }

                (*m_pValue)->SetupJSONObject();
                m_pJson = (*m_pValue);
            }

        }

    // Destructor.
        ~CJSONValuePointer()
        {
            // nothing to delete this time
        }

    // Overloaded Methods
        bool Parse (const json_t* pVal)
        {
            return m_pJson->Parse(pVal);
        }

        bool Dump (json_t*& pRet)
        {
            return m_pJson->Dump(pRet);
        }

    // Accessor Methods
        const TVal* GetValue() const { return *m_pValue; }
        TVal* GetDefaultValue() const { return m_DefaultValue; }

    private:
        TVal**                  m_pValue;
        TVal*                   m_DefaultValue;
        TVal*                   m_pJson;  // Can not use base class because that will not use class specializations!!!
};

#ifdef c_plus_plus_11
template<typename TVal, template< typename... > class SmartPointer, typename JVal>
class CJSONValueSmartPointer : public CJSONValue
{
    public:
        typedef TVal type;

        CJSONValueSmartPointer(const std::string& name, SmartPointer<TVal>* pval, TVal* defaultVal = NULL) : CJSONValue(JSON_NULL, name), m_pValue(pval), m_pJson(NULL), m_DefaultValue(defaultVal)
        {
            if(m_pValue)
            {
                if(!m_pValue->get())
                {
                    //********** Note **********//
                    // SmartPointer should delete
                    // memory
                    //**************************//

                    m_pValue->reset(new TVal);
                }

                m_pJson = new JVal(name, m_pValue->get()); // delete in the desctructor.
            }
        }

    // Destructor.
        ~CJSONValueSmartPointer()
        {
            if(m_pJson && !m_pJson->IsObject()) // do not delete json object just to be sure.
                delete m_pJson;
            m_pJson = NULL;
        }

    // Overloaded Methods
        bool Parse (const json_t* pVal)
        {
            return m_pJson->Parse(pVal);
        }

        bool Dump (json_t*& pRet)
        {
            return m_pJson->Dump(pRet);
        }

    // Accessor Methods
        const TVal* GetValue() const { return *m_pValue; }
        SmartPointer<TVal> GetDefaultValue() const { return m_DefaultValue; }
    private:
        SmartPointer<TVal>*     m_pValue;
        SmartPointer<TVal>      m_DefaultValue;
        JVal*                   m_pJson;
};

template<typename TVal, template< typename... > class SmartPointer>
class CJSONValueSmartPointer<TVal, SmartPointer, CJSONValueObject<TVal> > : public CJSONValue
{
    public:
        typedef TVal type;

        CJSONValueSmartPointer(const std::string& name, SmartPointer<TVal>* pval, TVal* defaultVal = NULL) : CJSONValue(JSON_NULL, name), m_pValue(pval),  m_DefaultValue(defaultVal), m_pJson(NULL)
        {
            if(m_pValue)
            {
                if(!m_pValue->get())
                {
                    //********** Note **********//
                    // SmartPointer should delete
                    // memory
                    //**************************//

                    m_pValue->reset(new TVal);
                }

                m_pValue->get()->SetupJSONObject();
                m_pJson = m_pValue->get();
            }
        }

    // Destructor.
        ~CJSONValueSmartPointer()
        {
        }

    // Overloaded Methods
        bool Parse (const json_t* pVal)
        {
            return m_pJson->Parse(pVal);
        }

        bool Dump (json_t*& pRet)
        {
            return m_pJson->Dump(pRet);
        }

    // Accessor Methods
        const TVal* GetValue() const { return *m_pValue; }
        SmartPointer<TVal> GetDefaultValue() const { return m_DefaultValue; }
    private:
        SmartPointer<TVal>*     m_pValue;
        SmartPointer<TVal>      m_DefaultValue;
        CJSONValueObject<TVal>* m_pJson;
};
#endif


// This class is used to perform the file IO and interface with the
// object/data classes defined above. If the order of the objects in
// the file is known then there is no limitation to how many you can
// have but could make the higher level protocols more complicated.
// ?? object-name: "name" for parsing different classes in the same file ??
class CJSONParser
{
    public:
        CJSONParser(size_t flags = (JSON_INDENT(4) | JSON_SORT_KEYS | JSON_PRESERVE_ORDER)) : m_pRoot(NULL), m_Flags(flags)
        {
        }

        ~CJSONParser()
        {
            json_decref(m_pRoot); // Realease ownership
        }

        bool Load(const char* pBuffer)
        {
            if(m_pRoot)
            {
                json_decref(m_pRoot); // Release ownership.
                m_pRoot = NULL;
            }

            m_pRoot = json_loads(pBuffer, 0, &m_LastError);
            if(!m_pRoot)
            {
                fprintf(stderr, "warning: %s \n", m_LastError.text);
                return false;
            }
            return true;
        }

        bool LoadFromString(const std::string& str)
        {
            return Load(str.c_str());
        }

        bool LoadFromBuffer(const char* pBuffer, const size_t& size)
        {
            if(m_pRoot)
            {
                json_decref(m_pRoot); // Release ownership.
                m_pRoot = NULL;
            }

            m_pRoot = json_loadb(pBuffer, size, 0, &m_LastError);
            if(!m_pRoot)
            {
                fprintf(stderr, "warning: %s \n", m_LastError.text);
                return false;
            }
            return true;
        }

        bool LoadFromFile(const std::string& Path)
        {
            if(m_pRoot)
            {
                json_decref(m_pRoot); // Release ownership.
                m_pRoot = NULL;
            }

            m_pRoot = json_load_file(Path.c_str(), 0, &m_LastError);
            if(!m_pRoot)
            {
                fprintf(stderr, "warning: %s \n", m_LastError.text);
                return false;
            }
            return true;
        }

        template<class TVal>
        bool ParseObjectFromArray(const size_t& index, CJSONValueObject<TVal>* pOject)
        {
            bool bParseSuccess = false;
            json_t* object_data = json_array_get(m_pRoot, index);
            bParseSuccess = pOject->Parse(object_data);

            return bParseSuccess;
        }

        template<class TVal>
        bool ParseObject(CJSONValueObject<TVal>* pOject)
        {
            bool bParseSuccess = false;
            bParseSuccess = pOject->Parse(m_pRoot);
            return bParseSuccess;
        }

        template<class TVal>
        bool DumpObjectToFile(const std::string& Path, CJSONValueObject<TVal>* pOject)
        {
            bool bDumpSuccess = false;
            if(m_pRoot)
            {
                json_decref(m_pRoot); // Release ownership.
                m_pRoot = NULL;
            }

            if(pOject->Dump(m_pRoot))
            {
                json_incref(m_pRoot); // decalare shared ownership.
                bDumpSuccess = (json_dump_file(m_pRoot, Path.c_str(), m_Flags) == 0);
                if(! bDumpSuccess )
                {
                    cout << "Error dumping file to disk!" << endl;;
                    perror("Error dumping file");
                }
            }
            else
            {
                std::cout << "Error dumping object! " << std::endl;
            }
            pOject->ClearBuffer();

            return bDumpSuccess;
        }

        template<class TVal>
        bool DumpObjectToString(std::string& ret, CJSONValueObject<TVal>* pOject)
        {
            ret.clear();
            if(m_pRoot)
            {
                json_decref(m_pRoot); // Release ownership.
                m_pRoot = NULL;
            }

            if(pOject->Dump(m_pRoot))
            {
                json_incref(m_pRoot); // decalare shared ownership.
                char* s = NULL;
                s = json_dumps(m_pRoot, m_Flags);
                if( !s )
                {
                    cout << "Error dumping file to string!" << endl;
                }
                else
                {
                    ret = std::string(s);   // move the data into the return string.
                    free(s);                // free the memory
                }
            }
            pOject->ClearBuffer();
            return ret.length() > 0;
        }


//        template<class TVal>
//        bool UpdateObjectToFile(const std::string& Path, CJSONValueObject<TVal>* pOject)
//        {
//            bool bUpdateSuccess = false;
//
//            if(m_pRoot)
//            {
//                json_decref(m_pRoot); // Release ownership.
//                m_pRoot = NULL;
//            }
//
//            json_t* pJson = NULL;
//            if(pOject->Dump(pJson) && LoadFromFile(Path))    // Current version && Previous version
//            {
//                if(json_object_update(m_pRoot, pJson) == 0) // update root with the object
//                {
//                    bUpdateSuccess = (json_dump_file(m_pRoot, Path.c_str(), m_Flags) == 0);
//                }else { cout << "Error could not update object" << endl;}
//            }else { cout << "Error could not dump object and/or Load from file. Object @ "<< pJson << " root @ "<< m_pRoot << endl;}
//
//            pOject->ClearBuffer();
//
//            return bUpdateSuccess;
//        }

        size_t RootArrayLength()
        {
            if ( IsRootArray() )
            {
                return json_array_size(m_pRoot);
            }
            return 0;
        }

        bool IsRootObject()
        {
            if(IsRootValid())
                return json_is_object(m_pRoot);
            return false;
        }

        bool IsRootArray()
        {
            if(IsRootValid())
                return json_is_array(m_pRoot);
            return false;
        }

        bool IsRootString()
        {
            if(IsRootValid())
                return json_is_string(m_pRoot);
            return false;
        }

        bool IsRootNumber()
        {
            if(IsRootValid())
                return json_is_number(m_pRoot);
            return false;
        }

        bool IsRootBool()
        {
            if(IsRootValid())
                return json_is_boolean(m_pRoot);
            return false;
        }

        bool IsRootValid()
        {
            return (m_pRoot && !json_is_null(m_pRoot));
        }

    private:
        json_t*         m_pRoot;
        json_error_t    m_LastError;
        size_t          m_Flags;
};


// Here is an idea to make these classes more accessible. Lets try it!
# if 0

template< class T>
class CJSONValueType {};

template<>
class CJSONValueType<int> : CJSONValueInt { CJSONValueType(const std::string& name, int * pval) : CJSONValueInt(name, pvalue) {};

template<>
class CJSONValueType<float> : CJSONValueFloat {CJSONValueType(const std::string& name, float * pval) : CJSONValueFloat(name, pvalue) {};

template<>
class CJSONValueType<double> : CJSONValueDouble {CJSONValueType(const std::string& name, double * pval) : CJSONValueDouble(name, pvalue) {};

template<>
class CJSONValueType<string> : CJSONValueString {CJSONValueType(const std::string& name, string * pval) : CJSONValueString(name, pvalue) {};

template<>
class CJSONValueType<bool> : CJSONValueBool {CJSONValueType(const std::string& name, bool * pval) : CJSONValueBool(name, pvalue) {};

template<class T>
class CJSONValueType< vector<T> > : CJSONValueArray<vector<T>, CJSONValueType<T> > {CJSONValueType(const std::string& name, vector<T> * pval) : CJSONValueArray<vector<T>, CJSONValueType<T> >(name, pvalue) {};

#ifdef c_plus_plus_11

template< typename... TVals >
class CJSON
{
    public:
        CJSON(std::string... Names) {}
        ~CJSON() {}
    private:
        std::vector<std::string>    m_Names;
        std::tuple<TVals...>        m_Values;
};

#endif

template< class ArrayType, class ValueType, class Alloc = std::allocator<ValueType> >
class array_allocator
{
    public:
        typedef Alloc       allocator_type;
        typedef ValueType   value_type;
        typedef ArrayType   array_type;
        array_allocator(const allocator_type& alloc = allocator_type()) : m_alloc(alloc) {}

        void allocate(array_type& array, const size_t& n) { array.resize(n); }

    private:
        allocator_type m_alloc;
};

template < class ArrayType, class ValueType, class Alloc = std::allocator<ValueType> >
class CJSONValueArrayEx : public CJSONValue
{
    public:
        typedef ArrayType type;

        CJSONValueArrayEx(const std::string& name, ArrayType* pval, const ArrayType& defaultVal = std::vector<TVal>()) : CJSONValue(JSON_ARRAY, name), m_pValue(pval), m_DefaultValue(defaultVal)
        {
            m_DefaultArrayValue = JVal("", NULL).GetDefaultValue();
        }

        CJSONValueArrayEx(const CJSONValueArray& src ) : CJSONValue(JSON_ARRAY, src.GetName())
        {
            CopyFrom(src);
        }

        ~CJSONValueArrayEx() {}

        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_array(pVal))
            {
                bParseSuccess = true;
                size_t n = json_array_size(pVal);
                json_t* data;

                for (size_t i = 0; i < n; i++)
                {
                    //TVal temp(m_DefaultArrayValue);


                    char array_number[30]; // should be enough space.
                    sprintf(&array_number[0], "-%zu", i);
                    //JVal tjson((m_name + std::string(array_number)), &temp);


                    data = json_array_get(pVal, i);
                    if ( data )
                    {
                        bParseSuccess = tjson.Parse(data) && bParseSuccess;
                        m_pValue->push_back(temp);
                    }
                    else{ cerr << "" << endl;}
                }
            }
            else{
                fprintf(stderr, "ERROR: %s is not an array as expected. \n", m_name.c_str());
            }

            return bParseSuccess;
        }

        bool Dump (json_t*& pRet)
        {
            ClearJValue();
            bool bDumpSuccess = true;
            pRet = json_array();
            if(pRet)
            {
                for( size_t i = 0; i < m_pValue->size(); i++)
                {
                    json_t* pVal = NULL;
                    TVal temp = m_pValue->at(i);

                    JVal tjson("", &temp);

                    bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;

                    if(pVal)
                        bDumpSuccess = (json_array_append(pRet, pVal) != -1) && bDumpSuccess;
                    else
                        std::cout << "Error could not dump array element. "<< m_name << "-"<< i << std::endl;
                }
            }
            else
            {
                bDumpSuccess = false;
                std::cout << "Error! Could not dump array. "<< m_name << std::endl;
            }
            m_pJValue = pRet;
            return bDumpSuccess;
        }


        const CJSONValueArrayEx& CopyFrom( const CJSONValueArrayEx& src )
        {
           m_pValue = &src.GetValue();
           m_DefaultValue = src.GetDefaultValue();
        }

    // Accessor Methods
        const TVal& GetValue() const { return *m_pValue; }
        const std::vector<TVal>& GetDefaultValue() const { return m_DefaultValue; }
    private:
        ArrayType*  m_pValue;
        ArrayType   m_DefaultValue;
        //ValueType   m_DefaultArrayValue;
};








#endif
// Now implement the particulars
template<class DerivedClass>
inline bool CJSONValueObject<DerivedClass>::LoadFromFile( const std::string& Path )
{
    CJSONParser json;
    json.LoadFromFile(Path);    // opens the JSON file and loads the data into buffer.
    if(json.IsRootObject())     // expecting an object
    {
        return json.ParseObject(m_pDerived); // Parses all the data for this object.
    }
    else
    {
        return false;
    }
}

template<class DerivedClass>
inline bool CJSONValueObject<DerivedClass>::SaveToFile( const std::string& Path )
{
    bool bSaveSuccess = false;
    CJSONParser json;
    bSaveSuccess = json.DumpObjectToFile(Path, m_pDerived);
    return bSaveSuccess;
}





}

#endif















/*
From the jansson documentation:

JSON_INDENT(n)
Pretty-print the result, using newlines between array and object items, and indenting with n spaces. The valid range for n is between 0 and 31 (inclusive), other values result in an undefined output. If JSON_INDENT is not used or n is 0, no newlines are inserted between array and object items.
JSON_COMPACT
This flag enables a compact representation, i.e. sets the separator between array and object items to "," and between object keys and values to ":". Without this flag, the corresponding separators are ", " and ": " for more readable output.
JSON_ENSURE_ASCII
If this flag is used, the output is guaranteed to consist only of ASCII characters. This is achived by escaping all Unicode characters outside the ASCII range.
JSON_SORT_KEYS
If this flag is used, all the objects in output are sorted by key. This is useful e.g. if two JSON texts are diffed or visually compared.
JSON_PRESERVE_ORDER
If this flag is used, object keys in the output are sorted into the same order in which they were first inserted to the object. For example, decoding a JSON text and then encoding with this flag preserves the order of object keys.
JSON_ENCODE_ANY
Specifying this flag makes it possible to encode any JSON value on its own. Without it, only objects and arrays can be passed as the root value to the encoding functions.

Note: Encoding any value may be useful in some scenarios, but it’s generally discouraged as it violates strict compatiblity with RFC 4627. If you use this flag, don’t expect interoperatibility with other JSON systems.

New in version 2.1.

JSON_ESCAPE_SLASH
Escape the / characters in strings with \/.

New in version 2.4.



JSON_REJECT_DUPLICATES
Issue a decoding error if any JSON object in the input text contains duplicate keys. Without this flag, the value of the last occurence of each key ends up in the result. Key equivalence is checked byte-by-byte, without special Unicode comparison algorithms.

New in version 2.1.

JSON_DECODE_ANY
By default, the decoder expects an array or object as the input. With this flag enabled, the decoder accepts any valid JSON value.

Note: Decoding any value may be useful in some scenarios, but it’s generally discouraged as it violates strict compatiblity with RFC 4627. If you use this flag, don’t expect interoperatibility with other JSON systems.

New in version 2.3.

JSON_DISABLE_EOF_CHECK
By default, the decoder expects that its whole input constitutes a valid JSON text, and issues an error if there’s extra data after the otherwise valid JSON input. With this flag enabled, the decoder stops after decoding a valid JSON array or object, and thus allows extra data after the JSON text.

Normally, reading will stop when the last ] or } in the JSON input is encountered. If both JSON_DISABLE_EOF_CHECK and JSON_DECODE_ANY flags are used, the decoder may read one extra UTF-8 code unit (up to 4 bytes of input). For example, decoding 4true correctly decodes the integer 4, but also reads the t. For this reason, if reading multiple consecutive values that are not arrays or objects, they should be separated by at least one whitespace character.

New in version 2.1.

JSON_DECODE_INT_AS_REAL
JSON defines only one number type. Jansson distinguishes between ints and reals. For more information see Real vs. Integer. With this flag enabled the decoder interprets all numbers as real values. Integers that do not have an exact double representation will silently result in a loss of precision. Integers that cause a double overflow will cause an error.

New in version 2.5.


*/
