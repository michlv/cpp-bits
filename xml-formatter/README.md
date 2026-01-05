# xml-formatter

Proof of concept if one can write lightweight XML formatter which will make it simple to write in C++, but also using C++ to help with validity of the document.

What I wanted to achive was something like:

```
        XML xml(os);
        (xml >> "HTML") * "A" / "va" * "B" / "vb" % "body text";
        (xml >> "inHTML");
        (xml >> "inHTML2");
        (xml >> "inHTML3");
```

But it turns out the operators are not that great idea, since without the right brackets the operators will be used directly on the primitites on the command line, so depending what the types are, it might not do what expected.

So the second attempt was:

```
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
```
