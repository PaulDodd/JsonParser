

#include "json_wrapper.h"
#include <sstream>

using namespace std;

class TestClass : public json::CJSONValueObject<TestClass>  // inherit the JSON Object for file parsing.
{
    public:
        TestClass() : CJSONValueObject("TestClass", this), testInt(0), testString("") { }
    
        TestClass(int i, string s) : CJSONValueObject("TestClass", this), testInt(i), testString(s) {}
    
        TestClass( const TestClass& src) : CJSONValueObject("TestClass", this)
        {
            cout << "copy called"<< endl;
            CopyFrom(src);
            SetupJSONObject();
        }
    
        ~TestClass() { }//cout << "Destryoing TestClass object" << endl; Print(); }
    
        void SetupJSONObject()
        {
            //cout << "TestClass::SetupJSONObject"<<endl;
            AddIntegerValue("obj_test_int", &testInt);
            AddStringValue("obj_test_string", &testString);
        }
    
        void Print()
        {
            cout <<" int: " << testInt << endl;
            cout <<" string: " << testString << endl;
            
        }
    
        const int& getint() const { return testInt; }
        const string& getstring() const { return testString; }
    
        TestClass& CopyFrom(const TestClass& src)
        {
            testInt = src.getint();
            testString = src.getstring();
            return *this;
        }
        TestClass& operator=(const TestClass& src) { return CopyFrom(src); }
    private:
        int     testInt;
        string  testString;
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
            {
                if( vec[i] )
                    delete vec[i];
                vec[i] = NULL;
            }
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
        test(): CJSONValueObject("test", this) { cout << &testObjVec << endl;}
        test(string path) : CJSONValueObject("", this) // root object has no name. (see parser comment)
        {
            SetupJSONObject();
            LoadFromFile(path);
        }
        ~test() {
//            cout << "destroying test" << endl;
//            cout << "encoding:" << &encoding << endl;
//            cout << "plugins:" << &plugins << endl;
//            cout << "tabConfig:" << &tabConfig << endl;
//            cout << "testObjVec:" << &testObjVec << endl;
          }
    
    // Code to parse from file!!!
        void SetupJSONObject()
        {
            this->AddStringValue("encoding", &encoding);
            this->AddStringArrayValue("plug-ins", &plugins);
            tabConfig.SetupJSONObject();
            this->AddObjectValue("indent", &tabConfig);
            
            AddNameValuePair<   std::vector< TestClass >,
                                json::CJSONValueArray<  TestClass,
                                                        json::CJSONValueObject<TestClass> > > ("ObjectArray", &testObjVec);
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
            cout << "printing " << &testObjVec << endl;
            for(size_t i = 0; i < testObjVec.size(); i++)
            {
                testObjVec[i].Print();
            }
            cout << "printing out " << &testObjVec << endl;
        }
        void AllocateSomeMem()
        {
            tabConfig.AllocateSomeMem();
            for(size_t i = 0; i < 2; i++)
            {
                stringstream ss;
                ss << i;
                string s;
                ss>>s;
                TestClass t(i, s);
                testObjVec.push_back(t);
            }
        }
    
    private:
        string              encoding;
        vector<string>      plugins;
        tabular             tabConfig;
        vector<TestClass>   testObjVec;
    
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
//    std::shared_ptr<int> pShared(new int);
//    std::unique_ptr<int> pUnique(new int);
//    
//    json::CJSONValueSmartPointer<int, std::shared_ptr, json::CJSONValueInt> jsonShared("SharedPtr", &pShared);
//    json::CJSONValueSmartPointer<int, std::unique_ptr, json::CJSONValueInt> jsonUnique("UniquePtr", &pUnique);
//    
//    
//    
//    tuple<int, int, double> t1(1, 2, 3.0);
//    tuple<json::CJSONValueInt, json::CJSONValueInt, json::CJSONValueDouble> t2( json::CJSONValueInt("Dummy", nullptr),
//                                                                                json::CJSONValueInt("Dummy", nullptr),
//                                                                                json::CJSONValueDouble("Dummy", nullptr));
//    
//    json::CJSONValueTuple<      tuple<int, int, double>,    // Templates galore....
//                                json::CJSONValueInt, json::CJSONValueInt, json::CJSONValueDouble> mytuple("test", &t1);
//    json_t* pRet = NULL;
//    mytuple.Dump(pRet);
//    json_decref(pRet);
#endif // end c++11 specific testing

    test parseTest;
    parseTest.SetupJSONObject();
    parseTest.LoadFromFile("dump_smart_pointer.json");

    parseTest.Print();
    
    
    cout << "Exiting program" << endl;
    return 0;
}









































