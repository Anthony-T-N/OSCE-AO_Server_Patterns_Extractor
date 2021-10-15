#define CURL_STATICLIB
#include <curl/curl.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

// https://stackoverflow.com/questions/21873048/getting-an-error-fopen-this-function-or-variable-may-be-unsafe-when-complin/21873153
#pragma warning(disable:4996);

std::string current_root_folder = "";
static std::string first_section = "http://osce14-p.activeupdate.trendmicro.com/activeupdate/";

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
                std::cout << "[-] Unable to open temp.ini;" << "\n";
                return;
            }
            input_file.open("temp.ini");
            std::cout << "[+] Opened temp.ini successfully;" << "\n\n";

            std::ofstream output_file;
            output_file.open(current_root_folder + "/server.ini");
            std::cout << "[+] Created server.ini successfully;" << "\n\n";

            // Cannot edit individual lines of an existing text file. Edited copy will need to be made.
            std::string input_file_line;
            bool commenting_enabled = false;
            bool server_section_found = false;
            while (std::getline(input_file, input_file_line))
            {
                if (input_file_line.find("[Server]") != std::string::npos)
                {
                    std::cout << "[+] [Server] Section Found;" << "\n";
                    std::cout << input_file_line << "\n\n";
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
            }
            input_file.close();
            output_file.close();
            if (server_section_found == false)
            {
                std::cout << "[-] WARNING: [Server] Section was not found;" << "\n";
                std::cout << "[-] Proceed with caution;" << "\n";
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
            // Function uses: <string>
            // Assumes extracted string has been processed by the url_builder(str) function.
            extracted_string.erase(extracted_string.find_last_of(".") + 1);
            return extracted_string + "sig";
        }
        static std::string url_builder(std::string extracted_string)
        {
            // Function uses: <string>
            extracted_string.erase(extracted_string.find_first_of(","));
            extracted_string.erase(0, extracted_string.find_first_of("=") + 1);
            std::string final_string = first_section + extracted_string;
            return final_string;
        }
        static std::string file_download_name(std::string url_name)
        {
            url_name.erase(0, url_name.find_last_of("/") + 1);
            return url_name;
        }
};


class ICRC_Class
{
    public:
        void icrc_pattern_identification()
        {
            // Function uses: <iostream>, <fstream>, <string>, <filesystem>

            // Read server.ini file.
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "[-] Unable to open server.ini;" << "\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "icrc".
                if (input_file_line.find("icrc") != std::string::npos)
                {
                    std::string extracted_url = Common_Class::url_builder(input_file_line);
                    std::string full_download_path = current_root_folder + "\\pattern\\icrc\\" + Common_Class::file_download_name(extracted_url);

                    char extracted_url_char[FILENAME_MAX];
                    char full_download_path_char[FILENAME_MAX];

                    strcpy(extracted_url_char, extracted_url.c_str());
                    strcpy(full_download_path_char, full_download_path.c_str());
                    Common_Class::download_file(extracted_url_char, full_download_path_char);

                    extracted_url = Common_Class::sig_builder(extracted_url);
                    full_download_path = current_root_folder + "\\pattern\\icrc\\" + Common_Class::file_download_name(Common_Class::sig_builder(extracted_url));

                    strcpy(extracted_url_char, extracted_url.c_str());
                    strcpy(full_download_path_char, full_download_path.c_str());
                    Common_Class::download_file(extracted_url_char, full_download_path_char);
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
            // Read server.ini file.
            std::ifstream input_file;
            std::cout << "[!] Opening server.ini for reading;" << "\n";
            if (std::filesystem::exists(current_root_folder + "/server.ini") == false)
            {
                std::cout << "[-] Unable to open server.ini;" << "\n";
                return;
            }
            input_file.open(current_root_folder + "/server.ini");
            std::string input_file_line;
            while (std::getline(input_file, input_file_line))
            {
                // Go through all lines in the "server.ini" file until a line contains "icrc".
                if (input_file_line.find("v_") != std::string::npos)
                {
                    std::cout << input_file_line << "\n";
                    std::string extracted_url = Common_Class::url_builder(input_file_line);
                    std::cout << extracted_url << "\n";
                    std::string full_download_path = current_root_folder + "\\pattern\\" + Common_Class::file_download_name(extracted_url);
                    std::cout << full_download_path << "\n";
                    //char extracted_url_char[FILENAME_MAX];
                    //char full_download_path_char[FILENAME_MAX];
                    
                    /*
                    

                    strcpy(extracted_url_char, extracted_url.c_str());
                    strcpy(full_download_path_char, full_download_path.c_str());
                    Common_Class::download_file(extracted_url_char, full_download_path_char);

                    extracted_url = Common_Class::sig_builder(extracted_url);
                    full_download_path = current_root_folder + "\\pattern\\icrc\\" + Common_Class::file_download_name(Common_Class::sig_builder(extracted_url));

                    strcpy(extracted_url_char, extracted_url.c_str());
                    strcpy(full_download_path_char, full_download_path.c_str());
                    Common_Class::download_file(extracted_url_char, full_download_path_char);
                    */
                }
            }
            input_file.close();
        }
};

int main()
{
    std::cout << "=======================================" << "\n";
    std::cout << "- OSCE-AO_Server_Patterns_Extractor console application" << "\n";
    std::cout << "- Console Application Version: 0.5" << "\n";
    std::cout << "- Created By: Anthony N." << "\n";
    std::cout << "- Current location of executable: " << std::filesystem::current_path() << "\n";
    std::cout << "=======================================" << "\n\n";

    // TODO: Provide options here;
    Common_Class baseline_obj;
    baseline_obj.extract_serverini_file();
    baseline_obj.directories_structure();
    baseline_obj.comment_server_section();
    std::cout << "Select Option:" << "\n";
    std::string user_input;
    std::getline(std::cin, user_input);
    if (user_input == "1")
    {
        ICRC_Class icrc_obj;
        icrc_obj.icrc_pattern_identification();
    }
    else if (user_input == "2")
    {
        VSAPI_Class vsapi_obj;
        vsapi_obj.vsapi_pattern_identification();
    }
    std::cout << "[!] END" << "\n";
    std::cout << "[!] Exiting..." << "\n\n";
    system("pause");
    return 0;
}

/*
Project Plan and Design

Brief : Console application used to assist with constructing URLs used to download pattern(s) files from Trend Micro. Redesign and improvement of Smart_Scan_Pattern_Extractor-URL_Builder.

[Version 1]

=== Minimum Functions ===
[-] Reimplement functions of Smart_Scan_Pattern_Extractor-URL_Builder. Organised as a class file.
[-] Replicate functions of Smart_Scan_Pattern_Extractor-URL_Builder and target towards to "Virus Pattern(s)".
[-] ApplyObject-oriented programming.

=== Flow map ===

Options 1/2 -> 1 -> extract_serverini_file(); -> directories_structure(); -> comment_server_section(); -> icrc_pattern_identification();
            -> 2 -> extract_serverini_file(); -> directories_structure(); -> comment_server_section(); -> vsapi_pattern_identification();

*/
