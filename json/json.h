/*
 * json.h
 *
 *  Created on: 19 nov. 2016
 *      Author: mattias
 */

#pragma once

#include <exception>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// disable warning "local declaration hides declaration of the same name in outer scope"
#pragma warning( disable : 6246 )

//! Example usage
//!
//! "hello.json":
//! {"testvalue": "hello"}
//!
//! cpp-code:
//! auto json = Json::LoadFile("hello.json");
//! std::cout << json["testvalue"].string() << "\n";
//!
//! auto output = Json{};
//! output["test"] = "hello"
//! output.saveFile("testoutput.json") // Saves to file
//! std::cout << output << std::endl;  // Prints to screen
class Json : public std::vector<Json> {
public:
    enum Type {
        None = 0,
        Null,
        Object,
        Array,
        String,
        Number,
        Boolean,
    };

    //! Create a json object that is of string type
    //! Note: If you want to parse a json string use "Parse" or "parse"
    Json(std::string str) {
        string(str);
    }

    //! Create a Json object with a specific type
    Json(Type type) : type(type) {}

    Json(std::istream &stream) {
        parse(stream);
    }

    // Default constructors. The Json object is movable
    Json() = default;
    Json(const Json &json) = default;
    Json(Json &&) = default;

    //! Create a json object from a file and return the new object
    static Json LoadFile(std::string fname) {
        std::ifstream file(fname);
        return Json::Parse(file);
    }

    //! Load a file to this object
    Json &loadFile(std::string fname) {
        std::ifstream{fname} >> *this;
        return *this;
    }

    //! Save this json object to specified file
    void saveFile(std::string fname) const {
        std::ofstream{fname} << *this;
    }

    //! Parse and replace this instance
    Json &parse(std::string str) {
        std::istringstream ss(str);
        parse(ss);
        return *this;
    }

    Json &parse(std::istream &ss);

    //! Create a Json object and return the resulting json
    static Json Parse(std::string string) {
        return Json{}.parse(std::move(string));
    }

    //! Same as load but on
    static Json Parse(std::istream &ss) {
        return Json{}.parse(ss);
    }

    //! Get child by index
    //! Notice that this is not relevant for type Object
    Json &operator[](int index) {
        return vector()[index];
    }

    //! Find child
    //! IF child does not exist, create a new child and return that
    Json &operator[](std::string n) {
        type = Object; // The operation converts the Json-object to a object
        value = "";
        auto f = find(n);
        if (f != end()) {
            return *f;
        }

        push_back(Json());
        back().name = n;
        return back();
    }

    //! Char version of []
    Json &operator[](const char *name) {
        return operator[](std::string(name));
    }

    //! Const version of []
    //! @throws std::out_of_range if child is not found
    const Json &operator[](std::string n) const {
        auto f = find(n);
        if (f != end()) {
            return *f;
        }
        else {
            throw std::out_of_range("could not find " + n + " in json");
        }
    }

    //! Try to find a child with a specific name
    //! @return the iterator with the child or end() if not found
    iterator find(std::string name) {
        for (auto it = begin(); it != end(); ++it) {
            if ((*it).name == name) {
                return it;
            }
        }
        return end();
    }

    //! const version of above
    const_iterator find(std::string name) const {
        for (auto it = begin(); it != end(); ++it) {
            if ((*it).name == name) {
                return it;
            }
        }
        return end();
    }

    //! Compare to string
    bool operator==(std::string value) {
        return this->value == value;
    }

    //! Copy value from other json object
    Json &operator=(const Json &json) {
        type = json.type;
        if (type == Array || type == Object) {
            value.clear();
            auto n = name; // The name get cleared for some reason
            vector() = json.vector();
            name = n;
            pos = json.pos;
        }
        else {
            value = json.value;
            name = json.name;
            pos = json.pos;
        }
        return *this;
    }

    Json &operator=(Json &&json) {
        type = json.type;
        if (type == Array || type == Object) {
            value.clear();
            auto n = name;
            vector() = std::move(json.vector());
            name = n;
        }
        else
            value = std::move(json.value);
        name = std::move(json.name);
        return *this;
    }

    Json &operator=(const std::string str) {
        return string(str);
    }

    //! Get the underlying vector type
    std::vector<Json> &vector() {
        return *((std::vector<Json> *)this);
    }

    //! Same as obove but const
    std::vector<Json> &vector() const {
        return *((std::vector<Json> *)this);
    }

    //! Convert to vector of string
    //! Notice that this only works if type == Array and each child is
    //! of type string
    //! usage:
    //! auto otherVector = std::vector<std::string>{json}
    operator std::vector<std::string>() const {
        auto ret = std::vector<std::string>{};
        ret.reserve(size());
        if (type != Array) {
            for (auto &it : *this) {
                ret.push_back(it.string());
            }
        }

        return ret;
    }

    //! Assign a vector of strings to this object
    //! Usage:
    //! json.vector(otherVector)
    Json &vector(const std::vector<std::string> &value) {
        clear();
        reserve(value.size());
        type = Array;

        for (auto &it : value) {
            emplace_back(it);
        }

        return *this;
    }

    //! Bool operation that can be used in if-statements to check if a child is
    //! set
    operator bool() {
        if (type == None || type == Null || value.empty()) {
            return false;
        }
        else {
            return true;
        }
    }

    //! Convert to string representation of json object
    inline std::string toString() {
        return stringify(2);
    }

    iterator remove(const char *name) {
        return remove(std::string(name));
    }

    iterator remove(std::string name) {
        auto f = find(name);
        return vector().erase(f);
    }

    static void indent(std::ostream &stream, int spaces) {
        for (int i = 0; i < spaces; ++i) {
            stream << " ";
        }
    }

    std::string stringify(int indent = 4) const {
        std::ostringstream ss;
        stringify(ss, indent, 0);
        return ss.str();
    }

    //! Return line number of where the object was found in file or string
    constexpr size_t line() const {
        return pos.line;
    }

    //! Return line number of where the object was found in file or string
    constexpr size_t row() const {
        return line();
    }

    //! Return column where the object was found in file or string
    constexpr size_t col() {
        return pos.col;
    }

    //! Try to get string value
    //! @throw std::runtime_error if not of type String
    std::string string() const {
        if (type == String) {
            return value;
        }
        else {
            throw std::runtime_error("Type in json is not string");
        }
    }

    // Set content o be a string
    Json &string(std::string value) {
        clear();
        type = Json::String;
        this->value = value;
        return *this;
    }

    static void escapeString(std::ostream &stream, std::string str);

    void stringify(std::ostream &stream,
                   int indent = 4,
                   int startIndent = 0) const;

    friend std::istream &operator>>(std::istream &stream, Json &json) {
        json.parse(stream);
        return stream;
    }

    friend std::ostream &operator<<(std::ostream &stream, const Json &json) {
        json.stringify(stream);
        return stream;
    }

    struct Position {
        unsigned line = 1;
        unsigned col = 1;

        operator std::string() const {
            return std::to_string(line) + ": " + std::to_string(col);
        }

        constexpr bool operator==(const Position other) const {
            return line == other.line && col == other.col;
        }
    };

    // Member variables
    std::string name;
    std::string value;
    Position pos;
    Type type = None;

    struct ParsingError : public std::exception {
        ParsingError(std::string info, Position position)
            : errorString(info + " at " + std::string{position}),
              position(position) {
            what();
        }

        const char *what() const noexcept override {
            return errorString.c_str();
        }

        std::string errorString;
        Position position;
    };

    class Token {
    public:
        enum Type {
            None,
            Null,
            BeginBrace,
            EndBrace,
            BeginBracket,
            EndBracket,
            String,
            Number,
            Coma,
            Colon,
            BooleanTrue,
            BooleanFalse,
        };

        std::string value;
        Type type = None;
        Token(){};
        Token(Type type) : type(type) {}
        Token(const Token &) = default;
        Token(Token &&token)
            : value(std::move(token.value)), type(token.type) {}

        bool empty() {
            return value.empty();
        }

        Token &operator=(const Token &token) {
            this->type = token.type;
            this->value = token.value;
            return *this;
        }
    };

private:
    static char getChar(std::istream &stream, Position &pos);

    static Token getNextToken(std::istream &stream, Position &pos);

    // Remove utf-8 byte order mask
    static void removeBom(std::istream &stream) {
        if (stream.peek() == 0xef) {
            char dummy[3];
            stream.read(dummy, 3);
        }
    }

    // Internal parse function
    void parse(std::istream &ss, Position &pos, Token rest = Token());
};

/// Definition of internal functions-------------------------------------------

inline char Json::getChar(std::istream &stream, Json::Position &pos) {
    if (stream.eof()) {
        throw ParsingError("Unexpected end of file ", pos);
    }
    auto c = stream.get();
    if (c == '\n') {
        pos.col = 1;
        ++pos.line;
    }
    else {
        ++pos.col;
    }
    return c;
}

inline Json::Token Json::getNextToken(std::istream &stream,
                                      Json::Position &pos) {
    Token ret;

    if (stream.eof()) {
        throw ParsingError("End of file when expecting character", pos);
    }
    char c = getChar(stream, pos);

    while (isspace(c)) {
        c = getChar(stream, pos);
        if (stream.eof()) {
            throw ParsingError("End of file when expecting character", pos);
        }
    }

    if (c == '"') {
        ret.type = Token::String;
        c = getChar(stream, pos);
        while (c != '"') {
            if (c == '\\') {
                c = getChar(stream, pos);
                switch (c) {
                case '"':
                    ret.value += c;
                    break;
                case 'b':
                    ret.value += '\b';
                    break;
                case 'r':
                    ret.value += '\r';
                    break;
                case 't':
                    ret.value += '\t';
                    break;
                case 'n':
                    ret.value += '\n';
                    break;
                case '\\':
                    ret.value += '\\';
                    break;
                case 'f':
                    ret.value += '\f';
                    break;
                case 'u':
                    ret.value += "\\u";
                    break;
                default:
                    throw ParsingError("illegal character in string", pos);
                    break;
                }
            }
            else {
                ret.value += c;
            }
            c = getChar(stream, pos);
        }
        return ret;
    }

    auto assertEq = [&pos, &stream](char c) {
        char nc = getChar(stream, pos);
        return (nc == c);
    };

    auto assertWord = [&assertEq](std::string_view word) {
        for (auto c : word) {
            if (!assertEq(c)) {
                return false;
            }
        }
        return true;
    };

    if (isdigit(c) || c == '.' || c == '-') {
        ret.value += c;
        c = getChar(stream, pos);

        while (isdigit(c) || c == '.') {
            ret.value += c;
            c = getChar(stream, pos);
        }
        stream.unget();

        ret.type = Token::Number;
        return ret;
    }
    else if (c == '{') {
        return Token(Token::BeginBrace);
    }
    else if (c == '}') {
        return Token(Token::EndBrace);
    }
    else if (c == '[') {
        return Token(Token::BeginBracket);
    }
    else if (c == ']') {
        return Token(Token::EndBracket);
    }
    else if (c == ',') {
        return Token(Token::Coma);
    }
    else if (c == ':') {
        return Token(Token::Colon);
    }
    else if (c == 'n') {
        if (assertWord("ull")) {
            return Token(Token::Null);
        }
        else {
            throw ParsingError(std::string{"unexpected token: "} + c, pos);
        }
    }
    else if (c == 't') {
        if (assertWord("rue")) {
            ret.type = Token::BooleanTrue;
            return ret;
        }
        else {
            throw ParsingError(std::string{"unexpected token: "} + c, pos);
        }
    }
    else if (c == 'f') {
        if (assertWord("alse")) {
            ret.type = Token::BooleanFalse;
            return ret;
        }
        else {
            throw ParsingError(std::string{"unexpected token: "} + c, pos);
        }
    }
    else if (c == -1) {
        // End of file probably because the file was empty
        return Token(Token::None);
    }
    else {
        throw ParsingError{std::string{"unexpected character: "} + c, pos};
    }
    return Token(Token::None);
}

inline void Json::parse(std::istream &ss,
                        Json::Position &pos,
                        Json::Token rest) {
    Token token = rest;

    if (pos == Position{1, 1}) {
        removeBom(ss);
    }
    if (rest.type == rest.None) {
        token = getNextToken(ss, pos);
        value = "";
        this->pos = pos;
    }
    if (token.type == token.String) {
        type = String;
        value = token.value;
        this->pos = pos;
    }
    else if (token.type == token.Number) {
        type = Number;
        value = token.value;
        this->pos = pos;
    }
    else if (token.type == token.BeginBrace) {
        type = Object;

        auto token = getNextToken(ss, pos);

        if (token.type == token.EndBrace) {
            return; // Empty array
        }

        bool running = true;
        while (running) {
            Json json;
            json.name = token.value;

            token = getNextToken(ss, pos);

            if (token.type != Token::Colon) {
                throw ParsingError(
                    "unexpexted token in object, expected ':' got " +
                        token.value,
                    pos);
            }

            token = getNextToken(ss, pos);

            json.parse(ss, pos, token);
            if (json.type == None) {
                throw ParsingError("error in array", pos);
            }

            push_back(json);

            token = getNextToken(ss, pos);

            if (token.type == token.EndBrace) {
                return; // End of array
            }
            if (token.type == token.Coma) {
                token = getNextToken(ss, pos);
            }
            else {
                throw ParsingError(
                    "unexpected character in array: " + token.value, pos);
            }
        }
    }
    else if (token.type == token.Null) {
        type = Null;
    }
    else if (token.type == token.BooleanTrue) {
        type = Boolean;
        value = "true";
    }
    else if (token.type == token.BooleanFalse) {
        type = Boolean;
        value = "false";
    }
    else if (token.type == token.BeginBracket) {
        type = Array;

        auto token = getNextToken(ss, pos);

        if (token.type == token.EndBracket) {
            return; // Empty array
        }

        bool running = true;
        while (running) {
            Json json;

            json.parse(ss, pos, token);
            if (json.type == None) {
                throw ParsingError{"error in array", pos};
            }

            push_back(json);

            token = getNextToken(ss, pos);

            if (token.type == token.EndBracket) {
                return; // End of array
            }
            if (token.type == token.Coma) {
                token = getNextToken(ss, pos);
            }
            else {
                throw ParsingError{
                    "unexpected character in array: " + token.value, pos};
            }
        }
    }
}

inline Json &Json::parse(std::istream &ss) {
    Position pos;
    parse(ss, pos);
    return *this;
}

inline void Json::escapeString(std::ostream &stream, std::string str) {
    stream << '"';
    for (auto c : str) {
        switch (c) {
        case '\n':
            stream << "\\n";
            break;
        case '\r':
            stream << "\\r";
            break;
        case '\t':
            stream << "\\t";
            break;
        case '\f':
            stream << "\\f";
            break;
        case '\b':
            stream << "\\b";
            break;
        case '"':
            stream << "\\\"";
            break;
        case '\\':
            stream << "\\\\";
            break;
        default:
            stream << c;
        }
    }
    stream << '"';
}

inline void Json::stringify(std::ostream &stream,
                            int indent,
                            int startIndent) const {
    if (type == Number) {
        stream << value;
    }
    else if (type == String) {
        escapeString(stream, value);
    }
    else if (type == Null) {
        stream << "null";
    }
    else if (type == Boolean) {
        stream << value;
    }
    else if (type == Object) {
        if (empty()) {
            stream << "{}";
            return;
        }
        stream << "{\n";
        bool first = true;
        for (auto &it : *this) {
            if (first) {
                first = false;
            }
            else {
                stream << ",\n";
            }
            this->indent(stream, (startIndent + 1) * indent);
            escapeString(stream, it.name);
            stream << ": ";
            it.stringify(stream, indent, startIndent + 1);
        }
        stream << "\n";
        this->indent(stream, startIndent * indent);
        stream << "}";
    }
    else if (type == Array) {
        if (empty()) {
            stream << "[]";
            return;
        }
        stream << "[\n";

        auto it = begin();

        this->indent(stream, (startIndent + 1) * indent);
        it->stringify(stream, indent, startIndent + 1);
        for (it = it + 1; it != end(); ++it) {
            stream << ",\n";
            this->indent(stream, (startIndent + 1) * indent);
            it->stringify(stream, indent, startIndent + 1);
        }
        stream << "\n";
        this->indent(stream, startIndent * indent);
        stream << "]";
    }
}
