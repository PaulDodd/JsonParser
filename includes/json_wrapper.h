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

class CJSONValueFloat : public CJSONValue
{
    public:
        CJSONValueFloat(const string& name) : CJSONValue(JSON_REAL, name), m_Value(NULL) {}
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
                        cout << "Error could not dump array element. "<< ((*m_Value)[i]) << endl;
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
        bool Parse (const json_t* pVal)
        {
            bool bParseSucess = true;
            
            json_t* data;
            map<string, CJSONValue* >::iterator iter;
            for(iter = m_Map.begin(); iter != m_Map.end(); iter++)
            {
                data = json_object_get(pVal, iter->first.c_str());
                bParseSucess = iter->second->Parse(data) && bParseSucess;
            }
            
            return bParseSucess;
        }
    
        bool Dump (json_t*& pRet)
        {
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
        void AddIntegerValue(const string& name, int* pval)
        {
            m_Map.insert(pair<string, CJSONValue* >(name,  new CJSONValueInt(name, pval)));
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























