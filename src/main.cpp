#include "session.hpp"

#include <cstdlib>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <signal.h>

namespace bf = boost::filesystem;

using namespace cataglyphis;

int main (int argc, char** argv)
{

    session s;

    if (argc != 2)
    {
        std::vector<std::string> v = s.available_elements ();
        for (std::vector<std::string>::const_iterator i = v.begin (); i != v.end ();
                ++i)
            std::cout << *i << std::endl;
        std::exit (0);
    }

    session::path_type path (argv[1]);

    if (!bf::exists (path))
        s.get_element (path.leaf (), path);
    else
        s.put_element (path);

    sigpause (-1);
}
