#ifndef CATAGLYPHIS_SESSION_HPP
#define CATAGLYPHIS_SESSION_HPP

#include "zeroconf.hpp"

#include <libtorrent/session.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

namespace cataglyphis
{

    class session
    {
    public:
        typedef boost::filesystem::path path_type;

        session ();

        void put_element (path_type const&);
        void get_element (std::string const&, path_type const&);
        
        std::vector<std::string> available_elements () const;

    private:
        struct provided_torrent;
        typedef boost::shared_ptr<provided_torrent> torrent_ptr;

        asio::io_service _ios;

        ::libtorrent::session _session;
        std::vector<torrent_ptr> _registry;
    };

}

#endif

