// Type your code here, or load an example.
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

#include <vector>

struct XML {
    std::ostream &os_;

    struct Tag {
        XML &xml_;
        const char *name_;
        bool empty_=true;
        bool moved_=false;

        Tag(XML &xml, const char *name) : xml_(xml), name_(name){
            xml_.os_ << "<" << name;
        };
        Tag(const Tag&) = delete;
        Tag(Tag&& other) noexcept 
        : xml_(other.xml_)
        , name_(other.name_)
        , empty_(other.empty_)
        {
          other.moved_ = true;
        };

        void close_open() {
            if (!empty_)
                return;
            xml_.os_ << ">";
            empty_=false;
        }
        ~Tag() {
            if (moved_)
                return;
            if (empty_) {
                xml_.os_ << "/>";
            } else {
                xml_.os_ << "</" << name_ << ">";
            }
        };

        // Named methods
        Tag& attr(const char *name, const char *val) {
            return (*this)*name/val;
        }
        Tag& text(const char *name) {
            return (*this)%name;
        }
        Tag& open(const char *name) {
            return (*this)>>name;
        }

        // Overloaded operators, but this is probably not much usable in wider use
        Tag& operator*(const char* name) {
            xml_.os_ << " " << name;
            return *this; 
        }
        Tag& operator/(const char* name) { 
            xml_.os_ << "=" << name;
            return *this; 
        }
        Tag& operator%(const char* name) {
            close_open();
            xml_.os_ << name;
            return *this; 
        }
        Tag& operator>>(const char *name) {
            close_open();
            return *this;
        }
    };

    XML(std::ostream &os) : os_(os){};
    void clear() {
        while (!stack_.empty()) {
            stack_.pop_back();
        }
    }
    ~XML(){
        clear();
    };

    std::vector<Tag> stack_;

    // Named methods
    XML& attr(const char *name, const char *val) {
        return (*this)*name/val;
    }
    XML& text(const char *name) {
        return (*this)%name;
    }
    XML& open(const char *name) {
        return (*this)>>name;
    }
    XML& add(const char *name) {
        return (*this)+name;
    }
    XML& close() {
        stack_.pop_back();
        return *this;
    }

    // Overloaded operators, but this is probably not much usable in wider use
    XML& operator*(const char* name) {
        stack_.back()*(name);
        return *this; 
    }
    XML& operator/(const char* name) { 
        stack_.back()/(name);
        return *this; 
    }
    XML& operator%(const char* name) {
        stack_.back()%(name);
        return *this; 
    }
    XML& operator<<(const char* name) {
        stack_.pop_back();
        if (strlen(name) != 0) {
            *this+(name);
        }
        return *this;
    }

    XML& operator>>(const char *name) {
        if (!stack_.empty())
            stack_.back()>>(name);
        stack_.emplace_back(Tag(*this, name));
        return *this;
    }
    XML& operator+(const char *name) {
        stack_.pop_back();
        return *this>>(name); 
    }
};

int main() {
    std::ostringstream os;
    {
        XML xml(os);
        (xml >> "HTML") * "A" / "va" * "B" / "vb" % "body text";
        (xml >> "inHTML");
        (xml >> "inHTML2");
        (xml >> "inHTML3");
    }
    std::cout << os.str() << std::endl;


    std::ostringstream os2;
    {
        XML xml2(os2);
        (xml2 >> "HTML") * "A" / "va" * "B" / "vb" % "body text";
        (xml2 >> "inHTML") % "body";
        (xml2 + "HTML2") * "A" / "vA";
    }
    std::cout << os2.str() << std::endl;

    std::ostringstream os3;
    {
        XML xml3(os3);
        (xml3 >> "HTML") * "A" / "va" * "B" / "vb" % "body text";
        (xml3 >> "inHTML") % "body";
        (xml3 >> "HTML2") * "A" / "vA";
        (xml3 >> "HTML3") * "A" / "vA";
        (xml3 >> "HTML4") * "A" / "vA";
        (xml3 + "HTML41") * "A" / "vA";
        (xml3 + "HTML42") * "A" / "vA";
        xml3 << "";
        (xml3 << "HTML22") * "A" / "vA";
        (xml3 + "HTML23") * "A" / "vA";
    }
    std::cout << os3.str() << std::endl;

    std::ostringstream os4;
    {
        XML xml4(os4);
        xml4.open("HTML").attr("A", "va").attr("B", "vb").text("body text");
        xml4.open("inHTML").text("body");
        xml4.open("HTML2").attr("A", "vA");
        xml4.open("HTML3").attr("A", "vA");
        xml4.open("HTML4").attr("A", "vA");
        xml4.add("HTML41").attr("A", "vA");
        xml4.add("HTML42").attr("A", "vA");
        xml4.close();
        xml4.close();
        xml4.add("HTML22").attr("A", "vA");
        xml4.add("HTML23").attr("A", "vA");
        xml4.close();
        xml4.text("inHTML-body2");
    }
    std::cout << os4.str() << std::endl;
};
