#pragma once

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

namespace HRML {
// Foward declarations and typedef
class Tag;
using TagPointer = std::unique_ptr<Tag>;
using AttributeMap = std::unordered_map<std::string, std::string>;
using TagMap = std::unordered_map<std::string, TagPointer>;
using StringVector = std::vector<std::string>;

struct Parser {
    static StringVector TokenizeTreeParse(std::string const&);
    static void TokenizeTreeQuery(std::string const&, StringVector&, std::string&);
    static bool IsNewTag(std::string const&);
};

class Tag {
public:
    void AddChildTag(TagPointer);
    void AddAttribute(std::string const& key, std::string const& value);

    friend class HrmlFile;

private:
    Tag* getChildTag(std::string const& tag_name_);

    std::string _name;
    AttributeMap _attributes;
    TagMap _child_tags;
    Tag* _parent_tag = nullptr;
};

class HrmlFile {
public:
    void Parse(int numLines_, std::istream& stream_);
    void Query(int numLines_, std::istream& stream_, std::ostream& os_);

private:
    TagPointer MakeNewTag(std::string const& line_, Tag* parent_tag_);
    bool Search(StringVector const& tags_, std::string& attribute_, std::string& value_);
    Tag _root;
};

}  // namespace HRML