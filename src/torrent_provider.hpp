#ifndef CATAGLYPHIS_TORRENT_PROVIDER_HPP
#define CATAGLYPHIS_TORRENT_PROVIDER_HPP

#include <boost/cstdint.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <libtorrent/torrent_info.hpp>

namespace cataglyphis
{

    class torrent_provider
    {
    public:
        torrent_provider (libtorrent::torrent_info const&, boost::asio::io_service&);

        boost::uint16_t port () const;

    private:
        class connection;

        void start_accept ();
        void handle_accept (boost::shared_ptr<connection>, boost::system::error_code const&);

        std::string torrent_;
        boost::asio::ip::tcp::acceptor acceptor_;
        boost::asio::io_service& io_service_;
    };

}

#endif
