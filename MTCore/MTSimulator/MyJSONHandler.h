#pragma once
//#include <nlohmann/json.hpp>
//#include <iostream>
//#include <fstream>
//
//class MyJSONHandler : public nlohmann::json_sax_t
//{
//public:
//    // מטפלים במפתחות
//    bool key(std::string& key) override {
//        std::cout << "Key: " << key << "\n";
//        return true;
//    }
//
//    // מטפלים במחרוזות
//    bool string(std::string& value) override {
//        std::cout << "String value: " << value << "\n";
//        return true;
//    }
//
//    // מטפלים במספרים
//    bool number_integer(number_integer_t val) override {
//        std::cout << "Integer value: " << val << "\n";
//        return true;
//    }
//
//    bool number_unsigned(number_unsigned_t val) override {
//        std::cout << "Unsigned value: " << val << "\n";
//        return true;
//    }
//
//    bool number_float(number_float_t val, const std::string& s) override {
//        std::cout << "Float value: " << val << "\n";
//        return true;
//    }
//
//    // מצבים נוספים
//    bool start_object(std::size_t elements) override { return true; }
//    bool end_object() override { return true; }
//    bool start_array(std::size_t elements) override { return true; }
//    bool end_array() override { return true; }
//    bool boolean(bool val) override { return true; }
//    bool null() override { return true; }
//
//    // מטפלים בשגיאות
//    bool parse_error(std::size_t position, const std::string& last_token, const nlohmann::json::exception& ex) override {
//        std::cerr << "Parse error at position " << position << ": " << ex.what() << "\n";
//        return false;
//    }
//};

