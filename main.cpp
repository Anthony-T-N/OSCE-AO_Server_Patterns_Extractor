#define CURL_STATICLIB
#include <curl/curl.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <map>

// https://stackoverflow.com/questions/21873048/getting-an-error-fopen-this-function-or-variable-may-be-unsafe-when-complin/21873153
#pragma warning(disable:4996);

// Global Variables
std::string current_root_folder = "";
const std::string first_url_section = "http://osce14-p.activeupdate.trendmicro.com/activeupdate/";
const std::string generic_download_path = "\\pattern\\";
float progress = 0.0;
int bar_width = 100;
std::map<std::string, float> component_map;
std::map<std::string, int> file_integrity_count_map;
int total_file_count = 0;

class Common_Class
{       
    public:
        void temp_method()
        {
            std::vector<std::string> temp_vector;
            bool PLMComponentList_switch = false;
            bool other_switch = false;

            // 1) Open server.ini file;
            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            // 2) Locate PLMComponentList;
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line == "" && other_switch == false)
                {
                    PLMComponentList_switch = false;
                }
                else if (input_file_line.find("PLMComponentList") != std::string::npos)
                {
                    PLMComponentList_switch = true;
                    other_switch = true;
                    continue;
                }
                if ((input_file_line.find(".zip") != std::string::npos && input_file_line.find("product/") != std::string::npos) && PLMComponentList_switch == true)
                {
                    input_file_line.erase(0, input_file_line.find_first_of("=") + 1);
                    temp_vector.push_back(input_file_line);
                }
            }
            for (int i = 0; i <= temp_vector.size() - 1; i++)
            {
                std::cout << "[" << i << "]" << " " << temp_vector[i] << "\n";
            }
        }
        /*
        Method to count all lines in the server.ini file that contain pattern/ , engine/ , product/ for integrity purposes.
        */
        void OSCE_AO_file_integrity_check(std::string input_file_line)
        {
            if (input_file_line.find("pattern/") != std::string::npos)
            {
                file_integrity_count_map["pattern/"]++;
            }
            else if (input_file_line.find("engine/") != std::string::npos)
            {
                file_integrity_count_map["engine/"]++;
            }
            else if (input_file_line.find("product/") != std::string::npos)
            {
                file_integrity_count_map["product/"]++;
            }
            total_file_count++;
        }
        static size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream)
        {
            size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
            return written;
        }
        void extract_serverini_file()
        {
            const char* av_url = "http://osce14-p.activeupdate.trendmicro.com/activeupdate/server.ini";
            std::string temp_int_filepath = std::filesystem::current_path().string() + "\\temp.ini";
            char temp_filename[FILENAME_MAX];
            // https://stackoverflow.com/questions/41915130/initializing-an-array-of-characters-with-a-string-variable
            strcpy(temp_filename, temp_int_filepath.c_str());
            std::cout << "[!] Temp filename/location: " << temp_filename << "\n\n";
            download_file(av_url, temp_filename);
        }
        static void download_file(const char* url, const char* full_pathname)
        {
            std::cout << "[!] Downloading the following: " << "\n";
            std::cout << url << "\n";
            std::cout << "[!] Downloading to: " << "\n";
            std::cout << full_pathname << "\n\n";

            CURL* curl;
            FILE* fp;
            CURLcode res;
            curl = curl_easy_init();
            if (curl)
            {
                fp = fopen(full_pathname, "wb");
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                res = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                if (res != CURLE_OK)
                {
                    fprintf(stderr, "curl_easy_perform() failed: %s\n",
                        curl_easy_strerror(res));
                    std::cout << "\033[31m" << "WARNING WARNING WARNING" << "\033[0m" << "\n";
                }
                fclose(fp);
            }
        }
        void comment_server_section()
        {
            // Function must occur after function root_folder_creation().
            std::ifstream input_file;
            std::cout << "[!] Opening temp.ini for reading;" << "\n";
            if (std::filesystem::exists("temp.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open temp.ini;" << "\033[0m" << "\n";
                return;
            }
            input_file.open("temp.ini");
            std::cout << "[+] Opened temp.ini successfully;" << "\n\n";

            std::ofstream output_file;
            output_file.open(current_root_folder + "/server.ini");
            std::cout << "[+] Created server.ini successfully;" << "\n";

            // Cannot edit individual lines of an existing text file. Edited copy will need to be made.
            std::string input_file_line;
            bool commenting_enabled = false;
            bool server_section_found = false;
            bool tmfwptn_switch = false;
            bool engine_switch = false;

            while (std::getline(input_file, input_file_line))
            {
                OSCE_AO_file_integrity_check(input_file_line);

                if (input_file_line.find("[Server]") != std::string::npos)
                {
                    std::cout << "[+] [Server] Section Found;" << "\n";
                    //std::cout << input_file_line << "\n\n";
                    server_section_found = true;
                    commenting_enabled = true;
                    output_file << input_file_line << "\n";
                    continue;
                }
                // Continue to comment [Server] section until empty space is located.
                if (commenting_enabled == true && input_file_line == "")
                {
                    commenting_enabled = false;
                }
                if (commenting_enabled == true)
                {
                    input_file_line = ";" + input_file_line;
                }
                output_file << input_file_line << "\n";
                

                // Temporary if statement to test progress bar.
                if (input_file_line.find("v_") != std::string::npos && input_file_line.find("P.4") != std::string::npos || input_file_line.find("vsapi") != std::string::npos && input_file_line.find("P.4") != std::string::npos)
                {
                    component_map["vsapi"]++;
                }
                // Go through all lines in the "server.ini" file until a line contains "icrc".
                if (input_file_line.find("icrc") != std::string::npos)
                {
                    component_map["icrc"]++;
                }
                if (input_file_line.find("tsc") != std::string::npos || input_file_line.find("tscptn") != std::string::npos)
                {
                    component_map["tscptn"]++;
                }
                if (input_file_line.find("w_") != std::string::npos || input_file_line.find("tmwhite") != std::string::npos)
                {
                    component_map["tmwhite"]++;
                }
                if (input_file_line.find("ssa_") != std::string::npos || input_file_line.find("ssaptn") != std::string::npos)
                {
                    component_map["ssaptn"]++;
                }
                if (input_file_line.find("sspda6_") != std::string::npos)
                {
                    component_map["sspda6_"]++;
                }
                // INFO: Identifies line with "tmfwptn" and extract all lines below it.
                if (input_file_line == "")
                {
                    tmfwptn_switch = false;
                }
                else if (input_file_line.find("tmfwptn") != std::string::npos || tmfwptn_switch == true)
                {
                    tmfwptn_switch = true;
                    component_map["tmfwptn"]++;
                }
                // [-] TODO: Testing shows 128%. Fix progress value of "trendxlm_".
                if (input_file_line.find("trendxlm_") != std::string::npos)
                {
                    component_map["trendxlm_"]++;
                }
                if (input_file_line == "")
                {
                    engine_switch = false;
                }
                else if (input_file_line.find("[ENGINE]") != std::string::npos || engine_switch == true)
                {
                    engine_switch = true;
                    component_map["[ENGINE]"]++;
                }
                if (input_file_line.find(".zip") != std::string::npos && input_file_line.find("product/") != std::string::npos)
                {
                    component_map["PLMComponentList"]++;
                }
            }
            std::cout << "\n";
            std::cout << "[DEBUG] Component Count Summary" << "\n";
            for (auto const& [key, val] : component_map)
            {
                std::cout << key        // string (key)
                    << ':'
                    << val        // string's value
                    << "\n";
            }
            std::cout << "\n";
            std::cout << "[DEBUG] File Integrity Count Summary" << "\n";
            for (auto const& [key, val] : file_integrity_count_map)
            {
                std::cout << key        // string (key)
                    << ':'
                    << val        // string's value
                    << "\n";
            }
            std::cout << "\n";
            input_file.close();
            output_file.close();
            if (server_section_found == false)
            {
                std::cout << "\033[4;31m" << "[-] WARNING: [Server] Section was not found;" << "\033[0m" << "\n";
                std::cout << "[!] Proceed with caution;" << "\n";
            }
            remove("temp.ini");
            std::cout << "[+] Deleted temp.ini successfully;" << "\n\n";
        }
        void root_folder_creation()
        {
            time_t rawtime;
            struct tm* timeinfo;
            char buffer[80];

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
            std::cout << "[!] Root folder name: " << buffer << "\n\n";

            std::string root_folder_name(buffer);
            std::filesystem::create_directories(root_folder_name);
            current_root_folder = root_folder_name;
            std::filesystem::create_directories(current_root_folder + "/pattern");
        }
        static std::string sig_builder(std::string extracted_string)
        {
            // Assumes extracted string has been processed by the url_builder(str) function.
            extracted_string.erase(extracted_string.find_last_of(".") + 1);
            return extracted_string + "sig";
        }
        static std::string url_builder(std::string extracted_string)
        {
            extracted_string.erase(extracted_string.find_first_of(","));
            extracted_string.erase(0, extracted_string.find_first_of("=") + 1);
            std::string final_string = first_url_section + extracted_string;
            return final_string;
        }
        static std::string file_download_name(std::string url_name)
        {
            url_name.erase(0, url_name.find_last_of("/") + 1);
            return url_name;
        }
        static void downloading_progress_bar(int &bar_width, float &progress_bar_value)
        {
            std::cout << "\033[1;97m" << "[" << "\033[0m";
            int pos = bar_width * progress;
            for (int i = 0; i < bar_width; i++)
            {
                if (i < pos)
                {
                    std::cout << "\033[1;97m" << "=" << "\033[0m";
                }
                else if (i == pos)
                {
                    std::cout << "\033[1;97m" << ">" << "\033[0m";
                }
                else
                {
                    std::cout << " ";
                }
            }
            std::cout << "\033[1;97m" << "] " << "\033[0m" << int(progress * 100.0) << " %\n";
            // [+] TODO: Determine total files to download beforehand. 100 / Total number of files = progress.
            progress += (100 / (progress_bar_value - 1)) / 100;
            std::cout << "\n";
        }
        static void download_file_allocation(std::string &input_file_line, std::string download_path, float &progress_bar_value)
        {
            std::string extracted_url = Common_Class::url_builder(input_file_line);
            std::string full_download_path = current_root_folder + download_path + Common_Class::file_download_name(extracted_url);

            char extracted_url_char[FILENAME_MAX];
            char full_download_path_char[FILENAME_MAX];

            strcpy(extracted_url_char, extracted_url.c_str());
            strcpy(full_download_path_char, full_download_path.c_str());
            Common_Class::download_file(extracted_url_char, full_download_path_char);

            extracted_url = Common_Class::sig_builder(extracted_url);
            full_download_path = current_root_folder + download_path + Common_Class::file_download_name(Common_Class::sig_builder(extracted_url));

            strcpy(extracted_url_char, extracted_url.c_str());
            strcpy(full_download_path_char, full_download_path.c_str());
            Common_Class::download_file(extracted_url_char, full_download_path_char);

            Common_Class::downloading_progress_bar(bar_width, progress_bar_value);
        }

        // TODO: Consolidate all common elements of all classes into a static method.
        static void open_ini()
        {
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
        }
};

class ICRC_Class
{
    public:
        void icrc_pattern_identification()
        {
            std::filesystem::create_directories(current_root_folder + "/pattern/icrc");
            const std::string icrc_download_path = "\\pattern\\icrc\\";
            
            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "icrc".
                if (input_file_line.find("icrc") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, icrc_download_path, component_map["icrc"]);
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

class VSAPI_Class
{
    public:
        void vsapi_pattern_identification()
        {
            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "vsapi".
                if (input_file_line.find("v_") != std::string::npos && input_file_line.find("P.4") != std::string::npos || input_file_line.find("vsapi") != std::string::npos && input_file_line.find("P.4") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path, component_map["vsapi"]);
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

class TSCPTN_Class
{
    public:
        void tscptn_pattern_identification()
        {
            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "tscptn" or "tsc".
                if (input_file_line.find("tsc") != std::string::npos || input_file_line.find("tscptn") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path, component_map["tscptn"]);
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

class TMWHITE_Class
{
    public:
        void tmwhite_pattern_identification()
        {
            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line.find("w_") != std::string::npos || input_file_line.find("tmwhite") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path, component_map["tmwhite"]);
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

class SSAPTN_Class
{
    public:
        void ssaptn_pattern_identification()
        {
            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line.find("ssa_") != std::string::npos || input_file_line.find("ssaptn") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path, component_map["ssaptn"]);
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

class SSPDA6_Class
{
    public:
        void sspda6_pattern_identification()
        {
            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line.find("sspda6_") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path, component_map["sspda6_"]);
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

// UNIQUE CLASS - Downloads a diverse range of files.
class TMFWPTN_Class
{
    public:
        void tmfwptn_pattern_identification()
        {
            bool tmfwptn_switch = false;

            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            // INFO: Identifies line with "tmfwptn" and extract all lines below it.
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line == "")
                {
                    tmfwptn_switch = false;
                }
                else if (input_file_line.find("tmfwptn") != std::string::npos || tmfwptn_switch == true)
                {
                    tmfwptn_switch = true;
                    if (input_file_line.find("rr") != std::string::npos)
                    {
                        std::filesystem::create_directories(current_root_folder + "/pattern/" + "rr\\");
                        Common_Class::download_file_allocation(input_file_line, (generic_download_path + "rr\\"), component_map["tmfwptn"]);
                    }
                    else
                    {
                        Common_Class::download_file_allocation(input_file_line, generic_download_path, component_map["tmfwptn"]);
                    }
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

// [+] TODO: Does not download P.48040252=pattern/tmtbd64_en_257364.zip,257364,196355
//                             P.48040251=pattern/tmtbd_en_257300.zip,257300,200743
class TRENDXLM_Class
{
    public:
        void trendxlm_pattern_identification()
        {
            bool trendxlm_switch = false;

            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line == "")
                {
                    trendxlm_switch = false;
                }
                else if (input_file_line.find("trendxlm") != std::string::npos)
                {
                    trendxlm_switch = true;
                }
                // Go through all lines in the "server.ini" file until a line contains "tscptn" or "tsc".
                if (input_file_line.find("pattern") != std::string::npos && trendxlm_switch == true)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path, component_map["trendxlm_"]);
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

// Class contains tailored download_file_allocation method.
class ENGINE_Class
{
    public:
        void engine_pattern_identification()
        {
            // INFO: Class specific operations here.
            bool engine_switch = false;
            bool SSAPI_switch = false;
            std::string ssapi_temp_path = "";
            std::filesystem::create_directories(current_root_folder + "/engine/");

            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");
            
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line == "")
                {
                    engine_switch = false;
                }
                else if (input_file_line.find("[ENGINE]") != std::string::npos)
                {
                    engine_switch = true;
                    continue;
                }
                // If line contains two /. Create folder between both /
                if (engine_switch == true && input_file_line.find_first_of("/") != input_file_line.find_last_of("/"))
                {
                    /*
                    E.22000010=SSENGINE_SSAPI32_V6,engine/ssapi32_v6/SSAPI32_v62-4015.zip,6.2.4015,592305,6.0.1000
                    E.22000020=SSENGINE_SSAPI64_V6,engine/ssapi64_v6/SSAPI64_v62-4015.zip,6.2.4015,897467,6.0.1000
                    */

                    std::string temp_line = input_file_line;
                    temp_line.erase(0, temp_line.find_first_of("/"));
                    temp_line.erase(temp_line.find_last_of("/") + 1);
                    std::filesystem::create_directories(current_root_folder + "\\engine\\" + temp_line);
                    ssapi_temp_path = temp_line;
                    SSAPI_switch = true;
                }
                if (engine_switch == true)
                {
                    // E.4=VSAPI32_NT_I386,engine/engv_nt386_v12500-1004.zip,12.500.1004,1350445,6.510.1002
                    std::string extracted_url = input_file_line;
                    extracted_url.erase(0, extracted_url.find_first_of(",") + 1);
                    // engine/engv_nt386_v12500-1004.zip,12.500.1004,1350445,6.510.1002
                    std::cout << "DEBUG: " << extracted_url << "\n";
                    extracted_url.erase(extracted_url.find_first_of(","));
                    // engine/engv_nt386_v12500-1004.zip
                    std::cout << "DEBUG: " << extracted_url << "\n";
                    extracted_url = first_url_section + extracted_url;
                    // URL Building ends here.
                    // http://osce14-p.activeupdate.trendmicro.com/activeupdate/engine/engv_nt386_v12500-1004.zip
                    std::cout << "DEBUG: " << extracted_url << "\n";
                    
                    std::string full_download_path = "";
                    if (SSAPI_switch == false)
                    {
                        full_download_path = current_root_folder + "\\engine\\" + Common_Class::file_download_name(extracted_url);
                    }
                    else
                    {
                       full_download_path = current_root_folder + "\\engine\\" + ssapi_temp_path + Common_Class::file_download_name(extracted_url);
                    }

                    char extracted_url_char[FILENAME_MAX];
                    char full_download_path_char[FILENAME_MAX];

                    strcpy(extracted_url_char, extracted_url.c_str());
                    strcpy(full_download_path_char, full_download_path.c_str());
                    Common_Class::download_file(extracted_url_char, full_download_path_char);

                    // SIG BUILDING HERE
                    std::cout << "SIG BUILDING HERE" << "\n";
                    extracted_url = Common_Class::sig_builder(extracted_url);
                    std::cout << "DEBUG: " << extracted_url << "\n";
                    if (SSAPI_switch == false)
                    {
                        full_download_path = current_root_folder + "\\engine\\" + Common_Class::file_download_name(Common_Class::sig_builder(extracted_url));
                    }
                    else
                    {
                        full_download_path = current_root_folder + "\\engine\\" + ssapi_temp_path + Common_Class::file_download_name(Common_Class::sig_builder(extracted_url));
                    }
                    std::cout << "DEBUG: " << full_download_path << "\n";

                    strcpy(extracted_url_char, extracted_url.c_str());
                    strcpy(full_download_path_char, full_download_path.c_str());
                    Common_Class::download_file(extracted_url_char, full_download_path_char);
                    Common_Class::downloading_progress_bar(bar_width, component_map["[ENGINE]"]);
                    SSAPI_switch = false;
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

// [+] TODO: Missing following files:
/*
[Info_602_10000_1_1]
Version=6.0
Build=3011
Path=product/enu/AddonSvcTmeeDepTool.zip,217053835
PA_Version=6.0.3011
Update_Path=product/enu/TmeeDepPatchAgent.zip,209602
Min=1.0
Max=7.0
*/
class PLMComponentList_Class
{
    public:
        void PLMComponentList_pattern_identification()
        {
            std::ifstream input_file;
            Common_Class::open_ini();
            input_file.open(current_root_folder + "/server.ini");

            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // PLMComponentList Detection First.
                if (input_file_line.find(".zip") != std::string::npos && input_file_line.find("product/") != std::string::npos)
                {
                    // http://osce14-p.activeupdate.trendmicro.com/activeupdate/product/osce14/enu/AddonSvcTMSM.zip - Massive file.

                    // New function here:
                    // Path=product/osce14/deu/AddonSvcTMSM.zip,626808853
                    // product/osce14/xxxx/AddonSvcTMSM.zip
                    // Create /xxxx/ directory.

                    std::string country_code = input_file_line;
                    country_code.erase(country_code.find_last_of("/"));
                    country_code.erase(0, country_code.find_last_of("/") + 1);
                    /*
                    std::cout << country_code << "\n";
                    std::cout << current_root_folder + "\\pattern\\product\\osce14\\" + country_code + "\\" << "\n";
                    //std::cin.get();
                    */

                    // Directory check here.
                    if (std::filesystem::is_directory(current_root_folder + "\\pattern\\product\\osce14\\" + country_code) == false)
                    {
                        std::filesystem::create_directories(current_root_folder + "\\pattern\\product\\osce14\\" + country_code + "\\");
                    }
                    if (std::filesystem::is_directory(current_root_folder + "\\pattern\\product\\" + country_code) == false)
                    {
                        std::filesystem::create_directories(current_root_folder + "\\pattern\\product\\" + country_code + "\\");
                    }
                    std::cout << input_file_line << "\n";
                    if (input_file_line.find("AddonSvcTMSM.zip") != std::string::npos)
                    {
                        // 2021-12-31\pattern\product\osce14\enu\AddonSvcTMSM.zip -> Large File -> Skip
                        continue;
                    }
                    // [-] TODO: Check size of file before downloading.
                    if (input_file_line.find("osce14") != std::string::npos)
                    {
                        //std::cout << "Contains osce14" << "\n";
                        //std::cout << input_file_line << "\n";
                        //std::cout << "\n";
                        Common_Class::download_file_allocation(input_file_line, (generic_download_path + "product\\osce14\\" + country_code + "\\"), component_map["PLMComponentList"]);
                    }
                    else
                    {
                        //std::cout << "===Does not contain osce14===" << "\n";
                        //std::cout << input_file_line << "\n";
                        //std::cout << "\n";
                        Common_Class::download_file_allocation(input_file_line, (generic_download_path + "product\\" + country_code + "\\"), component_map["PLMComponentList"]);
                    }
                }
            }
            input_file.close();
            progress = 0.0;
        }
};

int main()
{
    std::cout << "=======================================" << "\n";
    std::cout << "- OSCE-AO_Server_Patterns_Extractor console application" << "\n";
    std::cout << "- Console Application Version: 1" << "\n";
    std::cout << "- Created By: Anthony N." << "\n";
    std::cout << "- Current location of executable: " << std::filesystem::current_path() << "\n";
    std::cout << "=======================================" << "\n\n";

    Common_Class baseline_obj;
    baseline_obj.extract_serverini_file();
    baseline_obj.root_folder_creation();
    baseline_obj.comment_server_section();

    /* REMOVE AFTER TESTING TEMP_METHOD; */
    Common_Class().temp_method();
    std::cout << "Total_File_Count: " << total_file_count << " " << total_file_count*2 << "\n";

    std::vector<std::string> options_vector = 
    {
        "Download ICRC (Smart Scan Pattern(s)) files",
        "Download VSAPI (Virus Pattern(s)) files",
        "Download TSCPTN (Unknown Pattern(s)) files",
        "Download TMWHITE (IntelliTrap Pattern(s)) files",
        "Download SSAPTN (Spyware Active - Monitoring Pattern(s)) files",
        "Download SSPDA6 (Unknown Pattern(s)) files",
        "Download TMFWPTN (Unknown Pattern(s)) files",
        "Download TRENDXLM (Unknown Pattern(s)) files",
        "Download ENGINE (Unknown Pattern(s)) files",
        "Download PLMComponentList (Unknown Pattern(s)) files",
        "Download ALL pattern files"
    };
    while (true)
    {
        // [+] TODO: Reset global variables (Example: Progress bar).
        // [+] TODO: Other patterns cannot be downloaded unless ICRC patterns are downloaded first. Error unknown.
        // -> Solution: ICRC class/function creates "pattern" folder. Other class/functions do not.
        std::cout << "Select an option:" << "\n";
        for (int i = 0; i <= options_vector.size() - 1; i++)
        {
            std::cout << "[" << i << "]" << " " << options_vector[i] << "\n";
        }
        std::cout << "[exit] Exit" << "\n";
        std::cout << "Selection ?:" << "\n";
        std::cout << "> ";
        std::string user_input;
        std::getline(std::cin, user_input);
        std::cout << "\n";
        if (user_input == "0")
        {
            // Temp objects -- Check: Object not created here. Simply accessing methods from class.
            ICRC_Class().icrc_pattern_identification();
            std::cout << "[+] Completed downloading ICRC pattern files" << "\n\n";
        }
        else if (user_input == "1")
        {
            VSAPI_Class vsapi_obj;
            vsapi_obj.vsapi_pattern_identification();
            std::cout << "[+] Completed downloading VSAPI pattern files" << "\n\n";
        }
        else if (user_input == "2")
        {
            TSCPTN_Class tscptn_obj;
            tscptn_obj.tscptn_pattern_identification();
            std::cout << "[+] Completed downloading TSCPTN pattern files" << "\n\n";
        }
        else if (user_input == "3")
        {
            TMWHITE_Class tmwhite_obj;
            tmwhite_obj.tmwhite_pattern_identification();
            std::cout << "[+] Completed downloading TMWHITE pattern files" << "\n\n";
        }
        else if (user_input == "4")
        {
            SSAPTN_Class ssaptn_obj;
            ssaptn_obj.ssaptn_pattern_identification();
            std::cout << "[+] Completed downloading SSAPTN pattern files" << "\n\n";
        }
        else if (user_input == "5")
        {
            SSPDA6_Class sspda6_obj;
            sspda6_obj.sspda6_pattern_identification();
            std::cout << "[+] Completed downloading SSPDA6 pattern files" << "\n\n";
        }
        else if (user_input == "6")
        {
            TMFWPTN_Class tmfwptn_Class;
            tmfwptn_Class.tmfwptn_pattern_identification();
            std::cout << "[+] Completed downloading !!!!!!!!TMFWPTN!!!!!! pattern files" << "\n\n";
        }
        else if (user_input == "7")
        {
            TRENDXLM_Class trendxlm_obj;
            trendxlm_obj.trendxlm_pattern_identification();
            std::cout << "[+] Completed downloading TRENDXLM pattern files" << "\n\n";
        }
        else if (user_input == "8")
        {
            ENGINE_Class engine_obj;
            engine_obj.engine_pattern_identification();
            std::cout << "[+] Completed downloading engine pattern files" << "\n\n";
        }
        else if (user_input == "9")
        {
            // Temp objects -- Check: Object not created here. Simply accessing methods from class.
            PLMComponentList_Class().PLMComponentList_pattern_identification();
            std::cout << "[+] Completed downloading PLMComponentList pattern files" << "\n\n";
        }
        else if (user_input == "10")
        {
            // [-] TODO: Reset progress bar after every executed method.
            ICRC_Class().icrc_pattern_identification();
            VSAPI_Class().vsapi_pattern_identification();
            TSCPTN_Class().tscptn_pattern_identification();
            TMWHITE_Class().tmwhite_pattern_identification();
            SSAPTN_Class().ssaptn_pattern_identification();
            SSPDA6_Class().sspda6_pattern_identification();
            TMFWPTN_Class().tmfwptn_pattern_identification();
            TRENDXLM_Class().trendxlm_pattern_identification();
            ENGINE_Class().engine_pattern_identification();
            PLMComponentList_Class().PLMComponentList_pattern_identification();
            std::cout << "[+] Completed downloading ALL pattern files" << "\n\n";
        }
        else if (user_input == "exit")
        {
            break;
        }
        // Reset progress bar.
        // Is this needed when other methods from other classes already reset the progress variable.
        progress = 0.0;
        total_file_count = 0;
    }
    std::cout << "[!] END" << "\n";
    std::cout << "[!] Exiting..." << "\n\n";
    std::cin.get();
    return 0;
}

/*
Project Plan and Design

Brief : Console application used to assist with constructing URLs used to download pattern(s) files from Trend Micro. Redesign and improvement of Smart_Scan_Pattern_Extractor-URL_Builder.

[Version 1]

=== Minimum Functions ===
[+] Reimplement functions of Smart_Scan_Pattern_Extractor-URL_Builder. Organised as a class file.
[+] Replicate functions of Smart_Scan_Pattern_Extractor-URL_Builder and target towards to "Virus Pattern(s)".
[-] Apply Object-oriented programming.
[+] Progress bar during downloads.

=== TODO ===
- Determine what the unknown patterns refer to. Example: What is ssapi or tmfwptn.

=== Flow map ===

Options 1/2 -> 1 -> extract_serverini_file(); -> root_folder_creation(); -> comment_server_section(); -> icrc_pattern_identification();
            -> 2 -> extract_serverini_file(); -> root_folder_creation(); -> comment_server_section(); -> vsapi_pattern_identification();

=== Full Directory Structure ===

<< DATE >> - engine  - ssapi32_v6
                     - ssapi64_v6
           - pattern - icrc
                     - rr
           - product - osce14 - enu

=== Components ===
What we know so far:
http://files.trendmicro.com/products/scanmail/SMEX-12.0%20SP1-GM-1464-AG.pdf

[PATTERN]
- vsapi & v_ : Virus Scan API
- tscptn & tsc_ : 
- tmwhite & w_ : IntelliTrap 
- ssaptn & ssa_ : Spyware Active-monitoring Pattern
- sspda6
- icrc
- tmfwptn : Mutiple different patterns under tmfwptn
- trendxlm

- DCE: Damage Cleanup Engine
- DCT: Damage Cleanup Template

Note: The following below is found in the server.ini file. Investigation suggest these lines are linked to the PLMComponentList.

[All_Product]
MaxProductID=745
Product.177=Mobile Security Add-On Service,5.5,9.0
Product.462=OSCE Add-On Service - Trend Micro Security for Macintosh,1.0,3.9
Product.490=OSCE Add-On Service - VDI, 1.0, 3.0
Product.523=OSCE Add-On Service - DLP, 1.0, 2.0
Product.539=OSCE Add-On Service - ToolBox,1.0,2.0
Product.540=OSCE Add-On Service - ToolBox Client,1.0,2.0
Product.602=OSCE Add-On Service - TMEE Deployment Tool, 1.0, 7.0
Product.603=OSCE Add-On Service - TMEE Deployment Tool Client, 1.0, 7.0
Product.716=OSCE Add On Service Vulnerability Protection Server Windows x86, 1.0, 3.0
Product.717=OSCE Add On Service Vulnerability Protection Client Windows x86, 1.0, 3.0
Product.541=Endpoint Sensor Client 32bit,1.0,2.0
Product.542=Endpoint Sensor Client 64bit,1.0,2.0
Product.543=OSCE Add-On Service - Endpoint Sensor,1.0,2.0
Product.745=PLM25_OSCE_Addon_Service_CompList,1.0,3.0

//https://docs.trendmicro.com/all/ent/tmsm/v1.5/en-us/tmsm_1.5_olhsrv/tmsmag/tmsmag-serverclient/server_and_client_upgrade.htm
Line "Product.745=PLM25_OSCE_Addon_Service_CompList,1.0,3.0" is related to the following below:
Note number 745 in line above correlates with 745 in the section below.
;EN edition
[Info_745_10000_1_1]
Version=2.5
Build=1045
Path=product/osce14/enu/PLM25_OSCE_AOS_COMP_LIST.zip,166427
Min=1.0
Max=3.0
*/
