#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <map>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

void extract_nutritional_facts(const std::string &image_path) {
    tesseract::TessBaseAPI *ocr = new tesseract::TessBaseAPI();
    if (ocr->Init(NULL, "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        exit(1);
    }

    Pix *image = pixRead(image_path.c_str());
    if (!image) {
        std::cerr << "Could not read image.\n";
        exit(1);
    }

    ocr->SetImage(image);
    char *raw_text = ocr->GetUTF8Text();
    std::string text(raw_text);
    std::cout << "Raw OCR Output:\n" << text << "\n";

    delete[] raw_text;
    ocr->End();
    pixDestroy(&image);

    std::map<std::string, std::regex> patterns = {
        {"Calories", std::regex("Calories\\s+(\\d+)")},
        {"Total Fat", std::regex("Total Fat\\s+(\\d+g)")},
        {"Sodium", std::regex("Sodium\\s+(\\d+mg)")},
        {"Total Carbohydrate", std::regex("Total Carbohydrate\\s+(\\d+g)")},
        {"Protein", std::regex("Protein\\s+(\\d+g)")}
    };

    std::map<std::string, float> nutrition_facts;

    for (const auto &pair : patterns) {
        std::smatch match;
        if (std::regex_search(text, match, pair.second) && match.size() > 1) {
            std::string value = match.str(1);
            try {
                if (value.find("g") != std::string::npos) {
                    nutrition_facts[pair.first] = std::stof(value.substr(0, value.size() - 1));
                } else if (value.find("mg") != std::string::npos) {
                    nutrition_facts[pair.first] = std::stof(value.substr(0, value.size() - 2)) / 1000.0f;
                } else {
                    nutrition_facts[pair.first] = std::stof(value);
                }
            } catch (const std::exception &e) {
                std::cerr << "Could not convert value for " << pair.first << ": '" << value << "'\n";
            }
        }
    }

    std::cout << "\nExtracted Nutritional Facts:\n";
    for (const auto &fact : nutrition_facts) {
        std::cout << fact.first << ": " << fact.second << "\n";
    }
}

int main() {
    std::string image_path = "nutrition_label.jpg";
    extract_nutritional_facts(image_path);
    return 0;
}
