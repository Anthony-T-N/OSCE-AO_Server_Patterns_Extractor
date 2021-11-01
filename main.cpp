#define CURL_STATICLIB
#include <curl/curl.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

// https://stackoverflow.com/questions/21873048/getting-an-error-fopen-this-function-or-variable-may-be-unsafe-when-complin/21873153
#pragma warning(disable:4996);

// Global Variables
std::string current_root_folder = "";
const std::string first_url_section = "http://osce14-p.activeupdate.trendmicro.com/activeupdate/";
const std::string generic_download_path = "\\pattern\\";
float progress = 0.0;
int bar_width = 100;
double v_ = 0;

class Common_Class
{       
    public:
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
            // Function must occur after function directories_structure().
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
            while (std::getline(input_file, input_file_line))
            {
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
                if (input_file_line.find("v_") != std::string::npos && input_file_line.find("P.4") != std::string::npos || input_file_line.find("vsapi") != std::string::npos && input_file_line.find("P.4") != std::string::npos)
                {
                    v_ += 1;
                }
            }
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
        void directories_structure()
        {
            time_t rawtime;
            struct tm* timeinfo;
            char buffer[80];

            time(&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
            std::cout << "[!] Root folder name: " << buffer << "\n\n";

            std::string root_folder_name(buffer);
            std::filesystem::create_directories(root_folder_name + "/pattern/icrc");
            current_root_folder = root_folder_name;
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
        static void downloading_progress_bar(float &progress, int &bar_width)
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
            // TODO: Determine total files to download beforehand. 100 / Total number of files = progress.
            progress += (100 / (v_ - 1)) / 100;
            //std::cout << progress << "\n";
            //progress += 0.07142857;
            std::cout << "\n";
        }
        static void download_file_allocation(std::string &input_file_line, std::string download_path)
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

            Common_Class::downloading_progress_bar(progress, bar_width);
        }
};

class ICRC_Class
{
    public:
        void icrc_pattern_identification()
        {
            const std::string icrc_download_path = "\\pattern\\icrc\\";
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "icrc".
                if (input_file_line.find("icrc") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, icrc_download_path);
                }
            }
            input_file.close();
        }
};

class VSAPI_Class
{
    public:
        void vsapi_pattern_identification()
        {
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "vsapi".
                if (input_file_line.find("v_") != std::string::npos && input_file_line.find("P.4") != std::string::npos || input_file_line.find("vsapi") != std::string::npos && input_file_line.find("P.4") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path);
                }
            }
            input_file.close();
        }
};

class TSCPTN_Class
{
    public:
        void tscptn_pattern_identification()
        {
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "tscptn" or "tsc".
                if (input_file_line.find("tsc") != std::string::npos || input_file_line.find("tscptn") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path);
                }
            }
            input_file.close();
        }
};

class TMWHITE_Class
{
    public:
        void tmwhite_pattern_identification()
        {
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "tscptn" or "tsc".
                if (input_file_line.find("w_") != std::string::npos || input_file_line.find("tmwhite") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path);
                }
            }
            input_file.close();
        }
};

class SSAPTN_Class
{
    public:
        void ssaptn_pattern_identification()
        {
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "tscptn" or "tsc".
                if (input_file_line.find("ssa_") != std::string::npos || input_file_line.find("ssaptn") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path);
                }
            }
            input_file.close();
        }
};

class SSPDA6_Class
{
    public:
        void sspda6_pattern_identification()
        {
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "tscptn" or "tsc".
                if (input_file_line.find("sspda6_") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path);
                }
            }
            input_file.close();
        }
};

// UNIQUE CLASS
// pattern/rr/rr_1.10727.00.zip
class TMFWPTN_Class
{
    public:
        void tmfwptn_pattern_identification()
        {
            // TODO: Download all lines below "tmfwptn" until empty space has been reached.
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;

            bool tmfwptn_switch = false;
            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line == "")
                {
                    tmfwptn_switch = false;
                }
                else if (input_file_line.find("tmfwptn") != std::string::npos || tmfwptn_switch == true)
                {
                    tmfwptn_switch = true;
                    Common_Class::download_file_allocation(input_file_line, generic_download_path);
                }
            }
            input_file.close();
        }
};

class TRENDXLM_Class
{
    public:
        void trendxlm_pattern_identification()
        {
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "tscptn" or "tsc".
                if (input_file_line.find("trendxlm_") != std::string::npos)
                {
                    Common_Class::download_file_allocation(input_file_line, generic_download_path);
                }
            }
            input_file.close();
        }
};

class ENGINE_Class
{
    public:
        void engine_pattern_identification()
        {
            // TODO: Download all lines below "tmfwptn" until empty space has been reached.
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "\033[4;31m" << "[-] Unable to open server.ini;" << "\033[0m" << "\n\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;

            bool engine_switch = false;
            std::filesystem::create_directories(current_root_folder + "/engine/");

            /*
            * Requires additional conditions to detect and download. For now, skip
            E.22000010=SSENGINE_SSAPI32_V6,engine/ssapi32_v6/SSAPI32_v62-4015.zip,6.2.4015,592305,6.0.1000
            E.22000020=SSENGINE_SSAPI64_V6,engine/ssapi64_v6/SSAPI64_v62-4015.zip,6.2.4015,897467,6.0.1000
            */

            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line == "")
                {
                    engine_switch = false;
                }
                else if (input_file_line.find("[ENGINE]") != std::string::npos)
                {
                    engine_switch = true;
                }
                // If line contains two /. Create folder between both /
                else if (engine_switch == true && input_file_line.find_first_of("/") != input_file_line.find_last_of("/"))
                {
                    std::string temp_path = input_file_line;
                    temp_path.erase(0, temp_path.find_first_of("/"));
                    temp_path.erase(temp_path.find_last_of("/") + 1);
                    std::cout << temp_path << "\n";
                    std::filesystem::create_directories(current_root_folder + "/engine/" + temp_path);
                    system("pause");
                }
                else if (engine_switch == true && input_file_line.find("SSAPI") == std::string::npos)
                {
                    // E.4=VSAPI32_NT_I386,engine/engv_nt386_v12500-1004.zip,12.500.1004,1350445,6.510.1002

                    // 1) 
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
                    
                    std::string full_download_path = current_root_folder + "\\engine\\" + Common_Class::file_download_name(extracted_url);

                    char extracted_url_char[FILENAME_MAX];
                    char full_download_path_char[FILENAME_MAX];

                    strcpy(extracted_url_char, extracted_url.c_str());
                    strcpy(full_download_path_char, full_download_path.c_str());
                    Common_Class::download_file(extracted_url_char, full_download_path_char);

                    // SIG BUILDING HERE
                    std::cout << "SIG BUILDING HERE" << "\n";
                    extracted_url = Common_Class::sig_builder(extracted_url);
                    std::cout << "DEBUG: " << extracted_url << "\n";
                    full_download_path = current_root_folder + "\\engine\\" + Common_Class::file_download_name(Common_Class::sig_builder(extracted_url));
                    std::cout << "DEBUG: " << full_download_path << "\n";

                    strcpy(extracted_url_char, extracted_url.c_str());
                    strcpy(full_download_path_char, full_download_path.c_str());
                    Common_Class::download_file(extracted_url_char, full_download_path_char);
                }
            }
            input_file.close();
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
    baseline_obj.directories_structure();
    baseline_obj.comment_server_section();
    while (true)
    {
        // TODO: Reset global variables (Example: Progress bar).
        std::cout << "Select an option:" << "\n";
        std::cout << "[1] Download ICRC (Smart Scan Pattern(s)) files" << "\n";
        std::cout << "[2] Download VSAPI (Virus Pattern(s)) files" << "\n";
        std::cout << "[3] Download TSCPTN (Unknown Pattern(s)) files" << "\n";
        std::cout << "[4] Download TMWHITE (Unknown Pattern(s)) files" << "\n";
        std::cout << "[5] Download SSAPTN (Unknown Pattern(s)) files" << "\n";
        std::cout << "[6] Download SSPDA6 (Unknown Pattern(s)) files" << "\n";
        std::cout << "[7] Download TMFWPTN (Unknown Pattern(s)) files" << "\n";
        std::cout << "[8] Download TRENDXLM (Unknown Pattern(s)) files" << "\n";
        std::cout << "[9] Download ENGINE (Unknown Pattern(s)) files" << "\n";
        std::cout << "[exit] Exit" << "\n";
        std::cout << "Selection ?:" << "\n";
        std::cout << "> ";
        std::string user_input;
        std::getline(std::cin, user_input);
        std::cout << "\n";
        if (user_input == "1")
        {
            ICRC_Class icrc_obj;
            icrc_obj.icrc_pattern_identification();
            std::cout << "[+] Completed downloading ICRC pattern files" << "\n\n";
        }
        else if (user_input == "2")
        {
            VSAPI_Class vsapi_obj;
            vsapi_obj.vsapi_pattern_identification();
            std::cout << "[+] Completed downloading VSAPI pattern files" << "\n\n";
        }
        else if (user_input == "3")
        {
            TSCPTN_Class tscptn_obj;
            tscptn_obj.tscptn_pattern_identification();
            std::cout << "[+] Completed downloading TSCPTN pattern files" << "\n\n";
        }
        else if (user_input == "4")
        {
            TMWHITE_Class tmwhite_obj;
            tmwhite_obj.tmwhite_pattern_identification();
            std::cout << "[+] Completed downloading TMWHITE pattern files" << "\n\n";
        }
        else if (user_input == "5")
        {
            SSAPTN_Class ssaptn_obj;
            ssaptn_obj.ssaptn_pattern_identification();
            std::cout << "[+] Completed downloading SSAPTN pattern files" << "\n\n";
        }
        else if (user_input == "6")
        {
            SSPDA6_Class sspda6_obj;
            sspda6_obj.sspda6_pattern_identification();
            std::cout << "[+] Completed downloading SSPDA6 pattern files" << "\n\n";
        }
        else if (user_input == "7")
        {
            TMFWPTN_Class tmfwptn_Class;
            tmfwptn_Class.tmfwptn_pattern_identification();
            std::cout << "[+] Completed downloading !!!!!!!!TMFWPTN!!!!!! pattern files" << "\n\n";
        }
        else if (user_input == "8")
        {
            TRENDXLM_Class trendxlm_obj;
            trendxlm_obj.trendxlm_pattern_identification();
            std::cout << "[+] Completed downloading TRENDXLM pattern files" << "\n\n";
        }
        else if (user_input == "9")
        {
            ENGINE_Class engine_obj;
            engine_obj.engine_pattern_identification();
            std::cout << "[+] Completed downloading engine pattern files" << "\n\n";
        }
        else if (user_input == "exit")
        {
            break;
        }
    }
    std::cout << "[!] END" << "\n";
    std::cout << "[!] Exiting..." << "\n\n";
    // Don't use sys.
    system("pause");
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

=== Flow map ===

Options 1/2 -> 1 -> extract_serverini_file(); -> directories_structure(); -> comment_server_section(); -> icrc_pattern_identification();
            -> 2 -> extract_serverini_file(); -> directories_structure(); -> comment_server_section(); -> vsapi_pattern_identification();
*/
