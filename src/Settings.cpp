
#include "Settings.h"

#include <nstd/File.h>
#include <nstd/List.h>

void Settings::loadSettings(const String& file, Settings& settings)
{
    String conf;
    if (!File::readAll(file, conf))
        return;
    List<String> lines;
    conf.split(lines, "\n\r");
    for (List<String>::Iterator i = lines.begin(), end = lines.end(); i != end; ++i)
    {
        String line = *i;
        line.trim();
        const char* lineEnd = line.find('#');
        if (lineEnd)
            line.resize(lineEnd - (const char*)line);
        List<String> tokens;
        line.split(tokens, " \t");
    }

}

