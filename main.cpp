#include "main.h"
#include "curl.h"

// ฟังก์ชันสำหรับเขียนข้อมูลลงไฟล์
size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

int main() {
    std::ifstream stock_list("C:\\Users\\Thanimwas\\Downloads\\Data\\stock\\stock_data3.csv");

    if (!stock_list) {
        std::cerr << "Unable to open file stock_data3.csv" << std::endl;
        return 1;
    }

    std::string stock;
    std::string output_dir = "C:\\Users\\Thanimwas\\Downloads\\Data\\stock";
    std::string consolidated_file = output_dir + "\\stock_data.csv";
    std::ofstream out_file(consolidated_file, std::ios::out);
    if (!out_file) {
        std::cerr << "Unable to open file " << consolidated_file << std::endl;
        return 1;
    }

    // เขียนส่วนหัวของไฟล์ ใช้คั่นด้วย ','
    out_file << "<TICKER>,<OPEN>,<HIGH>,<LOW>,<CLOSE>,<Adj Close>,<VOL>" << std::endl;

    // วนลูปเพื่ออ่านข้อมูลหุ้น
    while (getline(stock_list, stock)) {
        std::string stock_symbol = stock.substr(0, stock.find(','));

        std::string stock_upper = stock_symbol + ".BK";
        std::transform(stock_upper.begin(), stock_upper.end(), stock_upper.begin(), ::toupper);

        if (stock_symbol.empty()) {
            std::cerr << "Skipping empty stock symbol." << std::endl;
            continue;
        }

        std::time_t period2 = std::time(nullptr);
        std::time_t period1 = period2 - (0* 24 * 60 * 60);

        

        std::string url = "https://query1.finance.yahoo.com/v7/finance/download/" + stock_upper;
        url += "?period1=" + std::to_string(period1);
        url += "&period2=" + std::to_string(period2);
        url += "&interval=1d&events=history&includeAdjustedClose=true";

        std::cout << "Downloading data for " << stock_upper << " from URL: " << url << std::endl;
        CURL* curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
            curl_easy_setopt(curl, CURLOPT_CAINFO, "C:\\Users\\Thanimwas\\AppData\\Local\\Programs\\Python\\Python312\\Lib\\site-packages\\certifi\\cacert.pem");
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "YourCustomUserAgentString");

            std::string temp_file_name = output_dir + "\\" + stock_symbol + "_temp.csv";
            FILE* temp_fp = fopen(temp_file_name.c_str(), "wb");

            if (temp_fp != nullptr) {
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, temp_fp);
                CURLcode res = curl_easy_perform(curl);

                fclose(temp_fp); // ปิดไฟล์ชั่วคราว

                long response_code;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

                if (response_code == 404) {
                    std::cerr << "Failed to download data for " << stock_upper << ". HTTP Response Code: 404. URL: " << url << std::endl;
                }
                else if (response_code == 429) {
                    std::cout << "Rate limit exceeded. Sleeping for 60 seconds..." << std::endl;
                    fclose(temp_fp);
                    curl_easy_cleanup(curl);
                    std::this_thread::sleep_for(std::chrono::seconds(60));
                    continue;
                }
                else if (res != CURLE_OK) {
                    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                }
                else {
                    std::ifstream temp_in_file(temp_file_name, std::ios::in);

                    if (temp_in_file) {
                        std::string line;
                        std::vector<std::string> rows;

                        getline(temp_in_file, line); // อ่านบรรทัดแรก
                        while (getline(temp_in_file, line)) {
                            rows.push_back(line);
                        }
                        temp_in_file.close();

                        if (!rows.empty()) {
                            for (const auto& row : rows) {
                                // แยกวันที่ออกจากข้อมูล
                                size_t first_comma_pos = row.find(',');
                                std::string row_without_date = row.substr(first_comma_pos + 1);

                                // แยกแต่ละค่าออกจาก row ที่แยกวันที่แล้ว
                                std::stringstream ss(row_without_date);
                                std::string open, high, low, close, adj_close, vol;

                                getline(ss, open, ',');
                                getline(ss, high, ',');
                                getline(ss, low, ',');
                                getline(ss, close, ',');
                                getline(ss, adj_close, ',');
                                getline(ss, vol, ',');

                                // เขียนข้อมูลลงไฟล์โดยใช้ ',' เป็นตัวคั่นระหว่างคอลัมน์
                                out_file << stock_symbol << "," << open << "," << high << "," << low << "," << close << "," << adj_close << "," << vol << std::endl;
                            }
                        }
                        else {
                            std::cerr << "No data found for " << stock_upper << std::endl;
                        }
                    }
                    else {
                        std::cerr << "Failed to read temporary file: " << temp_file_name << std::endl;
                    }

                    std::remove(temp_file_name.c_str());
                }
            }
            else {
                std::cerr << "Failed to open temporary file for writing: " << temp_file_name << std::endl;
            }

            curl_easy_cleanup(curl);
        }
        else {
            std::cerr << "Failed to initialize CURL." << std::endl;
        }
    }

    std::ifstream check_file(consolidated_file);
    if (check_file.peek() == std::ifstream::traits_type::eof()) {
        std::cerr << "Consolidated file is empty: " << consolidated_file << std::endl;
    }
    else {
        std::cout << "Data successfully written to " << consolidated_file << std::endl;
    }

    out_file.close();
    return 0;
}
