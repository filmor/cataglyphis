#include "fs_element.hpp"

#include <boost/filesystem/operations.hpp>

namespace bf = boost::filesystem;

namespace cataglyphis
{

    fs_element::fs_element (fs_element::path_type path)
        : _path (path)
    {
        if (bf::exists (path))
            _torrent = boost::shared_ptr<torrent> (new torrent (path));
    }

}

