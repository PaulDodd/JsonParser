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
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <map>



using namespace std;

namespace json {

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
// 5.   Write unit tests to test all the code.
//


class CJSONValue
{
    public:
        CJSONValue(json_type type, const string& name)
        {
            m_type = type;
            m_name = name;
            m_pJValue = NULL;
        }
        virtual ~CJSONValue()
        {
            //cout << " N ~CJSONValue " << TypeToString() << " " << m_pJValue << " has " << (m_pJValue ? m_pJValue->refcount : 0) << endl;
            json_decref( m_pJValue );
            //cout << " X ~CJSONValue " << TypeToString() << " " << m_pJValue << " has " << (m_pJValue ? m_pJValue->refcount : 0) << endl;
        }
    
    // Abstract methods
        virtual bool Parse (const json_t* pVal) = 0;
        virtual bool Dump (json_t*& pRet) = 0;
        // virtual bool IsValid() = 0;
    
    // Accessor Methods
        const string& GetName() const { return m_name; }
        bool IsInt()    { return m_type == JSON_INTEGER; }
        bool IsFloat()  { return m_type == JSON_REAL; }
        bool IsString() { return m_type == JSON_STRING; }
        bool IsArray()  { return m_type == JSON_ARRAY; }
        bool IsObject() { return m_type == JSON_OBJECT; }
        bool IsBool()   { return m_type == JSON_TRUE; }
        bool IsNull()   { return m_type == JSON_NULL; }
    
        string TypeToString()
        {
            string type;
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
        json_t*     m_pJValue;  // buffer to hold values for dump.
        json_type   m_type;
        string      m_name;
};

template< class NVal, json_type _type_ >
class CJSONValueNumber : public CJSONValue // may need an unsigned version of this class.
{
    public:
        CJSONValueNumber(const string& name, NVal * pval, const NVal& defaultVal = 0) : CJSONValue(_type_, name), m_pValue(pval), m_DefaultValue(defaultVal) {}
        ~CJSONValueNumber() {}
    
    // Overloaded Methods
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_number(pVal))
            {
                cout << "JSON number found" << endl;
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
            if(IsInt())
                pRet = json_integer(*m_pValue);
            else
                pRet = json_real(*m_pValue);
            
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
typedef CJSONValueNumber<double, JSON_REAL>         CJSONValueFloat; // Should probably rename this to double and make one for actual float.

// TODO: Remove the following once the code above proves itself.
//class CJSONValueInt : public CJSONValue // may need an unsigned version of this class.
//{
//    public:
//        CJSONValueInt(const string& name, int * pval, const int& defaultVal = 0) : CJSONValue(JSON_INTEGER, name), m_pValue(pval), m_DefaultValue(defaultVal) {}
//        ~CJSONValueInt() {}
//    
//    // Overloaded Methods
//        bool Parse (const json_t* pVal)
//        {
//            bool bParseSuccess = false;
//            if(json_is_integer(pVal))
//            {
//                // cout << "JSON integer found" << endl;
//                *m_pValue = int(json_number_value(pVal));
//            }
//            else{
//                fprintf(stderr, "ERROR: %s is not an integer as expected. \n", m_name.c_str());
//            }
//            return bParseSuccess;
//        }
//    
//        bool Dump (json_t*& pRet)
//        {
//            pRet = json_integer(*m_pValue);
//            m_pJValue = pRet;
//            return pRet != NULL;
//        }
//    
//    // Accessor Methods
//        const int& GetValue() const { return *m_pValue; }
//        const int& GetDefaultValue() const { return m_DefaultValue; }
//    
//    private:
//        int  m_DefaultValue;
//        int* m_pValue;
//};
//
//class CJSONValueUInt : public CJSONValue // may need an unsigned version of this class.
//{
//    public:
//        CJSONValueUInt(const string& name, size_t* pval, const size_t& defaultVal = 0) : CJSONValue(JSON_INTEGER, name), m_pValue(pval), m_DefaultValue(defaultVal) {}
//        ~CJSONValueUInt() {}
//    
//    // Overloaded Methods
//        bool Parse (const json_t* pVal)
//        {
//            bool bParseSuccess = false;
//            if(json_is_integer(pVal))
//            {
//                // cout << "JSON integer found" << endl;
//                *m_pValue = json_integer_value(pVal);
//            }
//            else{
//                fprintf(stderr, "ERROR: %s is not an integer as expected. \n", m_name.c_str());
//            }
//            return bParseSuccess;
//        }
//    
//        bool Dump (json_t*& pRet)
//        {
//            pRet = json_integer(*m_pValue);
//            m_pJValue = pRet;
//            return pRet != NULL;
//        }
//    
//    // Accessor Methods
//        const size_t& GetValue() const { return *m_pValue; }
//        const size_t& GetDefaultValue() const { return m_DefaultValue; }
//    private:
//        size_t  m_DefaultValue;
//        size_t* m_pValue;
//};
//
//class CJSONValueFloat : public CJSONValue
//{
//    public:
//        CJSONValueFloat(const string& name, double* pval, const double& defaultVal = 0.0) : CJSONValue(JSON_REAL, name), m_pValue(pval), m_DefaultValue(defaultVal) {}
//        ~CJSONValueFloat() {}
//    
//        bool Parse (const json_t* pVal)
//        {
//            bool bParseSuccess = false;
//            if(json_is_real(pVal))
//            {
//                // cout << "JSON real found" << endl;
//                *m_pValue = json_real_value(pVal);
//            }
//            else{
//                fprintf(stderr, "ERROR: %s is not an floating point number as expected. \n", m_name.c_str());
//            }
//            return bParseSuccess;
//        }
//    
//        bool Dump (json_t*& pRet)
//        {
//            pRet = json_real(*m_pValue);
//            m_pJValue = pRet;
//            return pRet != NULL;
//        }
//    
//        const double& GetValue() const { return *m_pValue; }
//        const double& GetDefaultValue() const { return m_DefaultValue; }
//    private:
//        double  m_DefaultValue;
//        double* m_pValue;
//};

class CJSONValueString : public CJSONValue
{
    public:
        CJSONValueString(const string& name, string* pval, const string& defaultVal = "") : CJSONValue(JSON_STRING, name), m_pValue(pval), m_DefaultValue(defaultVal) {}
        ~CJSONValueString() {}
    
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_string(pVal))
            {
                // cout << "JSON string found" << endl;
                bParseSuccess = true;
                *m_pValue = json_string_value(pVal);
            }
            else{
                fprintf(stderr, "ERROR: %s is not an string as expected. \n", m_name.c_str());
            }
            return bParseSuccess;
        }
    
        bool Dump (json_t*& pRet)
        {
            pRet = json_string(m_pValue->c_str());
            if(!pRet) cout << "Error could not dump string " << m_name << endl;
            m_pJValue = pRet;
            return pRet != NULL;
        }
    
        const string& GetValue() const { return *m_pValue; }
        const string& GetDefaultValue() const { return m_DefaultValue; }
    
    private:
        string  m_DefaultValue;
        string* m_pValue;
};

class CJSONValueBool : public CJSONValue
{
    public:
        CJSONValueBool(const string& name, bool * pval, const bool& defaultVal = false) : CJSONValue(JSON_TRUE, name), m_pValue(pval), m_DefaultValue(defaultVal)  {}
        ~CJSONValueBool() {}
    
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_boolean(pVal))
            {
                // cout << "JSON boolean found" << endl;
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
//            cout << "boolean value = " << boolalpha << *m_pValue << " @ " << m_pValue << " -> "<< pRet << endl;
            pRet = (*m_pValue) ? json_true() : json_false();
//            cout << "json boolean value @ " << pRet << endl;
            m_pJValue = pRet;
            return pRet != NULL;
        }
    
        const bool& GetValue() const { return *m_pValue; }
        const bool& GetDefaultValue() const { return m_DefaultValue; }
    
    private:
        bool  m_DefaultValue;
        bool* m_pValue;
};



// Will have to think about how to make this work for fixed array types of containers.
// Implemeted to work with the std::vector class for ease.
// ?? Could/Should make a CJSONValueFixedArray<> class and change the name of this class to be CJSONValueDynamicArray or CJSONValueVector ??
// TVal is object then we will need to call the SetupObjectClass.

template <class TVal, class JVal>
class CJSONValueArray : public CJSONValue
{
    public:
        CJSONValueArray(const string& name, vector<TVal>* pval, const vector<TVal>& defaultVal = vector<TVal>()) : CJSONValue(JSON_ARRAY, name), m_pValue(pval), m_DefaultValue(defaultVal)
        {
            m_DefaultArrayValue = JVal("", NULL).GetDefaultValue();
        }
    
        CJSONValueArray(const CJSONValueArray& src ) : CJSONValue(JSON_ARRAY, src.GetName())
        {
            cout << "Copy constructor called" << endl;
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
                
                // cout << "Array size = " << n << endl;
                
                for (size_t i = 0; i < n; i++)
                {
                    TVal temp = m_DefaultArrayValue;
                    
                    char array_number[30]; // should be enough space.
                    sprintf(&array_number[0], "-%zu", i);
                    JVal tjson((m_name + string(array_number)), &temp);
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
            bool bDumpSuccess = true;
            pRet = json_array();
            if(pRet)
            {
                for( size_t i = 0; i < m_pValue->size(); i++)
                {
                    json_t* pVal = NULL;
                    TVal temp = m_pValue->at(i);
                    
                    JVal tjson("", &temp);
                    
                    // cout << "pVal @ " << pVal << endl;
                    bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;
                    // cout << "pVal @ " << pVal << endl;
                    
                    if(pVal)
                        bDumpSuccess = (json_array_append(pRet, pVal) != -1) && bDumpSuccess;
                    else
                        cout << "Error could not dump array element. "<< m_name << "-"<< i << endl;
                }
            }
            else
            {
                bDumpSuccess = false;
                cout << "Error! Could not dump array. "<< m_name << endl;
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
        const vector<TVal>& GetDefaultValue() const { return m_DefaultValue; }
    private:
        vector<TVal>*   m_pValue;
        vector<TVal>    m_DefaultValue;
        TVal            m_DefaultArrayValue;
};




// This requires c++11.
#if __cplusplus >= 201103L
#include <tuple>
#include <type_traits>
#include <utility>

// My tuple utility functions.

// base case
template< std::size_t I = 0, class TVal, typename... Ts >
inline typename std::enable_if< I == sizeof...(Ts), void >::type put (
                                                                        std::tuple< Ts... >& t,
                                                                        TVal& val,
                                                                        const size_t& Index)
{
    return;
}

template< std::size_t I = 0, class TVal, typename... Ts >
inline typename std::enable_if< I < sizeof...(Ts), void >::type put (
                                                                        std::tuple< Ts... >& t,
                                                                        TVal& val,
                                                                        const size_t& Index)
{
    if(I == Index)
        std::get<I>(t) = decltype(std::get<I>(t))(val);
    else
        put< (I+1), TVal, Ts... > (t, val, Index);
}

// base case
template< std::size_t I = 0, class TVal, typename... Ts >
inline typename std::enable_if< I == sizeof... (Ts), bool >::type is_type (
                                                                            std::tuple< Ts... >& t,
                                                                            const size_t& Index)
{
    return false; // Index out of range so return false.
}

template< std::size_t I = 0, class TVal, typename... Ts >
inline typename std::enable_if< I < sizeof...(Ts), bool >::type is_type (
                                                                            std::tuple< Ts... >& t,
                                                                            const size_t& Index)
{
    if(I == Index)
        return typeid(std::get<I>(t)) == typeid(TVal);
    else
        return is_type<I+1, TVal, Ts...>(t, Index);
}


// base case
template< std::size_t I = 0, class TVal, typename... Ts >
inline typename std::enable_if< I == sizeof... (Ts), TVal* >::type pull (
                                                                            std::tuple< Ts... >& t,
                                                                            const size_t& Index)
{
    return NULL; // Index out of range so return NULL.
}

template< std::size_t I = 0,  class TVal, typename... Ts >
inline typename std::enable_if< I < sizeof...(Ts), TVal* >::type pull (
                                                                            std::tuple< Ts... >& t,
                                                                            const size_t& Index)
{
    if(I == Index)
        return (TVal*)&std::get<I>(t);
    else
        return pull<I+1, TVal, Ts...>(t, Index);
}



template< std::size_t I = 0>
inline typename std::enable_if< I == 5, void>::type count_to_five_or_less(const size_t& Index)
{
    return;
}

template< std::size_t I = 0>
inline typename std::enable_if< I < 5, void>::type count_to_five_or_less(const size_t& Index)
{
    if(I < Index)
        cout << I << endl;
    count_to_five_or_less<I+1>(Index);
}



/************************************************************************************************************************************************************************/


// Because how the template arguments pack the tuple must be comprised of the following types: bool, int, double, or string.
// Parse will fail if a different type is recieved.
// ?? Can this be generalized more ??
template<typename... TVals>
class CJSONValueTuple : public CJSONValue
{
    public:
        CJSONValueTuple(const string& name, tuple<TVals...>* pval, const tuple<TVals...>& defaultVal = tuple<TVals...>()) : CJSONValue(JSON_ARRAY, name), m_pValue(pval), m_DefaultValue(defaultVal) {}
    
    // Overloaded Methods
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
                    char array_number[30]; // should be enough space.
                    sprintf(&array_number[0], "-%zu", i);
                    string elemName(m_name + string(array_number));
                
                    data = json_array_get(pVal, i);
                    if(json_is_boolean(data))
                    {
                        bool temp;
                        CJSONValueBool jtemp(elemName, &temp);
                        jtemp.Parse(data);
                        // std::get<i>(*m_pValue) = temp;
                        put<0, bool, TVals...>(*m_pValue, temp, i);
                    }
                    else if(json_is_integer(data))
                    {
                        int temp;
                        CJSONValueInt jtemp(elemName, &temp);
                        jtemp.Parse(data);
                        put<0, int, TVals...>(*m_pValue, temp, i);
                    }
                    else if(json_is_real(data))
                    {
                        double temp;
                        CJSONValueFloat jtemp(elemName, &temp);
                        jtemp.Parse(data);
                        put<0, double, TVals...>(*m_pValue, temp, i);
                    }
                    else if(json_is_string(data))
                    {
                        string temp;
                        CJSONValueString jtemp(elemName, &temp);
                        jtemp.Parse(data);
                        put<0, string, TVals...>(*m_pValue, temp, i);
                    }
                    else{
                        bParseSuccess = false;
                        fprintf(stderr, "ERROR: %s Could not parse tuple element. Unknown type. \n", m_name.c_str());
                        break;
                    }
                }
            }
            else{
                fprintf(stderr, "ERROR: %s is not an array as expected. \n", m_name.c_str());
            }

            return bParseSuccess;
        }
    
        bool Dump (json_t*& pRet)
        {
            bool bDumpSuccess = true;
            pRet = json_array();
            if(pRet)
            {
                // size_t i = 0;
                for(size_t i = 0; i < std::tuple_size< tuple<TVals...> >::value; i++) // for( auto& val : *m_pValue)
                {
                    json_t* pVal = NULL;
                    

                    // cout << "data @ " << pTemp<< " = "<< *pTemp << endl;
                    if(is_type<0, bool, TVals...>(*m_pValue, i))
                    {
                        auto* pTemp = pull<0, bool, TVals...>(*m_pValue, i);
                        CJSONValueBool tjson("", pTemp);
                        bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;
                    }
                    else if(is_type<0, int, TVals...>(*m_pValue, i))
                    {
                        auto* pTemp = pull<0, int, TVals...>(*m_pValue, i);
                        CJSONValueInt tjson("", pTemp);
                        bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;
                    }
                    else if(is_type<0, double, TVals...>(*m_pValue, i) ||
                            is_type<0, float, TVals...>(*m_pValue, i) )
                    {
                        auto* pTemp = pull<0, double, TVals...>(*m_pValue, i);
                        CJSONValueFloat tjson("", pTemp);
                        bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;
                    }
                    else if(is_type<0, string, TVals...>(*m_pValue, i))
                    {
                        auto* pTemp = pull<0, string, TVals...>(*m_pValue, i);
                        CJSONValueString tjson("", pTemp);
                        bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;
                    }
                    else{
                        cout << "Error could not match data type array element. " << m_name << "[" << i << "]" << endl;
                        pVal = NULL;
                    }
                    
                    if(pVal)
                        bDumpSuccess = (json_array_append(pRet, pVal) != -1) && bDumpSuccess;
                    else
                        cout << "Error could not dump array element. " << m_name << "[" << i << "]" << endl;
            
                }
            }
            else
            {
                bDumpSuccess = false;
                cout << "Error! Could not dump array. "<< m_name << endl;
                
            }
            m_pJValue = pRet;
            return bDumpSuccess;
        }
    
    
        const tuple<TVals...>& GetDefaultValue() { return m_DefaultValue; }
    private:
        tuple<TVals...>*    m_pValue;
        tuple<TVals...>     m_DefaultValue;
};

#endif
class CJSONValueObject : public CJSONValue
{
    public:
        CJSONValueObject(const string& name, CJSONValueObject* pval) : CJSONValue(JSON_OBJECT, name), m_pDerived(pval) {}
        ~CJSONValueObject() { Destroy(); }
    
        void Destroy()
        {
            // cout << "Destroying " << m_name << " objects!" << endl;
            map<string, CJSONValue* >::iterator iter;
            for(iter = m_Map.begin(); iter != m_Map.end(); iter++)
            {
                if(!iter->second->IsObject()) // the memory was created by this class for all but objects.
                {
                    if(iter->second)
                        delete iter->second;
                    iter->second = NULL;
                }
            }
            
            m_Map.clear();
        }
    
    // Inherited Abstract Methods.
        virtual bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            // cout << "Calling CJSONValueObject::Parse" << endl;
            if(json_is_object(pVal))
            {
                bParseSuccess = true;
                json_t* data;
                map<string, CJSONValue* >::iterator iter;
                for(iter = m_Map.begin(); iter != m_Map.end(); iter++)
                {
                    data = json_object_get(pVal, iter->first.c_str());
                    if(data)
                        bParseSuccess = iter->second->Parse(data) && bParseSuccess;
                    else
                        cout << "Key (" << iter->first << ") was not found in file." << endl;
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
            //cout << "Calling CJSONValueObject::Dump" << endl;
            bool bDumpSuccess = true;
            
            pRet = json_object();
            
            //cout << "Dumping object " << m_name << " @ " << pRet << endl;
            
            if(pRet)
            {
                map<string, CJSONValue* >::iterator iter;
                for(iter = m_Map.begin(); iter != m_Map.end(); iter++)
                {
                    json_t* value = NULL;
                    if ( iter->second->Dump(value) )
                    {
                        if ( json_object_set(pRet, iter->first.c_str(), value) == -1)
                        {
                            bDumpSuccess = false;
                            cout << "Error! Could not add " << iter->first << " to object of size "<< json_object_size(pRet) << endl;
                        }
                    }
                    else
                    {
                        bDumpSuccess = false;
                        cout << "Error! Could not dump " << iter->first << endl;
                    }
                }
            }
            else
            {
                cout << "Error! could not create object!" << endl;
                bDumpSuccess = false;
            }
            m_pJValue = pRet;
            return bDumpSuccess;
        }
    
    // Abstract methods
        virtual void SetupJSONObject() = 0;
    
    // Class methods
        template<class TVal, class JVal>
        void AddNameValuePair(const string& name, TVal* pval)
        {
            map < string, CJSONValue* >::iterator iter = m_Map.find(name);
            if(iter == m_Map.end())
            {
                m_Map.insert( pair< string, CJSONValue* >(name, new JVal(name, pval)));
            }
            else
            {
                cout << "Already a key named "<< name << " is in the object. No action taken."<<endl;
            }
        }
    
        // Could Remove the following becasuse of the encompassing method above.
        void AddIntegerValue(const string& name, int* pval)
        {
            AddNameValuePair<int, CJSONValueInt>(name, pval);
        }
        void AddUIntegerValue(const string& name, size_t* pval)
        {
            AddNameValuePair<size_t, CJSONValueUInt>(name, pval);
        }
        void AddBoolValue(const string& name, bool* pval)
        {
            AddNameValuePair<bool, CJSONValueBool>(name, pval);
        }
        void AddFloatingPointValue(const string& name, double* pval)
        {
            AddNameValuePair<double, CJSONValueFloat>(name, pval);
        }
        void AddStringValue(const string& name, string* pval)
        {
            AddNameValuePair<string, CJSONValueString>(name, pval);
        }
        void AddStringArrayValue(const string& name, vector<string>* pval)
        {
            AddNameValuePair<vector<string>, CJSONValueArray<string, CJSONValueString> >(name, pval);
        }
    
        void AddObjectValue(const string& name, CJSONValueObject* pval)
        {
            map < string, CJSONValue* >::iterator iter = m_Map.find(name);
            if(iter == m_Map.end())
            {
                m_Map.insert(pair<string, CJSONValue* >(name,  pval));
            }
            else
            {
                cout << "Already a key named "<< name << " is in the object. No action taken."<<endl;
            }
        }
    
        CJSONValueObject* GetDerived()  { return m_pDerived; }
        const CJSONValueObject& GetDefaultValue()   { return *m_pDerived; }
    
    private:
        CJSONValueObject*                               m_pDerived;         // ?? remove this ??
        map < string, CJSONValue* >                     m_Map;              // map for each element in the object at this level. How to access data?
};


//                  CJSONValuePointer
//********************************************************************//
// TVal is a data type that can be parsed by the corresponding
// json class JVal. It is intended to allow for flexibility in
// parsing and different coding styles. Some special handling
// of memory will be required by the user so that all objects
// are created and destroyed properly. This class will allocate
// the memory if needed but will not destroy it.  That responsiblity
// rests on the calling root object.
//
// It is important to set *m_pValue to NULL if memory is needed to
// be allocated.  Otherwise no memory will be allocated.
//********************************************************************//

template< typename TVal, typename JVal>
class CJSONValuePointer : public CJSONValue
{
    public:
        CJSONValuePointer(const string& name, TVal** pval, TVal* defaultVal = NULL) : CJSONValue(JSON_NULL, name), m_pValue(pval), m_pJson(NULL), m_DefaultValue(defaultVal)
        {
            if(m_pValue)
            {
                // cout << "Pointer @ "<<*m_pValue<<endl;
                if(!*m_pValue)
                {
                    (*m_pValue) = new TVal; // must have default constructor. delete in the parent object.
                }
                
                m_pJson = new JVal(name, (*m_pValue)); // delete in the desctructor.
            }
        }
    
    // Destructor.
        ~CJSONValuePointer()
        {
            // cout << "Calling ~CJSONValuePointer "<< m_pJson << endl;
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
class CJSONValuePointer<TVal, CJSONValueObject> : public CJSONValue
{
    public:
        CJSONValuePointer(const string& name, TVal** pval, TVal* defaultVal = NULL) : CJSONValue(JSON_NULL, name), m_pValue(pval), m_pJson(NULL), m_DefaultValue(defaultVal)
        {
            if(m_pValue)
            {
                // cout << "Pointer to JSON object detected @ " << *m_pValue << endl;
                
                if(!(*m_pValue))
                {
                    (*m_pValue) = new TVal; // must have default constructor.
                }
                
                (*m_pValue)->SetupJSONObject();
                m_pJson = (*m_pValue);
            }

        }
    
    // Destructor.
        ~CJSONValuePointer()
        {
            // cout << "Calling ~CJSONValuePointer<TVal, CJSONValueObject>"<< m_pJson << endl;
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
        CJSONValueObject*       m_pJson;
};



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
            cout << " N ~CJSONParser " << " " << m_pRoot << " has " << (m_pRoot ? m_pRoot->refcount : 0) << endl;
            json_decref(m_pRoot); // deletes the buffer.
            cout << " N ~CJSONParser " << " " << m_pRoot << " has " << (m_pRoot ? m_pRoot->refcount : 0) << endl;
        }
    
        bool Load(const char* pBuffer)
        {
            m_pRoot = json_loads(pBuffer, 0, &m_LastError);
            if(!m_pRoot)
            {
                fprintf(stderr, "error: %s \n", m_LastError.text);
                return false;
            }
            return true;
        }
    
        bool LoadFromFile(const std::string& Path)
        {
            m_pRoot = json_load_file(Path.c_str(), 0, &m_LastError);
            if(!m_pRoot)
            {
                fprintf(stderr, "error: %s \n", m_LastError.text);
                return false;
            }
            return true;
        }
    
        bool ParseObjectFromArray(const int& index, CJSONValueObject* pOject)
        {
            bool bParseSuccess = false;
            json_t* object_data = json_array_get(m_pRoot, index);
            // cout << "object data = " << object_data << endl;
            bParseSuccess = pOject->Parse(object_data);
            
            return bParseSuccess;
        }
    
        bool ParseObject(CJSONValueObject* pOject)
        {
            bool bParseSuccess = false;
            bParseSuccess = pOject->Parse(m_pRoot);
            return bParseSuccess;
        }
    
        bool DumpObjectToFile(const string& Path, CJSONValueObject* pOject)
        {
            bool bDumpSuccess = false;
            
            if(m_pRoot) // Release ownership.
            {
                json_decref(m_pRoot);
                m_pRoot = NULL;
            }
            
            pOject->Dump(m_pRoot);
            if(m_pRoot)
            {
                json_incref(m_pRoot); // decalare shared ownership.
                bDumpSuccess = (json_dump_file(m_pRoot, Path.c_str(), m_Flags) == 0);
            }
            else
            {
                cout << "Error dumping object! " << endl;
            }
            
            return bDumpSuccess;
        }
    
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

}

#endif















/*
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







