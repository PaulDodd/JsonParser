

#include "json_wrapper.h"

class tabular : public json::CJSONValueObject  // inherit the JSON Object for file parsing.
{
    public:
        tabular() : CJSONValueObject("indent", this)
        {
            length = 0;
            bUseSpace = false;
        }
    
        void SetupJSONObject() // All th code you have to add to parse the file for this object!!!!
        {
            this->AddIntegerValue("length", &length);
            this->AddBoolValue("use_space", &bUseSpace);
        }
    
        void Print()
        {
            fprintf(stdout, "length = %i, use_space = %s \n", length, (bUseSpace ? "true" : "false"));
        }
    
    private:
        int     length;
        bool    bUseSpace;
    
};

class test : public json::CJSONValueObject
{
    public:
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
            json::CJSONParser json;
            json.LoadFromFile(Path);    // opens the JSON file and loads the data into buffer.
            if(json.IsRootArray())      // expecting an array of objects
            {
                json.ParseObjectFromArray(0, this); // Parses all the data for this object.
            }
            return true;
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
    
    private:
        string          encoding;
        vector<string>  plugins;
        tabular         tabConfig;
    
};

int main(int argc, const char * argv[])
{
    test newtest("test.json");
    newtest.Print();
    if(!newtest.DumpToFile("dump.json"))
    {
        cout << "Error dumping the file." << endl;
    }
    return 0;
}









































