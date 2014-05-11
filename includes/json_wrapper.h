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
#include <tuple>
#include <type_traits>
#include <utility>

using namespace std;


namespace json {

class CJSONValue
{
    public:
        CJSONValue(json_type type, const string& name)
        {
            m_type = type;
            m_name = name;
        }
        virtual ~CJSONValue() {}
    
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
    
    protected:
        json_type   m_type;
        string      m_name;
};

class CJSONValueInt : public CJSONValue // may need an unsigned version of this class.
{
    public:
        CJSONValueInt(const string& name, int * pval) : CJSONValue(JSON_INTEGER, name), m_Value(pval) {}
        ~CJSONValueInt() {}
    
    // Overloaded Methods
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_integer(pVal))
            {
                // cout << "JSON integer found" << endl;
                *m_Value = json_integer_value(pVal);
            }
            else{
                fprintf(stderr, "ERROR: %s is not an integer as expected.", m_name.c_str());
            }
            return bParseSuccess;
        }
    
        bool Dump (json_t*& pRet)
        {
            pRet = json_integer(*m_Value);
            return pRet != NULL;
        }
    
    // Accessor Methods
        const int& GetValue() const { return *m_Value; }
    
    private:
        int* m_Value;
};

class CJSONValueUInt : public CJSONValue // may need an unsigned version of this class.
{
    public:
        CJSONValueUInt(const string& name, size_t* pval) : CJSONValue(JSON_INTEGER, name), m_Value(pval) {}
        ~CJSONValueUInt() {}
    
    // Overloaded Methods
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_integer(pVal))
            {
                // cout << "JSON integer found" << endl;
                *m_Value = json_integer_value(pVal);
            }
            else{
                fprintf(stderr, "ERROR: %s is not an integer as expected.", m_name.c_str());
            }
            return bParseSuccess;
        }
    
        bool Dump (json_t*& pRet)
        {
            pRet = json_integer(*m_Value);
            return pRet != NULL;
        }
    
    // Accessor Methods
        const size_t& GetValue() const { return *m_Value; }
    
    private:
        size_t* m_Value;
};

class CJSONValueFloat : public CJSONValue
{
    public:
        CJSONValueFloat(const string& name, double* pval) : CJSONValue(JSON_REAL, name), m_Value(pval) {}
        ~CJSONValueFloat() {}
    
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_real(pVal))
            {
                // cout << "JSON real found" << endl;
                *m_Value = json_real_value(pVal);
            }
            else{
                fprintf(stderr, "ERROR: %s is not an floating point number as expected.", m_name.c_str());
            }
            return bParseSuccess;
        }
    
        bool Dump (json_t*& pRet)
        {
            pRet = json_real(*m_Value);
            return pRet != NULL;
        }
    
        const double& GetValue() const { return *m_Value; }
    private:
        double* m_Value;
};

class CJSONValueString : public CJSONValue
{
    public:
        CJSONValueString(const string& name, string* pval) : CJSONValue(JSON_STRING, name), m_Value(pval) {}
        ~CJSONValueString() {}
    
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_string(pVal))
            {
                // cout << "JSON string found" << endl;
                *m_Value = json_string_value(pVal);
            }
            else{
                fprintf(stderr, "ERROR: %s is not an string as expected.", m_name.c_str());
            }
            return bParseSuccess;
        }
    
        bool Dump (json_t*& pRet)
        {
            pRet = json_string(m_Value->c_str());
            if(!pRet) cout << "Error could not dump string " << m_name << endl;
            return pRet != NULL;
        }
    
        const string& GetValue() const { return *m_Value; }
    private:
        string* m_Value;
};

class CJSONValueBool : public CJSONValue
{
    public:
        CJSONValueBool(const string& name, bool * pval) : CJSONValue(JSON_STRING, name), m_Value(pval) {}
        ~CJSONValueBool() {}
    
        bool Parse (const json_t* pVal)
        {
            bool bParseSuccess = false;
            if(json_is_boolean(pVal))
            {
                // cout << "JSON boolean found" << endl;
                *m_Value = json_is_true(pVal);
            }
            else{
                fprintf(stderr, "ERROR: %s is not a boolean as expected.", m_name.c_str());
            }
            return bParseSuccess;
        }
    
        bool Dump (json_t*& pRet)
        {
            pRet = m_Value ? json_true() : json_false();
            return pRet != NULL;
        }
    
        const bool& GetValue() const { return *m_Value; }
    private:
        bool* m_Value;
};


// Will have to think about how to make this work for fixed array types of containers.
// Implemeted to work with the std::vector class for ease.
// ?? Could/Should make a CJSONValueFixedArray<> class and change the name of this class to be CJSONValueDynamicArray or CJSONValueVector ??
template <class TVal, class JVal>
class CJSONValueArray : public CJSONValue
{
    public:
        CJSONValueArray(const string& name, vector<TVal>* pval) : CJSONValue(JSON_ARRAY, name), m_Value(pval) {}
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
                    TVal temp;
                    char array_number[30]; // should be enough space.
                    sprintf(&array_number[0], "-%zu", i);
                    JVal tjson(m_name + string(array_number), &temp);
                    data = json_array_get(pVal, i);
                    bParseSuccess = tjson.Parse(data) && bParseSuccess;
                    
                    m_Value->push_back(temp);
                }
            }
            else{
                fprintf(stderr, "ERROR: %s is not an array as expected.", m_name.c_str());
            }

            return bParseSuccess;
        }
    
        bool Dump (json_t*& pRet)
        {
            bool bDumpSuccess = true;
            pRet = json_array();
            if(pRet)
            {
                for( size_t i = 0; i < m_Value->size(); i++)
                {
                    json_t* pVal = NULL;
                    JVal tjson("", &((*m_Value)[i]));
                    
                    cout << "pVal @ " << pVal << endl;
                    bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;
                    cout << "pVal @ " << pVal << endl;
                    
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
            return bDumpSuccess;
        }
    
        const TVal& GetValue() const { return *m_Value; }
    private:
        vector<TVal>* m_Value;
};


// This requires c++11.
#if __cplusplus >= 201103L


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
    return false; // Index out of range so return NULL.
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
        CJSONValueTuple(const string& name, tuple<TVals...>* pval) : CJSONValue(JSON_ARRAY, name), m_Value(pval) {}
    
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
                        // std::get<i>(*m_Value) = temp;
                        put<0, bool, TVals...>(*m_Value, temp, i);
                    }
                    else if(json_is_integer(data))
                    {
                        int temp;
                        CJSONValueInt jtemp(elemName, &temp);
                        jtemp.Parse(data);
                        put<0, int, TVals...>(*m_Value, temp, i);
                    }
                    else if(json_is_real(data))
                    {
                        double temp;
                        CJSONValueFloat jtemp(elemName, &temp);
                        jtemp.Parse(data);
                        put<0, double, TVals...>(*m_Value, temp, i);
                    }
                    else if(json_is_string(data))
                    {
                        string temp;
                        CJSONValueString jtemp(elemName, &temp);
                        jtemp.Parse(data);
                        put<0, string, TVals...>(*m_Value, temp, i);
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
                for(size_t i = 0; i < std::tuple_size< tuple<TVals...> >::value; i++)
                {
                    json_t* pVal = NULL;
                    

                    // cout << "data @ " << pTemp<< " = "<< *pTemp << endl;
                    if(is_type<0, bool, TVals...>(*m_Value, i))
                    {
                        auto* pTemp = pull<0, bool, TVals...>(*m_Value, i);
                        CJSONValueBool tjson("", pTemp);
                        bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;
                    }
                    else if(is_type<0, int, TVals...>(*m_Value, i))
                    {
                        auto* pTemp = pull<0, int, TVals...>(*m_Value, i);
                        CJSONValueInt tjson("", pTemp);
                        bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;
                    }
                    else if(is_type<0, double, TVals...>(*m_Value, i) ||
                            is_type<0, float, TVals...>(*m_Value, i) )
                    {
                        auto* pTemp = pull<0, double, TVals...>(*m_Value, i);
                        CJSONValueFloat tjson("", pTemp);
                        bDumpSuccess = tjson.Dump(pVal) && bDumpSuccess;
                    }
                    else if(is_type<0, string, TVals...>(*m_Value, i))
                    {
                        auto* pTemp = pull<0, string, TVals...>(*m_Value, i);
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
            return bDumpSuccess;
        }
    
    
    private:
        tuple<TVals...>* m_Value;
        //vector<type_info> m_Types;
};

#endif

class CJSONValueObject : public CJSONValue
{
    public:
        CJSONValueObject(const string& name, CJSONValueObject* pval) : CJSONValue(JSON_OBJECT, name), m_pDerived(pval) {}
        ~CJSONValueObject() { Destroy(); }
    
        void Destroy()
        {
            //cout << "Destroying objects!" << endl;
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
            bool bParseSucess = true;
            
            
            cout << "Calling CJSONValueObject::Parse" << endl;
            
            json_t* data;
            map<string, CJSONValue* >::iterator iter;
            for(iter = m_Map.begin(); iter != m_Map.end(); iter++)
            {
                data = json_object_get(pVal, iter->first.c_str());
                bParseSucess = iter->second->Parse(data) && bParseSucess;
            }
            
            return bParseSucess;
        }
    
        virtual bool Dump (json_t*& pRet)
        {
            cout << "Calling CJSONValueObject::Dump" << endl;
            bool bDumpSuccess = true;
            
            pRet = json_object();
            
            cout << "Dumping object " << m_name << " @ " << pRet << endl;
            
            if(pRet)
            {
                map<string, CJSONValue* >::iterator iter;
                for(iter = m_Map.begin(); iter != m_Map.end(); iter++)
                {
                    json_t* value = NULL;
                    if ( iter->second->Dump(value) )
                    {
                        if ( json_object_set_nocheck(pRet, iter->first.c_str(), value) == -1)
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
            
            return bDumpSuccess;
        }
    
    // Abstract methods
        virtual void SetupJSONObject() = 0;
    
    // Class methods
        template<class TVal, class JVal>
        void AddNameValuePair(const string& name, TVal* pval)
        {
            m_Map.insert( pair< string, CJSONValue* >(name, new JVal(name, pval)));
        }
    
        // Could Remove the following becasuse of the encompassing method above.
        void AddIntegerValue(const string& name, int* pval)
        {
            m_Map.insert(pair<string, CJSONValue* >(name,  new CJSONValueInt(name, pval)));
        }
        void AddUIntegerValue(const string& name, size_t* pval)
        {
            m_Map.insert(pair<string, CJSONValue* >(name,  new CJSONValueUInt(name, pval)));
        }
        void AddBoolValue(const string& name, bool* pval)
        {
            m_Map.insert(pair<string, CJSONValue* >(name,  new CJSONValueBool(name, pval)));
        }
        void AddStringValue(const string& name, string* pval)
        {
            m_Map.insert(pair<string, CJSONValue* >(name,  new CJSONValueString(name, pval)));
        }
        void AddStringArrayValue(const string& name, vector<string>* pval)
        {
            m_Map.insert(pair<string, CJSONValue* >(name,  new CJSONValueArray<string, CJSONValueString>(name, pval)));
        }
    
        void AddObjectValue(const string& name, CJSONValueObject* pval)
        {
            m_Map.insert(pair<string, CJSONValue* >(name,  pval));
        }
    
        CJSONValueObject* GetDerived() { return m_pDerived; }
    
    private:
        CJSONValueObject*                               m_pDerived;         // ?? remove this ??
        map < string, CJSONValue* >                     m_Map;              // map for each element in the object at this level. How to access data?
};


// This class is used to perform the file IO and interface with the
// object/data classes defined above. If the order of the objects in
// the file is known then there is no limitation to how many you can
// have but could make the higher level protocols more complicated.
// ?? object-name: "name" for parsing different classes in the same file ??
class CJSONParser
{
    public:
        CJSONParser() : m_pRoot(NULL)
        {
            cout << "json parser created." << endl;
            m_Flags = JSON_INDENT(4);
        }
    
        ~CJSONParser()
        {
            json_decref(m_pRoot); // better way to delete ?
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
            size_t flags = JSON_INDENT(4);
            if(pOject->Dump(m_pRoot))
            {
                bDumpSuccess = (json_dump_file(m_pRoot, Path.c_str(), flags) == 0);
            }
            else
            {
                cout << "Error dumping object! " << endl;
            }
            
            
            return bDumpSuccess;
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























