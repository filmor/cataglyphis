#ifndef CATAGLYPHIS_FS_ELEMENT_HPP
#define CATAGLYPHIS_FS_ELEMENT_HPP

#include "torrent.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace cataglyphis
{

    class fs_element
    {
    public:
        typedef boost::filesystem::wpath path_type;

        fs_element (path_type);

    private:
        path_type _path;
        boost::shared_ptr<torrent> _torrent;
    };

}

#endif

