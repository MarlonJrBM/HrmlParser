#include "hrmlparser.hh"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

// TODO - incorporate tests into the makefile

namespace HRML {

//-------------------------------------------------------------------------------------------------
// TokenizeTreeParse
//-------------------------------------------------------------------------------------------------
StringVector Parser::TokenizeTreeParse(std::string const& line_) {
    // Assumes it's a tag opening line, removes enclosing <>
    std::string pre_processed_line = line_.substr(1, line_.size() - 2);

    // Adds pre-processed tokens to vector
    std::istringstream iss(pre_processed_line);
    StringVector tokens(std::istream_iterator<std::string>{iss},
                        std::istream_iterator<std::string>());

    // And removes the = between keys and their values
    // And removes "" enclosing values
    for (auto it = tokens.begin(); it != tokens.end();) {
        if (*it == "=") {
            it = tokens.erase(it);
        } else {
            if ((*it)[0] == '\"') {
                *it = it->substr(1, it->size() - 2);
            }
            ++it;
        }
    }

    // For instance, the line below
    // <tag1 key1 = ¨val1¨ key2 = ¨val2¨>
    // Would become the following StringVector:
    // [tag1, key1, val1, key2, val2]

    return tokens;
}

//-------------------------------------------------------------------------------------------------
// TokenizeTreeQuery
//=================================================================================================
// Parses a query string, inserting the tags inside tags_ vector, and the attribute
// name inside attribute_
//-------------------------------------------------------------------------------------------------
void Parser::TokenizeTreeQuery(std::string const& line_, StringVector& tags_,
                               std::string& attribute_) {
    size_t pos = line_.rfind('~');
    assert(pos != std::string::npos);
    attribute_ = line_.substr(pos + 1);

    // TODO - exchange this for a tokenizer based on STL structures
    const std::string tag_string = line_.substr(0, pos);
    pos = 0;
    size_t new_pos = 0, old_pos = new_pos;
    do {
        new_pos = tag_string.find(".", old_pos);
        const size_t intermediate_pos =
            (new_pos == std::string::npos) ? std::string::npos : (new_pos - old_pos);
        const std::string tag_name = tag_string.substr(old_pos, intermediate_pos);
        tags_.push_back(tag_name);
        old_pos = new_pos + 1;  // ignoring the '.'
    } while (new_pos != std::string::npos);
}

//-------------------------------------------------------------------------------------------------
// IsNewTag
//-------------------------------------------------------------------------------------------------
bool Parser::IsNewTag(std::string const& line_) {
    assert(line_.size() > 2);
    return line_.substr(0, 2) != "</";
}

//-------------------------------------------------------------------------------------------------
// GetChildTag
//-------------------------------------------------------------------------------------------------
Tag* Tag::getChildTag(std::string const& tag_name_) {
    if (_child_tags.find(tag_name_) == _child_tags.end()) {
        return nullptr;
    } else {
        return _child_tags[tag_name_].get();
    }
}

//-------------------------------------------------------------------------------------------------
// AddAttribute
//-------------------------------------------------------------------------------------------------
void Tag::AddAttribute(std::string const& key_, std::string const& value_) {
    _attributes[key_] = value_;
}

//-------------------------------------------------------------------------------------------------
// AddChildTag
//-------------------------------------------------------------------------------------------------
void Tag::AddChildTag(TagPointer tag_) { _child_tags[tag_->_name] = std::move(tag_); }

//-------------------------------------------------------------------------------------------------
// MakeNewTag
//-------------------------------------------------------------------------------------------------
TagPointer HrmlFile::MakeNewTag(std::string const& line_, Tag* parent_tag_) {
    TagPointer new_tag = std::make_unique<Tag>();

    // Parses line here, fetching tag's attributes
    StringVector tokens = Parser::TokenizeTreeParse(line_);
    assert((tokens.size() >= 1) && ((tokens.size() % 2) == 1));
    new_tag->_name = tokens[0];

    for (size_t ii = 1; ii < tokens.size(); ii += 2) {
        const std::string& key = tokens[ii];
        const std::string& value = tokens[ii + 1];
        new_tag->AddAttribute(key, value);
    }

    new_tag->_parent_tag = parent_tag_;
    return new_tag;
}

//-------------------------------------------------------------------------------------------------
// Parse
//-------------------------------------------------------------------------------------------------
void HrmlFile::Parse(int numLines_, std::istream& stream_) {
    std::string line;
    Tag* current_tag(&_root);
    for (int ii = 0; ii < numLines_; ++ii) {
        std::getline(stream_, line);
        if (Parser::IsNewTag(line)) {
            TagPointer new_tag = MakeNewTag(line, current_tag);
            Tag* child_tag = new_tag.get();
            current_tag->AddChildTag(std::move(new_tag));
            current_tag = child_tag;
        } else {  // finds a closing tag (</...>)
            current_tag = current_tag->_parent_tag;
        }
    }

    // If everything went right, the below loop came all the back to the root
    assert(current_tag == &_root);
}

//-------------------------------------------------------------------------------------------------
// Query
//-------------------------------------------------------------------------------------------------
void HrmlFile::Query(int numLines_, std::istream& stream_, std::ostream& os_) {
    for (int ii = 0; ii < numLines_; ++ii) {
        std::string line, attribute, value;
        StringVector tags;
        std::getline(stream_, line);
        Parser::TokenizeTreeQuery(line, tags, attribute);
        const bool has_found = Search(tags, attribute, value);
        if (has_found) {
            os_ << value << std::endl;
        } else {
            os_ << "Not Found!" << std::endl;
        }
    }
}

//-------------------------------------------------------------------------------------------------
// Search
//-------------------------------------------------------------------------------------------------
bool HrmlFile::Search(StringVector const& tags_, std::string& attribute_, std::string& value_) {
    Tag* curr_node = &_root;

    for (const auto& tag : tags_) {
        curr_node = curr_node->getChildTag(tag);
        if (curr_node == nullptr) {
            return false;
        }
    }

    // If we made it up to here, time to look for the attribute now
    if (curr_node->_attributes.find(attribute_) == curr_node->_attributes.end()) {
        return false;
    } else {
        value_ = curr_node->_attributes[attribute_];
        return true;
    }
}

}  // namespace HRML

//-------------------------------------------------------------------------------------------------
// main
//-------------------------------------------------------------------------------------------------
int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    HRML::HrmlFile file;
    int tree_parse_count(0), tree_query_count(0);
    std::cin >> tree_parse_count >> tree_query_count;
    std::cin.ignore();  // Ignoring newline
    file.Parse(tree_parse_count, std::cin);
    file.Query(tree_query_count, std::cin, std::cout);
    return 0;
}
