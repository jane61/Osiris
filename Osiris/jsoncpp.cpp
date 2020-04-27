/// Json-cpp amalgamated source (http://jsoncpp.sourceforge.net/).
/// It is intended to be used with #include "json/json.h"

// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: LICENSE
// //////////////////////////////////////////////////////////////////////

/*
The JsonCpp library's source code, including accompanying documentation,
tests and demonstration applications, are licensed under the following
conditions...

Baptiste Lepilleur and The JsonCpp Authors explicitly disclaim copyright in all
jurisdictions which recognize such a disclaimer. In such jurisdictions,
this software is released into the Public Domain.

In jurisdictions which do not recognize Public Domain property (e.g. Germany as of
2010), this software is Copyright (c) 2007-2010 by Baptiste Lepilleur and
The JsonCpp Authors, and is released under the terms of the MIT License (see below).

In jurisdictions which recognize Public Domain property, the user of this
software may choose to accept it either as 1) Public Domain, 2) under the
conditions of the MIT License (see below), or 3) under the terms of dual
Public Domain/MIT License conditions described here, as they choose.

The MIT License is about as close to Public Domain as a license can get, and is
described in clear, concise terms at:

   http://en.wikipedia.org/wiki/MIT_License

The full text of the MIT License follows:

========================================================================
Copyright (c) 2007-2010 Baptiste Lepilleur and The JsonCpp Authors

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
========================================================================
(END LICENSE TEXT)

The MIT license is compatible with both the GPL and commercial
software, affording one all of the rights of Public Domain with the
minor nuisance of being required to keep the above copyright notice
and license text in the source code. Note also that by accepting the
Public Domain "license" you can re-license your copy using whatever
license you like.

*/

// //////////////////////////////////////////////////////////////////////
// End of content of file: LICENSE
// //////////////////////////////////////////////////////////////////////






#include "json/json.h"

#ifndef JSON_IS_AMALGAMATION
#error "Compile with -I PATH_TO_JSON_DIRECTORY"
#endif


// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_tool.h
// //////////////////////////////////////////////////////////////////////

// Copyright 2007-2010 Baptiste Lepilleur and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef LIB_JSONCPP_JSON_TOOL_H_INCLUDED
#define LIB_JSONCPP_JSON_TOOL_H_INCLUDED

#if !defined(JSON_IS_AMALGAMATION)
#include <json/config.h>
#endif

// Also support old flag NO_LOCALE_SUPPORT
#ifdef NO_LOCALE_SUPPORT
#define JSONCPP_NO_LOCALE_SUPPORT
#endif

#ifndef JSONCPP_NO_LOCALE_SUPPORT
#include <clocale>
#endif

/* This header provides common string manipulation support, such as UTF-8,
 * portable conversion from/to string...
 *
 * It is an internal header that must not be exposed.
 */

namespace Json {
    static inline char getDecimalPoint() {
#ifdef JSONCPP_NO_LOCALE_SUPPORT
        return '\0';
#else
        struct lconv* lc = localeconv();
        return lc ? *(lc->decimal_point) : '\0';
#endif
    }

    /// Converts a unicode code-point to UTF-8.
    static inline String codePointToUTF8(unsigned int cp) {
        String result;

        // based on description from http://en.wikipedia.org/wiki/UTF-8

        if (cp <= 0x7f) {
            result.resize(1);
            result[0] = static_cast<char>(cp);
        } else if (cp <= 0x7FF) {
            result.resize(2);
            result[1] = static_cast<char>(0x80 | (0x3f & cp));
            result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
        } else if (cp <= 0xFFFF) {
            result.resize(3);
            result[2] = static_cast<char>(0x80 | (0x3f & cp));
            result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
            result[0] = static_cast<char>(0xE0 | (0xf & (cp >> 12)));
        } else if (cp <= 0x10FFFF) {
            result.resize(4);
            result[3] = static_cast<char>(0x80 | (0x3f & cp));
            result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
            result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
            result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
        }

        return result;
    }

    enum {
        /// Constant that specify the size of the buffer that must be passed to
        /// uintToString.
        uintToStringBufferSize = 3 * sizeof(LargestUInt) + 1
    };

    // Defines a char buffer for use with uintToString().
    using UIntToStringBuffer = char[uintToStringBufferSize];

    /** Converts an unsigned integer to string.
     * @param value Unsigned integer to convert to string
     * @param current Input/Output string buffer.
     *        Must have at least uintToStringBufferSize chars free.
     */
    static inline void uintToString(LargestUInt value, char*& current) {
        *--current = 0;
        do {
            *--current = static_cast<char>(value % 10U + static_cast<unsigned>('0'));
            value /= 10;
        } while (value != 0);
    }

    /** Change ',' to '.' everywhere in buffer.
     *
     * We had a sophisticated way, but it did not work in WinCE.
     * @see https://github.com/open-source-parsers/jsoncpp/pull/9
     */
    template <typename Iter> Iter fixNumericLocale(Iter begin, Iter end) {
        for (; begin != end; ++begin) {
            if (*begin == ',') {
                *begin = '.';
            }
        }
        return begin;
    }

    template <typename Iter> void fixNumericLocaleInput(Iter begin, Iter end) {
        char decimalPoint = getDecimalPoint();
        if (decimalPoint == '\0' || decimalPoint == '.') {
            return;
        }
        for (; begin != end; ++begin) {
            if (*begin == '.') {
                *begin = decimalPoint;
            }
        }
    }

    /**
     * Return iterator that would be the new end of the range [begin,end), if we
     * were to delete zeros in the end of string, but not the last zero before '.'.
     */
    template <typename Iter> Iter fixZerosInTheEnd(Iter begin, Iter end) {
        for (; begin != end; --end) {
            if (*(end - 1) != '0') {
                return end;
            }
            // Don't delete the last zero before the decimal point.
            if (begin != (end - 1) && *(end - 2) == '.') {
                return end;
            }
        }
        return end;
    }

} // namespace Json

#endif // LIB_JSONCPP_JSON_TOOL_H_INCLUDED

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_tool.h
// //////////////////////////////////////////////////////////////////////






// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_reader.cpp
// //////////////////////////////////////////////////////////////////////

// Copyright 2007-2011 Baptiste Lepilleur and The JsonCpp Authors
// Copyright (C) 2016 InfoTeCS JSC. All rights reserved.
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include "json_tool.h"
#include <json/assertions.h>
#include <json/reader.h>
#include <json/value.h>
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <cassert>
#include <cstring>
#include <iostream>
#include <istream>
#include <limits>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include <cstdio>
#if __cplusplus >= 201103L

#if !defined(sscanf)
#define sscanf std::sscanf
#endif

#endif //__cplusplus

#if defined(_MSC_VER)
#if !defined(_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif //_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#endif //_MSC_VER

#if defined(_MSC_VER)
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

// Define JSONCPP_DEPRECATED_STACK_LIMIT as an appropriate integer at compile
// time to change the stack limit
#if !defined(JSONCPP_DEPRECATED_STACK_LIMIT)
#define JSONCPP_DEPRECATED_STACK_LIMIT 1000
#endif

static size_t const stackLimit_g =
JSONCPP_DEPRECATED_STACK_LIMIT; // see readValue()

namespace Json {

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
    using CharReaderPtr = std::unique_ptr<CharReader>;
#else
    using CharReaderPtr = std::auto_ptr<CharReader>;
#endif

    // Implementation of class Features
    // ////////////////////////////////

    Features::Features() = default;

    Features Features::all() { return {}; }

    Features Features::strictMode() {
        Features features;
        features.allowComments_ = false;
        features.allowTrailingCommas_ = false;
        features.strictRoot_ = true;
        features.allowDroppedNullPlaceholders_ = false;
        features.allowNumericKeys_ = false;
        return features;
    }

    // Implementation of class Reader
    // ////////////////////////////////

    bool Reader::containsNewLine(Reader::Location begin, Reader::Location end) {
        for (; begin < end; ++begin)
            if (*begin == '\n' || *begin == '\r')
                return true;
        return false;
    }

    // Class Reader
    // //////////////////////////////////////////////////////////////////

    Reader::Reader() : features_(Features::all()) {}

    Reader::Reader(const Features& features) : features_(features) {}

    bool Reader::parse(const std::string& document, Value& root,
        bool collectComments) {
        document_.assign(document.begin(), document.end());
        const char* begin = document_.c_str();
        const char* end = begin + document_.length();
        return parse(begin, end, root, collectComments);
    }

    bool Reader::parse(std::istream& is, Value& root, bool collectComments) {
        // std::istream_iterator<char> begin(is);
        // std::istream_iterator<char> end;
        // Those would allow streamed input from a file, if parse() were a
        // template function.

        // Since String is reference-counted, this at least does not
        // create an extra copy.
        String doc;
        std::getline(is, doc, static_cast<char> EOF);
        return parse(doc.data(), doc.data() + doc.size(), root, collectComments);
    }

    bool Reader::parse(const char* beginDoc, const char* endDoc, Value& root,
        bool collectComments) {
        if (!features_.allowComments_) {
            collectComments = false;
        }

        begin_ = beginDoc;
        end_ = endDoc;
        collectComments_ = collectComments;
        current_ = begin_;
        lastValueEnd_ = nullptr;
        lastValue_ = nullptr;
        commentsBefore_.clear();
        errors_.clear();
        while (!nodes_.empty())
            nodes_.pop();
        nodes_.push(&root);

        bool successful = readValue();
        Token token;
        skipCommentTokens(token);
        if (collectComments_ && !commentsBefore_.empty())
            root.setComment(commentsBefore_, commentAfter);
        if (features_.strictRoot_) {
            if (!root.isArray() && !root.isObject()) {
                // Set error location to start of doc, ideally should be first token found
                // in doc
                token.type_ = tokenError;
                token.start_ = beginDoc;
                token.end_ = endDoc;
                addError(
                    "A valid JSON document must be either an array or an object value.",
                    token);
                return false;
            }
        }
        return successful;
    }

    bool Reader::readValue() {
        // readValue() may call itself only if it calls readObject() or ReadArray().
        // These methods execute nodes_.push() just before and nodes_.pop)() just
        // after calling readValue(). parse() executes one nodes_.push(), so > instead
        // of >=.
        if (nodes_.size() > stackLimit_g)
            throwRuntimeError("Exceeded stackLimit in readValue().");

        Token token;
        skipCommentTokens(token);
        bool successful = true;

        if (collectComments_ && !commentsBefore_.empty()) {
            currentValue().setComment(commentsBefore_, commentBefore);
            commentsBefore_.clear();
        }

        switch (token.type_) {
        case tokenObjectBegin:
            successful = readObject(token);
            currentValue().setOffsetLimit(current_ - begin_);
            break;
        case tokenArrayBegin:
            successful = readArray(token);
            currentValue().setOffsetLimit(current_ - begin_);
            break;
        case tokenNumber:
            successful = decodeNumber(token);
            break;
        case tokenString:
            successful = decodeString(token);
            break;
        case tokenTrue: {
            Value v(true);
            currentValue().swapPayload(v);
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
        } break;
        case tokenFalse: {
            Value v(false);
            currentValue().swapPayload(v);
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
        } break;
        case tokenNull: {
            Value v;
            currentValue().swapPayload(v);
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
        } break;
        case tokenArraySeparator:
        case tokenObjectEnd:
        case tokenArrayEnd:
            if (features_.allowDroppedNullPlaceholders_) {
                // "Un-read" the current token and mark the current value as a null
                // token.
                current_--;
                Value v;
                currentValue().swapPayload(v);
                currentValue().setOffsetStart(current_ - begin_ - 1);
                currentValue().setOffsetLimit(current_ - begin_);
                break;
            } // Else, fall through...
        default:
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
            return addError("Syntax error: value, object or array expected.", token);
        }

        if (collectComments_) {
            lastValueEnd_ = current_;
            lastValue_ = &currentValue();
        }

        return successful;
    }

    void Reader::skipCommentTokens(Token& token) {
        if (features_.allowComments_) {
            do {
                readToken(token);
            } while (token.type_ == tokenComment);
        } else {
            readToken(token);
        }
    }

    bool Reader::readToken(Token& token) {
        skipSpaces();
        token.start_ = current_;
        Char c = getNextChar();
        bool ok = true;
        switch (c) {
        case '{':
            token.type_ = tokenObjectBegin;
            break;
        case '}':
            token.type_ = tokenObjectEnd;
            break;
        case '[':
            token.type_ = tokenArrayBegin;
            break;
        case ']':
            token.type_ = tokenArrayEnd;
            break;
        case '"':
            token.type_ = tokenString;
            ok = readString();
            break;
        case '/':
            token.type_ = tokenComment;
            ok = readComment();
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
            token.type_ = tokenNumber;
            readNumber();
            break;
        case 't':
            token.type_ = tokenTrue;
            ok = match("rue", 3);
            break;
        case 'f':
            token.type_ = tokenFalse;
            ok = match("alse", 4);
            break;
        case 'n':
            token.type_ = tokenNull;
            ok = match("ull", 3);
            break;
        case ',':
            token.type_ = tokenArraySeparator;
            break;
        case ':':
            token.type_ = tokenMemberSeparator;
            break;
        case 0:
            token.type_ = tokenEndOfStream;
            break;
        default:
            ok = false;
            break;
        }
        if (!ok)
            token.type_ = tokenError;
        token.end_ = current_;
        return ok;
    }

    void Reader::skipSpaces() {
        while (current_ != end_) {
            Char c = *current_;
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                ++current_;
            else
                break;
        }
    }

    bool Reader::match(const Char* pattern, int patternLength) {
        if (end_ - current_ < patternLength)
            return false;
        int index = patternLength;
        while (index--)
            if (current_[index] != pattern[index])
                return false;
        current_ += patternLength;
        return true;
    }

    bool Reader::readComment() {
        Location commentBegin = current_ - 1;
        Char c = getNextChar();
        bool successful = false;
        if (c == '*')
            successful = readCStyleComment();
        else if (c == '/')
            successful = readCppStyleComment();
        if (!successful)
            return false;

        if (collectComments_) {
            CommentPlacement placement = commentBefore;
            if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
                if (c != '*' || !containsNewLine(commentBegin, current_))
                    placement = commentAfterOnSameLine;
            }

            addComment(commentBegin, current_, placement);
        }
        return true;
    }

    String Reader::normalizeEOL(Reader::Location begin, Reader::Location end) {
        String normalized;
        normalized.reserve(static_cast<size_t>(end - begin));
        Reader::Location current = begin;
        while (current != end) {
            char c = *current++;
            if (c == '\r') {
                if (current != end && *current == '\n')
                    // convert dos EOL
                    ++current;
                // convert Mac EOL
                normalized += '\n';
            } else {
                normalized += c;
            }
        }
        return normalized;
    }

    void Reader::addComment(Location begin, Location end,
        CommentPlacement placement) {
        assert(collectComments_);
        const String& normalized = normalizeEOL(begin, end);
        if (placement == commentAfterOnSameLine) {
            assert(lastValue_ != nullptr);
            lastValue_->setComment(normalized, placement);
        } else {
            commentsBefore_ += normalized;
        }
    }

    bool Reader::readCStyleComment() {
        while ((current_ + 1) < end_) {
            Char c = getNextChar();
            if (c == '*' && *current_ == '/')
                break;
        }
        return getNextChar() == '/';
    }

    bool Reader::readCppStyleComment() {
        while (current_ != end_) {
            Char c = getNextChar();
            if (c == '\n')
                break;
            if (c == '\r') {
                // Consume DOS EOL. It will be normalized in addComment.
                if (current_ != end_ && *current_ == '\n')
                    getNextChar();
                // Break on Moc OS 9 EOL.
                break;
            }
        }
        return true;
    }

    void Reader::readNumber() {
        Location p = current_;
        char c = '0'; // stopgap for already consumed character
        // integral part
        while (c >= '0' && c <= '9')
            c = (current_ = p) < end_ ? *p++ : '\0';
        // fractional part
        if (c == '.') {
            c = (current_ = p) < end_ ? *p++ : '\0';
            while (c >= '0' && c <= '9')
                c = (current_ = p) < end_ ? *p++ : '\0';
        }
        // exponential part
        if (c == 'e' || c == 'E') {
            c = (current_ = p) < end_ ? *p++ : '\0';
            if (c == '+' || c == '-')
                c = (current_ = p) < end_ ? *p++ : '\0';
            while (c >= '0' && c <= '9')
                c = (current_ = p) < end_ ? *p++ : '\0';
        }
    }

    bool Reader::readString() {
        Char c = '\0';
        while (current_ != end_) {
            c = getNextChar();
            if (c == '\\')
                getNextChar();
            else if (c == '"')
                break;
        }
        return c == '"';
    }

    bool Reader::readObject(Token& token) {
        Token tokenName;
        String name;
        Value init(objectValue);
        currentValue().swapPayload(init);
        currentValue().setOffsetStart(token.start_ - begin_);
        while (readToken(tokenName)) {
            bool initialTokenOk = true;
            while (tokenName.type_ == tokenComment && initialTokenOk)
                initialTokenOk = readToken(tokenName);
            if (!initialTokenOk)
                break;
            if (tokenName.type_ == tokenObjectEnd &&
                (name.empty() ||
                    features_.allowTrailingCommas_)) // empty object or trailing comma
                return true;
            name.clear();
            if (tokenName.type_ == tokenString) {
                if (!decodeString(tokenName, name))
                    return recoverFromError(tokenObjectEnd);
            } else if (tokenName.type_ == tokenNumber && features_.allowNumericKeys_) {
                Value numberName;
                if (!decodeNumber(tokenName, numberName))
                    return recoverFromError(tokenObjectEnd);
                name = numberName.asString();
            } else {
                break;
            }

            Token colon;
            if (!readToken(colon) || colon.type_ != tokenMemberSeparator) {
                return addErrorAndRecover("Missing ':' after object member name", colon,
                    tokenObjectEnd);
            }
            Value& value = currentValue()[name];
            nodes_.push(&value);
            bool ok = readValue();
            nodes_.pop();
            if (!ok) // error already set
                return recoverFromError(tokenObjectEnd);

            Token comma;
            if (!readToken(comma) ||
                (comma.type_ != tokenObjectEnd && comma.type_ != tokenArraySeparator &&
                    comma.type_ != tokenComment)) {
                return addErrorAndRecover("Missing ',' or '}' in object declaration",
                    comma, tokenObjectEnd);
            }
            bool finalizeTokenOk = true;
            while (comma.type_ == tokenComment && finalizeTokenOk)
                finalizeTokenOk = readToken(comma);
            if (comma.type_ == tokenObjectEnd)
                return true;
        }
        return addErrorAndRecover("Missing '}' or object member name", tokenName,
            tokenObjectEnd);
    }

    bool Reader::readArray(Token& token) {
        Value init(arrayValue);
        currentValue().swapPayload(init);
        currentValue().setOffsetStart(token.start_ - begin_);
        int index = 0;
        for (;;) {
            skipSpaces();
            if (current_ != end_ && *current_ == ']' &&
                (index == 0 ||
                (features_.allowTrailingCommas_ &&
                    !features_.allowDroppedNullPlaceholders_))) // empty array or trailing
                                                                // comma
            {
                Token endArray;
                readToken(endArray);
                return true;
            }

            Value& value = currentValue()[index++];
            nodes_.push(&value);
            bool ok = readValue();
            nodes_.pop();
            if (!ok) // error already set
                return recoverFromError(tokenArrayEnd);

            Token currentToken;
            // Accept Comment after last item in the array.
            ok = readToken(currentToken);
            while (currentToken.type_ == tokenComment && ok) {
                ok = readToken(currentToken);
            }
            bool badTokenType = (currentToken.type_ != tokenArraySeparator &&
                currentToken.type_ != tokenArrayEnd);
            if (!ok || badTokenType) {
                return addErrorAndRecover("Missing ',' or ']' in array declaration",
                    currentToken, tokenArrayEnd);
            }
            if (currentToken.type_ == tokenArrayEnd)
                break;
        }
        return true;
    }

    bool Reader::decodeNumber(Token& token) {
        Value decoded;
        if (!decodeNumber(token, decoded))
            return false;
        currentValue().swapPayload(decoded);
        currentValue().setOffsetStart(token.start_ - begin_);
        currentValue().setOffsetLimit(token.end_ - begin_);
        return true;
    }

    bool Reader::decodeNumber(Token& token, Value& decoded) {
        // Attempts to parse the number as an integer. If the number is
        // larger than the maximum supported value of an integer then
        // we decode the number as a double.
        Location current = token.start_;
        bool isNegative = *current == '-';
        if (isNegative)
            ++current;
        // TODO: Help the compiler do the div and mod at compile time or get rid of
        // them.
        Value::LargestUInt maxIntegerValue =
            isNegative ? Value::LargestUInt(Value::maxLargestInt) + 1
            : Value::maxLargestUInt;
        Value::LargestUInt threshold = maxIntegerValue / 10;
        Value::LargestUInt value = 0;
        while (current < token.end_) {
            Char c = *current++;
            if (c < '0' || c > '9')
                return decodeDouble(token, decoded);
            auto digit(static_cast<Value::UInt>(c - '0'));
            if (value >= threshold) {
                // We've hit or exceeded the max value divided by 10 (rounded down). If
                // a) we've only just touched the limit, b) this is the last digit, and
                // c) it's small enough to fit in that rounding delta, we're okay.
                // Otherwise treat this number as a double to avoid overflow.
                if (value > threshold || current != token.end_ ||
                    digit > maxIntegerValue % 10) {
                    return decodeDouble(token, decoded);
                }
            }
            value = value * 10 + digit;
        }
        if (isNegative && value == maxIntegerValue)
            decoded = Value::minLargestInt;
        else if (isNegative)
            decoded = -Value::LargestInt(value);
        else if (value <= Value::LargestUInt(Value::maxInt))
            decoded = Value::LargestInt(value);
        else
            decoded = value;
        return true;
    }

    bool Reader::decodeDouble(Token& token) {
        Value decoded;
        if (!decodeDouble(token, decoded))
            return false;
        currentValue().swapPayload(decoded);
        currentValue().setOffsetStart(token.start_ - begin_);
        currentValue().setOffsetLimit(token.end_ - begin_);
        return true;
    }

    bool Reader::decodeDouble(Token& token, Value& decoded) {
        double value = 0;
        String buffer(token.start_, token.end_);
        IStringStream is(buffer);
        if (!(is >> value))
            return addError(
                "'" + String(token.start_, token.end_) + "' is not a number.", token);
        decoded = value;
        return true;
    }

    bool Reader::decodeString(Token& token) {
        String decoded_string;
        if (!decodeString(token, decoded_string))
            return false;
        Value decoded(decoded_string);
        currentValue().swapPayload(decoded);
        currentValue().setOffsetStart(token.start_ - begin_);
        currentValue().setOffsetLimit(token.end_ - begin_);
        return true;
    }

    bool Reader::decodeString(Token& token, String& decoded) {
        decoded.reserve(static_cast<size_t>(token.end_ - token.start_ - 2));
        Location current = token.start_ + 1; // skip '"'
        Location end = token.end_ - 1;       // do not include '"'
        while (current != end) {
            Char c = *current++;
            if (c == '"')
                break;
            if (c == '\\') {
                if (current == end)
                    return addError("Empty escape sequence in string", token, current);
                Char escape = *current++;
                switch (escape) {
                case '"':
                    decoded += '"';
                    break;
                case '/':
                    decoded += '/';
                    break;
                case '\\':
                    decoded += '\\';
                    break;
                case 'b':
                    decoded += '\b';
                    break;
                case 'f':
                    decoded += '\f';
                    break;
                case 'n':
                    decoded += '\n';
                    break;
                case 'r':
                    decoded += '\r';
                    break;
                case 't':
                    decoded += '\t';
                    break;
                case 'u': {
                    unsigned int unicode;
                    if (!decodeUnicodeCodePoint(token, current, end, unicode))
                        return false;
                    decoded += codePointToUTF8(unicode);
                } break;
                default:
                    return addError("Bad escape sequence in string", token, current);
                }
            } else {
                decoded += c;
            }
        }
        return true;
    }

    bool Reader::decodeUnicodeCodePoint(Token& token, Location& current,
        Location end, unsigned int& unicode) {

        if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
            return false;
        if (unicode >= 0xD800 && unicode <= 0xDBFF) {
            // surrogate pairs
            if (end - current < 6)
                return addError(
                    "additional six characters expected to parse unicode surrogate pair.",
                    token, current);
            if (*(current++) == '\\' && *(current++) == 'u') {
                unsigned int surrogatePair;
                if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair)) {
                    unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
                } else
                    return false;
            } else
                return addError("expecting another \\u token to begin the second half of "
                    "a unicode surrogate pair",
                    token, current);
        }
        return true;
    }

    bool Reader::decodeUnicodeEscapeSequence(Token& token, Location& current,
        Location end,
        unsigned int& ret_unicode) {
        if (end - current < 4)
            return addError(
                "Bad unicode escape sequence in string: four digits expected.", token,
                current);
        int unicode = 0;
        for (int index = 0; index < 4; ++index) {
            Char c = *current++;
            unicode *= 16;
            if (c >= '0' && c <= '9')
                unicode += c - '0';
            else if (c >= 'a' && c <= 'f')
                unicode += c - 'a' + 10;
            else if (c >= 'A' && c <= 'F')
                unicode += c - 'A' + 10;
            else
                return addError(
                    "Bad unicode escape sequence in string: hexadecimal digit expected.",
                    token, current);
        }
        ret_unicode = static_cast<unsigned int>(unicode);
        return true;
    }

    bool Reader::addError(const String& message, Token& token, Location extra) {
        ErrorInfo info;
        info.token_ = token;
        info.message_ = message;
        info.extra_ = extra;
        errors_.push_back(info);
        return false;
    }

    bool Reader::recoverFromError(TokenType skipUntilToken) {
        size_t const errorCount = errors_.size();
        Token skip;
        for (;;) {
            if (!readToken(skip))
                errors_.resize(errorCount); // discard errors caused by recovery
            if (skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
                break;
        }
        errors_.resize(errorCount);
        return false;
    }

    bool Reader::addErrorAndRecover(const String& message, Token& token,
        TokenType skipUntilToken) {
        addError(message, token);
        return recoverFromError(skipUntilToken);
    }

    Value& Reader::currentValue() { return *(nodes_.top()); }

    Reader::Char Reader::getNextChar() {
        if (current_ == end_)
            return 0;
        return *current_++;
    }

    void Reader::getLocationLineAndColumn(Location location, int& line,
        int& column) const {
        Location current = begin_;
        Location lastLineStart = current;
        line = 0;
        while (current < location && current != end_) {
            Char c = *current++;
            if (c == '\r') {
                if (*current == '\n')
                    ++current;
                lastLineStart = current;
                ++line;
            } else if (c == '\n') {
                lastLineStart = current;
                ++line;
            }
        }
        // column & line start at 1
        column = int(location - lastLineStart) + 1;
        ++line;
    }

    String Reader::getLocationLineAndColumn(Location location) const {
        int line, column;
        getLocationLineAndColumn(location, line, column);
        char buffer[18 + 16 + 16 + 1];
        jsoncpp_snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
        return buffer;
    }

    // Deprecated. Preserved for backward compatibility
    String Reader::getFormatedErrorMessages() const {
        return getFormattedErrorMessages();
    }

    String Reader::getFormattedErrorMessages() const {
        String formattedMessage;
        for (const auto& error : errors_) {
            formattedMessage +=
                "* " + getLocationLineAndColumn(error.token_.start_) + "\n";
            formattedMessage += "  " + error.message_ + "\n";
            if (error.extra_)
                formattedMessage +=
                "See " + getLocationLineAndColumn(error.extra_) + " for detail.\n";
        }
        return formattedMessage;
    }

    std::vector<Reader::StructuredError> Reader::getStructuredErrors() const {
        std::vector<Reader::StructuredError> allErrors;
        for (const auto& error : errors_) {
            Reader::StructuredError structured;
            structured.offset_start = error.token_.start_ - begin_;
            structured.offset_limit = error.token_.end_ - begin_;
            structured.message = error.message_;
            allErrors.push_back(structured);
        }
        return allErrors;
    }

    bool Reader::pushError(const Value& value, const String& message) {
        ptrdiff_t const length = end_ - begin_;
        if (value.getOffsetStart() > length || value.getOffsetLimit() > length)
            return false;
        Token token;
        token.type_ = tokenError;
        token.start_ = begin_ + value.getOffsetStart();
        token.end_ = begin_ + value.getOffsetLimit();
        ErrorInfo info;
        info.token_ = token;
        info.message_ = message;
        info.extra_ = nullptr;
        errors_.push_back(info);
        return true;
    }

    bool Reader::pushError(const Value& value, const String& message,
        const Value& extra) {
        ptrdiff_t const length = end_ - begin_;
        if (value.getOffsetStart() > length || value.getOffsetLimit() > length ||
            extra.getOffsetLimit() > length)
            return false;
        Token token;
        token.type_ = tokenError;
        token.start_ = begin_ + value.getOffsetStart();
        token.end_ = begin_ + value.getOffsetLimit();
        ErrorInfo info;
        info.token_ = token;
        info.message_ = message;
        info.extra_ = begin_ + extra.getOffsetStart();
        errors_.push_back(info);
        return true;
    }

    bool Reader::good() const { return errors_.empty(); }

    // Originally copied from the Features class (now deprecated), used internally
    // for features implementation.
    class OurFeatures {
    public:
        static OurFeatures all();
        bool allowComments_;
        bool allowTrailingCommas_;
        bool strictRoot_;
        bool allowDroppedNullPlaceholders_;
        bool allowNumericKeys_;
        bool allowSingleQuotes_;
        bool failIfExtra_;
        bool rejectDupKeys_;
        bool allowSpecialFloats_;
        size_t stackLimit_;
    }; // OurFeatures

    OurFeatures OurFeatures::all() { return {}; }

    // Implementation of class Reader
    // ////////////////////////////////

    // Originally copied from the Reader class (now deprecated), used internally
    // for implementing JSON reading.
    class OurReader {
    public:
        using Char = char;
        using Location = const Char*;
        struct StructuredError {
            ptrdiff_t offset_start;
            ptrdiff_t offset_limit;
            String message;
        };

        explicit OurReader(OurFeatures const& features);
        bool parse(const char* beginDoc, const char* endDoc, Value& root,
            bool collectComments = true);
        String getFormattedErrorMessages() const;
        std::vector<StructuredError> getStructuredErrors() const;

    private:
        OurReader(OurReader const&);      // no impl
        void operator=(OurReader const&); // no impl

        enum TokenType {
            tokenEndOfStream = 0,
            tokenObjectBegin,
            tokenObjectEnd,
            tokenArrayBegin,
            tokenArrayEnd,
            tokenString,
            tokenNumber,
            tokenTrue,
            tokenFalse,
            tokenNull,
            tokenNaN,
            tokenPosInf,
            tokenNegInf,
            tokenArraySeparator,
            tokenMemberSeparator,
            tokenComment,
            tokenError
        };

        class Token {
        public:
            TokenType type_;
            Location start_;
            Location end_;
        };

        class ErrorInfo {
        public:
            Token token_;
            String message_;
            Location extra_;
        };

        using Errors = std::deque<ErrorInfo>;

        bool readToken(Token& token);
        void skipSpaces();
        bool match(const Char* pattern, int patternLength);
        bool readComment();
        bool readCStyleComment(bool* containsNewLineResult);
        bool readCppStyleComment();
        bool readString();
        bool readStringSingleQuote();
        bool readNumber(bool checkInf);
        bool readValue();
        bool readObject(Token& token);
        bool readArray(Token& token);
        bool decodeNumber(Token& token);
        bool decodeNumber(Token& token, Value& decoded);
        bool decodeString(Token& token);
        bool decodeString(Token& token, String& decoded);
        bool decodeDouble(Token& token);
        bool decodeDouble(Token& token, Value& decoded);
        bool decodeUnicodeCodePoint(Token& token, Location& current, Location end,
            unsigned int& unicode);
        bool decodeUnicodeEscapeSequence(Token& token, Location& current,
            Location end, unsigned int& unicode);
        bool addError(const String& message, Token& token, Location extra = nullptr);
        bool recoverFromError(TokenType skipUntilToken);
        bool addErrorAndRecover(const String& message, Token& token,
            TokenType skipUntilToken);
        void skipUntilSpace();
        Value& currentValue();
        Char getNextChar();
        void getLocationLineAndColumn(Location location, int& line,
            int& column) const;
        String getLocationLineAndColumn(Location location) const;
        void addComment(Location begin, Location end, CommentPlacement placement);
        void skipCommentTokens(Token& token);

        static String normalizeEOL(Location begin, Location end);
        static bool containsNewLine(Location begin, Location end);

        using Nodes = std::stack<Value*>;

        Nodes nodes_{};
        Errors errors_{};
        String document_{};
        Location begin_ = nullptr;
        Location end_ = nullptr;
        Location current_ = nullptr;
        Location lastValueEnd_ = nullptr;
        Value* lastValue_ = nullptr;
        bool lastValueHasAComment_ = false;
        String commentsBefore_{};

        OurFeatures const features_;
        bool collectComments_ = false;
    }; // OurReader

    // complete copy of Read impl, for OurReader

    bool OurReader::containsNewLine(OurReader::Location begin,
        OurReader::Location end) {
        for (; begin < end; ++begin)
            if (*begin == '\n' || *begin == '\r')
                return true;
        return false;
    }

    OurReader::OurReader(OurFeatures const& features) : features_(features) {}

    bool OurReader::parse(const char* beginDoc, const char* endDoc, Value& root,
        bool collectComments) {
        if (!features_.allowComments_) {
            collectComments = false;
        }

        begin_ = beginDoc;
        end_ = endDoc;
        collectComments_ = collectComments;
        current_ = begin_;
        lastValueEnd_ = nullptr;
        lastValue_ = nullptr;
        commentsBefore_.clear();
        errors_.clear();
        while (!nodes_.empty())
            nodes_.pop();
        nodes_.push(&root);

        bool successful = readValue();
        nodes_.pop();
        Token token;
        skipCommentTokens(token);
        if (features_.failIfExtra_ && (token.type_ != tokenEndOfStream)) {
            addError("Extra non-whitespace after JSON value.", token);
            return false;
        }
        if (collectComments_ && !commentsBefore_.empty())
            root.setComment(commentsBefore_, commentAfter);
        if (features_.strictRoot_) {
            if (!root.isArray() && !root.isObject()) {
                // Set error location to start of doc, ideally should be first token found
                // in doc
                token.type_ = tokenError;
                token.start_ = beginDoc;
                token.end_ = endDoc;
                addError(
                    "A valid JSON document must be either an array or an object value.",
                    token);
                return false;
            }
        }
        return successful;
    }

    bool OurReader::readValue() {
        //  To preserve the old behaviour we cast size_t to int.
        if (nodes_.size() > features_.stackLimit_)
            throwRuntimeError("Exceeded stackLimit in readValue().");
        Token token;
        skipCommentTokens(token);
        bool successful = true;

        if (collectComments_ && !commentsBefore_.empty()) {
            currentValue().setComment(commentsBefore_, commentBefore);
            commentsBefore_.clear();
        }

        switch (token.type_) {
        case tokenObjectBegin:
            successful = readObject(token);
            currentValue().setOffsetLimit(current_ - begin_);
            break;
        case tokenArrayBegin:
            successful = readArray(token);
            currentValue().setOffsetLimit(current_ - begin_);
            break;
        case tokenNumber:
            successful = decodeNumber(token);
            break;
        case tokenString:
            successful = decodeString(token);
            break;
        case tokenTrue: {
            Value v(true);
            currentValue().swapPayload(v);
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
        } break;
        case tokenFalse: {
            Value v(false);
            currentValue().swapPayload(v);
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
        } break;
        case tokenNull: {
            Value v;
            currentValue().swapPayload(v);
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
        } break;
        case tokenNaN: {
            Value v(std::numeric_limits<double>::quiet_NaN());
            currentValue().swapPayload(v);
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
        } break;
        case tokenPosInf: {
            Value v(std::numeric_limits<double>::infinity());
            currentValue().swapPayload(v);
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
        } break;
        case tokenNegInf: {
            Value v(-std::numeric_limits<double>::infinity());
            currentValue().swapPayload(v);
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
        } break;
        case tokenArraySeparator:
        case tokenObjectEnd:
        case tokenArrayEnd:
            if (features_.allowDroppedNullPlaceholders_) {
                // "Un-read" the current token and mark the current value as a null
                // token.
                current_--;
                Value v;
                currentValue().swapPayload(v);
                currentValue().setOffsetStart(current_ - begin_ - 1);
                currentValue().setOffsetLimit(current_ - begin_);
                break;
            } // else, fall through ...
        default:
            currentValue().setOffsetStart(token.start_ - begin_);
            currentValue().setOffsetLimit(token.end_ - begin_);
            return addError("Syntax error: value, object or array expected.", token);
        }

        if (collectComments_) {
            lastValueEnd_ = current_;
            lastValueHasAComment_ = false;
            lastValue_ = &currentValue();
        }

        return successful;
    }

    void OurReader::skipCommentTokens(Token& token) {
        if (features_.allowComments_) {
            do {
                readToken(token);
            } while (token.type_ == tokenComment);
        } else {
            readToken(token);
        }
    }

    bool OurReader::readToken(Token& token) {
        skipSpaces();
        token.start_ = current_;
        Char c = getNextChar();
        bool ok = true;
        switch (c) {
        case '{':
            token.type_ = tokenObjectBegin;
            break;
        case '}':
            token.type_ = tokenObjectEnd;
            break;
        case '[':
            token.type_ = tokenArrayBegin;
            break;
        case ']':
            token.type_ = tokenArrayEnd;
            break;
        case '"':
            token.type_ = tokenString;
            ok = readString();
            break;
        case '\'':
            if (features_.allowSingleQuotes_) {
                token.type_ = tokenString;
                ok = readStringSingleQuote();
                break;
            } // else fall through
        case '/':
            token.type_ = tokenComment;
            ok = readComment();
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            token.type_ = tokenNumber;
            readNumber(false);
            break;
        case '-':
            if (readNumber(true)) {
                token.type_ = tokenNumber;
            } else {
                token.type_ = tokenNegInf;
                ok = features_.allowSpecialFloats_ && match("nfinity", 7);
            }
            break;
        case '+':
            if (readNumber(true)) {
                token.type_ = tokenNumber;
            } else {
                token.type_ = tokenPosInf;
                ok = features_.allowSpecialFloats_ && match("nfinity", 7);
            }
            break;
        case 't':
            token.type_ = tokenTrue;
            ok = match("rue", 3);
            break;
        case 'f':
            token.type_ = tokenFalse;
            ok = match("alse", 4);
            break;
        case 'n':
            token.type_ = tokenNull;
            ok = match("ull", 3);
            break;
        case 'N':
            if (features_.allowSpecialFloats_) {
                token.type_ = tokenNaN;
                ok = match("aN", 2);
            } else {
                ok = false;
            }
            break;
        case 'I':
            if (features_.allowSpecialFloats_) {
                token.type_ = tokenPosInf;
                ok = match("nfinity", 7);
            } else {
                ok = false;
            }
            break;
        case ',':
            token.type_ = tokenArraySeparator;
            break;
        case ':':
            token.type_ = tokenMemberSeparator;
            break;
        case 0:
            token.type_ = tokenEndOfStream;
            break;
        default:
            ok = false;
            break;
        }
        if (!ok)
            token.type_ = tokenError;
        token.end_ = current_;
        return ok;
    }

    void OurReader::skipSpaces() {
        while (current_ != end_) {
            Char c = *current_;
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                ++current_;
            else
                break;
        }
    }

    bool OurReader::match(const Char* pattern, int patternLength) {
        if (end_ - current_ < patternLength)
            return false;
        int index = patternLength;
        while (index--)
            if (current_[index] != pattern[index])
                return false;
        current_ += patternLength;
        return true;
    }

    bool OurReader::readComment() {
        const Location commentBegin = current_ - 1;
        const Char c = getNextChar();
        bool successful = false;
        bool cStyleWithEmbeddedNewline = false;

        const bool isCStyleComment = (c == '*');
        const bool isCppStyleComment = (c == '/');
        if (isCStyleComment) {
            successful = readCStyleComment(&cStyleWithEmbeddedNewline);
        } else if (isCppStyleComment) {
            successful = readCppStyleComment();
        }

        if (!successful)
            return false;

        if (collectComments_) {
            CommentPlacement placement = commentBefore;

            if (!lastValueHasAComment_) {
                if (lastValueEnd_ && !containsNewLine(lastValueEnd_, commentBegin)) {
                    if (isCppStyleComment || !cStyleWithEmbeddedNewline) {
                        placement = commentAfterOnSameLine;
                        lastValueHasAComment_ = true;
                    }
                }
            }

            addComment(commentBegin, current_, placement);
        }
        return true;
    }

    String OurReader::normalizeEOL(OurReader::Location begin,
        OurReader::Location end) {
        String normalized;
        normalized.reserve(static_cast<size_t>(end - begin));
        OurReader::Location current = begin;
        while (current != end) {
            char c = *current++;
            if (c == '\r') {
                if (current != end && *current == '\n')
                    // convert dos EOL
                    ++current;
                // convert Mac EOL
                normalized += '\n';
            } else {
                normalized += c;
            }
        }
        return normalized;
    }

    void OurReader::addComment(Location begin, Location end,
        CommentPlacement placement) {
        assert(collectComments_);
        const String& normalized = normalizeEOL(begin, end);
        if (placement == commentAfterOnSameLine) {
            assert(lastValue_ != nullptr);
            lastValue_->setComment(normalized, placement);
        } else {
            commentsBefore_ += normalized;
        }
    }

    bool OurReader::readCStyleComment(bool* containsNewLineResult) {
        *containsNewLineResult = false;

        while ((current_ + 1) < end_) {
            Char c = getNextChar();
            if (c == '*' && *current_ == '/')
                break;
            if (c == '\n')
                *containsNewLineResult = true;
        }

        return getNextChar() == '/';
    }

    bool OurReader::readCppStyleComment() {
        while (current_ != end_) {
            Char c = getNextChar();
            if (c == '\n')
                break;
            if (c == '\r') {
                // Consume DOS EOL. It will be normalized in addComment.
                if (current_ != end_ && *current_ == '\n')
                    getNextChar();
                // Break on Moc OS 9 EOL.
                break;
            }
        }
        return true;
    }

    bool OurReader::readNumber(bool checkInf) {
        Location p = current_;
        if (checkInf && p != end_ && *p == 'I') {
            current_ = ++p;
            return false;
        }
        char c = '0'; // stopgap for already consumed character
        // integral part
        while (c >= '0' && c <= '9')
            c = (current_ = p) < end_ ? *p++ : '\0';
        // fractional part
        if (c == '.') {
            c = (current_ = p) < end_ ? *p++ : '\0';
            while (c >= '0' && c <= '9')
                c = (current_ = p) < end_ ? *p++ : '\0';
        }
        // exponential part
        if (c == 'e' || c == 'E') {
            c = (current_ = p) < end_ ? *p++ : '\0';
            if (c == '+' || c == '-')
                c = (current_ = p) < end_ ? *p++ : '\0';
            while (c >= '0' && c <= '9')
                c = (current_ = p) < end_ ? *p++ : '\0';
        }
        return true;
    }
    bool OurReader::readString() {
        Char c = 0;
        while (current_ != end_) {
            c = getNextChar();
            if (c == '\\')
                getNextChar();
            else if (c == '"')
                break;
        }
        return c == '"';
    }

    bool OurReader::readStringSingleQuote() {
        Char c = 0;
        while (current_ != end_) {
            c = getNextChar();
            if (c == '\\')
                getNextChar();
            else if (c == '\'')
                break;
        }
        return c == '\'';
    }

    bool OurReader::readObject(Token& token) {
        Token tokenName;
        String name;
        Value init(objectValue);
        currentValue().swapPayload(init);
        currentValue().setOffsetStart(token.start_ - begin_);
        while (readToken(tokenName)) {
            bool initialTokenOk = true;
            while (tokenName.type_ == tokenComment && initialTokenOk)
                initialTokenOk = readToken(tokenName);
            if (!initialTokenOk)
                break;
            if (tokenName.type_ == tokenObjectEnd &&
                (name.empty() ||
                    features_.allowTrailingCommas_)) // empty object or trailing comma
                return true;
            name.clear();
            if (tokenName.type_ == tokenString) {
                if (!decodeString(tokenName, name))
                    return recoverFromError(tokenObjectEnd);
            } else if (tokenName.type_ == tokenNumber && features_.allowNumericKeys_) {
                Value numberName;
                if (!decodeNumber(tokenName, numberName))
                    return recoverFromError(tokenObjectEnd);
                name = numberName.asString();
            } else {
                break;
            }
            if (name.length() >= (1U << 30))
                throwRuntimeError("keylength >= 2^30");
            if (features_.rejectDupKeys_ && currentValue().isMember(name)) {
                String msg = "Duplicate key: '" + name + "'";
                return addErrorAndRecover(msg, tokenName, tokenObjectEnd);
            }

            Token colon;
            if (!readToken(colon) || colon.type_ != tokenMemberSeparator) {
                return addErrorAndRecover("Missing ':' after object member name", colon,
                    tokenObjectEnd);
            }
            Value& value = currentValue()[name];
            nodes_.push(&value);
            bool ok = readValue();
            nodes_.pop();
            if (!ok) // error already set
                return recoverFromError(tokenObjectEnd);

            Token comma;
            if (!readToken(comma) ||
                (comma.type_ != tokenObjectEnd && comma.type_ != tokenArraySeparator &&
                    comma.type_ != tokenComment)) {
                return addErrorAndRecover("Missing ',' or '}' in object declaration",
                    comma, tokenObjectEnd);
            }
            bool finalizeTokenOk = true;
            while (comma.type_ == tokenComment && finalizeTokenOk)
                finalizeTokenOk = readToken(comma);
            if (comma.type_ == tokenObjectEnd)
                return true;
        }
        return addErrorAndRecover("Missing '}' or object member name", tokenName,
            tokenObjectEnd);
    }

    bool OurReader::readArray(Token& token) {
        Value init(arrayValue);
        currentValue().swapPayload(init);
        currentValue().setOffsetStart(token.start_ - begin_);
        int index = 0;
        for (;;) {
            skipSpaces();
            if (current_ != end_ && *current_ == ']' &&
                (index == 0 ||
                (features_.allowTrailingCommas_ &&
                    !features_.allowDroppedNullPlaceholders_))) // empty array or trailing
                                                                // comma
            {
                Token endArray;
                readToken(endArray);
                return true;
            }
            Value& value = currentValue()[index++];
            nodes_.push(&value);
            bool ok = readValue();
            nodes_.pop();
            if (!ok) // error already set
                return recoverFromError(tokenArrayEnd);

            Token currentToken;
            // Accept Comment after last item in the array.
            ok = readToken(currentToken);
            while (currentToken.type_ == tokenComment && ok) {
                ok = readToken(currentToken);
            }
            bool badTokenType = (currentToken.type_ != tokenArraySeparator &&
                currentToken.type_ != tokenArrayEnd);
            if (!ok || badTokenType) {
                return addErrorAndRecover("Missing ',' or ']' in array declaration",
                    currentToken, tokenArrayEnd);
            }
            if (currentToken.type_ == tokenArrayEnd)
                break;
        }
        return true;
    }

    bool OurReader::decodeNumber(Token& token) {
        Value decoded;
        if (!decodeNumber(token, decoded))
            return false;
        currentValue().swapPayload(decoded);
        currentValue().setOffsetStart(token.start_ - begin_);
        currentValue().setOffsetLimit(token.end_ - begin_);
        return true;
    }

    bool OurReader::decodeNumber(Token& token, Value& decoded) {
        // Attempts to parse the number as an integer. If the number is
        // larger than the maximum supported value of an integer then
        // we decode the number as a double.
        Location current = token.start_;
        const bool isNegative = *current == '-';
        if (isNegative) {
            ++current;
        }

        // We assume we can represent the largest and smallest integer types as
        // unsigned integers with separate sign. This is only true if they can fit
        // into an unsigned integer.
        static_assert(Value::maxLargestInt <= Value::maxLargestUInt,
            "Int must be smaller than UInt");

        // We need to convert minLargestInt into a positive number. The easiest way
        // to do this conversion is to assume our "threshold" value of minLargestInt
        // divided by 10 can fit in maxLargestInt when absolute valued. This should
        // be a safe assumption.
        static_assert(Value::minLargestInt <= -Value::maxLargestInt,
            "The absolute value of minLargestInt must be greater than or "
            "equal to maxLargestInt");
        static_assert(Value::minLargestInt / 10 >= -Value::maxLargestInt,
            "The absolute value of minLargestInt must be only 1 magnitude "
            "larger than maxLargest Int");

        static constexpr Value::LargestUInt positive_threshold =
            Value::maxLargestUInt / 10;
        static constexpr Value::UInt positive_last_digit = Value::maxLargestUInt % 10;

        // For the negative values, we have to be more careful. Since typically
        // -Value::minLargestInt will cause an overflow, we first divide by 10 and
        // then take the inverse. This assumes that minLargestInt is only a single
        // power of 10 different in magnitude, which we check above. For the last
        // digit, we take the modulus before negating for the same reason.
        static constexpr auto negative_threshold =
            Value::LargestUInt(-(Value::minLargestInt / 10));
        static constexpr auto negative_last_digit =
            Value::UInt(-(Value::minLargestInt % 10));

        const Value::LargestUInt threshold =
            isNegative ? negative_threshold : positive_threshold;
        const Value::UInt max_last_digit =
            isNegative ? negative_last_digit : positive_last_digit;

        Value::LargestUInt value = 0;
        while (current < token.end_) {
            Char c = *current++;
            if (c < '0' || c > '9')
                return decodeDouble(token, decoded);

            const auto digit(static_cast<Value::UInt>(c - '0'));
            if (value >= threshold) {
                // We've hit or exceeded the max value divided by 10 (rounded down). If
                // a) we've only just touched the limit, meaing value == threshold,
                // b) this is the last digit, or
                // c) it's small enough to fit in that rounding delta, we're okay.
                // Otherwise treat this number as a double to avoid overflow.
                if (value > threshold || current != token.end_ ||
                    digit > max_last_digit) {
                    return decodeDouble(token, decoded);
                }
            }
            value = value * 10 + digit;
        }

        if (isNegative) {
            // We use the same magnitude assumption here, just in case.
            const auto last_digit = static_cast<Value::UInt>(value % 10);
            decoded = -Value::LargestInt(value / 10) * 10 - last_digit;
        } else if (value <= Value::LargestUInt(Value::maxLargestInt)) {
            decoded = Value::LargestInt(value);
        } else {
            decoded = value;
        }

        return true;
    }

    bool OurReader::decodeDouble(Token& token) {
        Value decoded;
        if (!decodeDouble(token, decoded))
            return false;
        currentValue().swapPayload(decoded);
        currentValue().setOffsetStart(token.start_ - begin_);
        currentValue().setOffsetLimit(token.end_ - begin_);
        return true;
    }

    bool OurReader::decodeDouble(Token& token, Value& decoded) {
        double value = 0;
        const String buffer(token.start_, token.end_);
        IStringStream is(buffer);
        if (!(is >> value)) {
            return addError(
                "'" + String(token.start_, token.end_) + "' is not a number.", token);
        }
        decoded = value;
        return true;
    }

    bool OurReader::decodeString(Token& token) {
        String decoded_string;
        if (!decodeString(token, decoded_string))
            return false;
        Value decoded(decoded_string);
        currentValue().swapPayload(decoded);
        currentValue().setOffsetStart(token.start_ - begin_);
        currentValue().setOffsetLimit(token.end_ - begin_);
        return true;
    }

    bool OurReader::decodeString(Token& token, String& decoded) {
        decoded.reserve(static_cast<size_t>(token.end_ - token.start_ - 2));
        Location current = token.start_ + 1; // skip '"'
        Location end = token.end_ - 1;       // do not include '"'
        while (current != end) {
            Char c = *current++;
            if (c == '"')
                break;
            if (c == '\\') {
                if (current == end)
                    return addError("Empty escape sequence in string", token, current);
                Char escape = *current++;
                switch (escape) {
                case '"':
                    decoded += '"';
                    break;
                case '/':
                    decoded += '/';
                    break;
                case '\\':
                    decoded += '\\';
                    break;
                case 'b':
                    decoded += '\b';
                    break;
                case 'f':
                    decoded += '\f';
                    break;
                case 'n':
                    decoded += '\n';
                    break;
                case 'r':
                    decoded += '\r';
                    break;
                case 't':
                    decoded += '\t';
                    break;
                case 'u': {
                    unsigned int unicode;
                    if (!decodeUnicodeCodePoint(token, current, end, unicode))
                        return false;
                    decoded += codePointToUTF8(unicode);
                } break;
                default:
                    return addError("Bad escape sequence in string", token, current);
                }
            } else {
                decoded += c;
            }
        }
        return true;
    }

    bool OurReader::decodeUnicodeCodePoint(Token& token, Location& current,
        Location end, unsigned int& unicode) {

        if (!decodeUnicodeEscapeSequence(token, current, end, unicode))
            return false;
        if (unicode >= 0xD800 && unicode <= 0xDBFF) {
            // surrogate pairs
            if (end - current < 6)
                return addError(
                    "additional six characters expected to parse unicode surrogate pair.",
                    token, current);
            if (*(current++) == '\\' && *(current++) == 'u') {
                unsigned int surrogatePair;
                if (decodeUnicodeEscapeSequence(token, current, end, surrogatePair)) {
                    unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (surrogatePair & 0x3FF);
                } else
                    return false;
            } else
                return addError("expecting another \\u token to begin the second half of "
                    "a unicode surrogate pair",
                    token, current);
        }
        return true;
    }

    bool OurReader::decodeUnicodeEscapeSequence(Token& token, Location& current,
        Location end,
        unsigned int& ret_unicode) {
        if (end - current < 4)
            return addError(
                "Bad unicode escape sequence in string: four digits expected.", token,
                current);
        int unicode = 0;
        for (int index = 0; index < 4; ++index) {
            Char c = *current++;
            unicode *= 16;
            if (c >= '0' && c <= '9')
                unicode += c - '0';
            else if (c >= 'a' && c <= 'f')
                unicode += c - 'a' + 10;
            else if (c >= 'A' && c <= 'F')
                unicode += c - 'A' + 10;
            else
                return addError(
                    "Bad unicode escape sequence in string: hexadecimal digit expected.",
                    token, current);
        }
        ret_unicode = static_cast<unsigned int>(unicode);
        return true;
    }

    bool OurReader::addError(const String& message, Token& token, Location extra) {
        ErrorInfo info;
        info.token_ = token;
        info.message_ = message;
        info.extra_ = extra;
        errors_.push_back(info);
        return false;
    }

    bool OurReader::recoverFromError(TokenType skipUntilToken) {
        size_t errorCount = errors_.size();
        Token skip;
        for (;;) {
            if (!readToken(skip))
                errors_.resize(errorCount); // discard errors caused by recovery
            if (skip.type_ == skipUntilToken || skip.type_ == tokenEndOfStream)
                break;
        }
        errors_.resize(errorCount);
        return false;
    }

    bool OurReader::addErrorAndRecover(const String& message, Token& token,
        TokenType skipUntilToken) {
        addError(message, token);
        return recoverFromError(skipUntilToken);
    }

    Value& OurReader::currentValue() { return *(nodes_.top()); }

    OurReader::Char OurReader::getNextChar() {
        if (current_ == end_)
            return 0;
        return *current_++;
    }

    void OurReader::getLocationLineAndColumn(Location location, int& line,
        int& column) const {
        Location current = begin_;
        Location lastLineStart = current;
        line = 0;
        while (current < location && current != end_) {
            Char c = *current++;
            if (c == '\r') {
                if (*current == '\n')
                    ++current;
                lastLineStart = current;
                ++line;
            } else if (c == '\n') {
                lastLineStart = current;
                ++line;
            }
        }
        // column & line start at 1
        column = int(location - lastLineStart) + 1;
        ++line;
    }

    String OurReader::getLocationLineAndColumn(Location location) const {
        int line, column;
        getLocationLineAndColumn(location, line, column);
        char buffer[18 + 16 + 16 + 1];
        jsoncpp_snprintf(buffer, sizeof(buffer), "Line %d, Column %d", line, column);
        return buffer;
    }

    String OurReader::getFormattedErrorMessages() const {
        String formattedMessage;
        for (const auto& error : errors_) {
            formattedMessage +=
                "* " + getLocationLineAndColumn(error.token_.start_) + "\n";
            formattedMessage += "  " + error.message_ + "\n";
            if (error.extra_)
                formattedMessage +=
                "See " + getLocationLineAndColumn(error.extra_) + " for detail.\n";
        }
        return formattedMessage;
    }

    std::vector<OurReader::StructuredError> OurReader::getStructuredErrors() const {
        std::vector<OurReader::StructuredError> allErrors;
        for (const auto& error : errors_) {
            OurReader::StructuredError structured;
            structured.offset_start = error.token_.start_ - begin_;
            structured.offset_limit = error.token_.end_ - begin_;
            structured.message = error.message_;
            allErrors.push_back(structured);
        }
        return allErrors;
    }

    class OurCharReader : public CharReader {
        bool const collectComments_;
        OurReader reader_;

    public:
        OurCharReader(bool collectComments, OurFeatures const& features)
            : collectComments_(collectComments), reader_(features) {}
        bool parse(char const* beginDoc, char const* endDoc, Value* root,
            String* errs) override {
            bool ok = reader_.parse(beginDoc, endDoc, *root, collectComments_);
            if (errs) {
                *errs = reader_.getFormattedErrorMessages();
            }
            return ok;
        }
    };

    CharReaderBuilder::CharReaderBuilder() { setDefaults(&settings_); }
    CharReaderBuilder::~CharReaderBuilder() = default;
    CharReader* CharReaderBuilder::newCharReader() const {
        bool collectComments = settings_["collectComments"].asBool();
        OurFeatures features = OurFeatures::all();
        features.allowComments_ = settings_["allowComments"].asBool();
        features.allowTrailingCommas_ = settings_["allowTrailingCommas"].asBool();
        features.strictRoot_ = settings_["strictRoot"].asBool();
        features.allowDroppedNullPlaceholders_ =
            settings_["allowDroppedNullPlaceholders"].asBool();
        features.allowNumericKeys_ = settings_["allowNumericKeys"].asBool();
        features.allowSingleQuotes_ = settings_["allowSingleQuotes"].asBool();

        // Stack limit is always a size_t, so we get this as an unsigned int
        // regardless of it we have 64-bit integer support enabled.
        features.stackLimit_ = static_cast<size_t>(settings_["stackLimit"].asUInt());
        features.failIfExtra_ = settings_["failIfExtra"].asBool();
        features.rejectDupKeys_ = settings_["rejectDupKeys"].asBool();
        features.allowSpecialFloats_ = settings_["allowSpecialFloats"].asBool();
        return new OurCharReader(collectComments, features);
    }
    static void getValidReaderKeys(std::set<String>* valid_keys) {
        valid_keys->clear();
        valid_keys->insert("collectComments");
        valid_keys->insert("allowComments");
        valid_keys->insert("allowTrailingCommas");
        valid_keys->insert("strictRoot");
        valid_keys->insert("allowDroppedNullPlaceholders");
        valid_keys->insert("allowNumericKeys");
        valid_keys->insert("allowSingleQuotes");
        valid_keys->insert("stackLimit");
        valid_keys->insert("failIfExtra");
        valid_keys->insert("rejectDupKeys");
        valid_keys->insert("allowSpecialFloats");
    }
    bool CharReaderBuilder::validate(Json::Value* invalid) const {
        Json::Value my_invalid;
        if (!invalid)
            invalid = &my_invalid; // so we do not need to test for NULL
        Json::Value& inv = *invalid;
        std::set<String> valid_keys;
        getValidReaderKeys(&valid_keys);
        Value::Members keys = settings_.getMemberNames();
        size_t n = keys.size();
        for (size_t i = 0; i < n; ++i) {
            String const& key = keys[i];
            if (valid_keys.find(key) == valid_keys.end()) {
                inv[key] = settings_[key];
            }
        }
        return inv.empty();
    }
    Value& CharReaderBuilder::operator[](const String& key) {
        return settings_[key];
    }
    // static
    void CharReaderBuilder::strictMode(Json::Value* settings) {
        //! [CharReaderBuilderStrictMode]
        (*settings)["allowComments"] = false;
        (*settings)["allowTrailingCommas"] = false;
        (*settings)["strictRoot"] = true;
        (*settings)["allowDroppedNullPlaceholders"] = false;
        (*settings)["allowNumericKeys"] = false;
        (*settings)["allowSingleQuotes"] = false;
        (*settings)["stackLimit"] = 1000;
        (*settings)["failIfExtra"] = true;
        (*settings)["rejectDupKeys"] = true;
        (*settings)["allowSpecialFloats"] = false;
        //! [CharReaderBuilderStrictMode]
    }
    // static
    void CharReaderBuilder::setDefaults(Json::Value* settings) {
        //! [CharReaderBuilderDefaults]
        (*settings)["collectComments"] = true;
        (*settings)["allowComments"] = true;
        (*settings)["allowTrailingCommas"] = true;
        (*settings)["strictRoot"] = false;
        (*settings)["allowDroppedNullPlaceholders"] = false;
        (*settings)["allowNumericKeys"] = false;
        (*settings)["allowSingleQuotes"] = false;
        (*settings)["stackLimit"] = 1000;
        (*settings)["failIfExtra"] = false;
        (*settings)["rejectDupKeys"] = false;
        (*settings)["allowSpecialFloats"] = false;
        //! [CharReaderBuilderDefaults]
    }

    //////////////////////////////////
    // global functions

    bool parseFromStream(CharReader::Factory const& fact, IStream& sin, Value* root,
        String* errs) {
        OStringStream ssin;
        ssin << sin.rdbuf();
        String doc = ssin.str();
        char const* begin = doc.data();
        char const* end = begin + doc.size();
        // Note that we do not actually need a null-terminator.
        CharReaderPtr const reader(fact.newCharReader());
        return reader->parse(begin, end, root, errs);
    }

    IStream& operator>>(IStream& sin, Value& root) {
        CharReaderBuilder b;
        String errs;
        bool ok = parseFromStream(b, sin, &root, &errs);
        if (!ok) {
            throwRuntimeError(errs);
        }
        return sin;
    }

} // namespace Json

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_reader.cpp
// //////////////////////////////////////////////////////////////////////






// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_valueiterator.inl
// //////////////////////////////////////////////////////////////////////

// Copyright 2007-2010 Baptiste Lepilleur and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

// included by json_value.cpp

namespace Json {

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class ValueIteratorBase
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    ValueIteratorBase::ValueIteratorBase() : current_() {}

    ValueIteratorBase::ValueIteratorBase(
        const Value::ObjectValues::iterator& current)
        : current_(current), isNull_(false) {}

    Value& ValueIteratorBase::deref() { return current_->second; }
    const Value& ValueIteratorBase::deref() const { return current_->second; }

    void ValueIteratorBase::increment() { ++current_; }

    void ValueIteratorBase::decrement() { --current_; }

    ValueIteratorBase::difference_type
        ValueIteratorBase::computeDistance(const SelfType& other) const {
        // Iterator for null value are initialized using the default
        // constructor, which initialize current_ to the default
        // std::map::iterator. As begin() and end() are two instance
        // of the default std::map::iterator, they can not be compared.
        // To allow this, we handle this comparison specifically.
        if (isNull_ && other.isNull_) {
            return 0;
        }

        // Usage of std::distance is not portable (does not compile with Sun Studio 12
        // RogueWave STL,
        // which is the one used by default).
        // Using a portable hand-made version for non random iterator instead:
        //   return difference_type( std::distance( current_, other.current_ ) );
        difference_type myDistance = 0;
        for (Value::ObjectValues::iterator it = current_; it != other.current_;
            ++it) {
            ++myDistance;
        }
        return myDistance;
    }

    bool ValueIteratorBase::isEqual(const SelfType& other) const {
        if (isNull_) {
            return other.isNull_;
        }
        return current_ == other.current_;
    }

    void ValueIteratorBase::copy(const SelfType& other) {
        current_ = other.current_;
        isNull_ = other.isNull_;
    }

    Value ValueIteratorBase::key() const {
        const Value::CZString czstring = (*current_).first;
        if (czstring.data()) {
            if (czstring.isStaticString())
                return Value(StaticString(czstring.data()));
            return Value(czstring.data(), czstring.data() + czstring.length());
        }
        return Value(czstring.index());
    }

    UInt ValueIteratorBase::index() const {
        const Value::CZString czstring = (*current_).first;
        if (!czstring.data())
            return czstring.index();
        return Value::UInt(-1);
    }

    String ValueIteratorBase::name() const {
        char const* keey;
        char const* end;
        keey = memberName(&end);
        if (!keey)
            return String();
        return String(keey, end);
    }

    char const* ValueIteratorBase::memberName() const {
        const char* cname = (*current_).first.data();
        return cname ? cname : "";
    }

    char const* ValueIteratorBase::memberName(char const** end) const {
        const char* cname = (*current_).first.data();
        if (!cname) {
            *end = nullptr;
            return nullptr;
        }
        *end = cname + (*current_).first.length();
        return cname;
    }

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class ValueConstIterator
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    ValueConstIterator::ValueConstIterator() = default;

    ValueConstIterator::ValueConstIterator(
        const Value::ObjectValues::iterator& current)
        : ValueIteratorBase(current) {}

    ValueConstIterator::ValueConstIterator(ValueIterator const& other)
        : ValueIteratorBase(other) {}

    ValueConstIterator& ValueConstIterator::
        operator=(const ValueIteratorBase& other) {
        copy(other);
        return *this;
    }

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class ValueIterator
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    ValueIterator::ValueIterator() = default;

    ValueIterator::ValueIterator(const Value::ObjectValues::iterator& current)
        : ValueIteratorBase(current) {}

    ValueIterator::ValueIterator(const ValueConstIterator& other)
        : ValueIteratorBase(other) {
        throwRuntimeError("ConstIterator to Iterator should never be allowed.");
    }

    ValueIterator::ValueIterator(const ValueIterator& other) = default;

    ValueIterator& ValueIterator::operator=(const SelfType& other) {
        copy(other);
        return *this;
    }

} // namespace Json

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_valueiterator.inl
// //////////////////////////////////////////////////////////////////////






// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_value.cpp
// //////////////////////////////////////////////////////////////////////

// Copyright 2011 Baptiste Lepilleur and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include <json/assertions.h>
#include <json/value.h>
#include <json/writer.h>
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <sstream>
#include <utility>

// Provide implementation equivalent of std::snprintf for older _MSC compilers
#if defined(_MSC_VER) && _MSC_VER < 1900
#include <stdarg.h>
static int msvc_pre1900_c99_vsnprintf(char* outBuf, size_t size,
    const char* format, va_list ap) {
    int count = -1;
    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);
    return count;
}

int JSON_API msvc_pre1900_c99_snprintf(char* outBuf, size_t size,
    const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    const int count = msvc_pre1900_c99_vsnprintf(outBuf, size, format, ap);
    va_end(ap);
    return count;
}
#endif

// Disable warning C4702 : unreachable code
#if defined(_MSC_VER)
#pragma warning(disable : 4702)
#endif

#define JSON_ASSERT_UNREACHABLE assert(false)

namespace Json {
    template <typename T>
    static std::unique_ptr<T> cloneUnique(const std::unique_ptr<T>& p) {
        std::unique_ptr<T> r;
        if (p) {
            r = std::unique_ptr<T>(new T(*p));
        }
        return r;
    }

    // This is a walkaround to avoid the static initialization of Value::null.
    // kNull must be word-aligned to avoid crashing on ARM.  We use an alignment of
    // 8 (instead of 4) as a bit of future-proofing.
#if defined(__ARMEL__)
#define ALIGNAS(byte_alignment) __attribute__((aligned(byte_alignment)))
#else
#define ALIGNAS(byte_alignment)
#endif

// static
    Value const& Value::nullSingleton() {
        static Value const nullStatic;
        return nullStatic;
    }

#if JSON_USE_NULLREF
    // for backwards compatibility, we'll leave these global references around, but
    // DO NOT use them in JSONCPP library code any more!
    // static
    Value const& Value::null = Value::nullSingleton();

    // static
    Value const& Value::nullRef = Value::nullSingleton();
#endif

#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
    template <typename T, typename U>
    static inline bool InRange(double d, T min, U max) {
        // The casts can lose precision, but we are looking only for
        // an approximate range. Might fail on edge cases though. ~cdunn
        return d >= static_cast<double>(min) && d <= static_cast<double>(max);
    }
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
    static inline double integerToDouble(Json::UInt64 value) {
        return static_cast<double>(Int64(value / 2)) * 2.0 +
            static_cast<double>(Int64(value & 1));
    }

    template <typename T> static inline double integerToDouble(T value) {
        return static_cast<double>(value);
    }

    template <typename T, typename U>
    static inline bool InRange(double d, T min, U max) {
        return d >= integerToDouble(min) && d <= integerToDouble(max);
    }
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)

    /** Duplicates the specified string value.
     * @param value Pointer to the string to duplicate. Must be zero-terminated if
     *              length is "unknown".
     * @param length Length of the value. if equals to unknown, then it will be
     *               computed using strlen(value).
     * @return Pointer on the duplicate instance of string.
     */
    static inline char* duplicateStringValue(const char* value, size_t length) {
        // Avoid an integer overflow in the call to malloc below by limiting length
        // to a sane value.
        if (length >= static_cast<size_t>(Value::maxInt))
            length = Value::maxInt - 1;

        auto newString = static_cast<char*>(malloc(length + 1));
        if (newString == nullptr) {
            throwRuntimeError("in Json::Value::duplicateStringValue(): "
                "Failed to allocate string value buffer");
        }
        memcpy(newString, value, length);
        newString[length] = 0;
        return newString;
    }

    /* Record the length as a prefix.
     */
    static inline char* duplicateAndPrefixStringValue(const char* value,
        unsigned int length) {
        // Avoid an integer overflow in the call to malloc below by limiting length
        // to a sane value.
        JSON_ASSERT_MESSAGE(length <= static_cast<unsigned>(Value::maxInt) -
            sizeof(unsigned) - 1U,
            "in Json::Value::duplicateAndPrefixStringValue(): "
            "length too big for prefixing");
        size_t actualLength = sizeof(length) + length + 1;
        auto newString = static_cast<char*>(malloc(actualLength));
        if (newString == nullptr) {
            throwRuntimeError("in Json::Value::duplicateAndPrefixStringValue(): "
                "Failed to allocate string value buffer");
        }
        *reinterpret_cast<unsigned*>(newString) = length;
        memcpy(newString + sizeof(unsigned), value, length);
        newString[actualLength - 1U] =
            0; // to avoid buffer over-run accidents by users later
        return newString;
    }
    inline static void decodePrefixedString(bool isPrefixed, char const* prefixed,
        unsigned* length, char const** value) {
        if (!isPrefixed) {
            *length = static_cast<unsigned>(strlen(prefixed));
            *value = prefixed;
        } else {
            *length = *reinterpret_cast<unsigned const*>(prefixed);
            *value = prefixed + sizeof(unsigned);
        }
    }
    /** Free the string duplicated by
     * duplicateStringValue()/duplicateAndPrefixStringValue().
     */
#if JSONCPP_USING_SECURE_MEMORY
    static inline void releasePrefixedStringValue(char* value) {
        unsigned length = 0;
        char const* valueDecoded;
        decodePrefixedString(true, value, &length, &valueDecoded);
        size_t const size = sizeof(unsigned) + length + 1U;
        memset(value, 0, size);
        free(value);
    }
    static inline void releaseStringValue(char* value, unsigned length) {
        // length==0 => we allocated the strings memory
        size_t size = (length == 0) ? strlen(value) : length;
        memset(value, 0, size);
        free(value);
    }
#else  // !JSONCPP_USING_SECURE_MEMORY
    static inline void releasePrefixedStringValue(char* value) { free(value); }
    static inline void releaseStringValue(char* value, unsigned) { free(value); }
#endif // JSONCPP_USING_SECURE_MEMORY

} // namespace Json

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// ValueInternals...
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
#if !defined(JSON_IS_AMALGAMATION)

#include "json_valueiterator.inl"
#endif // if !defined(JSON_IS_AMALGAMATION)

namespace Json {

#if JSON_USE_EXCEPTION
    Exception::Exception(String msg) : msg_(std::move(msg)) {}
    Exception::~Exception() JSONCPP_NOEXCEPT = default;
    char const* Exception::what() const JSONCPP_NOEXCEPT { return msg_.c_str(); }
    RuntimeError::RuntimeError(String const& msg) : Exception(msg) {}
    LogicError::LogicError(String const& msg) : Exception(msg) {}
    JSONCPP_NORETURN void throwRuntimeError(String const& msg) {
        throw RuntimeError(msg);
    }
    JSONCPP_NORETURN void throwLogicError(String const& msg) {
        throw LogicError(msg);
    }
#else // !JSON_USE_EXCEPTION
    JSONCPP_NORETURN void throwRuntimeError(String const& msg) { abort(); }
    JSONCPP_NORETURN void throwLogicError(String const& msg) { abort(); }
#endif

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class Value::CZString
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    // Notes: policy_ indicates if the string was allocated when
    // a string is stored.

    Value::CZString::CZString(ArrayIndex index) : cstr_(nullptr), index_(index) {}

    Value::CZString::CZString(char const* str, unsigned length,
        DuplicationPolicy allocate)
        : cstr_(str) {
        // allocate != duplicate
        storage_.policy_ = allocate & 0x3;
        storage_.length_ = length & 0x3FFFFFFF;
    }

    Value::CZString::CZString(const CZString& other) {
        cstr_ = (other.storage_.policy_ != noDuplication && other.cstr_ != nullptr
            ? duplicateStringValue(other.cstr_, other.storage_.length_)
            : other.cstr_);
        storage_.policy_ =
            static_cast<unsigned>(
                other.cstr_
                ? (static_cast<DuplicationPolicy>(other.storage_.policy_) ==
                    noDuplication
                    ? noDuplication
                    : duplicate)
                : static_cast<DuplicationPolicy>(other.storage_.policy_)) &
            3U;
        storage_.length_ = other.storage_.length_;
    }

    Value::CZString::CZString(CZString&& other)
        : cstr_(other.cstr_), index_(other.index_) {
        other.cstr_ = nullptr;
    }

    Value::CZString::~CZString() {
        if (cstr_ && storage_.policy_ == duplicate) {
            releaseStringValue(const_cast<char*>(cstr_),
                storage_.length_ + 1U); // +1 for null terminating
                                        // character for sake of
                                        // completeness but not actually
                                        // necessary
        }
    }

    void Value::CZString::swap(CZString& other) {
        std::swap(cstr_, other.cstr_);
        std::swap(index_, other.index_);
    }

    Value::CZString& Value::CZString::operator=(const CZString& other) {
        cstr_ = other.cstr_;
        index_ = other.index_;
        return *this;
    }

    Value::CZString& Value::CZString::operator=(CZString&& other) {
        cstr_ = other.cstr_;
        index_ = other.index_;
        other.cstr_ = nullptr;
        return *this;
    }

    bool Value::CZString::operator<(const CZString& other) const {
        if (!cstr_)
            return index_ < other.index_;
        // return strcmp(cstr_, other.cstr_) < 0;
        // Assume both are strings.
        unsigned this_len = this->storage_.length_;
        unsigned other_len = other.storage_.length_;
        unsigned min_len = std::min<unsigned>(this_len, other_len);
        JSON_ASSERT(this->cstr_ && other.cstr_);
        int comp = memcmp(this->cstr_, other.cstr_, min_len);
        if (comp < 0)
            return true;
        if (comp > 0)
            return false;
        return (this_len < other_len);
    }

    bool Value::CZString::operator==(const CZString& other) const {
        if (!cstr_)
            return index_ == other.index_;
        // return strcmp(cstr_, other.cstr_) == 0;
        // Assume both are strings.
        unsigned this_len = this->storage_.length_;
        unsigned other_len = other.storage_.length_;
        if (this_len != other_len)
            return false;
        JSON_ASSERT(this->cstr_ && other.cstr_);
        int comp = memcmp(this->cstr_, other.cstr_, this_len);
        return comp == 0;
    }

    ArrayIndex Value::CZString::index() const { return index_; }

    // const char* Value::CZString::c_str() const { return cstr_; }
    const char* Value::CZString::data() const { return cstr_; }
    unsigned Value::CZString::length() const { return storage_.length_; }
    bool Value::CZString::isStaticString() const {
        return storage_.policy_ == noDuplication;
    }

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class Value::Value
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    /*! \internal Default constructor initialization must be equivalent to:
     * memset( this, 0, sizeof(Value) )
     * This optimization is used in ValueInternalMap fast allocator.
     */
    Value::Value(ValueType type) {
        static char const emptyString[] = "";
        initBasic(type);
        switch (type) {
        case nullValue:
            break;
        case intValue:
        case uintValue:
            value_.int_ = 0;
            break;
        case realValue:
            value_.real_ = 0.0;
            break;
        case stringValue:
            // allocated_ == false, so this is safe.
            value_.string_ = const_cast<char*>(static_cast<char const*>(emptyString));
            break;
        case arrayValue:
        case objectValue:
            value_.map_ = new ObjectValues();
            break;
        case booleanValue:
            value_.bool_ = false;
            break;
        default:
            JSON_ASSERT_UNREACHABLE;
        }
    }

    Value::Value(Int value) {
        initBasic(intValue);
        value_.int_ = value;
    }

    Value::Value(UInt value) {
        initBasic(uintValue);
        value_.uint_ = value;
    }
#if defined(JSON_HAS_INT64)
    Value::Value(Int64 value) {
        initBasic(intValue);
        value_.int_ = value;
    }
    Value::Value(UInt64 value) {
        initBasic(uintValue);
        value_.uint_ = value;
    }
#endif // defined(JSON_HAS_INT64)

    Value::Value(double value) {
        initBasic(realValue);
        value_.real_ = value;
    }

    Value::Value(const char* value) {
        initBasic(stringValue, true);
        JSON_ASSERT_MESSAGE(value != nullptr,
            "Null Value Passed to Value Constructor");
        value_.string_ = duplicateAndPrefixStringValue(
            value, static_cast<unsigned>(strlen(value)));
    }

    Value::Value(const char* begin, const char* end) {
        initBasic(stringValue, true);
        value_.string_ =
            duplicateAndPrefixStringValue(begin, static_cast<unsigned>(end - begin));
    }

    Value::Value(const String& value) {
        initBasic(stringValue, true);
        value_.string_ = duplicateAndPrefixStringValue(
            value.data(), static_cast<unsigned>(value.length()));
    }

    Value::Value(const StaticString& value) {
        initBasic(stringValue);
        value_.string_ = const_cast<char*>(value.c_str());
    }

    Value::Value(bool value) {
        initBasic(booleanValue);
        value_.bool_ = value;
    }

    Value::Value(const Value& other) {
        dupPayload(other);
        dupMeta(other);
    }

    Value::Value(Value&& other) {
        initBasic(nullValue);
        swap(other);
    }

    Value::~Value() {
        releasePayload();
        value_.uint_ = 0;
    }

    Value& Value::operator=(const Value& other) {
        Value(other).swap(*this);
        return *this;
    }

    Value& Value::operator=(Value&& other) {
        other.swap(*this);
        return *this;
    }

    void Value::swapPayload(Value& other) {
        std::swap(bits_, other.bits_);
        std::swap(value_, other.value_);
    }

    void Value::copyPayload(const Value& other) {
        releasePayload();
        dupPayload(other);
    }

    void Value::swap(Value& other) {
        swapPayload(other);
        std::swap(comments_, other.comments_);
        std::swap(start_, other.start_);
        std::swap(limit_, other.limit_);
    }

    void Value::copy(const Value& other) {
        copyPayload(other);
        dupMeta(other);
    }

    ValueType Value::type() const {
        return static_cast<ValueType>(bits_.value_type_);
    }

    int Value::compare(const Value& other) const {
        if (*this < other)
            return -1;
        if (*this > other)
            return 1;
        return 0;
    }

    bool Value::operator<(const Value& other) const {
        int typeDelta = type() - other.type();
        if (typeDelta)
            return typeDelta < 0;
        switch (type()) {
        case nullValue:
            return false;
        case intValue:
            return value_.int_ < other.value_.int_;
        case uintValue:
            return value_.uint_ < other.value_.uint_;
        case realValue:
            return value_.real_ < other.value_.real_;
        case booleanValue:
            return value_.bool_ < other.value_.bool_;
        case stringValue: {
            if ((value_.string_ == nullptr) || (other.value_.string_ == nullptr)) {
                return other.value_.string_ != nullptr;
            }
            unsigned this_len;
            unsigned other_len;
            char const* this_str;
            char const* other_str;
            decodePrefixedString(this->isAllocated(), this->value_.string_, &this_len,
                &this_str);
            decodePrefixedString(other.isAllocated(), other.value_.string_, &other_len,
                &other_str);
            unsigned min_len = std::min<unsigned>(this_len, other_len);
            JSON_ASSERT(this_str && other_str);
            int comp = memcmp(this_str, other_str, min_len);
            if (comp < 0)
                return true;
            if (comp > 0)
                return false;
            return (this_len < other_len);
        }
        case arrayValue:
        case objectValue: {
            auto thisSize = value_.map_->size();
            auto otherSize = other.value_.map_->size();
            if (thisSize != otherSize)
                return thisSize < otherSize;
            return (*value_.map_) < (*other.value_.map_);
        }
        default:
            JSON_ASSERT_UNREACHABLE;
        }
        return false; // unreachable
    }

    bool Value::operator<=(const Value& other) const { return !(other < *this); }

    bool Value::operator>=(const Value& other) const { return !(*this < other); }

    bool Value::operator>(const Value& other) const { return other < *this; }

    bool Value::operator==(const Value& other) const {
        if (type() != other.type())
            return false;
        switch (type()) {
        case nullValue:
            return true;
        case intValue:
            return value_.int_ == other.value_.int_;
        case uintValue:
            return value_.uint_ == other.value_.uint_;
        case realValue:
            return value_.real_ == other.value_.real_;
        case booleanValue:
            return value_.bool_ == other.value_.bool_;
        case stringValue: {
            if ((value_.string_ == nullptr) || (other.value_.string_ == nullptr)) {
                return (value_.string_ == other.value_.string_);
            }
            unsigned this_len;
            unsigned other_len;
            char const* this_str;
            char const* other_str;
            decodePrefixedString(this->isAllocated(), this->value_.string_, &this_len,
                &this_str);
            decodePrefixedString(other.isAllocated(), other.value_.string_, &other_len,
                &other_str);
            if (this_len != other_len)
                return false;
            JSON_ASSERT(this_str && other_str);
            int comp = memcmp(this_str, other_str, this_len);
            return comp == 0;
        }
        case arrayValue:
        case objectValue:
            return value_.map_->size() == other.value_.map_->size() &&
                (*value_.map_) == (*other.value_.map_);
        default:
            JSON_ASSERT_UNREACHABLE;
        }
        return false; // unreachable
    }

    bool Value::operator!=(const Value& other) const { return !(*this == other); }

    const char* Value::asCString() const {
        JSON_ASSERT_MESSAGE(type() == stringValue,
            "in Json::Value::asCString(): requires stringValue");
        if (value_.string_ == nullptr)
            return nullptr;
        unsigned this_len;
        char const* this_str;
        decodePrefixedString(this->isAllocated(), this->value_.string_, &this_len,
            &this_str);
        return this_str;
    }

#if JSONCPP_USING_SECURE_MEMORY
    unsigned Value::getCStringLength() const {
        JSON_ASSERT_MESSAGE(type() == stringValue,
            "in Json::Value::asCString(): requires stringValue");
        if (value_.string_ == 0)
            return 0;
        unsigned this_len;
        char const* this_str;
        decodePrefixedString(this->isAllocated(), this->value_.string_, &this_len,
            &this_str);
        return this_len;
    }
#endif

    bool Value::getString(char const** begin, char const** end) const {
        if (type() != stringValue)
            return false;
        if (value_.string_ == nullptr)
            return false;
        unsigned length;
        decodePrefixedString(this->isAllocated(), this->value_.string_, &length,
            begin);
        *end = *begin + length;
        return true;
    }

    String Value::asString() const {
        switch (type()) {
        case nullValue:
            return "";
        case stringValue: {
            if (value_.string_ == nullptr)
                return "";
            unsigned this_len;
            char const* this_str;
            decodePrefixedString(this->isAllocated(), this->value_.string_, &this_len,
                &this_str);
            return String(this_str, this_len);
        }
        case booleanValue:
            return value_.bool_ ? "true" : "false";
        case intValue:
            return valueToString(value_.int_);
        case uintValue:
            return valueToString(value_.uint_);
        case realValue:
            return valueToString(value_.real_);
        default:
            JSON_FAIL_MESSAGE("Type is not convertible to string");
        }
    }

    Value::Int Value::asInt() const {
        switch (type()) {
        case intValue:
            JSON_ASSERT_MESSAGE(isInt(), "LargestInt out of Int range");
            return Int(value_.int_);
        case uintValue:
            JSON_ASSERT_MESSAGE(isInt(), "LargestUInt out of Int range");
            return Int(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt, maxInt),
                "double out of Int range");
            return Int(value_.real_);
        case nullValue:
            return 0;
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        default:
            break;
        }
        JSON_FAIL_MESSAGE("Value is not convertible to Int.");
    }

    Value::UInt Value::asUInt() const {
        switch (type()) {
        case intValue:
            JSON_ASSERT_MESSAGE(isUInt(), "LargestInt out of UInt range");
            return UInt(value_.int_);
        case uintValue:
            JSON_ASSERT_MESSAGE(isUInt(), "LargestUInt out of UInt range");
            return UInt(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt),
                "double out of UInt range");
            return UInt(value_.real_);
        case nullValue:
            return 0;
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        default:
            break;
        }
        JSON_FAIL_MESSAGE("Value is not convertible to UInt.");
    }

#if defined(JSON_HAS_INT64)

    Value::Int64 Value::asInt64() const {
        switch (type()) {
        case intValue:
            return Int64(value_.int_);
        case uintValue:
            JSON_ASSERT_MESSAGE(isInt64(), "LargestUInt out of Int64 range");
            return Int64(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, minInt64, maxInt64),
                "double out of Int64 range");
            return Int64(value_.real_);
        case nullValue:
            return 0;
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        default:
            break;
        }
        JSON_FAIL_MESSAGE("Value is not convertible to Int64.");
    }

    Value::UInt64 Value::asUInt64() const {
        switch (type()) {
        case intValue:
            JSON_ASSERT_MESSAGE(isUInt64(), "LargestInt out of UInt64 range");
            return UInt64(value_.int_);
        case uintValue:
            return UInt64(value_.uint_);
        case realValue:
            JSON_ASSERT_MESSAGE(InRange(value_.real_, 0, maxUInt64),
                "double out of UInt64 range");
            return UInt64(value_.real_);
        case nullValue:
            return 0;
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        default:
            break;
        }
        JSON_FAIL_MESSAGE("Value is not convertible to UInt64.");
    }
#endif // if defined(JSON_HAS_INT64)

    LargestInt Value::asLargestInt() const {
#if defined(JSON_NO_INT64)
        return asInt();
#else
        return asInt64();
#endif
    }

    LargestUInt Value::asLargestUInt() const {
#if defined(JSON_NO_INT64)
        return asUInt();
#else
        return asUInt64();
#endif
    }

    double Value::asDouble() const {
        switch (type()) {
        case intValue:
            return static_cast<double>(value_.int_);
        case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
            return static_cast<double>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
            return integerToDouble(value_.uint_);
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
        case realValue:
            return value_.real_;
        case nullValue:
            return 0.0;
        case booleanValue:
            return value_.bool_ ? 1.0 : 0.0;
        default:
            break;
        }
        JSON_FAIL_MESSAGE("Value is not convertible to double.");
    }

    float Value::asFloat() const {
        switch (type()) {
        case intValue:
            return static_cast<float>(value_.int_);
        case uintValue:
#if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
            return static_cast<float>(value_.uint_);
#else  // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
            // This can fail (silently?) if the value is bigger than MAX_FLOAT.
            return static_cast<float>(integerToDouble(value_.uint_));
#endif // if !defined(JSON_USE_INT64_DOUBLE_CONVERSION)
        case realValue:
            return static_cast<float>(value_.real_);
        case nullValue:
            return 0.0;
        case booleanValue:
            return value_.bool_ ? 1.0F : 0.0F;
        default:
            break;
        }
        JSON_FAIL_MESSAGE("Value is not convertible to float.");
    }

    bool Value::asBool() const {
        switch (type()) {
        case booleanValue:
            return value_.bool_;
        case nullValue:
            return false;
        case intValue:
            return value_.int_ != 0;
        case uintValue:
            return value_.uint_ != 0;
        case realValue: {
            // According to JavaScript language zero or NaN is regarded as false
            const auto value_classification = std::fpclassify(value_.real_);
            return value_classification != FP_ZERO && value_classification != FP_NAN;
        }
        default:
            break;
        }
        JSON_FAIL_MESSAGE("Value is not convertible to bool.");
    }

    bool Value::isConvertibleTo(ValueType other) const {
        switch (other) {
        case nullValue:
            return (isNumeric() && asDouble() == 0.0) ||
                (type() == booleanValue && !value_.bool_) ||
                (type() == stringValue && asString().empty()) ||
                (type() == arrayValue && value_.map_->empty()) ||
                (type() == objectValue && value_.map_->empty()) ||
                type() == nullValue;
        case intValue:
            return isInt() ||
                (type() == realValue && InRange(value_.real_, minInt, maxInt)) ||
                type() == booleanValue || type() == nullValue;
        case uintValue:
            return isUInt() ||
                (type() == realValue && InRange(value_.real_, 0, maxUInt)) ||
                type() == booleanValue || type() == nullValue;
        case realValue:
            return isNumeric() || type() == booleanValue || type() == nullValue;
        case booleanValue:
            return isNumeric() || type() == booleanValue || type() == nullValue;
        case stringValue:
            return isNumeric() || type() == booleanValue || type() == stringValue ||
                type() == nullValue;
        case arrayValue:
            return type() == arrayValue || type() == nullValue;
        case objectValue:
            return type() == objectValue || type() == nullValue;
        }
        JSON_ASSERT_UNREACHABLE;
        return false;
    }

    /// Number of values in array or object
    ArrayIndex Value::size() const {
        switch (type()) {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
        case stringValue:
            return 0;
        case arrayValue: // size of the array is highest index + 1
            if (!value_.map_->empty()) {
                ObjectValues::const_iterator itLast = value_.map_->end();
                --itLast;
                return (*itLast).first.index() + 1;
            }
            return 0;
        case objectValue:
            return ArrayIndex(value_.map_->size());
        }
        JSON_ASSERT_UNREACHABLE;
        return 0; // unreachable;
    }

    bool Value::empty() const {
        if (isNull() || isArray() || isObject())
            return size() == 0U;
        return false;
    }

    Value::operator bool() const { return !isNull(); }

    void Value::clear() {
        JSON_ASSERT_MESSAGE(type() == nullValue || type() == arrayValue ||
            type() == objectValue,
            "in Json::Value::clear(): requires complex value");
        start_ = 0;
        limit_ = 0;
        switch (type()) {
        case arrayValue:
        case objectValue:
            value_.map_->clear();
            break;
        default:
            break;
        }
    }

    void Value::resize(ArrayIndex newSize) {
        JSON_ASSERT_MESSAGE(type() == nullValue || type() == arrayValue,
            "in Json::Value::resize(): requires arrayValue");
        if (type() == nullValue)
            *this = Value(arrayValue);
        ArrayIndex oldSize = size();
        if (newSize == 0)
            clear();
        else if (newSize > oldSize)
            this->operator[](newSize - 1);
        else {
            for (ArrayIndex index = newSize; index < oldSize; ++index) {
                value_.map_->erase(index);
            }
            JSON_ASSERT(size() == newSize);
        }
    }

    Value& Value::operator[](ArrayIndex index) {
        JSON_ASSERT_MESSAGE(
            type() == nullValue || type() == arrayValue,
            "in Json::Value::operator[](ArrayIndex): requires arrayValue");
        if (type() == nullValue)
            *this = Value(arrayValue);
        CZString key(index);
        auto it = value_.map_->lower_bound(key);
        if (it != value_.map_->end() && (*it).first == key)
            return (*it).second;

        ObjectValues::value_type defaultValue(key, nullSingleton());
        it = value_.map_->insert(it, defaultValue);
        return (*it).second;
    }

    Value& Value::operator[](int index) {
        JSON_ASSERT_MESSAGE(
            index >= 0,
            "in Json::Value::operator[](int index): index cannot be negative");
        return (*this)[ArrayIndex(index)];
    }

    const Value& Value::operator[](ArrayIndex index) const {
        JSON_ASSERT_MESSAGE(
            type() == nullValue || type() == arrayValue,
            "in Json::Value::operator[](ArrayIndex)const: requires arrayValue");
        if (type() == nullValue)
            return nullSingleton();
        CZString key(index);
        ObjectValues::const_iterator it = value_.map_->find(key);
        if (it == value_.map_->end())
            return nullSingleton();
        return (*it).second;
    }

    const Value& Value::operator[](int index) const {
        JSON_ASSERT_MESSAGE(
            index >= 0,
            "in Json::Value::operator[](int index) const: index cannot be negative");
        return (*this)[ArrayIndex(index)];
    }

    void Value::initBasic(ValueType type, bool allocated) {
        setType(type);
        setIsAllocated(allocated);
        comments_ = Comments{};
        start_ = 0;
        limit_ = 0;
    }

    void Value::dupPayload(const Value& other) {
        setType(other.type());
        setIsAllocated(false);
        switch (type()) {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
            value_ = other.value_;
            break;
        case stringValue:
            if (other.value_.string_ && other.isAllocated()) {
                unsigned len;
                char const* str;
                decodePrefixedString(other.isAllocated(), other.value_.string_, &len,
                    &str);
                value_.string_ = duplicateAndPrefixStringValue(str, len);
                setIsAllocated(true);
            } else {
                value_.string_ = other.value_.string_;
            }
            break;
        case arrayValue:
        case objectValue:
            value_.map_ = new ObjectValues(*other.value_.map_);
            break;
        default:
            JSON_ASSERT_UNREACHABLE;
        }
    }

    void Value::releasePayload() {
        switch (type()) {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
            break;
        case stringValue:
            if (isAllocated())
                releasePrefixedStringValue(value_.string_);
            break;
        case arrayValue:
        case objectValue:
            delete value_.map_;
            break;
        default:
            JSON_ASSERT_UNREACHABLE;
        }
    }

    void Value::dupMeta(const Value& other) {
        comments_ = other.comments_;
        start_ = other.start_;
        limit_ = other.limit_;
    }

    // Access an object value by name, create a null member if it does not exist.
    // @pre Type of '*this' is object or null.
    // @param key is null-terminated.
    Value& Value::resolveReference(const char* key) {
        JSON_ASSERT_MESSAGE(
            type() == nullValue || type() == objectValue,
            "in Json::Value::resolveReference(): requires objectValue");
        if (type() == nullValue)
            *this = Value(objectValue);
        CZString actualKey(key, static_cast<unsigned>(strlen(key)),
            CZString::noDuplication); // NOTE!
        auto it = value_.map_->lower_bound(actualKey);
        if (it != value_.map_->end() && (*it).first == actualKey)
            return (*it).second;

        ObjectValues::value_type defaultValue(actualKey, nullSingleton());
        it = value_.map_->insert(it, defaultValue);
        Value& value = (*it).second;
        return value;
    }

    // @param key is not null-terminated.
    Value& Value::resolveReference(char const* key, char const* end) {
        JSON_ASSERT_MESSAGE(
            type() == nullValue || type() == objectValue,
            "in Json::Value::resolveReference(key, end): requires objectValue");
        if (type() == nullValue)
            *this = Value(objectValue);
        CZString actualKey(key, static_cast<unsigned>(end - key),
            CZString::duplicateOnCopy);
        auto it = value_.map_->lower_bound(actualKey);
        if (it != value_.map_->end() && (*it).first == actualKey)
            return (*it).second;

        ObjectValues::value_type defaultValue(actualKey, nullSingleton());
        it = value_.map_->insert(it, defaultValue);
        Value& value = (*it).second;
        return value;
    }

    Value Value::get(ArrayIndex index, const Value& defaultValue) const {
        const Value* value = &((*this)[index]);
        return value == &nullSingleton() ? defaultValue : *value;
    }

    bool Value::isValidIndex(ArrayIndex index) const { return index < size(); }

    Value const* Value::find(char const* begin, char const* end) const {
        JSON_ASSERT_MESSAGE(type() == nullValue || type() == objectValue,
            "in Json::Value::find(begin, end): requires "
            "objectValue or nullValue");
        if (type() == nullValue)
            return nullptr;
        CZString actualKey(begin, static_cast<unsigned>(end - begin),
            CZString::noDuplication);
        ObjectValues::const_iterator it = value_.map_->find(actualKey);
        if (it == value_.map_->end())
            return nullptr;
        return &(*it).second;
    }
    Value* Value::demand(char const* begin, char const* end) {
        JSON_ASSERT_MESSAGE(type() == nullValue || type() == objectValue,
            "in Json::Value::demand(begin, end): requires "
            "objectValue or nullValue");
        return &resolveReference(begin, end);
    }
    const Value& Value::operator[](const char* key) const {
        Value const* found = find(key, key + strlen(key));
        if (!found)
            return nullSingleton();
        return *found;
    }
    Value const& Value::operator[](const String& key) const {
        Value const* found = find(key.data(), key.data() + key.length());
        if (!found)
            return nullSingleton();
        return *found;
    }

    Value& Value::operator[](const char* key) {
        return resolveReference(key, key + strlen(key));
    }

    Value& Value::operator[](const String& key) {
        return resolveReference(key.data(), key.data() + key.length());
    }

    Value& Value::operator[](const StaticString& key) {
        return resolveReference(key.c_str());
    }

    Value& Value::append(const Value& value) { return append(Value(value)); }

    Value& Value::append(Value&& value) {
        JSON_ASSERT_MESSAGE(type() == nullValue || type() == arrayValue,
            "in Json::Value::append: requires arrayValue");
        if (type() == nullValue) {
            *this = Value(arrayValue);
        }
        return this->value_.map_->emplace(size(), std::move(value)).first->second;
    }

    bool Value::insert(ArrayIndex index, const Value& newValue) {
        return insert(index, Value(newValue));
    }

    bool Value::insert(ArrayIndex index, Value&& newValue) {
        JSON_ASSERT_MESSAGE(type() == nullValue || type() == arrayValue,
            "in Json::Value::insert: requires arrayValue");
        ArrayIndex length = size();
        if (index > length) {
            return false;
        }
        for (ArrayIndex i = length; i > index; i--) {
            (*this)[i] = std::move((*this)[i - 1]);
        }
        (*this)[index] = std::move(newValue);
        return true;
    }

    Value Value::get(char const* begin, char const* end,
        Value const& defaultValue) const {
        Value const* found = find(begin, end);
        return !found ? defaultValue : *found;
    }
    Value Value::get(char const* key, Value const& defaultValue) const {
        return get(key, key + strlen(key), defaultValue);
    }
    Value Value::get(String const& key, Value const& defaultValue) const {
        return get(key.data(), key.data() + key.length(), defaultValue);
    }

    bool Value::removeMember(const char* begin, const char* end, Value* removed) {
        if (type() != objectValue) {
            return false;
        }
        CZString actualKey(begin, static_cast<unsigned>(end - begin),
            CZString::noDuplication);
        auto it = value_.map_->find(actualKey);
        if (it == value_.map_->end())
            return false;
        if (removed)
            *removed = std::move(it->second);
        value_.map_->erase(it);
        return true;
    }
    bool Value::removeMember(const char* key, Value* removed) {
        return removeMember(key, key + strlen(key), removed);
    }
    bool Value::removeMember(String const& key, Value* removed) {
        return removeMember(key.data(), key.data() + key.length(), removed);
    }
    void Value::removeMember(const char* key) {
        JSON_ASSERT_MESSAGE(type() == nullValue || type() == objectValue,
            "in Json::Value::removeMember(): requires objectValue");
        if (type() == nullValue)
            return;

        CZString actualKey(key, unsigned(strlen(key)), CZString::noDuplication);
        value_.map_->erase(actualKey);
    }
    void Value::removeMember(const String& key) { removeMember(key.c_str()); }

    bool Value::removeIndex(ArrayIndex index, Value* removed) {
        if (type() != arrayValue) {
            return false;
        }
        CZString key(index);
        auto it = value_.map_->find(key);
        if (it == value_.map_->end()) {
            return false;
        }
        if (removed)
            *removed = it->second;
        ArrayIndex oldSize = size();
        // shift left all items left, into the place of the "removed"
        for (ArrayIndex i = index; i < (oldSize - 1); ++i) {
            CZString keey(i);
            (*value_.map_)[keey] = (*this)[i + 1];
        }
        // erase the last one ("leftover")
        CZString keyLast(oldSize - 1);
        auto itLast = value_.map_->find(keyLast);
        value_.map_->erase(itLast);
        return true;
    }

    bool Value::isMember(char const* begin, char const* end) const {
        Value const* value = find(begin, end);
        return nullptr != value;
    }
    bool Value::isMember(char const* key) const {
        return isMember(key, key + strlen(key));
    }
    bool Value::isMember(String const& key) const {
        return isMember(key.data(), key.data() + key.length());
    }

    Value::Members Value::getMemberNames() const {
        JSON_ASSERT_MESSAGE(
            type() == nullValue || type() == objectValue,
            "in Json::Value::getMemberNames(), value must be objectValue");
        if (type() == nullValue)
            return Value::Members();
        Members members;
        members.reserve(value_.map_->size());
        ObjectValues::const_iterator it = value_.map_->begin();
        ObjectValues::const_iterator itEnd = value_.map_->end();
        for (; it != itEnd; ++it) {
            members.push_back(String((*it).first.data(), (*it).first.length()));
        }
        return members;
    }

    static bool IsIntegral(double d) {
        double integral_part;
        return modf(d, &integral_part) == 0.0;
    }

    bool Value::isNull() const { return type() == nullValue; }

    bool Value::isBool() const { return type() == booleanValue; }

    bool Value::isInt() const {
        switch (type()) {
        case intValue:
#if defined(JSON_HAS_INT64)
            return value_.int_ >= minInt && value_.int_ <= maxInt;
#else
            return true;
#endif
        case uintValue:
            return value_.uint_ <= UInt(maxInt);
        case realValue:
            return value_.real_ >= minInt && value_.real_ <= maxInt &&
                IsIntegral(value_.real_);
        default:
            break;
        }
        return false;
    }

    bool Value::isUInt() const {
        switch (type()) {
        case intValue:
#if defined(JSON_HAS_INT64)
            return value_.int_ >= 0 && LargestUInt(value_.int_) <= LargestUInt(maxUInt);
#else
            return value_.int_ >= 0;
#endif
        case uintValue:
#if defined(JSON_HAS_INT64)
            return value_.uint_ <= maxUInt;
#else
            return true;
#endif
        case realValue:
            return value_.real_ >= 0 && value_.real_ <= maxUInt &&
                IsIntegral(value_.real_);
        default:
            break;
        }
        return false;
    }

    bool Value::isInt64() const {
#if defined(JSON_HAS_INT64)
        switch (type()) {
        case intValue:
            return true;
        case uintValue:
            return value_.uint_ <= UInt64(maxInt64);
        case realValue:
            // Note that maxInt64 (= 2^63 - 1) is not exactly representable as a
            // double, so double(maxInt64) will be rounded up to 2^63. Therefore we
            // require the value to be strictly less than the limit.
            return value_.real_ >= double(minInt64) &&
                value_.real_ < double(maxInt64) && IsIntegral(value_.real_);
        default:
            break;
        }
#endif // JSON_HAS_INT64
        return false;
    }

    bool Value::isUInt64() const {
#if defined(JSON_HAS_INT64)
        switch (type()) {
        case intValue:
            return value_.int_ >= 0;
        case uintValue:
            return true;
        case realValue:
            // Note that maxUInt64 (= 2^64 - 1) is not exactly representable as a
            // double, so double(maxUInt64) will be rounded up to 2^64. Therefore we
            // require the value to be strictly less than the limit.
            return value_.real_ >= 0 && value_.real_ < maxUInt64AsDouble &&
                IsIntegral(value_.real_);
        default:
            break;
        }
#endif // JSON_HAS_INT64
        return false;
    }

    bool Value::isIntegral() const {
        switch (type()) {
        case intValue:
        case uintValue:
            return true;
        case realValue:
#if defined(JSON_HAS_INT64)
            // Note that maxUInt64 (= 2^64 - 1) is not exactly representable as a
            // double, so double(maxUInt64) will be rounded up to 2^64. Therefore we
            // require the value to be strictly less than the limit.
            return value_.real_ >= double(minInt64) &&
                value_.real_ < maxUInt64AsDouble && IsIntegral(value_.real_);
#else
            return value_.real_ >= minInt && value_.real_ <= maxUInt &&
                IsIntegral(value_.real_);
#endif // JSON_HAS_INT64
        default:
            break;
        }
        return false;
    }

    bool Value::isDouble() const {
        return type() == intValue || type() == uintValue || type() == realValue;
    }

    bool Value::isNumeric() const { return isDouble(); }

    bool Value::isString() const { return type() == stringValue; }

    bool Value::isArray() const { return type() == arrayValue; }

    bool Value::isObject() const { return type() == objectValue; }

    Value::Comments::Comments(const Comments& that)
        : ptr_{ cloneUnique(that.ptr_) } {}

    Value::Comments::Comments(Comments&& that) : ptr_{ std::move(that.ptr_) } {}

    Value::Comments& Value::Comments::operator=(const Comments& that) {
        ptr_ = cloneUnique(that.ptr_);
        return *this;
    }

    Value::Comments& Value::Comments::operator=(Comments&& that) {
        ptr_ = std::move(that.ptr_);
        return *this;
    }

    bool Value::Comments::has(CommentPlacement slot) const {
        return ptr_ && !(*ptr_)[slot].empty();
    }

    String Value::Comments::get(CommentPlacement slot) const {
        if (!ptr_)
            return {};
        return (*ptr_)[slot];
    }

    void Value::Comments::set(CommentPlacement slot, String comment) {
        if (!ptr_) {
            ptr_ = std::unique_ptr<Array>(new Array());
        }
        // check comments array boundry.
        if (slot < CommentPlacement::numberOfCommentPlacement) {
            (*ptr_)[slot] = std::move(comment);
        }
    }

    void Value::setComment(String comment, CommentPlacement placement) {
        if (!comment.empty() && (comment.back() == '\n')) {
            // Always discard trailing newline, to aid indentation.
            comment.pop_back();
        }
        JSON_ASSERT(!comment.empty());
        JSON_ASSERT_MESSAGE(
            comment[0] == '\0' || comment[0] == '/',
            "in Json::Value::setComment(): Comments must start with /");
        comments_.set(placement, std::move(comment));
    }

    bool Value::hasComment(CommentPlacement placement) const {
        return comments_.has(placement);
    }

    String Value::getComment(CommentPlacement placement) const {
        return comments_.get(placement);
    }

    void Value::setOffsetStart(ptrdiff_t start) { start_ = start; }

    void Value::setOffsetLimit(ptrdiff_t limit) { limit_ = limit; }

    ptrdiff_t Value::getOffsetStart() const { return start_; }

    ptrdiff_t Value::getOffsetLimit() const { return limit_; }

    String Value::toStyledString() const {
        StreamWriterBuilder builder;

        String out = this->hasComment(commentBefore) ? "\n" : "";
        out += Json::writeString(builder, *this);
        out += '\n';

        return out;
    }

    Value::const_iterator Value::begin() const {
        switch (type()) {
        case arrayValue:
        case objectValue:
            if (value_.map_)
                return const_iterator(value_.map_->begin());
            break;
        default:
            break;
        }
        return {};
    }

    Value::const_iterator Value::end() const {
        switch (type()) {
        case arrayValue:
        case objectValue:
            if (value_.map_)
                return const_iterator(value_.map_->end());
            break;
        default:
            break;
        }
        return {};
    }

    Value::iterator Value::begin() {
        switch (type()) {
        case arrayValue:
        case objectValue:
            if (value_.map_)
                return iterator(value_.map_->begin());
            break;
        default:
            break;
        }
        return iterator();
    }

    Value::iterator Value::end() {
        switch (type()) {
        case arrayValue:
        case objectValue:
            if (value_.map_)
                return iterator(value_.map_->end());
            break;
        default:
            break;
        }
        return iterator();
    }

    // class PathArgument
    // //////////////////////////////////////////////////////////////////

    PathArgument::PathArgument() = default;

    PathArgument::PathArgument(ArrayIndex index)
        : index_(index), kind_(kindIndex) {}

    PathArgument::PathArgument(const char* key) : key_(key), kind_(kindKey) {}

    PathArgument::PathArgument(String key) : key_(std::move(key)), kind_(kindKey) {}

    // class Path
    // //////////////////////////////////////////////////////////////////

    Path::Path(const String& path, const PathArgument& a1, const PathArgument& a2,
        const PathArgument& a3, const PathArgument& a4,
        const PathArgument& a5) {
        InArgs in;
        in.reserve(5);
        in.push_back(&a1);
        in.push_back(&a2);
        in.push_back(&a3);
        in.push_back(&a4);
        in.push_back(&a5);
        makePath(path, in);
    }

    void Path::makePath(const String& path, const InArgs& in) {
        const char* current = path.c_str();
        const char* end = current + path.length();
        auto itInArg = in.begin();
        while (current != end) {
            if (*current == '[') {
                ++current;
                if (*current == '%')
                    addPathInArg(path, in, itInArg, PathArgument::kindIndex);
                else {
                    ArrayIndex index = 0;
                    for (; current != end && *current >= '0' && *current <= '9'; ++current)
                        index = index * 10 + ArrayIndex(*current - '0');
                    args_.push_back(index);
                }
                if (current == end || *++current != ']')
                    invalidPath(path, int(current - path.c_str()));
            } else if (*current == '%') {
                addPathInArg(path, in, itInArg, PathArgument::kindKey);
                ++current;
            } else if (*current == '.' || *current == ']') {
                ++current;
            } else {
                const char* beginName = current;
                while (current != end && !strchr("[.", *current))
                    ++current;
                args_.push_back(String(beginName, current));
            }
        }
    }

    void Path::addPathInArg(const String& /*path*/, const InArgs& in,
        InArgs::const_iterator& itInArg,
        PathArgument::Kind kind) {
        if (itInArg == in.end()) {
            // Error: missing argument %d
        } else if ((*itInArg)->kind_ != kind) {
            // Error: bad argument type
        } else {
            args_.push_back(**itInArg++);
        }
    }

    void Path::invalidPath(const String& /*path*/, int /*location*/) {
        // Error: invalid path.
    }

    const Value& Path::resolve(const Value& root) const {
        const Value* node = &root;
        for (const auto& arg : args_) {
            if (arg.kind_ == PathArgument::kindIndex) {
                if (!node->isArray() || !node->isValidIndex(arg.index_)) {
                    // Error: unable to resolve path (array value expected at position... )
                    return Value::nullSingleton();
                }
                node = &((*node)[arg.index_]);
            } else if (arg.kind_ == PathArgument::kindKey) {
                if (!node->isObject()) {
                    // Error: unable to resolve path (object value expected at position...)
                    return Value::nullSingleton();
                }
                node = &((*node)[arg.key_]);
                if (node == &Value::nullSingleton()) {
                    // Error: unable to resolve path (object has no member named '' at
                    // position...)
                    return Value::nullSingleton();
                }
            }
        }
        return *node;
    }

    Value Path::resolve(const Value& root, const Value& defaultValue) const {
        const Value* node = &root;
        for (const auto& arg : args_) {
            if (arg.kind_ == PathArgument::kindIndex) {
                if (!node->isArray() || !node->isValidIndex(arg.index_))
                    return defaultValue;
                node = &((*node)[arg.index_]);
            } else if (arg.kind_ == PathArgument::kindKey) {
                if (!node->isObject())
                    return defaultValue;
                node = &((*node)[arg.key_]);
                if (node == &Value::nullSingleton())
                    return defaultValue;
            }
        }
        return *node;
    }

    Value& Path::make(Value& root) const {
        Value* node = &root;
        for (const auto& arg : args_) {
            if (arg.kind_ == PathArgument::kindIndex) {
                if (!node->isArray()) {
                    // Error: node is not an array at position ...
                }
                node = &((*node)[arg.index_]);
            } else if (arg.kind_ == PathArgument::kindKey) {
                if (!node->isObject()) {
                    // Error: node is not an object at position...
                }
                node = &((*node)[arg.key_]);
            }
        }
        return *node;
    }

} // namespace Json

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_value.cpp
// //////////////////////////////////////////////////////////////////////






// //////////////////////////////////////////////////////////////////////
// Beginning of content of file: src/lib_json/json_writer.cpp
// //////////////////////////////////////////////////////////////////////

// Copyright 2011 Baptiste Lepilleur and The JsonCpp Authors
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSON_IS_AMALGAMATION)
#include "json_tool.h"
#include <json/writer.h>
#endif // if !defined(JSON_IS_AMALGAMATION)
#include <cassert>
#include <cstring>
#include <iomanip>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#if __cplusplus >= 201103L
#include <cmath>
#include <cstdio>

#if !defined(isnan)
#define isnan std::isnan
#endif

#if !defined(isfinite)
#define isfinite std::isfinite
#endif

#else
#include <cmath>
#include <cstdio>

#if defined(_MSC_VER)
#if !defined(isnan)
#include <float.h>
#define isnan _isnan
#endif

#if !defined(isfinite)
#include <float.h>
#define isfinite _finite
#endif

#if !defined(_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES)
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif //_CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES

#endif //_MSC_VER

#if defined(__sun) && defined(__SVR4) // Solaris
#if !defined(isfinite)
#include <ieeefp.h>
#define isfinite finite
#endif
#endif

#if defined(__hpux)
#if !defined(isfinite)
#if defined(__ia64) && !defined(finite)
#define isfinite(x)                                                            \
  ((sizeof(x) == sizeof(float) ? _Isfinitef(x) : _IsFinite(x)))
#endif
#endif
#endif

#if !defined(isnan)
// IEEE standard states that NaN values will not compare to themselves
#define isnan(x) (x != x)
#endif

#if !defined(__APPLE__)
#if !defined(isfinite)
#define isfinite finite
#endif
#endif
#endif

#if defined(_MSC_VER)
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif

namespace Json {

#if __cplusplus >= 201103L || (defined(_CPPLIB_VER) && _CPPLIB_VER >= 520)
    using StreamWriterPtr = std::unique_ptr<StreamWriter>;
#else
    using StreamWriterPtr = std::auto_ptr<StreamWriter>;
#endif

    String valueToString(LargestInt value) {
        UIntToStringBuffer buffer;
        char* current = buffer + sizeof(buffer);
        if (value == Value::minLargestInt) {
            uintToString(LargestUInt(Value::maxLargestInt) + 1, current);
            *--current = '-';
        } else if (value < 0) {
            uintToString(LargestUInt(-value), current);
            *--current = '-';
        } else {
            uintToString(LargestUInt(value), current);
        }
        assert(current >= buffer);
        return current;
    }

    String valueToString(LargestUInt value) {
        UIntToStringBuffer buffer;
        char* current = buffer + sizeof(buffer);
        uintToString(value, current);
        assert(current >= buffer);
        return current;
    }

#if defined(JSON_HAS_INT64)

    String valueToString(Int value) { return valueToString(LargestInt(value)); }

    String valueToString(UInt value) { return valueToString(LargestUInt(value)); }

#endif // # if defined(JSON_HAS_INT64)

    namespace {
        String valueToString(double value, bool useSpecialFloats,
            unsigned int precision, PrecisionType precisionType) {
            // Print into the buffer. We need not request the alternative representation
            // that always has a decimal point because JSON doesn't distinguish the
            // concepts of reals and integers.
            if (!isfinite(value)) {
                static const char* const reps[2][3] = { {"NaN", "-Infinity", "Infinity"},
                                                       {"null", "-1e+9999", "1e+9999"} };
                return reps[useSpecialFloats ? 0 : 1]
                    [isnan(value) ? 0 : (value < 0) ? 1 : 2];
            }

            String buffer(size_t(36), '\0');
            while (true) {
                int len = jsoncpp_snprintf(
                    &*buffer.begin(), buffer.size(),
                    (precisionType == PrecisionType::significantDigits) ? "%.*g" : "%.*f",
                    precision, value);
                assert(len >= 0);
                auto wouldPrint = static_cast<size_t>(len);
                if (wouldPrint >= buffer.size()) {
                    buffer.resize(wouldPrint + 1);
                    continue;
                }
                buffer.resize(wouldPrint);
                break;
            }

            buffer.erase(fixNumericLocale(buffer.begin(), buffer.end()), buffer.end());

            // strip the zero padding from the right
            if (precisionType == PrecisionType::decimalPlaces) {
                buffer.erase(fixZerosInTheEnd(buffer.begin(), buffer.end()), buffer.end());
            }

            // try to ensure we preserve the fact that this was given to us as a double on
            // input
            if (buffer.find('.') == buffer.npos && buffer.find('e') == buffer.npos) {
                buffer += ".0";
            }
            return buffer;
        }
    } // namespace

    String valueToString(double value, unsigned int precision,
        PrecisionType precisionType) {
        return valueToString(value, false, precision, precisionType);
    }

    String valueToString(bool value) { return value ? "true" : "false"; }

    static bool isAnyCharRequiredQuoting(char const* s, size_t n) {
        assert(s || !n);

        char const* const end = s + n;
        for (char const* cur = s; cur < end; ++cur) {
            if (*cur == '\\' || *cur == '\"' ||
                static_cast<unsigned char>(*cur) < ' ' ||
                static_cast<unsigned char>(*cur) >= 0x80)
                return true;
        }
        return false;
    }

    static unsigned int utf8ToCodepoint(const char*& s, const char* e) {
        const unsigned int REPLACEMENT_CHARACTER = 0xFFFD;

        unsigned int firstByte = static_cast<unsigned char>(*s);

        if (firstByte < 0x80)
            return firstByte;

        if (firstByte < 0xE0) {
            if (e - s < 2)
                return REPLACEMENT_CHARACTER;

            unsigned int calculated =
                ((firstByte & 0x1F) << 6) | (static_cast<unsigned int>(s[1]) & 0x3F);
            s += 1;
            // oversized encoded characters are invalid
            return calculated < 0x80 ? REPLACEMENT_CHARACTER : calculated;
        }

        if (firstByte < 0xF0) {
            if (e - s < 3)
                return REPLACEMENT_CHARACTER;

            unsigned int calculated = ((firstByte & 0x0F) << 12) |
                ((static_cast<unsigned int>(s[1]) & 0x3F) << 6) |
                (static_cast<unsigned int>(s[2]) & 0x3F);
            s += 2;
            // surrogates aren't valid codepoints itself
            // shouldn't be UTF-8 encoded
            if (calculated >= 0xD800 && calculated <= 0xDFFF)
                return REPLACEMENT_CHARACTER;
            // oversized encoded characters are invalid
            return calculated < 0x800 ? REPLACEMENT_CHARACTER : calculated;
        }

        if (firstByte < 0xF8) {
            if (e - s < 4)
                return REPLACEMENT_CHARACTER;

            unsigned int calculated = ((firstByte & 0x07) << 18) |
                ((static_cast<unsigned int>(s[1]) & 0x3F) << 12) |
                ((static_cast<unsigned int>(s[2]) & 0x3F) << 6) |
                (static_cast<unsigned int>(s[3]) & 0x3F);
            s += 3;
            // oversized encoded characters are invalid
            return calculated < 0x10000 ? REPLACEMENT_CHARACTER : calculated;
        }

        return REPLACEMENT_CHARACTER;
    }

    static const char hex2[] = "000102030405060708090a0b0c0d0e0f"
        "101112131415161718191a1b1c1d1e1f"
        "202122232425262728292a2b2c2d2e2f"
        "303132333435363738393a3b3c3d3e3f"
        "404142434445464748494a4b4c4d4e4f"
        "505152535455565758595a5b5c5d5e5f"
        "606162636465666768696a6b6c6d6e6f"
        "707172737475767778797a7b7c7d7e7f"
        "808182838485868788898a8b8c8d8e8f"
        "909192939495969798999a9b9c9d9e9f"
        "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
        "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
        "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
        "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
        "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
        "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";

    static String toHex16Bit(unsigned int x) {
        const unsigned int hi = (x >> 8) & 0xff;
        const unsigned int lo = x & 0xff;
        String result(4, ' ');
        result[0] = hex2[2 * hi];
        result[1] = hex2[2 * hi + 1];
        result[2] = hex2[2 * lo];
        result[3] = hex2[2 * lo + 1];
        return result;
    }

    static String valueToQuotedStringN(const char* value, unsigned length,
        bool emitUTF8 = false) {
        if (value == nullptr)
            return "";

        if (!isAnyCharRequiredQuoting(value, length))
            return String("\"") + value + "\"";
        // We have to walk value and escape any special characters.
        // Appending to String is not efficient, but this should be rare.
        // (Note: forward slashes are *not* rare, but I am not escaping them.)
        String::size_type maxsize = length * 2 + 3; // allescaped+quotes+NULL
        String result;
        result.reserve(maxsize); // to avoid lots of mallocs
        result += "\"";
        char const* end = value + length;
        for (const char* c = value; c != end; ++c) {
            switch (*c) {
            case '\"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
                // case '/':
                // Even though \/ is considered a legal escape in JSON, a bare
                // slash is also legal, so I see no reason to escape it.
                // (I hope I am not misunderstanding something.)
                // blep notes: actually escaping \/ may be useful in javascript to avoid </
                // sequence.
                // Should add a flag to allow this compatibility mode and prevent this
                // sequence from occurring.
            default: {
                if (emitUTF8) {
                    result += *c;
                } else {
                    unsigned int codepoint = utf8ToCodepoint(c, end);
                    const unsigned int FIRST_NON_CONTROL_CODEPOINT = 0x20;
                    const unsigned int LAST_NON_CONTROL_CODEPOINT = 0x7F;
                    const unsigned int FIRST_SURROGATE_PAIR_CODEPOINT = 0x10000;
                    // don't escape non-control characters
                    // (short escape sequence are applied above)
                    if (FIRST_NON_CONTROL_CODEPOINT <= codepoint &&
                        codepoint <= LAST_NON_CONTROL_CODEPOINT) {
                        result += static_cast<char>(codepoint);
                    } else if (codepoint <
                        FIRST_SURROGATE_PAIR_CODEPOINT) { // codepoint is in Basic
                                                          // Multilingual Plane
                        result += "\\u";
                        result += toHex16Bit(codepoint);
                    } else { // codepoint is not in Basic Multilingual Plane
                             // convert to surrogate pair first
                        codepoint -= FIRST_SURROGATE_PAIR_CODEPOINT;
                        result += "\\u";
                        result += toHex16Bit((codepoint >> 10) + 0xD800);
                        result += "\\u";
                        result += toHex16Bit((codepoint & 0x3FF) + 0xDC00);
                    }
                }
            } break;
            }
        }
        result += "\"";
        return result;
    }

    String valueToQuotedString(const char* value) {
        return valueToQuotedStringN(value, static_cast<unsigned int>(strlen(value)));
    }

    // Class Writer
    // //////////////////////////////////////////////////////////////////
    Writer::~Writer() = default;

    // Class FastWriter
    // //////////////////////////////////////////////////////////////////

    FastWriter::FastWriter()

        = default;

    void FastWriter::enableYAMLCompatibility() { yamlCompatibilityEnabled_ = true; }

    void FastWriter::dropNullPlaceholders() { dropNullPlaceholders_ = true; }

    void FastWriter::omitEndingLineFeed() { omitEndingLineFeed_ = true; }

    String FastWriter::write(const Value& root) {
        document_.clear();
        writeValue(root);
        if (!omitEndingLineFeed_)
            document_ += '\n';
        return document_;
    }

    void FastWriter::writeValue(const Value& value) {
        switch (value.type()) {
        case nullValue:
            if (!dropNullPlaceholders_)
                document_ += "null";
            break;
        case intValue:
            document_ += valueToString(value.asLargestInt());
            break;
        case uintValue:
            document_ += valueToString(value.asLargestUInt());
            break;
        case realValue:
            document_ += valueToString(value.asDouble());
            break;
        case stringValue: {
            // Is NULL possible for value.string_? No.
            char const* str;
            char const* end;
            bool ok = value.getString(&str, &end);
            if (ok)
                document_ += valueToQuotedStringN(str, static_cast<unsigned>(end - str));
            break;
        }
        case booleanValue:
            document_ += valueToString(value.asBool());
            break;
        case arrayValue: {
            document_ += '[';
            ArrayIndex size = value.size();
            for (ArrayIndex index = 0; index < size; ++index) {
                if (index > 0)
                    document_ += ',';
                writeValue(value[index]);
            }
            document_ += ']';
        } break;
        case objectValue: {
            Value::Members members(value.getMemberNames());
            document_ += '{';
            for (auto it = members.begin(); it != members.end(); ++it) {
                const String& name = *it;
                if (it != members.begin())
                    document_ += ',';
                document_ += valueToQuotedStringN(name.data(),
                    static_cast<unsigned>(name.length()));
                document_ += yamlCompatibilityEnabled_ ? ": " : ":";
                writeValue(value[name]);
            }
            document_ += '}';
        } break;
        }
    }

    // Class StyledWriter
    // //////////////////////////////////////////////////////////////////

    StyledWriter::StyledWriter() = default;

    String StyledWriter::write(const Value& root) {
        document_.clear();
        addChildValues_ = false;
        indentString_.clear();
        writeCommentBeforeValue(root);
        writeValue(root);
        writeCommentAfterValueOnSameLine(root);
        document_ += '\n';
        return document_;
    }

    void StyledWriter::writeValue(const Value& value) {
        switch (value.type()) {
        case nullValue:
            pushValue("null");
            break;
        case intValue:
            pushValue(valueToString(value.asLargestInt()));
            break;
        case uintValue:
            pushValue(valueToString(value.asLargestUInt()));
            break;
        case realValue:
            pushValue(valueToString(value.asDouble()));
            break;
        case stringValue: {
            // Is NULL possible for value.string_? No.
            char const* str;
            char const* end;
            bool ok = value.getString(&str, &end);
            if (ok)
                pushValue(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
            else
                pushValue("");
            break;
        }
        case booleanValue:
            pushValue(valueToString(value.asBool()));
            break;
        case arrayValue:
            writeArrayValue(value);
            break;
        case objectValue: {
            Value::Members members(value.getMemberNames());
            if (members.empty())
                pushValue("{}");
            else {
                writeWithIndent("{");
                indent();
                auto it = members.begin();
                for (;;) {
                    const String& name = *it;
                    const Value& childValue = value[name];
                    writeCommentBeforeValue(childValue);
                    writeWithIndent(valueToQuotedString(name.c_str()));
                    document_ += " : ";
                    writeValue(childValue);
                    if (++it == members.end()) {
                        writeCommentAfterValueOnSameLine(childValue);
                        break;
                    }
                    document_ += ',';
                    writeCommentAfterValueOnSameLine(childValue);
                }
                unindent();
                writeWithIndent("}");
            }
        } break;
        }
    }

    void StyledWriter::writeArrayValue(const Value& value) {
        unsigned size = value.size();
        if (size == 0)
            pushValue("[]");
        else {
            bool isArrayMultiLine = isMultilineArray(value);
            if (isArrayMultiLine) {
                writeWithIndent("[");
                indent();
                bool hasChildValue = !childValues_.empty();
                unsigned index = 0;
                for (;;) {
                    const Value& childValue = value[index];
                    writeCommentBeforeValue(childValue);
                    if (hasChildValue)
                        writeWithIndent(childValues_[index]);
                    else {
                        writeIndent();
                        writeValue(childValue);
                    }
                    if (++index == size) {
                        writeCommentAfterValueOnSameLine(childValue);
                        break;
                    }
                    document_ += ',';
                    writeCommentAfterValueOnSameLine(childValue);
                }
                unindent();
                writeWithIndent("]");
            } else // output on a single line
            {
                assert(childValues_.size() == size);
                document_ += "[ ";
                for (unsigned index = 0; index < size; ++index) {
                    if (index > 0)
                        document_ += ", ";
                    document_ += childValues_[index];
                }
                document_ += " ]";
            }
        }
    }

    bool StyledWriter::isMultilineArray(const Value& value) {
        ArrayIndex const size = value.size();
        bool isMultiLine = size * 3 >= rightMargin_;
        childValues_.clear();
        for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
            const Value& childValue = value[index];
            isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
                !childValue.empty());
        }
        if (!isMultiLine) // check if line length > max line length
        {
            childValues_.reserve(size);
            addChildValues_ = true;
            ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
            for (ArrayIndex index = 0; index < size; ++index) {
                if (hasCommentForValue(value[index])) {
                    isMultiLine = true;
                }
                writeValue(value[index]);
                lineLength += static_cast<ArrayIndex>(childValues_[index].length());
            }
            addChildValues_ = false;
            isMultiLine = isMultiLine || lineLength >= rightMargin_;
        }
        return isMultiLine;
    }

    void StyledWriter::pushValue(const String& value) {
        if (addChildValues_)
            childValues_.push_back(value);
        else
            document_ += value;
    }

    void StyledWriter::writeIndent() {
        if (!document_.empty()) {
            char last = document_[document_.length() - 1];
            if (last == ' ') // already indented
                return;
            if (last != '\n') // Comments may add new-line
                document_ += '\n';
        }
        document_ += indentString_;
    }

    void StyledWriter::writeWithIndent(const String& value) {
        writeIndent();
        document_ += value;
    }

    void StyledWriter::indent() { indentString_ += String(indentSize_, ' '); }

    void StyledWriter::unindent() {
        assert(indentString_.size() >= indentSize_);
        indentString_.resize(indentString_.size() - indentSize_);
    }

    void StyledWriter::writeCommentBeforeValue(const Value& root) {
        if (!root.hasComment(commentBefore))
            return;

        document_ += '\n';
        writeIndent();
        const String& comment = root.getComment(commentBefore);
        String::const_iterator iter = comment.begin();
        while (iter != comment.end()) {
            document_ += *iter;
            if (*iter == '\n' && ((iter + 1) != comment.end() && *(iter + 1) == '/'))
                writeIndent();
            ++iter;
        }

        // Comments are stripped of trailing newlines, so add one here
        document_ += '\n';
    }

    void StyledWriter::writeCommentAfterValueOnSameLine(const Value& root) {
        if (root.hasComment(commentAfterOnSameLine))
            document_ += " " + root.getComment(commentAfterOnSameLine);

        if (root.hasComment(commentAfter)) {
            document_ += '\n';
            document_ += root.getComment(commentAfter);
            document_ += '\n';
        }
    }

    bool StyledWriter::hasCommentForValue(const Value& value) {
        return value.hasComment(commentBefore) ||
            value.hasComment(commentAfterOnSameLine) ||
            value.hasComment(commentAfter);
    }

    // Class StyledStreamWriter
    // //////////////////////////////////////////////////////////////////

    StyledStreamWriter::StyledStreamWriter(String indentation)
        : document_(nullptr), indentation_(std::move(indentation)),
        addChildValues_(), indented_(false) {}

    void StyledStreamWriter::write(OStream& out, const Value& root) {
        document_ = &out;
        addChildValues_ = false;
        indentString_.clear();
        indented_ = true;
        writeCommentBeforeValue(root);
        if (!indented_)
            writeIndent();
        indented_ = true;
        writeValue(root);
        writeCommentAfterValueOnSameLine(root);
        *document_ << "\n";
        document_ = nullptr; // Forget the stream, for safety.
    }

    void StyledStreamWriter::writeValue(const Value& value) {
        switch (value.type()) {
        case nullValue:
            pushValue("null");
            break;
        case intValue:
            pushValue(valueToString(value.asLargestInt()));
            break;
        case uintValue:
            pushValue(valueToString(value.asLargestUInt()));
            break;
        case realValue:
            pushValue(valueToString(value.asDouble()));
            break;
        case stringValue: {
            // Is NULL possible for value.string_? No.
            char const* str;
            char const* end;
            bool ok = value.getString(&str, &end);
            if (ok)
                pushValue(valueToQuotedStringN(str, static_cast<unsigned>(end - str)));
            else
                pushValue("");
            break;
        }
        case booleanValue:
            pushValue(valueToString(value.asBool()));
            break;
        case arrayValue:
            writeArrayValue(value);
            break;
        case objectValue: {
            Value::Members members(value.getMemberNames());
            if (members.empty())
                pushValue("{}");
            else {
                writeWithIndent("{");
                indent();
                auto it = members.begin();
                for (;;) {
                    const String& name = *it;
                    const Value& childValue = value[name];
                    writeCommentBeforeValue(childValue);
                    writeWithIndent(valueToQuotedString(name.c_str()));
                    *document_ << " : ";
                    writeValue(childValue);
                    if (++it == members.end()) {
                        writeCommentAfterValueOnSameLine(childValue);
                        break;
                    }
                    *document_ << ",";
                    writeCommentAfterValueOnSameLine(childValue);
                }
                unindent();
                writeWithIndent("}");
            }
        } break;
        }
    }

    void StyledStreamWriter::writeArrayValue(const Value& value) {
        unsigned size = value.size();
        if (size == 0)
            pushValue("[]");
        else {
            bool isArrayMultiLine = isMultilineArray(value);
            if (isArrayMultiLine) {
                writeWithIndent("[");
                indent();
                bool hasChildValue = !childValues_.empty();
                unsigned index = 0;
                for (;;) {
                    const Value& childValue = value[index];
                    writeCommentBeforeValue(childValue);
                    if (hasChildValue)
                        writeWithIndent(childValues_[index]);
                    else {
                        if (!indented_)
                            writeIndent();
                        indented_ = true;
                        writeValue(childValue);
                        indented_ = false;
                    }
                    if (++index == size) {
                        writeCommentAfterValueOnSameLine(childValue);
                        break;
                    }
                    *document_ << ",";
                    writeCommentAfterValueOnSameLine(childValue);
                }
                unindent();
                writeWithIndent("]");
            } else // output on a single line
            {
                assert(childValues_.size() == size);
                *document_ << "[ ";
                for (unsigned index = 0; index < size; ++index) {
                    if (index > 0)
                        *document_ << ", ";
                    *document_ << childValues_[index];
                }
                *document_ << " ]";
            }
        }
    }

    bool StyledStreamWriter::isMultilineArray(const Value& value) {
        ArrayIndex const size = value.size();
        bool isMultiLine = size * 3 >= rightMargin_;
        childValues_.clear();
        for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
            const Value& childValue = value[index];
            isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
                !childValue.empty());
        }
        if (!isMultiLine) // check if line length > max line length
        {
            childValues_.reserve(size);
            addChildValues_ = true;
            ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
            for (ArrayIndex index = 0; index < size; ++index) {
                if (hasCommentForValue(value[index])) {
                    isMultiLine = true;
                }
                writeValue(value[index]);
                lineLength += static_cast<ArrayIndex>(childValues_[index].length());
            }
            addChildValues_ = false;
            isMultiLine = isMultiLine || lineLength >= rightMargin_;
        }
        return isMultiLine;
    }

    void StyledStreamWriter::pushValue(const String& value) {
        if (addChildValues_)
            childValues_.push_back(value);
        else
            *document_ << value;
    }

    void StyledStreamWriter::writeIndent() {
        // blep intended this to look at the so-far-written string
        // to determine whether we are already indented, but
        // with a stream we cannot do that. So we rely on some saved state.
        // The caller checks indented_.
        *document_ << '\n' << indentString_;
    }

    void StyledStreamWriter::writeWithIndent(const String& value) {
        if (!indented_)
            writeIndent();
        *document_ << value;
        indented_ = false;
    }

    void StyledStreamWriter::indent() { indentString_ += indentation_; }

    void StyledStreamWriter::unindent() {
        assert(indentString_.size() >= indentation_.size());
        indentString_.resize(indentString_.size() - indentation_.size());
    }

    void StyledStreamWriter::writeCommentBeforeValue(const Value& root) {
        if (!root.hasComment(commentBefore))
            return;

        if (!indented_)
            writeIndent();
        const String& comment = root.getComment(commentBefore);
        String::const_iterator iter = comment.begin();
        while (iter != comment.end()) {
            *document_ << *iter;
            if (*iter == '\n' && ((iter + 1) != comment.end() && *(iter + 1) == '/'))
                // writeIndent();  // would include newline
                *document_ << indentString_;
            ++iter;
        }
        indented_ = false;
    }

    void StyledStreamWriter::writeCommentAfterValueOnSameLine(const Value& root) {
        if (root.hasComment(commentAfterOnSameLine))
            *document_ << ' ' << root.getComment(commentAfterOnSameLine);

        if (root.hasComment(commentAfter)) {
            writeIndent();
            *document_ << root.getComment(commentAfter);
        }
        indented_ = false;
    }

    bool StyledStreamWriter::hasCommentForValue(const Value& value) {
        return value.hasComment(commentBefore) ||
            value.hasComment(commentAfterOnSameLine) ||
            value.hasComment(commentAfter);
    }

    //////////////////////////
    // BuiltStyledStreamWriter

    /// Scoped enums are not available until C++11.
    struct CommentStyle {
        /// Decide whether to write comments.
        enum Enum {
            None, ///< Drop all comments.
            Most, ///< Recover odd behavior of previous versions (not implemented yet).
            All   ///< Keep all comments.
        };
    };

    struct BuiltStyledStreamWriter : public StreamWriter {
        BuiltStyledStreamWriter(String indentation, CommentStyle::Enum cs,
            String colonSymbol, String nullSymbol,
            String endingLineFeedSymbol, bool useSpecialFloats,
            bool emitUTF8, unsigned int precision,
            PrecisionType precisionType);
        int write(Value const& root, OStream* sout) override;

    private:
        void writeValue(Value const& value);
        void writeArrayValue(Value const& value);
        bool isMultilineArray(Value const& value);
        void pushValue(String const& value);
        void writeIndent();
        void writeWithIndent(String const& value);
        void indent();
        void unindent();
        void writeCommentBeforeValue(Value const& root);
        void writeCommentAfterValueOnSameLine(Value const& root);
        static bool hasCommentForValue(const Value& value);

        using ChildValues = std::vector<String>;

        ChildValues childValues_;
        String indentString_;
        unsigned int rightMargin_;
        String indentation_;
        CommentStyle::Enum cs_;
        String colonSymbol_;
        String nullSymbol_;
        String endingLineFeedSymbol_;
        bool addChildValues_ : 1;
        bool indented_ : 1;
        bool useSpecialFloats_ : 1;
        bool emitUTF8_ : 1;
        unsigned int precision_;
        PrecisionType precisionType_;
    };
    BuiltStyledStreamWriter::BuiltStyledStreamWriter(
        String indentation, CommentStyle::Enum cs, String colonSymbol,
        String nullSymbol, String endingLineFeedSymbol, bool useSpecialFloats,
        bool emitUTF8, unsigned int precision, PrecisionType precisionType)
        : rightMargin_(74), indentation_(std::move(indentation)), cs_(cs),
        colonSymbol_(std::move(colonSymbol)), nullSymbol_(std::move(nullSymbol)),
        endingLineFeedSymbol_(std::move(endingLineFeedSymbol)),
        addChildValues_(false), indented_(false),
        useSpecialFloats_(useSpecialFloats), emitUTF8_(emitUTF8),
        precision_(precision), precisionType_(precisionType) {}
    int BuiltStyledStreamWriter::write(Value const& root, OStream* sout) {
        sout_ = sout;
        addChildValues_ = false;
        indented_ = true;
        indentString_.clear();
        writeCommentBeforeValue(root);
        if (!indented_)
            writeIndent();
        indented_ = true;
        writeValue(root);
        writeCommentAfterValueOnSameLine(root);
        *sout_ << endingLineFeedSymbol_;
        sout_ = nullptr;
        return 0;
    }
    void BuiltStyledStreamWriter::writeValue(Value const& value) {
        switch (value.type()) {
        case nullValue:
            pushValue(nullSymbol_);
            break;
        case intValue:
            pushValue(valueToString(value.asLargestInt()));
            break;
        case uintValue:
            pushValue(valueToString(value.asLargestUInt()));
            break;
        case realValue:
            pushValue(valueToString(value.asDouble(), useSpecialFloats_, precision_,
                precisionType_));
            break;
        case stringValue: {
            // Is NULL is possible for value.string_? No.
            char const* str;
            char const* end;
            bool ok = value.getString(&str, &end);
            if (ok)
                pushValue(valueToQuotedStringN(str, static_cast<unsigned>(end - str),
                    emitUTF8_));
            else
                pushValue("");
            break;
        }
        case booleanValue:
            pushValue(valueToString(value.asBool()));
            break;
        case arrayValue:
            writeArrayValue(value);
            break;
        case objectValue: {
            Value::Members members(value.getMemberNames());
            if (members.empty())
                pushValue("{}");
            else {
                writeWithIndent("{");
                indent();
                auto it = members.begin();
                for (;;) {
                    String const& name = *it;
                    Value const& childValue = value[name];
                    writeCommentBeforeValue(childValue);
                    writeWithIndent(valueToQuotedStringN(
                        name.data(), static_cast<unsigned>(name.length()), emitUTF8_));
                    *sout_ << colonSymbol_;
                    writeValue(childValue);
                    if (++it == members.end()) {
                        writeCommentAfterValueOnSameLine(childValue);
                        break;
                    }
                    *sout_ << ",";
                    writeCommentAfterValueOnSameLine(childValue);
                }
                unindent();
                writeWithIndent("}");
            }
        } break;
        }
    }

    void BuiltStyledStreamWriter::writeArrayValue(Value const& value) {
        unsigned size = value.size();
        if (size == 0)
            pushValue("[]");
        else {
            bool isMultiLine = (cs_ == CommentStyle::All) || isMultilineArray(value);
            if (isMultiLine) {
                writeWithIndent("[");
                indent();
                bool hasChildValue = !childValues_.empty();
                unsigned index = 0;
                for (;;) {
                    Value const& childValue = value[index];
                    writeCommentBeforeValue(childValue);
                    if (hasChildValue)
                        writeWithIndent(childValues_[index]);
                    else {
                        if (!indented_)
                            writeIndent();
                        indented_ = true;
                        writeValue(childValue);
                        indented_ = false;
                    }
                    if (++index == size) {
                        writeCommentAfterValueOnSameLine(childValue);
                        break;
                    }
                    *sout_ << ",";
                    writeCommentAfterValueOnSameLine(childValue);
                }
                unindent();
                writeWithIndent("]");
            } else // output on a single line
            {
                assert(childValues_.size() == size);
                *sout_ << "[";
                if (!indentation_.empty())
                    *sout_ << " ";
                for (unsigned index = 0; index < size; ++index) {
                    if (index > 0)
                        *sout_ << ((!indentation_.empty()) ? ", " : ",");
                    *sout_ << childValues_[index];
                }
                if (!indentation_.empty())
                    *sout_ << " ";
                *sout_ << "]";
            }
        }
    }

    bool BuiltStyledStreamWriter::isMultilineArray(Value const& value) {
        ArrayIndex const size = value.size();
        bool isMultiLine = size * 3 >= rightMargin_;
        childValues_.clear();
        for (ArrayIndex index = 0; index < size && !isMultiLine; ++index) {
            Value const& childValue = value[index];
            isMultiLine = ((childValue.isArray() || childValue.isObject()) &&
                !childValue.empty());
        }
        if (!isMultiLine) // check if line length > max line length
        {
            childValues_.reserve(size);
            addChildValues_ = true;
            ArrayIndex lineLength = 4 + (size - 1) * 2; // '[ ' + ', '*n + ' ]'
            for (ArrayIndex index = 0; index < size; ++index) {
                if (hasCommentForValue(value[index])) {
                    isMultiLine = true;
                }
                writeValue(value[index]);
                lineLength += static_cast<ArrayIndex>(childValues_[index].length());
            }
            addChildValues_ = false;
            isMultiLine = isMultiLine || lineLength >= rightMargin_;
        }
        return isMultiLine;
    }

    void BuiltStyledStreamWriter::pushValue(String const& value) {
        if (addChildValues_)
            childValues_.push_back(value);
        else
            *sout_ << value;
    }

    void BuiltStyledStreamWriter::writeIndent() {
        // blep intended this to look at the so-far-written string
        // to determine whether we are already indented, but
        // with a stream we cannot do that. So we rely on some saved state.
        // The caller checks indented_.

        if (!indentation_.empty()) {
            // In this case, drop newlines too.
            *sout_ << '\n' << indentString_;
        }
    }

    void BuiltStyledStreamWriter::writeWithIndent(String const& value) {
        if (!indented_)
            writeIndent();
        *sout_ << value;
        indented_ = false;
    }

    void BuiltStyledStreamWriter::indent() { indentString_ += indentation_; }

    void BuiltStyledStreamWriter::unindent() {
        assert(indentString_.size() >= indentation_.size());
        indentString_.resize(indentString_.size() - indentation_.size());
    }

    void BuiltStyledStreamWriter::writeCommentBeforeValue(Value const& root) {
        if (cs_ == CommentStyle::None)
            return;
        if (!root.hasComment(commentBefore))
            return;

        if (!indented_)
            writeIndent();
        const String& comment = root.getComment(commentBefore);
        String::const_iterator iter = comment.begin();
        while (iter != comment.end()) {
            *sout_ << *iter;
            if (*iter == '\n' && ((iter + 1) != comment.end() && *(iter + 1) == '/'))
                // writeIndent();  // would write extra newline
                *sout_ << indentString_;
            ++iter;
        }
        indented_ = false;
    }

    void BuiltStyledStreamWriter::writeCommentAfterValueOnSameLine(
        Value const& root) {
        if (cs_ == CommentStyle::None)
            return;
        if (root.hasComment(commentAfterOnSameLine))
            *sout_ << " " + root.getComment(commentAfterOnSameLine);

        if (root.hasComment(commentAfter)) {
            writeIndent();
            *sout_ << root.getComment(commentAfter);
        }
    }

    // static
    bool BuiltStyledStreamWriter::hasCommentForValue(const Value& value) {
        return value.hasComment(commentBefore) ||
            value.hasComment(commentAfterOnSameLine) ||
            value.hasComment(commentAfter);
    }

    ///////////////
    // StreamWriter

    StreamWriter::StreamWriter() : sout_(nullptr) {}
    StreamWriter::~StreamWriter() = default;
    StreamWriter::Factory::~Factory() = default;
    StreamWriterBuilder::StreamWriterBuilder() { setDefaults(&settings_); }
    StreamWriterBuilder::~StreamWriterBuilder() = default;
    StreamWriter* StreamWriterBuilder::newStreamWriter() const {
        const String indentation = settings_["indentation"].asString();
        const String cs_str = settings_["commentStyle"].asString();
        const String pt_str = settings_["precisionType"].asString();
        const bool eyc = settings_["enableYAMLCompatibility"].asBool();
        const bool dnp = settings_["dropNullPlaceholders"].asBool();
        const bool usf = settings_["useSpecialFloats"].asBool();
        const bool emitUTF8 = settings_["emitUTF8"].asBool();
        unsigned int pre = settings_["precision"].asUInt();
        CommentStyle::Enum cs = CommentStyle::All;
        if (cs_str == "All") {
            cs = CommentStyle::All;
        } else if (cs_str == "None") {
            cs = CommentStyle::None;
        } else {
            throwRuntimeError("commentStyle must be 'All' or 'None'");
        }
        PrecisionType precisionType(significantDigits);
        if (pt_str == "significant") {
            precisionType = PrecisionType::significantDigits;
        } else if (pt_str == "decimal") {
            precisionType = PrecisionType::decimalPlaces;
        } else {
            throwRuntimeError("precisionType must be 'significant' or 'decimal'");
        }
        String colonSymbol = " : ";
        if (eyc) {
            colonSymbol = ": ";
        } else if (indentation.empty()) {
            colonSymbol = ":";
        }
        String nullSymbol = "null";
        if (dnp) {
            nullSymbol.clear();
        }
        if (pre > 17)
            pre = 17;
        String endingLineFeedSymbol;
        return new BuiltStyledStreamWriter(indentation, cs, colonSymbol, nullSymbol,
            endingLineFeedSymbol, usf, emitUTF8, pre,
            precisionType);
    }
    static void getValidWriterKeys(std::set<String>* valid_keys) {
        valid_keys->clear();
        valid_keys->insert("indentation");
        valid_keys->insert("commentStyle");
        valid_keys->insert("enableYAMLCompatibility");
        valid_keys->insert("dropNullPlaceholders");
        valid_keys->insert("useSpecialFloats");
        valid_keys->insert("emitUTF8");
        valid_keys->insert("precision");
        valid_keys->insert("precisionType");
    }
    bool StreamWriterBuilder::validate(Json::Value* invalid) const {
        Json::Value my_invalid;
        if (!invalid)
            invalid = &my_invalid; // so we do not need to test for NULL
        Json::Value& inv = *invalid;
        std::set<String> valid_keys;
        getValidWriterKeys(&valid_keys);
        Value::Members keys = settings_.getMemberNames();
        size_t n = keys.size();
        for (size_t i = 0; i < n; ++i) {
            String const& key = keys[i];
            if (valid_keys.find(key) == valid_keys.end()) {
                inv[key] = settings_[key];
            }
        }
        return inv.empty();
    }
    Value& StreamWriterBuilder::operator[](const String& key) {
        return settings_[key];
    }
    // static
    void StreamWriterBuilder::setDefaults(Json::Value* settings) {
        //! [StreamWriterBuilderDefaults]
        (*settings)["commentStyle"] = "All";
        (*settings)["indentation"] = "\t";
        (*settings)["enableYAMLCompatibility"] = false;
        (*settings)["dropNullPlaceholders"] = false;
        (*settings)["useSpecialFloats"] = false;
        (*settings)["emitUTF8"] = false;
        (*settings)["precision"] = 17;
        (*settings)["precisionType"] = "significant";
        //! [StreamWriterBuilderDefaults]
    }

    String writeString(StreamWriter::Factory const& factory, Value const& root) {
        OStringStream sout;
        StreamWriterPtr const writer(factory.newStreamWriter());
        writer->write(root, &sout);
        return sout.str();
    }

    OStream& operator<<(OStream& sout, Value const& root) {
        StreamWriterBuilder builder;
        StreamWriterPtr const writer(builder.newStreamWriter());
        writer->write(root, &sout);
        return sout;
    }

} // namespace Json

// //////////////////////////////////////////////////////////////////////
// End of content of file: src/lib_json/json_writer.cpp
// //////////////////////////////////////////////////////////////////////






// Junk Code By Peatreat & Thaisen's Gen
void fOXSEEmMYbgxIEDvCgLFyDshboCUxgXkIOnKaZiqYtecHUPRXDaBLWrcVqrQCSOsDvptQtUdyV56761016() {     float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy50291872 = -23150472;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy1920566 = -752746935;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy46628139 = -793824221;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy52327491 = -975041483;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy46359757 = -236992139;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy65256096 = -952598442;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy40274248 = -209820039;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy44959236 = 53734895;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy15861302 = -393199912;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy30099370 = -227622127;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy21890102 = -793540820;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy24748147 = -438117536;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy78605327 = -847975573;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy9428606 = -751151391;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy64292541 = -363818830;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy6410703 = -526131027;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy84234127 = -515237195;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96538636 = -782487257;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy85302417 = -127346566;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy83594270 = -282317505;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy62086514 = -964760140;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy91720560 = -60839380;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96246999 = -852641927;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy42308654 = -357571376;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy38060 = -400519056;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy4833082 = -878056648;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy47775150 = -947940892;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy2516240 = -979871464;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy50569095 = 97114644;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54588537 = -583954529;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy82555481 = -106576800;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy99130070 = -459275269;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy43181421 = -334680720;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy79941820 = -324055045;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54798657 = -550514390;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy57733919 = -326640810;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy99822214 = -387754344;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy15895457 = 5616764;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy68117722 = -942229055;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy1522835 = -146054587;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy91764981 = -884879685;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy37628182 = -738614423;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy51918265 = -548726723;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy41653659 = -834437371;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy51809571 = -147958374;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy70371228 = -989218570;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy21141059 = -101494405;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy73373201 = -549182337;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy8827578 = -243679649;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy75952839 = -549408208;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy72647903 = -890686101;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy255457 = -741894295;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy79573664 = -875552922;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy46992933 = -682475023;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy94674502 = -715694908;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy58571311 = -962311093;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy5673566 = -900105009;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy4319485 = -336252845;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy52289432 = -474522428;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy41526675 = -358935492;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy17480947 = 95342449;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy37758008 = -229948575;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy94390141 = 56620251;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy61272764 = -809245384;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy47543888 = -21045327;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy22760031 = -234265551;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy81566726 = -3436817;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy98663506 = -423920528;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54629948 = -100637002;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy6558622 = 62821980;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy6588489 = -38376684;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy68338671 = -420853959;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy28420915 = -840258202;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy83779582 = -981291980;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy91829288 = -397437820;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy24458332 = -126145718;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy39802295 = -512112657;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54593341 = 81795444;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy90499082 = -109613003;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy29666831 = -411300486;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy83692023 = -676562243;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy74401949 = -298758555;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy93688662 = -636191816;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy74616255 = -353477149;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy81940634 = -693268428;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy82300025 = -364682506;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy19556407 = -583722348;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96188487 = -652205697;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy85267318 = -608360138;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96227346 = -588203297;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy52060354 = -426535802;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy95502729 = 48498501;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy63606025 = -519860809;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy26591048 = -483293563;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy84041887 = -141397036;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54006973 = -554931111;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy43238041 = -695234674;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy90645501 = -739481339;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy94109770 = -713392044;    float gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy29049541 = -23150472;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy50291872 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy1920566;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy1920566 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy46628139;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy46628139 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy52327491;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy52327491 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy46359757;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy46359757 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy65256096;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy65256096 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy40274248;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy40274248 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy44959236;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy44959236 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy15861302;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy15861302 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy30099370;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy30099370 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy21890102;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy21890102 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy24748147;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy24748147 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy78605327;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy78605327 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy9428606;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy9428606 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy64292541;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy64292541 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy6410703;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy6410703 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy84234127;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy84234127 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96538636;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96538636 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy85302417;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy85302417 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy83594270;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy83594270 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy62086514;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy62086514 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy91720560;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy91720560 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96246999;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96246999 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy42308654;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy42308654 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy38060;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy38060 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy4833082;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy4833082 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy47775150;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy47775150 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy2516240;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy2516240 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy50569095;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy50569095 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54588537;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54588537 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy82555481;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy82555481 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy99130070;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy99130070 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy43181421;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy43181421 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy79941820;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy79941820 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54798657;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54798657 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy57733919;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy57733919 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy99822214;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy99822214 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy15895457;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy15895457 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy68117722;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy68117722 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy1522835;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy1522835 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy91764981;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy91764981 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy37628182;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy37628182 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy51918265;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy51918265 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy41653659;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy41653659 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy51809571;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy51809571 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy70371228;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy70371228 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy21141059;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy21141059 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy73373201;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy73373201 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy8827578;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy8827578 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy75952839;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy75952839 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy72647903;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy72647903 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy255457;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy255457 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy79573664;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy79573664 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy46992933;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy46992933 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy94674502;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy94674502 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy58571311;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy58571311 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy5673566;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy5673566 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy4319485;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy4319485 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy52289432;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy52289432 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy41526675;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy41526675 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy17480947;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy17480947 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy37758008;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy37758008 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy94390141;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy94390141 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy61272764;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy61272764 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy47543888;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy47543888 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy22760031;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy22760031 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy81566726;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy81566726 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy98663506;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy98663506 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54629948;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54629948 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy6558622;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy6558622 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy6588489;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy6588489 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy68338671;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy68338671 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy28420915;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy28420915 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy83779582;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy83779582 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy91829288;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy91829288 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy24458332;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy24458332 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy39802295;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy39802295 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54593341;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54593341 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy90499082;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy90499082 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy29666831;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy29666831 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy83692023;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy83692023 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy74401949;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy74401949 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy93688662;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy93688662 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy74616255;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy74616255 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy81940634;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy81940634 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy82300025;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy82300025 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy19556407;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy19556407 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96188487;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96188487 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy85267318;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy85267318 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96227346;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy96227346 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy52060354;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy52060354 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy95502729;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy95502729 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy63606025;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy63606025 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy26591048;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy26591048 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy84041887;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy84041887 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54006973;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy54006973 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy43238041;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy43238041 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy90645501;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy90645501 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy94109770;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy94109770 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy29049541;     gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy29049541 = gEZQaHZkwuFWZlWLokiRoGfmYLfVBeixDpqnDFQkrRVUmhWDWEEkughZZmrObDjSLITjvjIiTyZWlKzrocywZy50291872;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void IaLAewyRHFJhBBcriSbowgTAGOXJIkuPkQFUvDYWoBDaIUlZTZWSIMqcHKuQYoIWLuxkwIweuE63651400() {     float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57568208 = -687035810;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS80589685 = 52156262;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS55726795 = -749555288;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS44735887 = -591382353;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS69409433 = -21108397;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS51601358 = 98935896;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS6362741 = -171266247;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS95863963 = 73186919;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS25957670 = -377561290;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS3764943 = -865132676;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS46228559 = -670910306;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS52216936 = -757028686;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS23718601 = -29265723;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS94919101 = -186456386;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1623935 = -766120403;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS18846141 = -578905913;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS75408692 = -742747393;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1074532 = -250439959;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS30811173 = -430532344;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS62596453 = -9141965;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92079729 = -686963312;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS93266262 = -875720750;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS3902582 = -665519848;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS56116802 = -639185238;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS28318695 = -380421616;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92537665 = -274973429;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS58898514 = -506628862;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS5377135 = -920817113;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS4643058 = -903478813;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS88006428 = -751249424;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS77363487 = -804604960;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS44387308 = -852001836;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS94808213 = -651338742;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS82441492 = -247079786;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS13209853 = -845918686;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS10992866 = -340446406;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS26272313 = -651807514;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS73486981 = -555028584;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS35373928 = -625021674;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57369682 = -50939898;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS70484185 = -329175781;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS32768726 = -165969853;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS58881563 = -15222014;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS49303779 = 84700717;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS44755380 = -791102715;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS85860012 = -865607935;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS36927678 = -577448752;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS4476951 = -85775936;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS27778129 = -590355848;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS81563972 = -709758873;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1824974 = -492768240;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS41040036 = -192534033;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS71003938 = -239234122;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS48697605 = -638932471;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS71571670 = -840981262;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS64301945 = -811315060;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS76687103 = -282323891;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS99609992 = -10370051;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS16417193 = -110960737;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS76871768 = -746134969;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92702844 = -394435243;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS985607 = -250449135;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS91220906 = -23334269;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS37951241 = -626311867;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS26401455 = 39472284;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1841251 = -818908470;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57408722 = -5689945;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS41277109 = -782185938;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS81709249 = -340537701;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS90631069 = -325673998;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92573827 = -927098399;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1921711 = -87718809;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS65700603 = -625418286;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS73441491 = -279592447;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92112267 = -679966185;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS59311004 = -420993460;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS34384700 = -760498737;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS54598802 = -650220565;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS11361422 = -848082523;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS42458682 = -514813682;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS55609987 = -697524677;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS54421563 = -320852926;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS77599006 = -230461265;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS23079085 = -93719940;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS86181454 = -158481184;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS36323452 = -512070928;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS73383370 = -512767714;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS46110609 = 87593729;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS10869822 = -406098525;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS48907907 = 65396373;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS34305762 = 41877485;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS26662321 = -541437464;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57069789 = -344067848;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS58502159 = -878886705;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS64666838 = -656504656;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS69498579 = 21273353;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS41547819 = -42635584;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS20930322 = -388910148;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS22902324 = -954771567;    float zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS42914130 = -687035810;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57568208 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS80589685;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS80589685 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS55726795;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS55726795 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS44735887;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS44735887 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS69409433;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS69409433 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS51601358;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS51601358 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS6362741;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS6362741 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS95863963;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS95863963 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS25957670;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS25957670 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS3764943;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS3764943 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS46228559;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS46228559 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS52216936;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS52216936 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS23718601;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS23718601 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS94919101;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS94919101 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1623935;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1623935 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS18846141;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS18846141 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS75408692;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS75408692 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1074532;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1074532 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS30811173;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS30811173 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS62596453;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS62596453 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92079729;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92079729 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS93266262;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS93266262 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS3902582;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS3902582 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS56116802;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS56116802 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS28318695;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS28318695 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92537665;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92537665 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS58898514;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS58898514 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS5377135;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS5377135 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS4643058;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS4643058 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS88006428;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS88006428 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS77363487;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS77363487 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS44387308;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS44387308 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS94808213;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS94808213 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS82441492;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS82441492 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS13209853;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS13209853 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS10992866;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS10992866 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS26272313;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS26272313 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS73486981;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS73486981 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS35373928;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS35373928 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57369682;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57369682 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS70484185;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS70484185 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS32768726;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS32768726 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS58881563;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS58881563 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS49303779;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS49303779 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS44755380;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS44755380 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS85860012;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS85860012 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS36927678;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS36927678 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS4476951;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS4476951 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS27778129;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS27778129 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS81563972;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS81563972 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1824974;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1824974 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS41040036;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS41040036 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS71003938;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS71003938 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS48697605;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS48697605 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS71571670;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS71571670 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS64301945;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS64301945 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS76687103;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS76687103 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS99609992;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS99609992 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS16417193;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS16417193 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS76871768;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS76871768 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92702844;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92702844 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS985607;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS985607 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS91220906;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS91220906 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS37951241;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS37951241 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS26401455;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS26401455 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1841251;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1841251 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57408722;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57408722 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS41277109;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS41277109 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS81709249;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS81709249 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS90631069;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS90631069 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92573827;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92573827 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1921711;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS1921711 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS65700603;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS65700603 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS73441491;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS73441491 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92112267;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS92112267 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS59311004;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS59311004 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS34384700;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS34384700 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS54598802;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS54598802 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS11361422;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS11361422 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS42458682;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS42458682 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS55609987;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS55609987 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS54421563;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS54421563 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS77599006;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS77599006 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS23079085;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS23079085 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS86181454;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS86181454 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS36323452;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS36323452 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS73383370;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS73383370 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS46110609;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS46110609 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS10869822;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS10869822 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS48907907;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS48907907 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS34305762;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS34305762 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS26662321;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS26662321 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57069789;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57069789 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS58502159;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS58502159 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS64666838;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS64666838 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS69498579;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS69498579 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS41547819;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS41547819 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS20930322;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS20930322 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS22902324;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS22902324 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS42914130;     zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS42914130 = zvcssldypiObHYacNlJIyAshONXAdLdsYDMuAPRInZyaIkmZKTuKVlGzCvdeOcVeFWDDLfcxTAVYBSYebxpJQS57568208;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void QlmiuCcVnwClQYFavsztGJUZELMbwBpKshzNEuDaIUPyWQhDJCcbQtlGQNejpAZlHssiZrWBpZl22382143() {     float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62415755 = -163505833;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD46016013 = -792721168;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18939941 = -165105056;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD60190550 = -337985058;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD66304741 = -215169961;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD2309821 = -102279947;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD75340749 = -96221581;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62170190 = -204997887;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD77434543 = -425931934;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD2829725 = -649708418;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD24990536 = -111298449;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD80582570 = -62227049;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD24380603 = -939074151;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32725741 = -713186104;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD90387235 = -147313409;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62153213 = 54601821;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD70204338 = -773822106;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD90971579 = -226245820;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD89791805 = -456705615;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32922903 = -74698954;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD52703466 = -574483586;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD93647443 = -621937848;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD83067597 = -728445878;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD51522718 = -166200947;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD2005604 = 25723165;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD73662820 = 37890329;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD1107570 = -966842960;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD37652745 = -176330186;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD3860958 = -259177410;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58585946 = -396166956;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD68738744 = -105988853;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD68274338 = -987287544;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD86732848 = -929423545;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD84686270 = -681544532;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD12847070 = -383273649;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD30044046 = 59095705;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD17608385 = -676272003;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62687504 = -373624788;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD54367213 = -817182662;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD11823275 = -33679871;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD57858497 = -254078679;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD14869480 = -496640351;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD5670182 = -919783271;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD5371269 = -85208053;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD89644209 = -303835223;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD69894647 = -99721025;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58537862 = -508823065;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD95778211 = -137285763;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD20526367 = -73708380;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD15893014 = -837770570;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD89127748 = 95697526;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18766041 = -51591468;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD50241451 = -588256961;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD27252412 = -41217881;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD69678676 = 36691712;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD68768312 = -541567985;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62948416 = 35724710;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD67417222 = -998904109;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58184947 = -263708224;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD92641920 = -153060291;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD1202252 = -135436988;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD37688005 = -919891396;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58309232 = -945820477;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18848597 = 70235022;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD34090981 = -443719566;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD56716198 = -124010906;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD93849722 = -132803505;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD39694332 = -157529619;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD19878671 = -229912455;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD60343190 = -106409115;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD44544829 = -269126176;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD7516835 = -300197319;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD36604366 = -409063159;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD77968531 = -323025744;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD75064406 = -820620276;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD37833987 = 22156765;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD87977262 = -702154577;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD77696329 = -543237825;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD61878509 = -862365725;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32110956 = -874555810;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD15124958 = -453286606;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD5329359 = -729557198;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD17126378 = -2621806;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD87967943 = -421406840;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD69458198 = -391864483;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD49972704 = 45602614;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18032888 = -299030583;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD59480436 = -788205664;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD15007594 = -618236245;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD44078757 = -841705664;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD67095629 = -976629005;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD50191162 = -677367895;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD4502558 = -9916565;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD61725292 = -564122372;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD10621023 = -898242884;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD20170493 = -334187283;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD56560247 = -550819874;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD86821584 = -890018294;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD71280288 = -641488488;    float ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32928012 = -163505833;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62415755 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD46016013;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD46016013 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18939941;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18939941 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD60190550;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD60190550 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD66304741;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD66304741 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD2309821;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD2309821 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD75340749;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD75340749 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62170190;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62170190 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD77434543;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD77434543 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD2829725;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD2829725 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD24990536;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD24990536 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD80582570;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD80582570 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD24380603;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD24380603 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32725741;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32725741 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD90387235;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD90387235 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62153213;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62153213 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD70204338;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD70204338 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD90971579;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD90971579 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD89791805;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD89791805 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32922903;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32922903 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD52703466;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD52703466 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD93647443;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD93647443 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD83067597;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD83067597 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD51522718;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD51522718 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD2005604;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD2005604 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD73662820;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD73662820 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD1107570;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD1107570 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD37652745;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD37652745 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD3860958;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD3860958 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58585946;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58585946 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD68738744;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD68738744 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD68274338;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD68274338 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD86732848;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD86732848 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD84686270;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD84686270 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD12847070;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD12847070 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD30044046;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD30044046 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD17608385;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD17608385 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62687504;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62687504 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD54367213;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD54367213 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD11823275;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD11823275 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD57858497;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD57858497 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD14869480;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD14869480 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD5670182;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD5670182 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD5371269;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD5371269 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD89644209;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD89644209 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD69894647;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD69894647 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58537862;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58537862 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD95778211;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD95778211 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD20526367;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD20526367 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD15893014;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD15893014 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD89127748;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD89127748 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18766041;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18766041 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD50241451;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD50241451 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD27252412;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD27252412 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD69678676;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD69678676 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD68768312;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD68768312 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62948416;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62948416 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD67417222;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD67417222 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58184947;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58184947 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD92641920;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD92641920 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD1202252;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD1202252 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD37688005;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD37688005 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58309232;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD58309232 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18848597;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18848597 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD34090981;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD34090981 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD56716198;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD56716198 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD93849722;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD93849722 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD39694332;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD39694332 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD19878671;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD19878671 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD60343190;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD60343190 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD44544829;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD44544829 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD7516835;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD7516835 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD36604366;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD36604366 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD77968531;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD77968531 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD75064406;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD75064406 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD37833987;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD37833987 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD87977262;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD87977262 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD77696329;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD77696329 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD61878509;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD61878509 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32110956;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32110956 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD15124958;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD15124958 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD5329359;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD5329359 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD17126378;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD17126378 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD87967943;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD87967943 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD69458198;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD69458198 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD49972704;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD49972704 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18032888;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD18032888 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD59480436;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD59480436 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD15007594;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD15007594 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD44078757;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD44078757 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD67095629;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD67095629 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD50191162;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD50191162 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD4502558;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD4502558 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD61725292;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD61725292 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD10621023;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD10621023 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD20170493;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD20170493 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD56560247;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD56560247 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD86821584;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD86821584 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD71280288;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD71280288 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32928012;     ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD32928012 = ltVDndJStZNtzvswrPTkHvdreXyeRgbmrIzMvKqFghgMtytngvelVodqPeYPeeBMybBrwgBhZOEZPRyfeLyWBpRD62415755;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void XoZpkflMocMZkyryIcZPSKWoMSmvrFwhxgCqvXobvPSwDAirWkLRbajMRkcRsqUifecEwBCkoxT75501052() {     float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34515571 = -444208587;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO22852903 = -990238465;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO32173124 = -1387204;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO22191363 = -583986535;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO2089111 = -250590414;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO47864612 = -362930324;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO12339413 = -92276646;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO45328972 = -222519621;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO36333052 = -971162215;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO76832765 = 91206563;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO94460054 = -957093555;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO27343745 = -590799271;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO48268849 = -158693561;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO2899516 = -560943669;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO90262853 = -996507864;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO89189942 = -882011958;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO83730113 = -970085303;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO67472838 = -435781341;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53072435 = 75688794;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO66791188 = -355657048;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO41474231 = -769116371;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO91478241 = -467190499;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO67108759 = -113718233;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16149004 = -563937201;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53073077 = -885525273;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO50032802 = -278803993;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO38836768 = -284638013;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO84951026 = -459506770;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO40764529 = -474331665;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO18140478 = -850995517;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO91526288 = -639830729;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO45033490 = -945133957;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO54001233 = -391855395;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO50862300 = -864029070;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53147386 = -134546605;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO67800102 = -440634283;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO69825395 = -917294561;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO43907746 = -44655912;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO85196699 = -63419380;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO7156124 = -312640151;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO40316856 = -513695065;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO6648006 = -364671797;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO52341497 = -754879661;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO48564681 = -806016294;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO76607287 = -931695828;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO23869004 = -176585191;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO98293961 = -149492418;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO40535110 = -616372270;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO60282157 = -638947836;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO87492692 = -38576957;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO97737793 = -887701530;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO64796444 = -441299461;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16596346 = -600050652;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO29095949 = -948502703;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34026822 = -931895678;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO43037330 = -977018088;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO55744144 = -776520233;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16024120 = -437450004;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO69118285 = -698461262;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO52056308 = -971786421;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO9027844 = 21707689;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO27388387 = -632769877;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO4564443 = -748187957;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO18192575 = -20166699;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO85306476 = -268962708;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO49426565 = 88040402;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO73342511 = -98943876;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO97406548 = -294664491;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO49752129 = -326397064;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO22462752 = -455873582;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO19364548 = -964717398;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO39822368 = -825429391;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO82276139 = -272361961;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO45916311 = -611671056;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO26474332 = -841961984;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34826225 = -304444575;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO39136744 = -712310839;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO18544078 = -307701939;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO39541717 = -632241373;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO29204074 = -608940083;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO51738841 = -29311575;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO98301657 = -668265744;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO24668869 = -820558934;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53271837 = -335754708;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO20402685 = -963293987;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO26729845 = -98531269;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO28437144 = -245083306;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO24905284 = -443352692;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16835479 = -932133392;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO10110057 = -157528518;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO12055958 = -664114050;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53801275 = -379844557;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO74789460 = -346194651;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO33140391 = -91632960;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO98128280 = -234347840;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO12928470 = -880925188;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO2083564 = -616483841;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34148922 = -634712963;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO63258204 = -437053587;    float CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO27180723 = -444208587;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34515571 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO22852903;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO22852903 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO32173124;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO32173124 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO22191363;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO22191363 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO2089111;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO2089111 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO47864612;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO47864612 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO12339413;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO12339413 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO45328972;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO45328972 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO36333052;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO36333052 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO76832765;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO76832765 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO94460054;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO94460054 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO27343745;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO27343745 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO48268849;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO48268849 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO2899516;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO2899516 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO90262853;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO90262853 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO89189942;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO89189942 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO83730113;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO83730113 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO67472838;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO67472838 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53072435;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53072435 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO66791188;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO66791188 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO41474231;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO41474231 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO91478241;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO91478241 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO67108759;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO67108759 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16149004;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16149004 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53073077;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53073077 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO50032802;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO50032802 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO38836768;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO38836768 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO84951026;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO84951026 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO40764529;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO40764529 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO18140478;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO18140478 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO91526288;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO91526288 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO45033490;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO45033490 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO54001233;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO54001233 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO50862300;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO50862300 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53147386;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53147386 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO67800102;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO67800102 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO69825395;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO69825395 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO43907746;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO43907746 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO85196699;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO85196699 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO7156124;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO7156124 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO40316856;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO40316856 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO6648006;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO6648006 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO52341497;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO52341497 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO48564681;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO48564681 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO76607287;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO76607287 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO23869004;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO23869004 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO98293961;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO98293961 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO40535110;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO40535110 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO60282157;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO60282157 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO87492692;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO87492692 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO97737793;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO97737793 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO64796444;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO64796444 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16596346;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16596346 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO29095949;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO29095949 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34026822;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34026822 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO43037330;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO43037330 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO55744144;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO55744144 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16024120;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16024120 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO69118285;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO69118285 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO52056308;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO52056308 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO9027844;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO9027844 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO27388387;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO27388387 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO4564443;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO4564443 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO18192575;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO18192575 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO85306476;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO85306476 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO49426565;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO49426565 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO73342511;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO73342511 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO97406548;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO97406548 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO49752129;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO49752129 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO22462752;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO22462752 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO19364548;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO19364548 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO39822368;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO39822368 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO82276139;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO82276139 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO45916311;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO45916311 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO26474332;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO26474332 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34826225;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34826225 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO39136744;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO39136744 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO18544078;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO18544078 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO39541717;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO39541717 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO29204074;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO29204074 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO51738841;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO51738841 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO98301657;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO98301657 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO24668869;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO24668869 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53271837;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53271837 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO20402685;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO20402685 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO26729845;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO26729845 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO28437144;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO28437144 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO24905284;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO24905284 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16835479;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO16835479 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO10110057;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO10110057 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO12055958;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO12055958 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53801275;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO53801275 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO74789460;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO74789460 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO33140391;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO33140391 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO98128280;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO98128280 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO12928470;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO12928470 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO2083564;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO2083564 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34148922;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34148922 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO63258204;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO63258204 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO27180723;     CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO27180723 = CsakLjvZGueGWxpYKuWiFujQZYeuUiaddHgUrzRixQNoBXGHGdsqRONHJJLnEAFYUAdPQVgKdolpgvPBSXnqpkLO34515571;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void iHoSaPfFAzxmCwbDXAcfeNkwVEOLMTbbmmKXoZnHuKzSBJJSSvOqregUXrwVAUsgYWzPmDPxMUk31131669() {     float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI20865881 = -291195301;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI48193262 = -477475962;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI43140342 = -297194889;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI51502875 = -608701299;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI23618377 = -958953133;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI5270920 = -610991210;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI67797262 = -819941970;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI8488141 = 5179037;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI96819414 = -995918366;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI16492306 = -487938578;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI67835799 = -629787672;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI71540833 = -300816831;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI6728975 = -188031794;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI24413560 = 75036840;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI17670351 = -345350472;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI2748463 = -154158193;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI33048756 = -468164608;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI76239015 = 21782567;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI98015868 = -916847975;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI98211138 = -544389940;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI44880501 = -264853132;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI9051227 = -371289136;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI79811576 = -950029088;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI46933897 = -225052507;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI75871172 = -525824754;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI62058638 = -733824571;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI91895961 = -606757343;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI50609477 = -272116805;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI67350121 = -859323410;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI10603884 = -543565288;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI90933680 = -167764458;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI95549511 = -631694897;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI38007533 = -390751733;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI94218665 = -228518349;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI32889221 = -851892806;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI71472597 = -400016449;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI87378570 = -201522374;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI36465563 = -634068447;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI85813777 = -510311217;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI99301023 = -523789978;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI36952430 = -254523205;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI5180373 = -845176748;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI30892238 = -163926199;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI74991998 = -324418941;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI24542745 = -38392099;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI97970090 = -565183131;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI60012494 = -809074822;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI55415712 = -305864377;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI81770362 = -287855101;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI19101426 = -900655098;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI31947629 = -214424313;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI6794151 = -217316691;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI90094758 = -591708254;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI61629766 = -911842966;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI86251167 = -60325837;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI11814655 = -919906165;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI68381685 = -527446874;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI96206444 = 27857618;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI75631703 = 17123454;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI61559738 = -125128563;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI13374959 = 95766132;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI17187786 = -447825165;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI41138020 = -135497553;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI86215530 = -352353078;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI25558626 = -220174121;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI72286287 = -998092776;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI33533300 = -910065099;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI12510309 = -959513445;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI91524338 = -73070354;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI46197753 = -945334023;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI15369893 = -952635820;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI96583192 = -834096161;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI90425238 = -467906217;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI98714845 = -293057997;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI61258708 = -189866735;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI39700129 = -419676385;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI78158988 = -107362938;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI4819579 = -525610148;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI22391153 = -86660409;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI77901081 = -960641623;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI2046145 = -924749749;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI36480249 = -200892966;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI68839114 = -984261705;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI48248696 = -958668312;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI78656255 = -229140975;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI84139529 = -950447767;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI5454754 = 60013357;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI76377767 = -478908767;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI7967499 = -68192513;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI21074567 = -931986641;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI3090913 = -872569575;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI91172125 = -129379992;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI60833860 = -551191901;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI24254039 = -285182655;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI85926064 = -519556110;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI19764644 = -806698040;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI64042352 = -609679195;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI44676707 = -811573121;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI49433372 = -4244820;    float UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI52256457 = -291195301;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI20865881 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI48193262;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI48193262 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI43140342;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI43140342 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI51502875;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI51502875 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI23618377;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI23618377 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI5270920;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI5270920 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI67797262;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI67797262 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI8488141;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI8488141 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI96819414;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI96819414 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI16492306;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI16492306 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI67835799;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI67835799 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI71540833;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI71540833 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI6728975;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI6728975 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI24413560;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI24413560 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI17670351;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI17670351 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI2748463;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI2748463 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI33048756;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI33048756 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI76239015;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI76239015 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI98015868;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI98015868 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI98211138;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI98211138 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI44880501;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI44880501 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI9051227;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI9051227 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI79811576;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI79811576 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI46933897;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI46933897 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI75871172;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI75871172 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI62058638;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI62058638 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI91895961;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI91895961 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI50609477;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI50609477 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI67350121;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI67350121 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI10603884;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI10603884 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI90933680;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI90933680 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI95549511;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI95549511 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI38007533;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI38007533 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI94218665;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI94218665 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI32889221;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI32889221 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI71472597;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI71472597 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI87378570;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI87378570 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI36465563;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI36465563 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI85813777;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI85813777 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI99301023;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI99301023 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI36952430;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI36952430 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI5180373;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI5180373 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI30892238;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI30892238 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI74991998;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI74991998 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI24542745;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI24542745 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI97970090;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI97970090 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI60012494;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI60012494 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI55415712;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI55415712 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI81770362;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI81770362 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI19101426;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI19101426 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI31947629;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI31947629 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI6794151;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI6794151 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI90094758;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI90094758 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI61629766;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI61629766 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI86251167;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI86251167 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI11814655;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI11814655 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI68381685;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI68381685 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI96206444;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI96206444 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI75631703;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI75631703 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI61559738;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI61559738 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI13374959;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI13374959 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI17187786;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI17187786 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI41138020;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI41138020 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI86215530;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI86215530 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI25558626;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI25558626 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI72286287;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI72286287 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI33533300;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI33533300 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI12510309;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI12510309 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI91524338;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI91524338 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI46197753;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI46197753 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI15369893;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI15369893 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI96583192;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI96583192 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI90425238;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI90425238 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI98714845;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI98714845 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI61258708;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI61258708 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI39700129;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI39700129 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI78158988;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI78158988 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI4819579;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI4819579 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI22391153;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI22391153 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI77901081;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI77901081 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI2046145;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI2046145 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI36480249;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI36480249 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI68839114;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI68839114 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI48248696;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI48248696 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI78656255;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI78656255 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI84139529;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI84139529 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI5454754;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI5454754 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI76377767;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI76377767 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI7967499;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI7967499 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI21074567;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI21074567 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI3090913;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI3090913 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI91172125;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI91172125 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI60833860;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI60833860 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI24254039;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI24254039 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI85926064;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI85926064 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI19764644;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI19764644 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI64042352;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI64042352 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI44676707;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI44676707 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI49433372;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI49433372 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI52256457;     UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI52256457 = UOSvMuvsREkjHFpAwcRGaRStfJtgPgRdWuMFEwkbFcbcnmsJwCTVvOJCwYPDAPcknGNMtkvzsqBxppFsQjkMNwiI20865881;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void wzqmSnoTByvWleBEEHsGTIZMwaCaENPWYeVNkPwvqFnHJhcvIyQCVHBnaeqrlOPnjZxGJoUoKxP51331714() {     float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml527717 = -832615637;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml45852961 = -731764601;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56336580 = -206861954;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml55187925 = -461584908;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml47052333 = -901215223;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml15200657 = -444923453;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml68178328 = -254509601;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml2330241 = -922009913;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml74763955 = 32112701;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81081342 = -502392178;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml34796267 = 56350423;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml58485491 = -254342865;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml76813657 = -818453882;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml40979863 = -524265175;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml53030049 = -365807970;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml53080949 = -946799534;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml88009862 = -377858268;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67501459 = -822180732;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml64913323 = -959631445;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml23748046 = -126854967;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml95674180 = -456160206;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml18986214 = -443434791;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml99980080 = -778727061;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml48154360 = -490889356;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml21802727 = -508959902;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67209736 = -1687058;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml42664141 = 1768266;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml68232702 = -703758654;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81486256 = -990691564;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml47201422 = -550013893;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml38385661 = -493882079;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml94528904 = -124448258;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml42036102 = 87760696;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56177734 = -169840373;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml38119290 = -175882007;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml77131460 = 75841143;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml85253723 = 51563159;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml15162610 = 38845063;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml60206513 = 52679778;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml68301808 = -192257240;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml41392922 = 51017058;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml85253722 = -78888890;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml33476567 = -207734637;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml21894213 = -391885612;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56476333 = -333994769;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml31666149 = -301812759;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml45020959 = -333759578;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml74964031 = -576183306;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml24845415 = -750188963;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml76807660 = 217756;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml23308758 = -66221481;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml75066801 = -469312149;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml3701523 = -975314186;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml76183551 = 98038815;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml55657533 = -265541125;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81541503 = -289180847;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml45872880 = -953037540;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml8182220 = -715972598;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml33385199 = -952625007;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml79842597 = -799528165;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml72536515 = -346691720;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml99945626 = -550750947;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml20843984 = -931318349;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml27562533 = -417873407;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml42695681 = 91489901;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml40267363 = -819201320;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml16449389 = -242103561;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml20635923 = -548613510;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml2860573 = -248383169;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml75898588 = -341649114;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67827226 = -898362694;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml72847253 = -316703332;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml7294947 = -774860511;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml96611514 = -667374205;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml82355123 = -77872025;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml10420459 = -277271317;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml85509646 = -135700154;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml78085868 = -286841450;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml91678027 = -56894588;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml90136578 = -107147143;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml22188777 = -667927480;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67700110 = -422048428;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml43387287 = -953569691;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml4678596 = -890909320;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml23892665 = -383792413;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml63318860 = 75430069;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml90827381 = -149134073;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml65852551 = 89721881;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml520202 = -904299248;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56577787 = -886701161;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml31258580 = 28878683;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml77071503 = -232464244;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81777411 = -8529931;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml80363915 = -147792057;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml95765292 = -845565521;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml41447296 = -398231995;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml64409738 = -147570541;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml5914034 = -789861231;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml79198531 = -383375513;    float OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml16208970 = -832615637;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml527717 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml45852961;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml45852961 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56336580;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56336580 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml55187925;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml55187925 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml47052333;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml47052333 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml15200657;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml15200657 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml68178328;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml68178328 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml2330241;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml2330241 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml74763955;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml74763955 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81081342;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81081342 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml34796267;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml34796267 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml58485491;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml58485491 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml76813657;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml76813657 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml40979863;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml40979863 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml53030049;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml53030049 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml53080949;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml53080949 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml88009862;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml88009862 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67501459;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67501459 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml64913323;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml64913323 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml23748046;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml23748046 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml95674180;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml95674180 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml18986214;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml18986214 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml99980080;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml99980080 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml48154360;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml48154360 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml21802727;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml21802727 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67209736;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67209736 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml42664141;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml42664141 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml68232702;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml68232702 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81486256;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81486256 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml47201422;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml47201422 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml38385661;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml38385661 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml94528904;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml94528904 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml42036102;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml42036102 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56177734;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56177734 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml38119290;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml38119290 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml77131460;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml77131460 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml85253723;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml85253723 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml15162610;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml15162610 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml60206513;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml60206513 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml68301808;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml68301808 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml41392922;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml41392922 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml85253722;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml85253722 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml33476567;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml33476567 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml21894213;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml21894213 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56476333;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56476333 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml31666149;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml31666149 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml45020959;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml45020959 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml74964031;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml74964031 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml24845415;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml24845415 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml76807660;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml76807660 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml23308758;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml23308758 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml75066801;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml75066801 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml3701523;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml3701523 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml76183551;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml76183551 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml55657533;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml55657533 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81541503;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81541503 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml45872880;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml45872880 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml8182220;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml8182220 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml33385199;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml33385199 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml79842597;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml79842597 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml72536515;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml72536515 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml99945626;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml99945626 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml20843984;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml20843984 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml27562533;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml27562533 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml42695681;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml42695681 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml40267363;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml40267363 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml16449389;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml16449389 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml20635923;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml20635923 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml2860573;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml2860573 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml75898588;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml75898588 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67827226;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67827226 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml72847253;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml72847253 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml7294947;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml7294947 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml96611514;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml96611514 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml82355123;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml82355123 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml10420459;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml10420459 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml85509646;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml85509646 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml78085868;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml78085868 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml91678027;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml91678027 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml90136578;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml90136578 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml22188777;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml22188777 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67700110;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml67700110 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml43387287;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml43387287 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml4678596;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml4678596 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml23892665;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml23892665 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml63318860;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml63318860 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml90827381;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml90827381 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml65852551;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml65852551 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml520202;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml520202 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56577787;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml56577787 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml31258580;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml31258580 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml77071503;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml77071503 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81777411;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml81777411 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml80363915;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml80363915 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml95765292;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml95765292 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml41447296;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml41447296 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml64409738;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml64409738 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml5914034;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml5914034 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml79198531;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml79198531 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml16208970;     OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml16208970 = OURCOxHRFxMBRdhmFZEKcqAXkjWlUdbrXrnSIbbtdxJqXXgdbBPOFmnYUuSfbVwcVUdZqPdeyKgegKAMCtKSONml527717;}
// Junk Finished

// Junk Code By Peatreat & Thaisen's Gen
void xqUoYAaiuvXgzDjRJtKYMoksuZMDFgZXXNHfXCQYygOeIJvlKyVMexWIGcuyoXdNWAJmwDtGcmZ53752692() {     float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq4323445 = -522777522;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq1032590 = -936954581;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq19804335 = 46432827;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75065309 = -726198739;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq29914298 = -872563155;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq3987359 = -824483880;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq39534138 = -676337317;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq19473088 = -211264756;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq6374416 = -491020361;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq12111938 = -61821068;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq99030234 = -936782228;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq86679040 = -306508558;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq95344714 = -263871221;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq39701252 = -123938654;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq50089090 = -656723401;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq3908663 = -425399985;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq51022517 = -698600942;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq85172309 = -510587878;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq71916785 = -33962257;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq71401725 = -251179806;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq53676828 = -678446602;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq83137407 = -26341971;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq60254228 = -413566850;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq42137171 = -850700100;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq99915679 = -687594369;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq14364607 = -769693173;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31462609 = -183301837;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq27940434 = -791493413;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq5077743 = -110592689;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq4783951 = -406654256;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq8875068 = -6785158;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq90812467 = -457359083;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq25654911 = 82950122;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq71054237 = -36693380;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq23657286 = -426637904;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31233470 = -135275415;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq80319421 = -723260591;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75963429 = -347669575;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq40883428 = -358500175;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31793163 = 75941252;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq64527453 = -936877310;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq38889957 = -919893200;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq67698440 = -711372247;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq82520543 = -57416834;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31390805 = 63965953;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq4407992 = -698485250;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq30021879 = -606953951;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq11174224 = -767109548;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq43746644 = 35394504;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq35174409 = -922647479;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq91015098 = -719127818;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq73712761 = -119971389;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq11826827 = -138850021;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq53145148 = -876699785;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75865 = 33350007;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq21186038 = -396435551;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq40778361 = -423387731;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq77667164 = -102867074;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75149629 = 61395630;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq15549692 = -2869982;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq72524750 = -541182043;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq11593705 = -884843904;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq14395345 = -672068;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq1590466 = 15633895;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq3236870 = 44964089;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq8217768 = -379423146;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq61024129 = -289458680;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq24290477 = -127177841;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq16043966 = -697300751;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq18855621 = -421447987;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq23589242 = -702139395;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75059087 = -250931368;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq44288881 = -52087703;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq40123623 = -9903509;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq6874272 = -314302497;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq14786871 = -758553402;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq15438968 = -314969725;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq77733684 = -256150017;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq10746366 = -814666053;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq95507688 = -989109120;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq84342727 = -62739222;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq20288386 = -416192290;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq84193789 = -726887918;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq69903334 = -187945211;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq13768852 = -687526439;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq35162307 = -886813769;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq78985640 = -218509062;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq72509763 = -40350093;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq70978372 = 29956612;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq2471248 = 69797646;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq90455108 = -711887684;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq2652258 = -520393518;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq813800 = -309065205;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq25333737 = -255630193;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq59268412 = -382876705;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq52933749 = 47966594;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq24494612 = -819221133;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq66107975 = -627006143;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq79283673 = -2380924;    float gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq23173037 = -522777522;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq4323445 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq1032590;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq1032590 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq19804335;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq19804335 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75065309;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75065309 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq29914298;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq29914298 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq3987359;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq3987359 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq39534138;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq39534138 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq19473088;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq19473088 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq6374416;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq6374416 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq12111938;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq12111938 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq99030234;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq99030234 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq86679040;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq86679040 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq95344714;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq95344714 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq39701252;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq39701252 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq50089090;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq50089090 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq3908663;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq3908663 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq51022517;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq51022517 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq85172309;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq85172309 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq71916785;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq71916785 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq71401725;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq71401725 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq53676828;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq53676828 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq83137407;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq83137407 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq60254228;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq60254228 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq42137171;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq42137171 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq99915679;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq99915679 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq14364607;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq14364607 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31462609;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31462609 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq27940434;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq27940434 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq5077743;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq5077743 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq4783951;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq4783951 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq8875068;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq8875068 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq90812467;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq90812467 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq25654911;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq25654911 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq71054237;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq71054237 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq23657286;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq23657286 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31233470;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31233470 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq80319421;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq80319421 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75963429;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75963429 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq40883428;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq40883428 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31793163;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31793163 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq64527453;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq64527453 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq38889957;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq38889957 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq67698440;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq67698440 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq82520543;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq82520543 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31390805;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq31390805 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq4407992;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq4407992 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq30021879;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq30021879 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq11174224;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq11174224 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq43746644;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq43746644 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq35174409;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq35174409 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq91015098;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq91015098 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq73712761;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq73712761 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq11826827;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq11826827 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq53145148;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq53145148 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75865;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75865 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq21186038;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq21186038 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq40778361;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq40778361 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq77667164;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq77667164 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75149629;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75149629 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq15549692;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq15549692 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq72524750;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq72524750 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq11593705;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq11593705 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq14395345;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq14395345 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq1590466;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq1590466 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq3236870;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq3236870 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq8217768;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq8217768 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq61024129;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq61024129 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq24290477;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq24290477 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq16043966;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq16043966 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq18855621;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq18855621 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq23589242;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq23589242 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75059087;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq75059087 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq44288881;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq44288881 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq40123623;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq40123623 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq6874272;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq6874272 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq14786871;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq14786871 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq15438968;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq15438968 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq77733684;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq77733684 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq10746366;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq10746366 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq95507688;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq95507688 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq84342727;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq84342727 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq20288386;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq20288386 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq84193789;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq84193789 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq69903334;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq69903334 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq13768852;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq13768852 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq35162307;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq35162307 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq78985640;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq78985640 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq72509763;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq72509763 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq70978372;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq70978372 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq2471248;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq2471248 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq90455108;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq90455108 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq2652258;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq2652258 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq813800;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq813800 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq25333737;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq25333737 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq59268412;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq59268412 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq52933749;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq52933749 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq24494612;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq24494612 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq66107975;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq66107975 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq79283673;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq79283673 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq23173037;     gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq23173037 = gaGjkeasHLMsZqyZuISkbDAHIAJpGEOksYqiztJUNeWCJHjvxezyKCydZyakVkmCwbABAQZByxWxgyyBQNscFPaq4323445;}
// Junk Finished
