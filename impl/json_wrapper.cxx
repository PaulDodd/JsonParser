

#include "json_wrapper.h"

using namespace std;

class TestClass : public json::CJSONValueObject<TestClass>  // inherit the JSON Object for file parsing.
{
    public:
        TestClass() : CJSONValueObject("", this), testInt(0), testString("") {}
    
        TestClass(int i, string s) : CJSONValueObject("", this), testInt(i), testString(s) {}
    
        void SetupJSONObject()
        {
            //cout << "TestClass::SetupJSONObject"<<endl;
            AddIntegerValue("obj_test_int", &testInt);
            AddStringValue("obj_test_string", &testString);
        }
    
    private:
        int testInt;
        string testString;
};

class tabular : public json::CJSONValueObject<tabular>  // inherit the JSON Object for file parsing.
{
    public:
        tabular() : CJSONValueObject("indent", this)
        {
            length = 0;
            bUseSpace = false;

        }
        ~tabular()
        {
            for(size_t i = 0; i < vec.size(); i++)
                delete vec[i];
            vec.clear();
            
//            for(size_t i = 0; i < vec2.size(); i++)
//                delete vec2[i];
//            vec2.clear();
        }
    
        void SetupJSONObject() // All the code you have to add to parse the file for this object!!!!
        {
            //cout << "tabular::SetupJSONObject"<<endl;
            this->AddIntegerValue("length", &length);
            this->AddBoolValue("use_space", &bUseSpace);
            
            //cout << "Checkpoint 1" << endl;
            // ok there are a few template parameters.
            AddNameValuePair<   std::vector<int*>,
                                json::CJSONValueArray<  int*,
                                                        json::CJSONValuePointer<    int,
                                                                                    json::CJSONValueInt > > >("IntPointerArray", &vec);
        #ifdef c_plus_plus_11
            //cout << "Checkpoint 2" << endl;
            AddNameValuePair<   std::vector< std::shared_ptr<TestClass> >,
                                json::CJSONValueArray<  std::shared_ptr<TestClass>,
                                                        json::CJSONValueSmartPointer<   TestClass,
                                                                                        std::shared_ptr,
                                                                                        json::CJSONValueObject<TestClass> > > >("ObjectPointerArray", &vec2);

        #endif
        }
    
        void AllocateSomeMem()
        {
            char str[12] = "test_string";
            for ( size_t i = 0; i < 10; i++)
            {
                int* pInt = new int;
                *pInt = int(i);
                vec.push_back(pInt);
        #ifdef c_plus_plus_11
                std::shared_ptr<TestClass> pTest( new TestClass(i, string(&str[i])));
                vec2.push_back(pTest);
        #endif
            }
        }
    
        void Print()
        {
            fprintf(stdout, "length = %i, use_space = %s \n", length, (bUseSpace ? "true" : "false"));
        }
    
    private:
        int                     length;
        bool                    bUseSpace;
        vector<int*>            vec;
        #ifdef c_plus_plus_11
        vector< std::shared_ptr<TestClass> >      vec2;
        #endif
    
};

class test : public json::CJSONValueObject<test>
{
    public:
        test(): CJSONValueObject("", this) {}
        test(string path) : CJSONValueObject("", this) // root object has no name. (see parser comment)
        {
            SetupJSONObject();
            LoadFromFile(path);
        }
        ~test() {}
    
    // Code to parse from file!!!
        void SetupJSONObject()
        {
            this->AddStringValue("encoding", &encoding);
            this->AddStringArrayValue("plug-ins", &plugins);
            tabConfig.SetupJSONObject();
            this->AddObjectValue("indent", &tabConfig);
        }
    
        bool LoadFromFile(const string& Path)
        {
            //cout << "test::LoadFromFile"<<endl;
            json::CJSONParser json;
            json.LoadFromFile(Path);    // opens the JSON file and loads the data into buffer.
            
            return json.ParseObject(this); // Parses all the data for this object.
        }
        bool DumpToFile(const string& Path)
        {
            json::CJSONParser json;
            return json.DumpObjectToFile(Path, this);
        }
    
    // End code to parse file.
    
        void Print()
        {
            printf("Encoding = %s \nplugins : \n", encoding.c_str());
            for(size_t i = 0; i < plugins.size(); i++)
            {
                printf("\t %s \n", plugins[i].c_str());
            }
            tabConfig.Print();
        }
        void AllocateSomeMem()
        {
            tabConfig.AllocateSomeMem();
        }
    
    private:
        string          encoding;
        vector<string>  plugins;
        tabular         tabConfig;
    
};

int main(int argc, const char * argv[])
{
    test newtest("test.json");
    newtest.AllocateSomeMem();
    newtest.Print();
    if(!newtest.DumpToFile("dump_smart_pointer.json"))
    {
        cout << "Error dumping the file." << endl;
    }
    
#ifdef c_plus_plus_11                       // c++11 specific testing
    std::shared_ptr<int> pShared(new int);
    std::unique_ptr<int> pUnique(new int);
    
    json::CJSONValueSmartPointer<int, std::shared_ptr, json::CJSONValueInt> jsonShared("SharedPtr", &pShared);
    json::CJSONValueSmartPointer<int, std::unique_ptr, json::CJSONValueInt> jsonUnique("UniquePtr", &pUnique);
    
    
    
    tuple<int, int, double> t1(1, 2, 3.0);
    tuple<json::CJSONValueInt, json::CJSONValueInt, json::CJSONValueDouble> t2( json::CJSONValueInt("Dummy", nullptr),
                                                                                json::CJSONValueInt("Dummy", nullptr),
                                                                                json::CJSONValueDouble("Dummy", nullptr));
    
    json::CJSONValueTuple<      tuple<int, int, double>,    // Templates galore....
                                json::CJSONValueInt, json::CJSONValueInt, json::CJSONValueDouble> mytuple("test", &t1);
    json_t* pRet = NULL;
    mytuple.Dump(pRet);
    json_decref(pRet);
#endif // end c++11 specific testing

    return 0;
}









































