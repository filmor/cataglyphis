#ifndef CATAGLYPHIS_TORRENT_PROVIDER_HPP
#define CATAGLYPHIS_TORRENT_PROVIDER_HPP

#include <boost/cstdint.hpp>
#include <asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <libtorrent/torrent_info.hpp>

namespace cataglyphis
{

    class torrent_provider
    {
    public:
        torrent_provider (libtorrent::torrent_info const&, asio::io_service&);

        boost::uint16_t port () const;

    private:
        class connection;

        void start_accept ();
        void handle_accept (boost::shared_ptr<connection>, asio::error_code const&);

        libtorrent::torrent_info const& _torrent;
        asio::ip::tcp::acceptor _acceptor;
        asio::io_service& _io_service;
    };

}

#endif
